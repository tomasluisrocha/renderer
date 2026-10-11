#include <cmath>
#include <cstdlib>
#include <ctime>
#include "tgaimage.h"
#include "model.h"
#include "geometry.h"

constexpr int width = 800;
constexpr int height = 800;

constexpr TGAColor white = {255, 255, 255, 255}; // attention, BGRA order
constexpr TGAColor green = {0, 255, 0, 255};
constexpr TGAColor red = {0, 0, 255, 255};
constexpr TGAColor blue = {255, 128, 64, 255};
constexpr TGAColor yellow = {0, 200, 255, 255};

void line(int ax, int ay, int bx, int by, TGAImage &framebuffer, TGAColor color)
{
	bool steep = std::abs(ax - bx) < std::abs(ay - by);
	if (steep)
	{
		std::swap(ax, ay);
		std::swap(bx, by);
	}
	if (ax > bx)
	{
		std::swap(ax, bx);
		std::swap(ay, by);
	}
	int y = ay;
	int ierror = 0;
	for (int x = ax; x <= bx; x++)
	{
		if (steep)
		{
			framebuffer.set(y, x, color);
		}
		else
		{
			framebuffer.set(x, y, color);
		}
		ierror += 2 * std::abs(by - ay);
		if (ierror > (bx - ax))
		{
			y += by > ay ? 1 : -1;
			ierror -= 2 * (bx - ax);
		}
	}
}

vec2 projection(const vec3 point)
{
	// Simple ortho
	return {(point.x + 1) * width / 2, (point.y + 1) * height / 2};
}

double signed_triangle_area(int ax, int ay, int bx, int by, int cx, int cy)
{
	return .5 * ((by - ay) * (bx + ax) + (cy - by) * (cx + bx) + (ay - cy) * (ax + cx));
}

void triangle(int ax, int ay, int bx, int by, int cx, int cy, TGAImage &framebuffer, TGAColor color)
{
	int bbminx = std::min(std::min(ax, bx), cx); // bounding box for the triangle
	int bbminy = std::min(std::min(ay, by), cy); // defined by its top left and bottom right corners
	int bbmaxx = std::max(std::max(ax, bx), cx);
	int bbmaxy = std::max(std::max(ay, by), cy);
	double total_area = signed_triangle_area(ax, ay, bx, by, cx, cy);
	if (total_area < 1)
	{
		return;
	}

#pragma omp parallel for
	for (int x = bbminx; x <= bbmaxx; x++)
	{
		for (int y = bbminy; y <= bbmaxy; y++)
		{
			double alpha = signed_triangle_area(x, y, bx, by, cx, cy) / total_area;
			double beta = signed_triangle_area(ax, ay, x, y, cx, cy) / total_area;
			double gamma = signed_triangle_area(ax, ay, bx, by, x, y) / total_area;
			if (alpha < 0 || beta < 0 || gamma < 0)
			{
				continue;
			}
			framebuffer.set(x, y, color);
		}
	}
}

int main(int argc, char **argv)
{
	if (argc != 2)
	{
		std::cerr << "Usage: " << argv[0] << " obj/model.obj" << std::endl;
		return -1;
	}
	Model model(argv[1]);
	TGAImage framebuffer(width, height, TGAImage::RGB);

	// Iterate over every face and draw the three edges

	for (int i = 0; i < model.nfaces(); i++)
	{
		vec2 a = projection(model.vert(i, 0));
		vec2 b = projection(model.vert(i, 1));
		vec2 c = projection(model.vert(i, 2));
		TGAColor rnd;
		for (int c = 0; c < 3; c++)
			rnd[c] = std::rand() % 255;
		triangle(a.x, a.y, b.x, b.y, c.x, c.y, framebuffer, rnd);
	}

	for (int i = 0; i < model.nverts(); i++)
	{
		vec2 point = projection(model.vert(i));
		framebuffer.set(point.x, point.y, white);
	}

	framebuffer.write_tga_file("framebuffer.tga");
	return 0;
}