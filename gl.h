#pragma once
#include <limits>
#include <utility>
#include <vector>
#include "tgaimage.h"
#include "geometry.h"

extern mat<4, 4> ModelView, Viewport, Perspective; // state matrices
extern std::vector<double> zbuffer;				   // depth buffer

// What every z-buffer cell holds until a fragment is written there.
// Shared so that init_zbuffer() and anything that reads the buffer agree on it.
constexpr double ZBUFFER_EMPTY = -std::numeric_limits<double>::max();

void lookat(const vec3 eye, const vec3 center, const vec3 up);
void init_perspective(const double f);
void init_viewport(const int x, const int y, const int w, const int h);
void init_zbuffer(const int width, const int height);

struct IShader
{
	virtual ~IShader() = default;
	// bar: barycentric coordinates of the fragment. Returns {discard, color}
	virtual std::pair<bool, TGAColor> fragment(const vec3 bar) const = 0;
};

typedef vec4 Triangle[3]; // a triangle primitive is made of three ordered points
void rasterize(const Triangle &clip, const IShader &shader, TGAImage &framebuffer);