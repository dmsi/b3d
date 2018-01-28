//
// This source file is a part of borsch.3d
//
// Copyright (C) borsch.3d team 2017-2018
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
// THE SOFTWARE.
//

#include "meshloader.h"
#include <vector>
#include <fstream>
#include <sstream>
#include <iostream>

std::shared_ptr<Mesh> MeshLoader::LoadDsm(const std::string& filename) {
  std::ifstream in(filename, std::ios::in);
  if (!in.is_open()) {
    throw std::runtime_error("MeshLoader::LoadDsm() - Cant open file " + filename);
  }

  std::string line;
  int n_vertices, n_indices;

  in >> n_vertices >> n_indices;

  std::shared_ptr<Mesh> mesh(new Mesh(n_vertices, n_indices));
  for (int i = 0; i < n_vertices; ++i) {
    in >> mesh->vertices[i].x >> mesh->vertices[i].y >> mesh->vertices[i].z
       >> mesh->normals[i].x  >> mesh->normals[i].y  >> mesh->normals[i].z
       >> mesh->colors[i].r   >> mesh->colors[i].g   >> mesh->colors[i].b    >> mesh->colors[i].a
       >> mesh->uv[i].x       >> mesh->uv[i].y;
  }

  for (int i = 0; i < n_indices; ++i) {
    in >> mesh->indices[i];
  }

  return mesh;
}

void MeshLoader::ExportDsm(std::shared_ptr<Mesh> mesh, const std::string& filename) {
  std::ofstream out(filename, std::ios::out);
  if (!out.is_open()) {
    throw std::runtime_error("MeshLoader::ExportDsm() - Cant open file " + filename);
  }

  out << mesh->vertices.size() << std::endl << mesh->indices.size() << std::endl;

  for (int i = 0; i < mesh->vertices.size(); ++i) {
    out << mesh->vertices[i].x << " " << mesh->vertices[i].y << " " << mesh->vertices[i].z << " "
        << mesh->normals[i].x  << " " << mesh->normals[i].y  << " " << mesh->normals[i].z  << " "
        << mesh->colors[i].r   << " " << mesh->colors[i].g   << " " << mesh->colors[i].b   << " " << mesh->colors[i].a << " "
        << mesh->uv[i].x       << " " << mesh->uv[i].y       << " " << std::endl;
  }

  for (int i = 0; i < mesh->indices.size(); ++i) {
    out << mesh->indices[i] << std::endl;
  }
}
