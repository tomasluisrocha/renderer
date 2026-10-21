#include <vector>
#include <string>
#include "geometry.h"

class Model
{
    std::vector<vec3> verts = {};
    std::vector<int> facet_vrt = {};

    std::vector<vec3> normals = {};
    std::vector<int> facet_nrm = {};

public:
    Model(const std::string filename);
    int nverts() const;
    int nfaces() const;
    vec3 vert(const int i) const;
    vec3 vert(const int iface, const int nthvert) const;
    vec3 get_normal(const int i) const;
    vec3 get_normal(const int iface, const int nthvert) const;
};
