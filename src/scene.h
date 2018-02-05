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
#include "batch_actor.h"
#include "camera.h"
#include "light.h"
#include "material/material.h"
#include "material/pass.h"
#include "rendertarget.h"
#include "util.h"
#include <memory>
#include <map>
#include <string>

////////////////////////////////////////////////////////////////////////////
// The scene for our actors...
//
// TODO: 
// Break frame building down logically to B-PS-S-PV-V-E steps, which stands 
// for: Begin, Pre-Simulation, Simulation, Pre-Visualisation, Visualisation, 
// End. 
// In multithreading mode B, PS, E can be use as sync points maybe.
// https://www.youtube.com/watch?v=8AjRD6mU96s @32:00
////////////////////////////////////////////////////////////////////////////
class Scene {
 public:
  void Update();
  void Draw();

  template <typename TComponent, typename... TArgs>
  auto Add(const std::string& name, TArgs&&... args);

  template <typename TComponent, typename... TArgs>
  auto Get(TArgs&&... args);

  // TODO submit render target
  /*
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
  }*/
  
  void SetSceneUniforms(Pass& pass, const Camera& camera);

 private:
  std::map<std::string, std::shared_ptr<Camera>> cameras_;
  std::map<std::string, std::shared_ptr<Actor>>  actors_;
  std::map<std::string, std::shared_ptr<Light>>  lights_;

  // TODO this is a dynamic batch which updates every frame
  // add a static batch which updates once, or from time to
  // time...
  StdBatch::Storage std_batch_;

  std::map<int, std::shared_ptr<RenderTarget>>   render_targets_;
};

#include "scene.inl"

#endif // _SCENE_H_AE8DC1BD_C8C7_48C5_8A09_CB588DCD2575_
