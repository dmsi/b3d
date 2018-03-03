//
// This source file is a part of $PROJECT_NAME$
//
// Copyright (C) $COPYRIGHT$
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

#ifndef _MESH_H_0DBCC8DE_F0FB_4E34_A4EC_505710111974_
#define _MESH_H_0DBCC8DE_F0FB_4E34_A4EC_505710111974_ 

#include "glm_main.h"
#include <vector>
#include <functional>

//////////////////////////////////////////////////////////////////////////////
// Just instead of full-inheritence-virtualization 
//////////////////////////////////////////////////////////////////////////////
struct MeshFilterView {
  size_t                    n_vertices;
  size_t                    n_indices;
  int                       index_type;
  std::function<void(void)> Bind;
  std::function<void(void)> Unbind;
};

//////////////////////////////////////////////////////////////////////////////
// The following attribute layout:
// Per vertex
// #0 position  vec3
// #1 normals   vec3
// #2 color     vec4
// #3 uv        vec2
// #4 tangent   vec3 [TBD]
// #5 bitangent vec3 [TBD]
// #6 Extra1    vec4
// #7 Extra2    vec4
// Per instance
// #8 - 15
//////////////////////////////////////////////////////////////////////////////
class MeshFilterBase {
 public:

  // 0-7 reserved for per-vertex, 8-15 - per instance
  // Normal mesh layout using reserved slots for each attributes:
  //  0 - vec3 position
  //  1 - vec3 normal
  //  2 - vec4 color
  //  3 - vec2 uv
  //  4 - vec3 tangent
  //  5 - vec3 bitangent
  //  6 - vec4 anything 
  //  7 - vec4 anything
  // Flexible mesh can use layout based
  enum Attribute {
    kPosition = 0,
    kNormal,
    kColor,
    kUv,
    kTangent,
    kBiTangent,
    kExtra1,
    kExtra2,
    kPerInstance,
    kIndices = 16,
    kTotal = 17
  };

  virtual MeshFilterView GetView() = 0;
};

//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
class Mesh {
 public:
  Mesh() {
  }

  Mesh(int num_vertices, int num_indices) 
    :  indices                 (num_indices),
       vertices                (num_vertices),
       normals                 (num_vertices),
       colors                  (num_vertices),
       uv                      (num_vertices) {
  }

  void Reset() {
    std::fill(indices.begin(), indices.end(), 0);
    std::fill(vertices.begin(), vertices.end(), glm::vec3(0,0,0));
    std::fill(normals.begin(), normals.end(), glm::vec3(0,0,0));
    std::fill(colors.begin(), colors.end(), glm::vec4(0,0,0,0));
    std::fill(uv.begin(), uv.end(), glm::vec2(0,0));
  }

  void Clear() {
    indices.clear();
    vertices.clear();
    normals.clear();
    colors.clear();
    uv.clear();
  }

  bool IsValid() const {
    if (vertices.empty()) {
      return false;
    }

    auto len = vertices.size();

    if (!normals.empty() && normals.size() != len) {
      return false;
    }
    if (!colors.empty() && colors.size() != len) {
      return false;
    }
    if (!uv.empty() && uv.size() != len) {
      return false;
    }

    return true;
  }

  void RecalculateNormals();

  // TODO: indices16 and indices32
  std::vector< uint32_t >       indices;
  std::vector< glm::vec3 >      vertices;
  std::vector< glm::vec3 >      normals;
  std::vector< glm::vec4 >      colors;
  std::vector< glm::vec2 >      uv;
};

#endif // _MESH_H_0DBCC8DE_F0FB_4E34_A4EC_505710111974_
