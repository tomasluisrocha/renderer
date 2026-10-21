#include <fstream>
#include <sstream>
#include "model.h"

Model::Model(const std::string filename)
{
    std::ifstream in;
    in.open(filename, std::ifstream::in);
    if (in.fail())
        return;
    std::string line;
    while (!in.eof())
    {
        std::getline(in, line);
        std::istringstream iss(line);
        char trash;
        if (!line.compare(0, 2, "v "))
        {
            iss >> trash;
            vec4 v = {0, 0, 0, 1};
            for (int i : {0, 1, 2})
                iss >> v[i];
            verts.push_back(v);
        }
        else if (!line.compare(0, 3, "vn "))
        {
            std::string prefix;
            iss >> prefix;
            vec4 v;
            for (int i : {0, 1, 2})
                iss >> v[i];
            normals.push_back(v);
        }
        else if (!line.compare(0, 3, "vt "))
        {
            std::string prefix;
            iss >> prefix;
            vec2 uv;
            for (int i : {0, 1})
                iss >> uv[i];
            tex.push_back({uv.x, 1 - uv.y});
        }
        else if (!line.compare(0, 2, "f "))
        {
            int f, t, n, cnt = 0;
            iss >> trash;
            while (iss >> f >> trash >> t >> trash >> n)
            {
                facet_vrt.push_back(--f);
                facet_tex.push_back(--t);
                facet_nrm.push_back(--n);
                cnt++;
            }
            if (3 != cnt)
            {
                std::cerr << "Error: the obj file is upposed to be triangulated" << std::endl;
                return;
            }
        }
    }
    std::cerr << "# v# " << nverts() << " f# " << nfaces() << std::endl;
    auto load_texture = [&filename](const std::string suffix, TGAImage &img)
    {
        size_t dot = filename.find_last_of(".");
        if (dot == std::string::npos)
            return;
        std::string texfile = filename.substr(0, dot) + suffix;
        std::cerr << "texture file " << texfile << " loading " << (img.read_tga_file(texfile.c_str()) ? "ok" : "failed") << std::endl;
    };
    load_texture("_nm.tga", normalmap);
    load_texture("_diffuse.tga", diffusemap);
    load_texture("_spec.tga", specularmap);
}

int Model::nverts() const { return verts.size(); }
int Model::nfaces() const { return facet_vrt.size() / 3; }

vec4 Model::get_vert(const int i) const { return verts[i]; }
vec4 Model::get_vert(const int iface, const int nthvert) const { return verts[facet_vrt[iface * 3 + nthvert]]; }

vec4 Model::get_normal(const vec2 &uv) const
{
    TGAColor c = normalmap.get(uv[0] * normalmap.width(), uv[1] * normalmap.height());
    return vec4{(double)c[2], (double)c[1], (double)c[0], 0} * 2. / 255. - vec4{1, 1, 1, 0};
}

TGAColor Model::get_color(const vec2 &uv) const
{
    TGAColor c = diffusemap.get(uv[0] * diffusemap.width(), uv[1] * diffusemap.height());
    return c;
}

double Model::get_specular(const vec2 &uv) const
{
    return specularmap.get(uv[0] * specularmap.width(), uv[1] * specularmap.height())[0] / 255.0;
}

vec4 Model::get_normal(const int iface, const int nthvert) const { return normals[facet_nrm[iface * 3 + nthvert]]; }

vec2 Model::get_uv(const int iface, const int nthvert) const
{
    return tex[facet_tex[iface * 3 + nthvert]];
}
