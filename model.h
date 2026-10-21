#include <vector>
#include <string>
#include "geometry.h"
#include "tgaimage.h"

class Model
{
    std::vector<vec4> verts = {};
    std::vector<int> facet_vrt = {};

    std::vector<vec4> normals = {};
    std::vector<int> facet_nrm = {};

    std::vector<vec2> tex = {};
    std::vector<int> facet_tex = {};

    TGAImage normalmap = {};
    TGAImage diffusemap = {};
    TGAImage specularmap = {};

public:
    Model(const std::string filename);
    int nverts() const;
    int nfaces() const;
    vec4 get_vert(const int i) const;
    vec4 get_vert(const int iface, const int nthvert) const;
    vec4 get_normal(const vec2 &uv) const;
    vec4 get_normal(const int iface, const int nthvert) const;
    TGAColor get_color(const vec2 &uv) const;
    double get_specular(const vec2 &uv) const;
    vec2 get_uv(const int iface, const int nthvert) const;
};
