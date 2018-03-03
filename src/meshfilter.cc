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
  
MeshFilter::MeshFilter() 
    : bake_mode_(kStatic),
      index_type_(GL_UNSIGNED_INT) {
  vao_.reset(new VertexArrayObject());
  attrib_slots_.flip();
}

void MeshFilter::Bake() {
  if (!mesh_) {
    ABORT_F("Mesh not set");
  }

  if (!mesh_->IsValid()) {
    ABORT_F("Mesh is not valid");
  }

  AdjustSlots();
  RecalculateIndexType();

  auto usage = GetUsage();

  vao_->Bind();
  if (attrib_slots_[MeshFilterBase::kPosition]) {
    vao_->Upload(0, VertexArrayObject::PackedData::Pack(mesh_->vertices), usage);
  }

  if (attrib_slots_[MeshFilterBase::kNormal] && !mesh_->normals.empty()) {
    vao_->Upload(1, VertexArrayObject::PackedData::Pack(mesh_->normals), usage);
  }
  
  if (attrib_slots_[MeshFilterBase::kColor] && !mesh_->colors.empty()) {
    vao_->Upload(2, VertexArrayObject::PackedData::Pack(mesh_->colors), usage);
  }

  if (attrib_slots_[MeshFilterBase::kUv] && !mesh_->uv.empty()) {
    vao_->Upload(3, VertexArrayObject::PackedData::Pack(mesh_->uv), usage);
  }
  
  if (attrib_slots_[MeshFilterBase::kIndices] && !mesh_->indices.empty()) {
    if (index_type_ == GL_UNSIGNED_SHORT) {
      std::vector<uint16_t> indices(mesh_->indices.begin(), mesh_->indices.end());
      vao_->UploadIndices(VertexArrayObject::PackedData::Pack(indices), usage);
    } else {
      vao_->UploadIndices(VertexArrayObject::PackedData::Pack(mesh_->indices), usage);
    }
  }

  // TODO: tangent and bitangent
  // Extra needs to be uploaded manually as it does not belong to the mesh

  vao_->Unbind();
}

void MeshFilter::AdjustSlots() {
  if (attrib_slots_[MeshFilterBase::kNormal] && mesh_->normals.empty()) {
    attrib_slots_[MeshFilterBase::kNormal] = 0;
  } else if (!mesh_->normals.empty()) {
    attrib_slots_[MeshFilterBase::kNormal] = 1;
  }
  if (attrib_slots_[MeshFilterBase::kColor] && mesh_->colors.empty()) {
    attrib_slots_[MeshFilterBase::kColor] = 0;
  } else if (!mesh_->colors.empty()) {
    attrib_slots_[MeshFilterBase::kColor] = 1;
  }
  if (attrib_slots_[MeshFilterBase::kUv] && mesh_->uv.empty()) {
    attrib_slots_[MeshFilterBase::kUv] = 0;
  } else if (!mesh_->uv.empty()) {
    attrib_slots_[MeshFilterBase::kUv] = 1;
  }
  if (attrib_slots_[MeshFilterBase::kIndices] && mesh_->indices.empty()) {
    attrib_slots_[MeshFilterBase::kIndices] = 0;
  } else if (!mesh_->indices.empty()) {
    attrib_slots_[MeshFilterBase::kIndices] = 1;
  }
}

void MeshFilter::RecalculateIndexType() {
  if (!mesh_ || mesh_->indices.empty()) return;

  auto max = std::max_element(mesh_->indices.begin(), mesh_->indices.end());
  if (max == mesh_->indices.end()) return;

  if (*max < std::numeric_limits<uint16_t>::max()) {
    index_type_ = GL_UNSIGNED_SHORT;
  } else {
    index_type_ = GL_UNSIGNED_INT;
  }
}
