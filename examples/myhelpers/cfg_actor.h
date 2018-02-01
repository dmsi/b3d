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

#ifndef _CFG_ACTOR_H_27D7FA3C_99DC_4E63_BAA2_1B2599029AC5_
#define _CFG_ACTOR_H_27D7FA3C_99DC_4E63_BAA2_1B2599029AC5_ 

//
// Hell for maintanance, but whatever makes the interface 
// simplier I guess...
//

template <typename TActor, typename Ret>
class CfgActorBase {
 public:
  using TexturePtr = std::shared_ptr<::Texture>;
  using TextureMap = std::map<int, TexturePtr>;
  using MaterialPtr = std::shared_ptr<::Material>;
  using MeshPtr = std::shared_ptr<::Mesh>;
  using TagsMap = std::map<int, std::vector<std::string>>;

  template <typename... TArgs>
  CfgActorBase(Scene& scene, TArgs&&... args) {
    client_ = scene.Add<TActor>(std::forward<TArgs>(args)...);
  }

  explicit CfgActorBase(std::shared_ptr<TActor> client)
    : client_(client) {
    if (!client_) {
      throw std::invalid_argument("Cfg<TActor>::Cfg() - " \
          "TActor is nullptr!");
    }
  }

  Ret& CastSelf() {
    return *((Ret*)this);
  }

  Ret& Tags(int pass, std::vector<std::string> tags) {
    tags_map_.emplace(pass, std::move(tags));
    return CastSelf();
  }

  Ret& Material(std::string mat) {
    material_name_ = std::move(mat);
    return CastSelf(); 
  }

  Ret& Material(MaterialPtr mat) {
    material_ = mat;
    return CastSelf(); 
  }

  Ret& Mesh(std::string mesh) {
    mesh_name_ = std::move(mesh);
    return CastSelf(); 
  }

  Ret& Mesh(MeshPtr mesh) {
    mesh_ = mesh;
    return CastSelf(); 
  }

  Ret& Model(std::string mesh, std::string mat) {
    mesh_name_ = std::move(mesh);
    material_name_ = std::move(mat);
    return CastSelf(); 
  }

  Ret& Model(MeshPtr mesh, MaterialPtr mat) {
    mesh_ = mesh;
    material_ = mat;
    return CastSelf(); 
  }

  Ret& Position(const glm::vec3& pos) {
    position_ = pos;
    return CastSelf(); 
  }

  template <typename... TArgs>
  Ret& Position(TArgs... args) {
    auto xyz = std::make_tuple(std::forward<float>(args)...);
    return Position(glm::vec3(std::get<0>(xyz), std::get<1>(xyz), std::get<2>(xyz)));
  }

  Ret& EulerAngles(const glm::vec3& euler) {
    euler_ = euler;
    return CastSelf(); 
  }

  template <typename... TArgs>
  Ret& EulerAngles(TArgs... args) {
    auto xyz = std::make_tuple(std::forward<float>(args)...);
    return EulerAngles(glm::vec3(std::get<0>(xyz), std::get<1>(xyz), std::get<2>(xyz)));
  }
  
  Ret& Scale(const glm::vec3& scale) {
    scale_ = scale;
    return CastSelf(); 
  }

  template <typename... TArgs>
  Ret& Scale(TArgs... args) {
    auto xyz = std::make_tuple(std::forward<float>(args)...);
    return Scale(glm::vec3(std::get<0>(xyz), std::get<1>(xyz), std::get<2>(xyz)));
  }

  Ret& Texture(int slot, std::shared_ptr<Texture> tex) {
    texture_map_.emplace(slot, tex);
    return CastSelf(); 
  }

  template <typename T, typename... TArgs>
  Ret& Action(TArgs&&... args) {
    client_->template AddAction<T>(std::forward<TArgs>(args)...);
    return CastSelf(); 
  }

  template <typename T>
  Ret& Parent(std::shared_ptr<T> mum) {
    // All position, scale, rotation happens in local space
    Transformation::SetParent(mum->transform, client_->transform);
    return CastSelf();
  }

  auto GetClient() const {
    return client_;
  }

  auto Done() {
    assert(client_);
    // TODO once protection...

    if (!material_ && !material_name_.empty()) {
      material_ = MaterialLoader::Load(material_name_);
    }

    if (!mesh_ && !mesh_name_.empty()) {
      mesh_ = MeshLoader::Load(mesh_name_);
    }

    if (material_) {
      client_->template AddComponent<MeshRenderer>()->SetMaterial(material_);
    }

    if (mesh_) {
      client_->template AddComponent<MeshFilter>()->SetMesh(mesh_);
    }

    client_->transform->SetLocalPosition(position_);
    client_->transform->SetLocalEulerAngles(euler_);
    client_->transform->SetLocalScale(scale_);

    // It can be already set up by the action
    material_ = client_->template GetComponent<::Material>();

    if (material_) {
      for (auto& t : texture_map_) {
        client_ 
          ->template GetComponent<MeshRenderer>()
          ->GetMaterial()
          ->SetTexture(t.first, t.second);
      }
      
      size_t np = material_->GetPassNumber();
      for (auto& t : tags_map_) {
        if (t.first < np) {
          material_->GetPass(t.first)->SetTags(t.second);
        }
      }

    } else {
      if (!texture_map_.empty()) {
        // There can be a conflict if user relies on an action
        // to generate material on Start() which actually happens
        // inside the main loop, but Done is happening after.
        // I can introduce special internal action for this. 
        // which will try to apply material in Update() several
        // times and then removes itself.
        std::cerr << "WARNING!!! Cant assign textures for " 
                  << client_->GetName() << " "
                  << "Material component has not been added!" 
                  << std::endl;
      }
    }

    return client_;
  }

 protected:
  std::shared_ptr<TActor> client_;

  std::string material_name_;
  std::string mesh_name_;
  MaterialPtr material_;
  MeshPtr     mesh_;

  glm::vec3 position_ = glm::vec3(0, 0, 0);
  glm::vec3 scale_ = glm::vec3(1, 1, 1);
  glm::vec3 euler_ = glm::vec3(0, 0, 0);

  TextureMap texture_map_;
  TagsMap tags_map_; // materal pass tags
};

template <>
class Cfg<Actor> : public CfgActorBase<Actor, Cfg<Actor>> {
 public:
  using Self = Cfg<Actor>;
  using Base = CfgActorBase<Actor, Self>;

  template <typename... TArgs>
  Cfg(TArgs&&... args) : Base(std::forward<TArgs>(args)...) {}
};

template <>
class Cfg<Light> : public CfgActorBase<Light, Cfg<Light>> {
 public:
  using Self = Cfg<Light>;
  using Base = CfgActorBase<Light, Self>;

  template <typename... TArgs>
  Cfg(TArgs&&... args) : Base(std::forward<TArgs>(args)...) {}

  auto& Color(const ::Color color) {
    client_->SetColor(color);
    return *this;
  }

  template <typename... TArgs>
  auto& Color(TArgs... args) {
    auto rgba = std::make_tuple(std::forward<float>(args)...);
    return Color(::Color(std::get<0>(rgba), std::get<1>(rgba), 
                       std::get<2>(rgba), std::get<3>(rgba)));
  }

  auto& Attenuation(const glm::vec3& attenuation) {
    client_->SetAttenuation(attenuation);
    return *this;
  }

  template <typename... TArgs>
  auto& Attenuation(TArgs... args) {
    auto xyz = std::make_tuple(args...);
    return Attenuation(glm::vec3(std::get<0>(xyz), 
                                 std::get<1>(xyz),
                                 std::get<2>(xyz)));
  }
  
  auto Done() {
    client_->transform->SetLocalPosition(position_);
    client_->transform->SetLocalEulerAngles(euler_);
    client_->transform->SetLocalScale(scale_);
    return client_;
  }
};

template <>
class Cfg<Camera> : public CfgActorBase<Camera, Cfg<Camera>> {
 public:
  using Self = Cfg<Camera>;
  using Base = CfgActorBase<Camera, Self>;

  template <typename... TArgs>
  Cfg(TArgs&&... args) : Base(std::forward<TArgs>(args)...) {}

  auto& Perspective(float fov, float ratio, float near, float far) {
    client_->SetPerspective(fov, ratio, near, far);
    return *this;
  }

  auto& Ortho(float left, float top, float right, float bottom, float near, float far) {
    client_->SetOrtho(left, top, right, bottom, near, far);
    return *this;
  }

  auto Done() {
    client_->transform->SetLocalPosition(position_);
    client_->transform->SetLocalEulerAngles(euler_);
    client_->transform->SetLocalScale(scale_);
    return client_;
  }
};

#endif // _CFG_ACTOR_H_27D7FA3C_99DC_4E63_BAA2_1B2599029AC5_
