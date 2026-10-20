#include <cmath>
#include <limits>
#include <cstdlib>
#include <ctime>
#include <algorithm>
#include "tgaimage.h"
#include "model.h"
#include "geometry.h"

constexpr TGAColor white = {255, 255, 255, 255}; // attention, BGRA order
constexpr TGAColor green = {0, 255, 0, 255};
constexpr TGAColor red = {0, 0, 255, 255};
constexpr TGAColor blue = {255, 128, 64, 255};
constexpr TGAColor yellow = {0, 200, 255, 255};

mat<4, 4> ModelView, Viewport, Perspective;

void viewport(const int x, const int y, const int w, const int h)
{
	Viewport = {{{w / 2., 0, 0, x + w / 2.}, {0, h / 2., 0, y + h / 2.}, {0, 0, 1, 0}, {0, 0, 0, 1}}};
}

void perspective(const double f)
{
	Perspective = {{{1, 0, 0, 0}, {0, 1, 0, 0}, {0, 0, 1, 0}, {0, 0, -1 / f, 1}}};
}

void lookat(const vec3 eye, const vec3 center, const vec3 up)
{
	vec3 n = unit_v(eye - center);
	vec3 l = unit_v(cross(up, n));
	vec3 m = unit_v(cross(n, l));
	ModelView = mat<4, 4>{{{l.x, l.y, l.z, 0}, {m.x, m.y, m.z, 0}, {n.x, n.y, n.z, 0}, {0, 0, 0, 1}}} *
				mat<4, 4>{{{1, 0, 0, -center.x}, {0, 1, 0, -center.y}, {0, 0, 1, -center.z}, {0, 0, 0, 1}}};
}

void rasterize(const vec4 clip[3], std::vector<double> &zbuffer, TGAImage &framebuffer, const TGAColor color)
{
	vec4 ndc[3] = {clip[0] / clip[0].w, clip[1] / clip[1].w, clip[2] / clip[2].w};					 // normalized device coordinates
	vec2 screen[3] = {(Viewport * ndc[0]).xy(), (Viewport * ndc[1]).xy(), (Viewport * ndc[2]).xy()}; // screen coordinates

	mat<3, 3> ABC = {{{screen[0].x, screen[0].y, 1.}, {screen[1].x, screen[1].y, 1.}, {screen[2].x, screen[2].y, 1.}}};
	if (det(ABC) < 1)
		return; // backface culling + discarding triangles that cover less than a pixel
	mat<3, 3> ABC_it = invert_transpose(ABC);

	double bbminx, bbmaxx, bbminy, bbmaxy;
	std::tie(bbminx, bbmaxx) = std::minmax({screen[0].x, screen[1].x, screen[2].x}); // bounding box for the triangle
	std::tie(bbminy, bbmaxy) = std::minmax({screen[0].y, screen[1].y, screen[2].y}); // defined by its top left and bottom right corners

#pragma omp parallel for
	for (int x = std::max<int>(bbminx, 0); x <= std::min<int>(bbmaxx, framebuffer.width() - 1); x++)
	{ // clip the bounding box by the screen
		for (int y = std::max<int>(bbminy, 0); y <= std::min<int>(bbmaxy, framebuffer.height() - 1); y++)
		{
			vec3 bc = ABC_it * vec3{static_cast<double>(x), static_cast<double>(y), 1.}; // barycentric coordinates of {x,y} w.r.t the triangle
			if (bc.x < 0 || bc.y < 0 || bc.z < 0)
				continue; // negative barycentric coordinate => the pixel is outside the triangle
			double z = bc * vec3{ndc[0].z, ndc[1].z, ndc[2].z};
			if (z <= zbuffer[x + y * framebuffer.width()])
				continue;
			zbuffer[x + y * framebuffer.width()] = z;
			framebuffer.set(x, y, color);
		}
	}
}

TGAImage draw_zbuffer(const std::vector<double> &zbuffer, int width, int height)
{
	TGAImage zbuffer_img(width, height, TGAImage::GRAYSCALE); // starts all black

	const double empty = -std::numeric_limits<double>::max(); // the value the buffer was initialized with

	// Find the depth range of the pixels that were actually written
	double min_z = std::numeric_limits<double>::max();
	double max_z = empty;
	for (double z : zbuffer)
	{
		if (z != empty)
		{
			min_z = std::min(min_z, z);
			max_z = std::max(max_z, z);
		}
	}

	// Nothing was drawn: return the black image
	if (max_z == empty)
		return zbuffer_img;

	// If every written pixel has the same depth, avoid dividing by zero
	double range = max_z - min_z;
	if (range == 0)
		range = 1;

	for (int x = 0; x < width; x++)
	{
		for (int y = 0; y < height; y++)
		{
			double z = zbuffer[x + y * width];
			if (z != empty)
			{
				unsigned char val = static_cast<unsigned char>(255.0 * (z - min_z) / range);
				zbuffer_img.set(x, y, {val});
			}
		}
	}

	return zbuffer_img;
}

int main(int argc, char **argv)
{
	if (argc < 2)
	{
		std::cerr << "Usage: " << argv[0] << " obj/model.obj" << std::endl;
		return -1;
	}

	constexpr int width = 800; // output image size
	constexpr int height = 800;
	constexpr vec3 eye{-1, 0, 2};	// camera position
	constexpr vec3 center{0, 0, 0}; // camera direction
	constexpr vec3 up{0, 1, 0};		// camera up vector

	lookat(eye, center, up);
	perspective(norm(eye - center));
	viewport(0, 0, width, height);

	TGAImage framebuffer(width, height, TGAImage::RGB);
	std::vector<double> zbuffer(width * height, -std::numeric_limits<double>::max());

	for (int m = 1; m < argc; m++)
	{
		Model model(argv[m]);
		for (int f = 0; f < model.nfaces(); f++)
		{
			vec4 clip[3];
			for (int d : {0, 1, 2})
			{ // assemble the primitive
				vec3 v = model.vert(f, d);
				clip[d] = Perspective * ModelView * vec4{v.x, v.y, v.z, 1.};
			}
			TGAColor rnd;
			for (int c = 0; c < 3; c++)
				rnd[c] = std::rand() % 256;
			rasterize(clip, zbuffer, framebuffer, rnd); // rasterize the primitive
		}
	}

	TGAImage zbuffer_tga = draw_zbuffer(zbuffer, width, height);
	zbuffer_tga.write_tga_file("zbuffer.tga");

	framebuffer.write_tga_file("framebuffer.tga");
	return 0;
}