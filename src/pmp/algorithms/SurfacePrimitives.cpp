// Copyright 2011-2020 the Polygon Mesh Processing Library developers.
// Distributed under a MIT-style license, see LICENSE.txt for details.

#include "SurfacePrimitives.h"
#include "SurfaceSubdivision.h"
#include "DifferentialGeometry.h"

namespace pmp {

void project_to_unit_sphere(SurfaceMesh& mesh)
{
    for (auto v : mesh.vertices())
    {
        auto p = mesh.position(v);
        auto n = norm(p);
        mesh.position(v) = (1.0 / n) * p;
    }
}

void SurfacePrimitives::tetrahedron()
{
    mesh_.clear();
    float a = 1.0f / 3.0f;
    float b = sqrt(8.0f / 9.0f);
    float c = sqrt(2.0f / 9.0f);
    float d = sqrt(2.0f / 3.0f);

    auto v0 = mesh_.add_vertex(Point(0, 0, 1));
    auto v1 = mesh_.add_vertex(Point(-c, d, -a));
    auto v2 = mesh_.add_vertex(Point(-c, -d, -a));
    auto v3 = mesh_.add_vertex(Point(b, 0, -a));

    mesh_.add_triangle(v0, v1, v2);
    mesh_.add_triangle(v0, v2, v3);
    mesh_.add_triangle(v0, v3, v1);
    mesh_.add_triangle(v3, v2, v1);
}

void SurfacePrimitives::hexahedron()
{
    mesh_.clear();

    float a = 1.0f / sqrt(3.0f);
    auto v0 = mesh_.add_vertex(Point(-a, -a, -a));
    auto v1 = mesh_.add_vertex(Point(a, -a, -a));
    auto v2 = mesh_.add_vertex(Point(a, a, -a));
    auto v3 = mesh_.add_vertex(Point(-a, a, -a));
    auto v4 = mesh_.add_vertex(Point(-a, -a, a));
    auto v5 = mesh_.add_vertex(Point(a, -a, a));
    auto v6 = mesh_.add_vertex(Point(a, a, a));
    auto v7 = mesh_.add_vertex(Point(-a, a, a));

    mesh_.add_quad(v3, v2, v1, v0);
    mesh_.add_quad(v2, v6, v5, v1);
    mesh_.add_quad(v5, v6, v7, v4);
    mesh_.add_quad(v0, v4, v7, v3);
    mesh_.add_quad(v3, v7, v6, v2);
    mesh_.add_quad(v1, v5, v4, v0);
}

void SurfacePrimitives::octahedron()
{
    hexahedron();
    dualize(mesh_);
    project_to_unit_sphere(mesh_);
}

void SurfacePrimitives::dodecahedron()
{
    icosahedron();
    dualize(mesh_);
    project_to_unit_sphere(mesh_);
}

void SurfacePrimitives::icosahedron()
{
    mesh_.clear();

    // adapted from http://paulbourke.net/geometry/platonic/
    float radius = 1.0;
    float sqrt5 = sqrt(5.0f);
    float phi = (1.0f + sqrt5) * 0.5f; // golden ratio
    // ratio of edge length to radius
    float ratio = sqrt(10.0f + (2.0f * sqrt5)) / (4.0f * phi);
    float a = (radius / ratio) * 0.5;
    float b = (radius / ratio) / (2.0f * phi);

    auto v1 = mesh_.add_vertex(Point(0, b, -a));
    auto v2 = mesh_.add_vertex(Point(b, a, 0));
    auto v3 = mesh_.add_vertex(Point(-b, a, 0));
    auto v4 = mesh_.add_vertex(Point(0, b, a));
    auto v5 = mesh_.add_vertex(Point(0, -b, a));
    auto v6 = mesh_.add_vertex(Point(-a, 0, b));
    auto v7 = mesh_.add_vertex(Point(0, -b, -a));
    auto v8 = mesh_.add_vertex(Point(a, 0, -b));
    auto v9 = mesh_.add_vertex(Point(a, 0, b));
    auto v10 = mesh_.add_vertex(Point(-a, 0, -b));
    auto v11 = mesh_.add_vertex(Point(b, -a, 0));
    auto v12 = mesh_.add_vertex(Point(-b, -a, 0));

    mesh_.add_triangle(v3, v2, v1);
    mesh_.add_triangle(v2, v3, v4);
    mesh_.add_triangle(v6, v5, v4);
    mesh_.add_triangle(v5, v9, v4);
    mesh_.add_triangle(v8, v7, v1);
    mesh_.add_triangle(v7, v10, v1);
    mesh_.add_triangle(v12, v11, v5);
    mesh_.add_triangle(v11, v12, v7);
    mesh_.add_triangle(v10, v6, v3);
    mesh_.add_triangle(v6, v10, v12);
    mesh_.add_triangle(v9, v8, v2);
    mesh_.add_triangle(v8, v9, v11);
    mesh_.add_triangle(v3, v6, v4);
    mesh_.add_triangle(v9, v2, v4);
    mesh_.add_triangle(v10, v3, v1);
    mesh_.add_triangle(v2, v8, v1);
    mesh_.add_triangle(v12, v10, v7);
    mesh_.add_triangle(v8, v11, v7);
    mesh_.add_triangle(v6, v12, v5);
    mesh_.add_triangle(v11, v9, v5);
}

void SurfacePrimitives::icosphere(size_t n_subdivisions)
{
    icosahedron();
    SurfaceSubdivision subdiv(mesh_);
    for (size_t i = 0; i < n_subdivisions; i++)
    {
        subdiv.loop();
        project_to_unit_sphere(mesh_);
    }
}

void SurfacePrimitives::quad_sphere(size_t n_subdivisions)
{
    hexahedron();
    SurfaceSubdivision subdiv(mesh_);
    for (size_t i = 0; i < n_subdivisions; i++)
    {
        subdiv.catmull_clark();
        project_to_unit_sphere(mesh_);
    }
}

void SurfacePrimitives::uv_sphere(const Point& center, Scalar radius,
                                  size_t n_slices, size_t n_stacks)
{
    mesh_.clear();

    // add top vertex
    auto v0 = mesh_.add_vertex(Point(center[0], center[1] + radius, center[2]));

    // generate vertices per stack / slice
    for (size_t i = 0; i < n_stacks - 1; i++)
    {
        auto phi = M_PI * double(i + 1) / double(n_stacks);
        for (size_t j = 0; j < n_slices; ++j)
        {
            auto theta = 2.0 * M_PI * double(j) / double(n_slices);
            auto x = center[0] + radius * std::sin(phi) * std::cos(theta);
            auto y = center[1] + radius * std::cos(phi);
            auto z = center[2] + radius * std::sin(phi) * std::sin(theta);
            mesh_.add_vertex(Point(x, y, z));
        }
    }

    // add bottom vertex
    auto v1 = mesh_.add_vertex(Point(center[0], center[1] - radius, center[2]));

    // add top / bottom triangles
    size_t i0, i1;
    for (size_t i = 0; i < n_slices; ++i)
    {
        i0 = i + 1;
        i1 = (i + 1) % n_slices + 1;
        mesh_.add_triangle(v0, Vertex(i1), Vertex(i0));

        i0 = i + n_slices * (n_stacks - 2) + 1;
        i1 = (i + 1) % n_slices + n_slices * (n_stacks - 2) + 1;
        mesh_.add_triangle(v1, Vertex(i0), Vertex(i1));
    }

    // add quads per stack / slice
    size_t i2, i3;
    for (size_t j = 0; j < n_stacks - 2; ++j)
    {
        size_t idx0 = j * n_slices + 1;
        size_t idx1 = (j + 1) * n_slices + 1;
        for (size_t i = 0; i < n_slices; ++i)
        {
            i0 = idx0 + i;
            i1 = idx0 + (i + 1) % n_slices;
            i2 = idx1 + (i + 1) % n_slices;
            i3 = idx1 + i;
            mesh_.add_quad(Vertex(i0), Vertex(i1), Vertex(i2), Vertex(i3));
        }
    }
}

} // namespace pmp