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

// TODO: it is growing bigger... Need to redesign.
void Scene::Update() {
  for (auto& rt: render_targets_) {
    rt.second->StartNewFrame();
  }

  for (auto& kv: cameras_) {
    kv.second->Update();
  }

  for (auto& kv: lights_) {
    kv.second->Update();
  }

  // Regular actors
  for (auto& kv: actors_) {
    kv.second->Update();
    for (auto& rt: render_targets_) {
      rt.second->AddActor(kv.second);
    }
  }

  // Update and draw pools ...
  for (auto& kv: actor_pools_) {
    kv.second->Update();

    for (auto& a: *kv.second) {
      a->Update();
      for (auto& rt: render_targets_) {
        rt.second->AddActor(a);
      }
    }
  }

  // Update all batched actors
  for (auto& kv: std_batch_.actors_) {
    kv.second->Update();
  }

  // Update all batches
  for (auto& kv: std_batch_.batches_) {
    if (kv.second->BatchSize() == 0) 
      continue;

    kv.second->Update();
    kv.second->UploadInstances();

    for (auto& rt: render_targets_) {
      rt.second->AddActor(kv.second);
    }
  }
}

void Scene::Draw() {
  for (auto& rt: render_targets_) {
    rt.second->Draw(*this);
  }
}

void Scene::SetSceneUniforms(Pass& pass, const Camera& camera) {
  pass.SuTextures();

  for (auto& kv: lights_) {
    Light& light = *kv.second;
    if (light.GetType() == Light::kDirectional) {
      pass.SuDirLight(light.GetDirection(), light.GetColor());
    }
  }
}
