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

#ifndef _MESHRENDERER_H_5B91DDA6_5485_41DC_A13B_6E5F7B334E05_
#define _MESHRENDERER_H_5B91DDA6_5485_41DC_A13B_6E5F7B334E05_ 

#include "gl_main.h"
#include "meshfilter.h"
#include "material/material.h"
#include <memory>

//////////////////////////////////////////////////////////////////////////////
// Even more Unity3D-ness 
//////////////////////////////////////////////////////////////////////////////
class MeshRenderer {
 public:
  enum PrimitiveType {
    kPtPoints,
    kPtLineStrip,
    kPtLineLoop,
    kPtLines,
    kPtLineStringAdjacency,
    kPtLinesAdjacency,
    kPtTriangleStrip,
    kPtTriangleFan,
    kPtTriangles,
    kPtTriangleStripAdjacency,
    kPtTrianglesAdjacency,
    kPtPatches,
  };

  enum Indexing {
    kIndexTrue,
    kIndexFalse,
    kIndexAuto
  };

  PrimitiveType primitive   = kPtTriangles;
  Indexing      indexing    = kIndexAuto;
  size_t        n_instances = 1;

  void SetMaterial(std::shared_ptr<Material> material) {
    material_ = material;
  }
  
  std::shared_ptr<Material> GetMaterial() {
    return material_;
  }

  ////////////////////////////////////////////////////////////////////////////
  // TODO(DS) add support for other types that GL_TRIANGLES
  ////////////////////////////////////////////////////////////////////////////
  void DrawCall(MeshFilter& mesh_filter) {
    if (auto mesh = mesh_filter.GetMesh()) {
      mesh_filter.Bind();
      DoDraw(mesh.get(), mesh_filter.GetSlot(MeshFilter::Slot::kIndices));
      //if (mesh_filter.GetSlot(MeshFilter::Slot::kIndices)) {
      //  glDrawElementsInstanced(GL_TRIANGLES, mesh->indices.size(), GL_UNSIGNED_SHORT, nullptr, 1);
      //} else {
      //  glDrawArrays(GL_TRIANGLES, 0, mesh->vertices.size());
      //}
      mesh_filter.Unbind();
    }
  }

 private:
  void DoDraw(Mesh* mesh, bool has_indices) {
    assert(mesh);
    assert(n_instances >= 0);
    if (!has_indices) assert(indexing != kIndexTrue);

    bool use_indices = 
      indexing == kIndexTrue or
      (has_indices and indexing == kIndexAuto);
  
    if (use_indices) {
      glDrawElementsInstanced(ToOpenGL(primitive), mesh->indices.size(), 
                              GL_UNSIGNED_INT, nullptr, n_instances);
      //glDrawElementsInstanced(ToOpenGL(primitive), mesh->indices.size(), 
      //                        GL_UNSIGNED_SHORT, nullptr, n_instances);
    } else {
      // TODO!!! parametrize it
      glPatchParameteri(GL_PATCH_VERTICES, mesh->vertices.size());
      //glDrawArrays(ToOpenGL(primitive), 0, mesh->vertices.size());
      glDrawArraysInstanced(ToOpenGL(primitive), 0, mesh->vertices.size(), 
                            n_instances);
    }
  }

  GLenum ToOpenGL(PrimitiveType prim_type) {
    switch(prim_type) {
      case  kPtPoints:                 return GL_POINTS;
      case  kPtLineStrip:              return GL_LINE_STRIP;
      case  kPtLineLoop:               return GL_LINE_LOOP;
      case  kPtLines:                  return GL_LINES;
      case  kPtLineStringAdjacency:    return GL_LINE_STRIP_ADJACENCY;
      case  kPtLinesAdjacency:         return GL_LINES_ADJACENCY;
      case  kPtTriangleStrip:          return GL_TRIANGLE_STRIP;
      case  kPtTriangleFan:            return GL_TRIANGLE_FAN;
      case  kPtTriangles:              return GL_TRIANGLES;
      case  kPtTriangleStripAdjacency: return GL_TRIANGLE_STRIP_ADJACENCY;
      case  kPtTrianglesAdjacency:     return GL_TRIANGLES_ADJACENCY;
      case  kPtPatches:                return GL_PATCHES;
      default: assert(false);          return GL_TRIANGLES;
    }
  }
  
  std::shared_ptr<Material> material_;
};

#endif // _MESHRENDERER_H_5B91DDA6_5485_41DC_A13B_6E5F7B334E05_
