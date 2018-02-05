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

#include <cstdint>
#include <memory>
#include <vector>
#include <algorithm>
#include <exception>
#include <iostream>

//////////////////////////////////////////////////////////////////////////////
// Unity3D-ness 
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

  std::vector< uint16_t >       indices;
  std::vector< glm::vec3 >      vertices;
  std::vector< glm::vec3 >      normals;
  std::vector< glm::vec4 >      colors;
  std::vector< glm::vec2 >      uv;
};

//////////////////////////////////////////////////////////////////////////////
// More Unity3D-ness 
//
// The following attribute layout:
// #0 position float3
// #1 normals  float3
// #2 color    float4
// #3 uv       float2
//////////////////////////////////////////////////////////////////////////////
class MeshFilter {
 public:
  enum BakeMode {kStatic, kDynamic};
  enum Slot : uint8_t { 
    kPositions  = 1 << 0, 
    kNormals    = 1 << 1, 
    kColors     = 1 << 2,
    kUv         = 1 << 3,
    kUv1        = 1 << 4,
    kUv2        = 1 << 5,
    kUv3        = 1 << 6,
    kIndices    = 1 << 7,
    kNone       = 0x00
  };

  ////////////////////////////////////////////////////////////////////////////
  // Once mesh is set, the filter is going to bake it and upload to vRAM.
  // By default position and indices slots are enabled, as it's the most
  // common case.
  ////////////////////////////////////////////////////////////////////////////
  MeshFilter() 
    : bake_mode_           (kStatic),
      enabled_slots_       (kPositions | kIndices | kNormals | kColors | kUv | kUv1 | kUv2 | kUv3) {
    vao_.reset(new VertexArrayObject(bake_mode_ == kStatic));
  }

  void SoftCopy(MeshFilter& from) {
    bake_mode_      =  from.bake_mode_;
    enabled_slots_  =  from.enabled_slots_;
    mesh_           =  from.mesh_;
    vao_            =  from.vao_;
  }

  void SetMode(BakeMode mode) {
    if (mode != bake_mode_) {
      bake_mode_ = mode;
      vao_.reset(new VertexArrayObject(bake_mode_ == kStatic));

      if (mesh_) {
        Bake();
      }
    }
  }

  ////////////////////////////////////////////////////////////////////////////
  // Slot operations. Can set more that one slot at once using bit | op.
  ////////////////////////////////////////////////////////////////////////////
  void SetSlot(uint8_t slots) {
    if (slots == kNone) {
      enabled_slots_ = kNone;
    } else {
      enabled_slots_ |= slots;

      if (mesh_) {
        Bake();
      }
    }
  }

  void ResetSlot(uint8_t slots) {
    enabled_slots_ &= ~slots;
  }

  bool GetSlot(Slot slot) {
    return enabled_slots_ & slot;
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
  // In order to manipulate per/instance attributes
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
  void UploadPerInstance(TArgs&&... args) {
    assert(vao_);
    vao_->Upload<TLayout>(std::forward<TArgs>(args)...);
  }
  
  ////////////////////////////////////////////////////////////////////////////
  // Some debug-output 
  ////////////////////////////////////////////////////////////////////////////
  void PrintSlotMap() {
    std::cerr << "kPositions | " << GetSlot(kPositions) << std::endl;
    std::cerr << "kNormals   | " << GetSlot(kNormals)   << std::endl;
    std::cerr << "kColors    | " << GetSlot(kColors)    << std::endl;
    std::cerr << "kUv        | " << GetSlot(kUv)        << std::endl;
    std::cerr << "kUv1       | " << GetSlot(kUv1)       << std::endl;
    std::cerr << "kUv2       | " << GetSlot(kUv2)       << std::endl;
    std::cerr << "kUv3       | " << GetSlot(kUv3)       << std::endl;
    std::cerr << "kIndices   | " << GetSlot(kIndices)   << std::endl;
    std::cerr << "kNone      | " << GetSlot(kNone)      << std::endl;
  }

 private:
  ////////////////////////////////////////////////////////////////////////////
  // Mesh bakery ....
  ////////////////////////////////////////////////////////////////////////////
  void Bake() {
    if (!mesh_) {
      throw std::logic_error("MeshFilter::Bake() the mesh has not been set, or deleted. Use MeshFilter::SetMesh()");
    }

    if (!mesh_->IsValid()) {
      throw std::logic_error("MeshFilter::Bake() the mesh is not valid. Please use a valid mesh.");
    }

    AdjustSlots();

    vao_->Bind();
    if (enabled_slots_ & kPositions) {
      vao_->Upload(0, VertexArrayObject::PackedData::Pack(mesh_->vertices));
    }

    if ((enabled_slots_ & kNormals) && !mesh_->normals.empty()) {
      vao_->Upload(1, VertexArrayObject::PackedData::Pack(mesh_->normals));
    }
    
    if ((enabled_slots_ & kColors) && !mesh_->colors.empty()) {
      vao_->Upload(2, VertexArrayObject::PackedData::Pack(mesh_->colors));
    }

    if ((enabled_slots_ & kUv) && !mesh_->uv.empty()) {
      vao_->Upload(3, VertexArrayObject::PackedData::Pack(mesh_->uv));
    }
    
    if ((enabled_slots_ & kIndices) && !mesh_->indices.empty()) {
      vao_->UploadIndices(VertexArrayObject::PackedData::Pack(mesh_->indices));
    }

    vao_->Unbind();
  }

  void AdjustSlots() {
    if (GetSlot(kNormals) && mesh_->normals.empty()) {
      ResetSlot(kNormals);
    } else if (!mesh_->normals.empty()) {
      enabled_slots_ |= kNormals;
    }
    if (GetSlot(kColors) && mesh_->colors.empty()) {
      ResetSlot(kColors);
    } else if (!mesh_->colors.empty()) {
      enabled_slots_ |= kColors;
    }
    if (GetSlot(kUv) && mesh_->uv.empty()) {
      ResetSlot(kUv);
    } else if (!mesh_->uv.empty()) {
      enabled_slots_ |= kUv;
    }
    if (GetSlot(kIndices) && mesh_->indices.empty()) {
      ResetSlot(kIndices);
    } else if (!mesh_->indices.empty()) {
      enabled_slots_ |= kIndices;
    }
  }

  ////////////////////////////////////////////////////////////////////////////
  // Baker and the ingridients 
  ////////////////////////////////////////////////////////////////////////////
  BakeMode                              bake_mode_;
  uint8_t                               enabled_slots_;
  std::shared_ptr<Mesh>                 mesh_;
  std::shared_ptr<VertexArrayObject>    vao_;
};

#endif // _MESHFILTER_H_B169C760_F9D6_42B9_81B7_91955A69A250_
