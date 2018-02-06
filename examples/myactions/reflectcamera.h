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

#ifndef _REFLECTCAMERA_H_DA7838DE_1402_4992_A2BA_CD12C3034549_
#define _REFLECTCAMERA_H_DA7838DE_1402_4992_A2BA_CD12C3034549_ 

#include <memory>
#include "glm_main.h"
#include "action.h"

//////////////////////////////////////////////////////////////////////////////
// Takes the camera pointer and feflects it across horizontal plane with 
// custom elevation (water_level). When attached to a camera it modifies its 
// transform.
//
// Useful to make water reflections.
//
// Usage example:
// auto main_camera = scene.Add<Camera>("camera.main");
// ...
// auto reflected_camera = scene.Add<Camera>("camera.reflected");
// reflected_camera->AddAction<ReflectCamera>()->camera = main_camera;
//////////////////////////////////////////////////////////////////////////////
struct ReflectCamera: public Action {
  std::shared_ptr<Camera> camera;
  float                   water_level;

  ReflectCamera(std::shared_ptr<Transformation> t, 
                std::shared_ptr<Camera> c, float wl) 
    : Action(t), camera(c), water_level(wl) {}

  void Start() override {
    Camera* c = dynamic_cast<Camera*>(&transform->GetActor());
    if (c) {
      // If we move it to update it will allow us to automatically
      // capture projection of the tracking camera.
      //
      // On the other hand it might be useful to set projection 
      // of the reflected camera separately.. Right?
      c->CopyProjection(camera);
    }
  }

  void Update() override {
    auto pos = camera->transform->GetLocalPosition();
    auto rot = camera->transform->GetLocalEulerAngles();
    float d = 2 * (pos.y - water_level);
    pos.y -= d;
    rot.z = -rot.z;
    rot.x = -rot.x;
    transform->SetLocalPosition(pos);
    transform->SetLocalEulerAngles(rot);
  }
};

#endif // _REFLECTCAMERA_H_DA7838DE_1402_4992_A2BA_CD12C3034549_
