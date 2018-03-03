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

class Material;

template <class TComponent, typename... TArgs>
inline auto Actor::AddComponent(TArgs&&... args) {
  // C++17 constexpr dispatching
  if constexpr (std::is_same<TComponent, MeshRenderer>::value) {
    // 
    // MeshRenderer
    //
    mesh_renderer_ = std::make_shared<MeshRenderer>(
        std::forward<TArgs>(args)...);

    return mesh_renderer_;
  } else 
  if constexpr (std::is_base_of<MeshFilterBase, TComponent>::value) {
    //
    // MeshFilter
    //
    auto ptr = std::make_shared<TComponent>(
        std::forward<TArgs>(args)...);

    mesh_filter_ = ptr;

    return ptr;
  } else {
    //
    // None of above 
    // 
    static_assert(cppness::dependent_false<TComponent>::value, 
        "TComponent not recognized");
  }
}

////////////////////////////////////////////////////////////////////////////
// Returns std::shared_ptr<TComponent>
////////////////////////////////////////////////////////////////////////////
template <class TComponent>
inline auto Actor::GetComponent() {
  if constexpr (std::is_same<TComponent, MeshRenderer>::value) {
    //
    // MeshRenderer
    //
    return mesh_renderer_;
  } else
  if constexpr (std::is_base_of<MeshFilterBase, TComponent>::value) {
    //
    // MeshFilter
    //
    return std::dynamic_pointer_cast<TComponent>(mesh_filter_);
  } else
  if constexpr (std::is_same<TComponent, Material>::value) {
    //
    // Material (neat accessor)
    //
    if (mesh_renderer_) return mesh_renderer_->GetMaterial();
    else                return std::shared_ptr<Material>();

  } else
  if constexpr (std::is_same<TComponent, Mesh>::value) {
    //
    // Mesh (neat accessor)
    //
    auto mf = std::dynamic_pointer_cast<MeshFilter>(mesh_filter_); 
    if (mf) return mf->GetMesh();
    else    return std::shared_ptr<Mesh>();

  } else {
    //
    // None of above
    //
    static_assert(cppness::dependent_false<TComponent>::value,
        "TComponent not recognized");
  }
}

////////////////////////////////////////////////////////////////////////////
// Replaces component
////////////////////////////////////////////////////////////////////////////
template <class TComponent>
inline void Actor::SetComponent(std::shared_ptr<TComponent> component) {
  // C++17 constexpr dispatching
  if constexpr (std::is_same<TComponent, MeshRenderer>::value) {
    // 
    // MeshRenderer
    //
    mesh_renderer_ = component; 
  } else 
  if constexpr (std::is_base_of<MeshFilterBase, TComponent>::value) {
    //
    // MeshFilter
    //
    mesh_filter_ = component;
  } else {
    //
    // None of above 
    // 
    static_assert(cppness::dependent_false<TComponent>::value, 
        "TComponent not recognized");
  }
}

