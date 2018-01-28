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

#ifndef _LOADMODEL_H_0F903EB2_EE1C_41D8_91D1_4D5AE6AD03D0_
#define _LOADMODEL_H_0F903EB2_EE1C_41D8_91D1_4D5AE6AD03D0_ 

#include <memory>
#include <string>
#include <map>
#include "glm_main.h"
#include "texture.h"
#include "material/material_loader.h"
#include "meshrenderer.h"
#include "scene.h"
#include "action.h"

//////////////////////////////////////////////////////////////////////////////
// This is a bit on the fragile side and assumes you're gonna play by the 
// rules...
// Syntax sugar right?
// scene.AddActor("SuperPuperActor")
//  ->AddAction<LoadModel>()
//   ->Setup(&scene, "model.dsm", "material.mat")
//   ->Position(0, 100, 0)
//   ->Scale(3, 1, 3)
//   ->Done()
//  ->AddAction<Rotator>()->rotation_speed = glm::vec3(0, 45, 0);
//
// Done() - when called it make attempt to load the model - Start() wont be 
//          using no matter if Done has loaded or not.
//        - when not called  - Start() will load the model.
// This action will be removed after Done() or first Start()
//////////////////////////////////////////////////////////////////////////////
class LoadModel : public Action {
 public:
  using TexturePtr = std::shared_ptr<::Texture>;
  using TextureMap = std::map<int, TexturePtr>;

  LoadModel(std::shared_ptr<Transformation> transform) : Action(transform) {}

  void Start() override {
    if (scene_ && !model_name_.empty() && !material_name_.empty()) {
      Create();
    }
    transform->GetActor().RemoveAction<LoadModel>();
  }

  LoadModel* Setup(Scene* scene, 
             const std::string& model, 
             const std::string& material,
             const glm::vec3& position = glm::vec3(0),
             const glm::vec3& rotation = glm::vec3(0),
             const glm::vec3& scale = glm::vec3(1)) {
    scene_    = scene;
    model_name_    = model;
    material_name_ = material;
    position_ = position;
    rotation_ = rotation;
    scale_    = scale;
    return this;
  }

  LoadModel* Position(const glm::vec3& position) {
    position_ = position;
    return this;
  }

  LoadModel* EulerAngles(const glm::vec3& rotation) {
    rotation_ = rotation;
    return this;
  }

  LoadModel* Scale(const glm::vec3& scale) {
    scale_ = scale;
    return this;
  }

  template <typename... TArgs>
  LoadModel* Position(TArgs... args) {
    auto xyz = std::make_tuple(std::forward<float>(args)...);
    return Position(glm::vec3(std::get<0>(xyz), std::get<1>(xyz), std::get<2>(xyz)));
  }

  template <typename... TArgs>
  LoadModel* EulerAngles(TArgs... args) {
    auto xyz = std::make_tuple(std::forward<float>(args)...);
    return EulerAngles(glm::vec3(std::get<0>(xyz), std::get<1>(xyz), std::get<2>(xyz)));
  }

  template <typename... TArgs>
  LoadModel* Scale(TArgs... args) {
    auto xyz = std::make_tuple(std::forward<float>(args)...);
    return Scale(glm::vec3(std::get<0>(xyz), std::get<1>(xyz), std::get<2>(xyz)));
  }

  LoadModel* Texture(int slot, TexturePtr texture) {
    this->texture_map_.emplace(slot, texture);
    return this;
  }

  Actor* Done() {
    Create();
    ready_ = true;
    transform->GetActor().RemoveAction<LoadModel>();
    return &transform->GetActor();
  }

 private:
  void Create() {
    if (!ready_) {
      if (scene_) {
        transform->GetActor().AddComponent<MeshRenderer>()
          ->SetMaterial(MaterialLoader::Load(material_name_));
        transform->GetActor().AddComponent<MeshFilter>()
          ->SetMesh(MeshLoader::Load(model_name_));

        transform->SetLocalPosition(position_);
        transform->SetLocalEulerAngles(rotation_);
        transform->SetLocalScale(scale_);

        for (auto& t : texture_map_) {
          transform->GetActor()
            . GetComponent<MeshRenderer>()
            ->GetMaterial()
            ->SetTexture(0, t.second);
        }
        ready_ = true;
      }
    }
  }
  
  Scene*       scene_     = nullptr;
  std::string  model_name_;
  std::string  material_name_;

  glm::vec3    position_  = glm::vec3(0);
  glm::vec3    rotation_  = glm::vec3(0);
  glm::vec3    scale_     = glm::vec3(1);

  TextureMap   texture_map_;

  bool         ready_     = false;
};

#endif // _LOADMODEL_H_0F903EB2_EE1C_41D8_91D1_4D5AE6AD03D0_
