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

// GetComponent
namespace Actor_cppness {
  template <class TComponent>
  inline 
  auto GetComponent(Actor& actor) {
    std::cerr << "GetComponent GENERIC!" << std::endl;
    return std::shared_ptr<TComponent>();
  }
  
  template <>
  inline
  auto GetComponent<MeshRenderer>(Actor& actor) {
    return std::shared_ptr<MeshRenderer>(actor.mesh_renderer_);
  }

  template <>
  inline
  auto GetComponent<MeshFilter>(Actor& actor) {
    return std::shared_ptr<MeshFilter>(actor.mesh_filter_);
  }

  template <>
  inline
  auto GetComponent<Material>(Actor& actor) {
    if (auto mr = GetComponent<MeshRenderer>(actor)) {
      return mr->GetMaterial();
    } else {
      return std::shared_ptr<Material>();
    }
  }
}

// AddComponent 
namespace Actor_cppness { 
  template <class TComponent> 
  inline
  auto AddComponent(Actor& actor) {
    std::cerr << "AddComponent GENERIC!" << std::endl;
  }

  template <>
  inline
  auto AddComponent<MeshRenderer>(Actor& actor) {
    std::cerr << "[actor " << actor.GetName() << "] Mesh Renderer added!" << std::endl;
    actor.mesh_renderer_ = std::shared_ptr<MeshRenderer>(new MeshRenderer());
    return actor.mesh_renderer_; 
  }
  
  template <>
  inline
  auto AddComponent<MeshFilter>(Actor& actor) {
    std::cerr << "[actor " << actor.GetName() << "] Mesh Filter added!" << std::endl;
    actor.mesh_filter_ = std::shared_ptr<MeshFilter>(new MeshFilter());
    return actor.mesh_filter_;
  }
}

