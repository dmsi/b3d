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

#include "scene.h"
#include <exception>


void Scene::SubmitActor(std::shared_ptr<Actor> actor) {
  if (!actor) {
    throw std::logic_error("Scene::SubmitActor() - dont even try to submit \
                            nullptr instead of an actor!");
  }
  actors_[actor->GetName()] = actor;
}

void Scene::SubmitLight(std::shared_ptr<Light> light) {
  if (!light) {
    throw std::logic_error("Scene::SubmitLight() - dont even try to submit \
                            nullptr instead of a light!");
  }
  lights_[light->GetName()] = light;
}

void Scene::SubmitCamera(std::shared_ptr<Camera> camera) {
  if (!camera) {
    throw std::logic_error("Scene::SubmitCamera() - dont even try to submit \
                            nullptr instead of a camera!");
  }
  cameras_[camera->GetName()] = camera;
}

void Scene::Update() {
  //render_queue_.Clear();
  for (auto& rt: render_targets_) {
    rt.second->StartNewFrame();
  }

  //if (camera_) {
  //  camera_->Update();
  //}
  
  for (auto& kv: cameras_) {
    kv.second->Update();
  }

  for (auto& kv: lights_) {
    kv.second->Update();
  }

  for (auto& kv: actors_) {
    kv.second->Update();
    for (auto& rt: render_targets_) {
      rt.second->AddActor(kv.second);
    }
  }
}

void Scene::Draw() {
  //if (!camera_) {
  //  throw std::logic_error("Scene::Draw() - camera not set!");
  //}

  for (auto& rt: render_targets_) {
    rt.second->Draw(*this);
  }
  //render_queue_.Draw(*this, *camera_);
}

// TODO all that string manipulations are very expensive, 
// it's better to pre-build string names or use direct uniform locations
// as ints...
void Scene::SetSceneUniforms(Pass& pass, const Camera& camera) {
  int n_point = 0;
  int n_directional = 0;
  int n_spot = 0;

  std::string prefix;
  auto set_uniform = [&prefix, &pass] (auto name, auto num, auto value) {
    pass.SetUniform(prefix + name + "_" + std::to_string(num), value);
  };

  // Bind textures slots to shader uniforms as TEXTURE_0 = 0, TEXTURE_1 = 1, etc
  for (int i = 0; i < Material::kMaxTextureSlots; ++i) {
    prefix = "";
    set_uniform("TEXTURE", i, i);
  }

  // Set lights
  for (auto& kv: lights_) {
    Light& light = *kv.second;
    switch (light.GetType()) {
      case Light::kDirectional: {
        // Direction in camera space?
        // http://www.lighthouse3d.com/tutorials/glsl-tutorial/directional-lights-per-pixel/
        prefix = "SU_DIRECTIONAL_LIGHT_";
        set_uniform("COLOR",     n_directional, light.GetColor());
        set_uniform("DIRECTION", n_directional, light.GetDirection());
        set_uniform("INTENSITY", n_directional, light.GetIntensity());
        set_uniform("POSITION",  n_directional, light.GetPosition());
        n_directional++;
      } break;

      case Light::kSpot: {
        prefix = "SU_SPOT_LIGHT";
        set_uniform("COLOR",     n_spot, light.GetColor());
        set_uniform("DIRECTION", n_spot, light.GetDirection());
        set_uniform("POSITION",  n_spot, light.GetPosition());
        set_uniform("INTENSITY", n_spot, light.GetIntensity());
        set_uniform("RANGE",     n_spot, light.GetRange());
        set_uniform("ANGLE",     n_spot, light.GetSpotAngle());
        n_spot++;
      } break;

      case Light::kPoint: {
        prefix = "SU_POINT_LIGHT_";
        set_uniform("COLOR",     n_point, light.GetColor());
        set_uniform("POSITION",  n_point, light.GetPosition());
        set_uniform("INTENSITY", n_point, light.GetIntensity());
        set_uniform("RANGE",     n_point, light.GetRange());
        n_point++;
      } break;
    }
  }
}
