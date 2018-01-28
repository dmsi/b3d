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

#ifndef _SCENE_H_AE8DC1BD_C8C7_48C5_8A09_CB588DCD2575_
#define _SCENE_H_AE8DC1BD_C8C7_48C5_8A09_CB588DCD2575_ 

#include "renderqueue.h"
#include "actor.h"
#include "camera.h"
#include "light.h"
#include "material/material.h"
#include "material/pass.h"
#include "rendertarget.h"
#include <memory>
#include <map>
#include <string>

////////////////////////////////////////////////////////////////////////////
// C++ is cumbersome 
////////////////////////////////////////////////////////////////////////////
class Scene;
namespace Scene_CppSugar {
  template <class TEntity>
  std::shared_ptr<TEntity> Add(const std::string& name, Scene& to);

  template <class TEntity>
  std::shared_ptr<TEntity> Get(const std::string& name, Scene& to);
}

////////////////////////////////////////////////////////////////////////////
// The scene for our actors...
// TODO: Combine Update and Draw into a single methid for the client code
////////////////////////////////////////////////////////////////////////////
class Scene {
 public:
  ////////////////////////////////////////////////////////////////////////////
  // Adds/modifies the actor
  ////////////////////////////////////////////////////////////////////////////
  void SubmitActor(std::shared_ptr<Actor> actor);
  void SubmitLight(std::shared_ptr<Light> light);
  void SubmitCamera(std::shared_ptr<Camera> camera);

  void Update();
  void Draw();

  template <class TEntity>
  std::shared_ptr<TEntity> Add(const std::string& name) {
    return Scene_CppSugar::Add<TEntity>(name, *this);
  }

  template <class TEntity>
  std::shared_ptr<TEntity> Get(const std::string& name) {
    return Scene_CppSugar::Get<TEntity>(name, *this);
  }

  std::shared_ptr<RenderTarget> AddRenderTarget(const std::string& name, int order = 0) {
    auto it = render_targets_.lower_bound(order);

    if (it != render_targets_.end() && it->first == order) {
      // RenderTarget with the same order has been added before 
      throw std::logic_error("Scene::AddRenderTarget() - order " +
                             std::to_string(order) + 
                             " already added!");
    } 

    return render_targets_.emplace_hint(
        it, 
        order, 
        std::make_shared<RenderTarget>(name)
        )->second;
  }
  
  void SetSceneUniforms(Pass& pass, const Camera& camera);

 internal:
  std::map<std::string, std::shared_ptr<Camera>> cameras_;
  std::map<std::string, std::shared_ptr<Actor>>  actors_;
  std::map<std::string, std::shared_ptr<Light>>  lights_;
 private:
  std::map<int, std::shared_ptr<RenderTarget>>   render_targets_;
};

namespace Scene_CppSugar {
  template <>
  inline
  std::shared_ptr<Actor> Add(const std::string& name, Scene& to) {
    auto actor = std::shared_ptr<Actor>(new Actor(name));
    to.SubmitActor(actor);
    return actor;
  }
  
  template <>
  inline 
  std::shared_ptr<Camera> Add(const std::string& name, Scene& to) {
    auto camera = std::shared_ptr<Camera>(new Camera(name));
    to.SubmitCamera(camera);
    return camera;
  }

  template <>
  inline
  std::shared_ptr<Light> Add(const std::string& name, Scene& to) {
    auto light = std::shared_ptr<Light>(new Light(name, Light::kDirectional));
    to.SubmitLight(light);
    return light;
  }

  template <>
  inline
  std::shared_ptr<Actor> Get(const std::string& name, Scene& to) {
    std::shared_ptr<Actor> result;
    auto it = to.actors_.find(name);
    if (it != to.actors_.end()) {
      result = it->second;
    }
    return result;
  }
  
  template <>
  inline
  std::shared_ptr<Light> Get(const std::string& name, Scene& to) {
    std::shared_ptr<Light> result;
    auto it = to.lights_.find(name);
    if (it != to.lights_.end()) {
      result = it->second;
    }
    return result;
  }
  
  template <>
  inline
  std::shared_ptr<Camera> Get(const std::string& name, Scene& to) {
    std::shared_ptr<Camera> result;
    auto it = to.cameras_.find(name);
    if (it != to.cameras_.end()) {
      result = it->second;
    }
    return result;
  }

}

#endif // _SCENE_H_AE8DC1BD_C8C7_48C5_8A09_CB588DCD2575_
