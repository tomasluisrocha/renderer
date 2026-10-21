# Software Rasterizer in C++

A CPU software rasterizer written in C++ with no external graphics libraries, built by following the [Tiny Renderer](https://haqr.eu/tinyrenderer/) course by Dmitry V. Sokolov.

![African Head render](render/african_head.png)
![African Head z-buffer render](render/african_head_zbuffer.png)

## Features

- **Mesh Processing & OBJ Loader**: Parses OBJ models with vertices, normals, texture coordinates, and triangulated faces, and loads the model's texture maps automatically (see [Textures](#textures)).
- **Linear Algebra Library**: Custom vector and matrix math classes supporting arbitrary dimensions, matrix multiplication, determinants, and matrix transposition/inversion.
- **Transformation Pipeline**: Complete camera look-at positioning (`ModelView`), perspective projection (`Perspective`), and viewport mapping (`Viewport`).
- **Triangle Rasterization**: Barycentric coordinate evaluation with bounding-box clipping and backface culling.
- **Z-Buffering**: Per-pixel depth interpolation and depth testing for accurate hidden-surface removal.
- **Shading & Lighting**:
  - Programmable shader interface (`IShader`) with vertex and fragment stages.
  - Full Phong reflection model combining ambient, diffuse, and specular light components.
  - Texture mapping with a diffuse color map, a normal map for per-pixel normals, and a specular map that controls how shiny each part of the surface is.
  - Alpha cutout: fragments from transparent texels are discarded, so meshes such as an eye's cornea don't cover what is behind them.
- **Performance**: Multi-threaded pixel rasterization powered by OpenMP.
- **Output Visualization**: Color framebuffer and normalized grayscale depth buffer exported as TGA images.

## Build and Run

Requires CMake and a C++20 compiler with OpenMP support.

```bash
cmake -B build
cmake --build build -j
./build/tinyrenderer path/to/model.obj
```

The rendered image is saved to `framebuffer.tga` and the visualized depth buffer is written to `zbuffer.tga`.

Several OBJ files can be passed at once. They are drawn into the same image and depth buffer, which is how a head and its separate eye meshes are rendered together:

```bash
./build/tinyrenderer path/to/head.obj path/to/eye.obj
```

The loader expects triangulated OBJ files whose faces use the `v/vt/vn` form (vertex, texture coordinate and normal indices). Models are not included in this repository.

## Textures

For a model `path/name.obj`, the loader looks next to the OBJ for three TGA files. Each name is built by dropping the `.obj` extension and appending a suffix:

| File | Suffix | Used for |
|---|---|---|
| `path/name_diffuse.tga` | `_diffuse.tga` | Base surface color |
| `path/name_nm.tga` | `_nm.tga` | Normal map: RGB channels are the XYZ components of the normal |
| `path/name_spec.tga` | `_spec.tga` | Specular map: scales the intensity of the highlight (one channel is read) |

For example, `african_head.obj` is rendered with `african_head_diffuse.tga`, `african_head_nm.tga` and `african_head_spec.tga`.

Notes:

- The normal map must be in **object (global) coordinates**. Tangent-space normal maps, such as the course's `_nm_tangent.tga` files, are not supported.
- Uncompressed and RLE-compressed TGA files are both accepted.
- The loader prints a message saying whether each file loaded. All three maps are expected: a missing one leaves that map empty, which gives a black diffuse color and wrong normals, so the model will not render correctly.
- A 32-bit diffuse map is treated as having an alpha channel, and texels with alpha below 128 are discarded. Maps with 24 bits per pixel are always opaque.

## Credits

This project follows the Tiny Renderer course, "Software rendering in 500 lines of bare C++", by Dmitry V. Sokolov: <https://haqr.eu/tinyrenderer/>.

The starting code comes from the course's [repository](https://github.com/ssloy/tinyrenderer): the TGA image class (`tgaimage.h`, `tgaimage.cpp`) and the CMake build configuration. It is included under the course's permissive license, whose notice is kept in `LICENSE-tinyrenderer.txt`.