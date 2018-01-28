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

#include "meshfilter.h"
#include "glm_main.h"
#include <vector>
#include <list>
#include <iostream>

void Mesh::RecalculateNormals() {
  size_t n_vertices = vertices.size();
  size_t n_indices = indices.size();
  size_t n_triangles = n_indices / 3;

  if (indices.empty()) {
    return;
  }

  typedef std::list<glm::vec3> PerVertexNormalsList;
  std::vector<PerVertexNormalsList> per_vertex_normals(n_vertices);
  normals.clear();
  normals.resize(n_vertices);

  // 0 1 2 3 4 5  => n_indices = 6, n_triangles = 2
  // t = 0 => 0 1 2
  // t = 1 => 3 4 5
  for (size_t t = 0; t < n_triangles; ++t) {
    size_t i0 = indices[t * 3 + 0];
    size_t i1 = indices[t * 3 + 1];
    size_t i2 = indices[t * 3 + 2];
    const glm::vec3& v0 = vertices.at(i0);
    const glm::vec3& v1 = vertices.at(i1);
    const glm::vec3& v2 = vertices.at(i2);

    glm::vec3 n = glm::cross(v1 - v0, v2 - v0);
    per_vertex_normals[i0].push_back(n);
    per_vertex_normals[i1].push_back(n);
    per_vertex_normals[i2].push_back(n);
  }

  for (size_t i = 0; i < n_vertices; ++i) {
    glm::vec3& normal = normals[i];
    PerVertexNormalsList& normals_list = per_vertex_normals[i];
    for (auto& n: normals_list) {
      normal += n;
    }
    normal = glm::normalize(normal);
  }
}

