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

#ifndef _MESHFILTER_H_B169C760_F9D6_42B9_81B7_91955A69A250_
#define _MESHFILTER_H_B169C760_F9D6_42B9_81B7_91955A69A250_ 

#include "vertexarrayobject.h"
#include "common/logging.h"
#include "mesh.h"

#include <cstdint>
#include <memory>
#include <vector>
#include <algorithm>
#include <exception>
#include <iostream>

//////////////////////////////////////////////////////////////////////////////
// Bakes mesh to VRAM using starndard mesh layout
//////////////////////////////////////////////////////////////////////////////
class MeshFilter : public MeshFilterBase {
 public:
  enum BakeMode {kStatic, kDynamic};

  MeshFilterView GetView() override {
    return MeshFilterView {
      mesh_ ? mesh_->vertices.size() : 0,
      mesh_ && !mesh_->indices.empty() ? mesh_->indices.size() : 0,
      index_type_, 
      [this]() {Bind();},
      [this]() {Unbind();}
    };
  }

  ////////////////////////////////////////////////////////////////////////////
  // Once mesh is set, the filter is going to bake it and upload to vRAM.
  ////////////////////////////////////////////////////////////////////////////
  MeshFilter();

  void SetMode(BakeMode mode) {
    if (mode != bake_mode_) {
      bake_mode_ = mode;
      vao_.reset(new VertexArrayObject());

      if (mesh_) {
        Bake();
      }
    }
  }

  void SetMesh(std::shared_ptr<Mesh> mesh) {
    mesh_ = mesh;
    if (mesh_) {
      Bake();
    }
  }

  std::shared_ptr<Mesh> GetMesh() {
    return mesh_;
  }

  void Bind() {
    vao_->Bind();
  }

  void Unbind() {
    vao_->Unbind();
  }

  ////////////////////////////////////////////////////////////////////////////
  // Bypass Map/Unmap this directly to the VAO for now
  // In order to manipulate flexible/per-instance attributes
  ////////////////////////////////////////////////////////////////////////////
  template <typename TLayout, typename... TArgs>
  auto Map(TArgs&&... args) {
    assert(vao_);
    return vao_->Map<TLayout>(std::forward<TArgs>(args)...);
  }
  
  template <typename... TArgs>
  auto Unmap(TArgs&&... args) {
    assert(vao_);
    vao_->Unmap(std::forward<TArgs>(args)...);
  }

  template <typename TLayout, typename... TArgs>
  void Upload(TArgs&&... args) {
    assert(vao_);
    vao_->Upload<TLayout>(std::forward<TArgs>(args)...);
  }

 private:
  ////////////////////////////////////////////////////////////////////////////
  // Mesh bakery ....
  ////////////////////////////////////////////////////////////////////////////
  void Bake(); 
  void AdjustSlots(); 
  void RecalculateIndexType();

  VertexArrayObject::Usage GetUsage() const {
    switch (bake_mode_) {
      case kStatic  : return VertexArrayObject::kUsageStatic;
      case kDynamic : return VertexArrayObject::kUsageDynamic; 
      default       : ABORT_F("Invalid bake mode");
    }
  }


  ////////////////////////////////////////////////////////////////////////////
  // Baker and the ingridients 
  ////////////////////////////////////////////////////////////////////////////
  BakeMode                              bake_mode_;
  std::shared_ptr<Mesh>                 mesh_;
  std::shared_ptr<VertexArrayObject>    vao_;
  std::bitset<MeshFilterBase::kTotal>   attrib_slots_;
  int                                   index_type_;
};

#endif // _MESHFILTER_H_B169C760_F9D6_42B9_81B7_91955A69A250_
