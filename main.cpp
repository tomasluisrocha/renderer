#include <algorithm>
#include <cstdlib>
#include <iostream>
#include <limits>
#include <vector>
#include "gl.h"
#include "model.h"

constexpr TGAColor white = {255, 255, 255, 255}; // attention, BGRA order
constexpr TGAColor green = {0, 255, 0, 255};
constexpr TGAColor red = {0, 0, 255, 255};
constexpr TGAColor blue = {255, 128, 64, 255};
constexpr TGAColor yellow = {0, 200, 255, 255};

// Paints every fragment of a triangle with one flat color
struct RandomShader : IShader
{
	const Model &model;
	TGAColor color = {};
	vec3 tri[3]; // triangle in eye coordinates

	RandomShader(const Model &m) : model(m) {}

	// Runs once per triangle corner: object coordinates in, clip coordinates out
	vec4 vertex(const int face, const int vert)
	{
		vec3 v = model.vert(face, vert); // current vertex in object coordinates
		vec4 gl_Position = ModelView * embed(v);
		tri[vert] = gl_Position.xyz();	  // in eye coordinates
		return Perspective * gl_Position; // in clip coordinates
	}

	std::pair<bool, TGAColor> fragment(const vec3 bar) const override
	{
		return {false, color}; // do not discard the pixel
	}
};

struct PhongShader : IShader
{
	const Model &model;
	vec3 light_dir;
	double shine_exp;
	bool smooth;

	vec3 tri[3]; // triangle in eye coordinates
	vec3 normal[3];

	PhongShader(const Model &model, const vec3 &light, double shine_exp)
		: model(model),
		  light_dir(unit_v((ModelView * vec4{light.x, light.y, light.z, 0.}).xyz())),	// Eye coordinates
		  shine_exp(shine_exp),
		  smooth(true) {}

	PhongShader(const Model &model, const vec3 &light, double shine_exp, bool smooth)
		: model(model),
		  light_dir(unit_v((ModelView * vec4{light.x, light.y, light.z, 0.}).xyz())),
		  shine_exp(shine_exp),
		  smooth(smooth) {}

	vec4 vertex(const int face, const int vert)
	{
		vec3 v = model.vert(face, vert); // current vertex in object coordinates
		vec4 gl_Position = ModelView * embed(v);
		tri[vert] = gl_Position.xyz(); // in eye coordinates
		normal[vert] = unit_v((ModelView * embed(model.get_normal(face, vert), 0)).xyz());
		return Perspective * gl_Position; // in clip coordinates
	}

	std::pair<bool, TGAColor> fragment(const vec3 bar) const override
	{
		TGAColor gl_FragColor = {255, 255, 255, 255}; // output color of the fragment
		vec3 L = unit_v(light_dir);
		vec3 N;
		if (smooth)
		{
			N = unit_v(normal[0] * bar.x + normal[1] * bar.y + normal[2] * bar.z); // Smooth Shading
		}
		else
		{
			N = unit_v(cross((tri[1] - tri[0]), (tri[2] - tri[0]))); // Flat Shading
		}
		vec3 P = tri[0] * bar.x + tri[1] * bar.y + tri[2] * bar.z;

		// Ambient
		double ambient = .3;

		// Diffuse
		double diffuse_dot = N * L;
		double diffuse = std::max(0.0, diffuse_dot);

		// Specula
		double specular = 0.0;
		if (diffuse_dot > 0.0)
		{
			vec3 R = unit_v(N * (2.0 * diffuse_dot) - L);
			vec3 V = unit_v(-P);
			double cos_beta = std::max(0.0, R * V);
			specular = std::pow(cos_beta, shine_exp);
		}

		for (int channel : {0, 1, 2})
			gl_FragColor[channel] *= std::min(1., ambient + .4 * diffuse + .9 * specular);
		return {false, gl_FragColor}; // do not discard the pixel
	}
};

TGAImage
draw_zbuffer(const std::vector<double> &depth, int width, int height)
{
	TGAImage zbuffer_img(width, height, TGAImage::GRAYSCALE); // starts all black

	// Find the depth range of the pixels that were actually written
	double min_z = std::numeric_limits<double>::max();
	double max_z = ZBUFFER_EMPTY;
	for (double z : depth)
	{
		if (z != ZBUFFER_EMPTY)
		{
			min_z = std::min(min_z, z);
			max_z = std::max(max_z, z);
		}
	}

	// Nothing was drawn: return the black image
	if (max_z == ZBUFFER_EMPTY)
		return zbuffer_img;

	// If every written pixel has the same depth, avoid dividing by zero
	double range = max_z - min_z;
	if (range == 0)
		range = 1;

	for (int x = 0; x < width; x++)
	{
		for (int y = 0; y < height; y++)
		{
			double z = depth[x + y * width];
			if (z != ZBUFFER_EMPTY)
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
	constexpr vec3 light{1, 1, 1};
	constexpr vec3 eye{-1, 0, 2};	// camera position
	constexpr vec3 center{0, 0, 0}; // point the camera looks at
	constexpr vec3 up{0, 1, 0};		// camera up vector

	lookat(eye, center, up);											   // build the ModelView   matrix
	init_perspective(norm(eye - center));								   // build the Perspective matrix
	init_viewport(width / 16, height / 16, width * 7 / 8, height * 7 / 8); // build the Viewport    matrix
	init_zbuffer(width, height);
	TGAImage framebuffer(width, height, TGAImage::RGB, {177, 195, 209, 255});

	for (int m = 1; m < argc; m++)
	{						  // iterate through all input objects
		Model model(argv[m]); // load the data
		PhongShader shader(model, light, 35.0);
		for (int f = 0; f < model.nfaces(); f++)
		{										  // iterate through all facets
			Triangle clip = {shader.vertex(f, 0), // assemble the primitive
							 shader.vertex(f, 1),
							 shader.vertex(f, 2)};
			rasterize(clip, shader, framebuffer); // rasterize the primitive
		}
	}

	TGAImage zbuffer_tga = draw_zbuffer(zbuffer, width, height);
	zbuffer_tga.write_tga_file("zbuffer.tga");

	framebuffer.write_tga_file("framebuffer.tga");
	return 0;
}