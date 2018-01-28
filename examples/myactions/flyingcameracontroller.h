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

#ifndef _ACTION_FLYINGCAMERA_H_25F5D31D_E068_435C_A85E_5558AB6C4B1E_
#define _ACTION_FLYINGCAMERA_H_25F5D31D_E068_435C_A85E_5558AB6C4B1E_ 

#include "action.h"

struct FlyingCameraController: public Action {
  float moving_speed;
  float mouse_sensivity;

  FlyingCameraController(std::shared_ptr<Transformation> transform,
                         float moving_speed = 100,
                         float mouse_sensivity = 50) : 
    Action(transform) {
    Setup(mouse_sensivity, moving_speed);
  }

  void Setup(float mouse_sensivity, float moving_speed) {
    this->mouse_sensivity = mouse_sensivity;
    this->moving_speed = moving_speed;
  }

  void Update() override {
    float h_axis  = GetInput().GetAxis("Horizontal");
    float v_axis  = GetInput().GetAxis("Vertical");
    float x_mouse = GetInput().GetAxis("Mouse X"); 
    float y_mouse = GetInput().GetAxis("Mouse Y");
    float dt      = GetTimer().GetTimeDelta();

    transform->Rotate(glm::vec3(y_mouse, x_mouse, 0) * mouse_sensivity);

    transform->Translate(moving_speed * transform->GetForward() * (-v_axis) * dt);
    transform->Translate(moving_speed * transform->GetRight()   * (h_axis)  * dt);
  }
};

#endif // _ACTION_FLYINGCAMERA_H_25F5D31D_E068_435C_A85E_5558AB6C4B1E_
