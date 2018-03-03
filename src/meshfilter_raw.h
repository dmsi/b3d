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

#ifndef _MESHFILTER_RAW_H_7AB064A6_7904_44FE_9B18_3D605B388369_
#define _MESHFILTER_RAW_H_7AB064A6_7904_44FE_9B18_3D605B388369_ 

#include "mesh.h"
#include "vertexarrayobject.h"

///////////////////////////////////////////////////////////////////////////////
// No mesh, bypass interface directly to VAO
///////////////////////////////////////////////////////////////////////////////
class BypassMeshFilter : public MeshFilterBase {
 public:
  using Usage = VertexArrayObject::Usage;
  
  MeshFilterView GetView() override {
    return MeshFilterView {
      n_vertices_, 
      n_indices_, 
      index_type_,
      [this](){vao_->Bind();},
      [this](){vao_->Unbind();}
    };
  }

  BypassMeshFilter() {
    index_type_ = GL_UNSIGNED_INT;
    n_vertices_ = 0;
    n_indices_ = 0;
    vao_ = std::make_shared<VertexArrayObject>();
  }

  void UploadIndices(const std::vector<uint16_t>& indices, Usage usage) {
    if (indices.empty()) return;

    index_type_ = GL_UNSIGNED_INT;
    n_indices_ = indices.size();
    vao_->UploadIndices(VertexArrayObject::PackedData::Pack(indices), usage);
  }

  void UploadIndices(const std::vector<uint32_t>& indices, Usage usage) {
    if (indices.empty()) return;
    
    n_indices_ = indices.size();

    auto max = *std::max_element(indices.begin(), indices.end());
    if (max < std::numeric_limits<uint16_t>::max()) {
      std::vector<uint16_t> indices16(indices.begin(), indices.end());
      index_type_ = GL_UNSIGNED_SHORT;
      UploadIndices(indices16, usage);
    } else {
      index_type_ = GL_UNSIGNED_INT;
      vao_->UploadIndices(VertexArrayObject::PackedData::Pack(indices), usage);
    }
  }

  template <typename TLayout, typename... TArgs>
  void Upload(TArgs&&... args) {
    assert(vao_);
    if (not TLayout::IsPerInstance()) {
      auto nv = std::get<1>(std::tuple<TArgs...>(args...));
      if (n_vertices_ != 0 && n_vertices_ != nv) {
        ABORT_F("Number of vertices does not match");
      }
      n_vertices_ = nv;
    }
    vao_->Upload<TLayout>(std::forward<TArgs>(args)...);
  }

 private:
  std::shared_ptr<VertexArrayObject> vao_;
  int                                index_type_;
  size_t                             n_vertices_;
  size_t                             n_indices_;
};

///////////////////////////////////////////////////////////////////////////////
// Layout-based mesh
///////////////////////////////////////////////////////////////////////////////
template <typename TLayout>
struct LayoutMesh {
  LayoutMesh(size_t n_verts) 
    : n_vertices(n_verts), 
      buffer_(TLayout::Stride() * n_verts) {
    assert(n_vertices > 0);
  }

  template <size_t N>
  void Set(std::initializer_list< typename TLayout::template TAttribute<N> > lst) {
    if (lst.size() != n_vertices)
      ABORT_F("Out of range");
    size_t idx = 0;
    BufferView view(&buffer_[0], buffer_.size());
    for (auto& v : lst) {
      TLayout::template Set<N>(view, idx, v);
      idx++;
    }
  }

  template <size_t N, typename T>
  void Set(size_t pos, const T& value) {
    if (pos >= n_vertices) ABORT_F("Out of range");
    BufferView view(&buffer_[0], buffer_.size());
    TLayout::template Set<N>(view, pos, value);
  }

  void* GetData() {
    return &buffer_[0];
  }

  size_t GetVertexCount() const {
    return n_vertices;
  }

  std::vector<uint32_t> indices;

 private:
  size_t n_vertices;
  std::vector<uint8_t> buffer_;
};

///////////////////////////////////////////////////////////////////////////////
// LayoutBased mesh filter
///////////////////////////////////////////////////////////////////////////////
template <typename TLayout>
class LayoutMeshFilter : public MeshFilterBase {
 public:
  using TMesh = LayoutMesh<TLayout>;

  MeshFilterView GetView() override {
    return bypass_.GetView();
  }

  void SetMesh(std::shared_ptr<TMesh> mesh, VertexArrayObject::Usage usage) {
    if (mesh && mesh != mesh_) {
      mesh_ = mesh;
      bypass_.Upload<TLayout>(
          0, mesh_->GetVertexCount(), mesh_->GetVertexCount(), 
          mesh_->GetData(), usage);
      bypass_.UploadIndices(mesh_->indices, usage);
    }
  }

  BypassMeshFilter& GetBypass() {
    return bypass_;
  }

 private:
  std::shared_ptr<TMesh> mesh_;
  BypassMeshFilter       bypass_;
};


#endif // _MESHFILTER_RAW_H_7AB064A6_7904_44FE_9B18_3D605B388369_
