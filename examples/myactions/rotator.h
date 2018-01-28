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

#ifndef _ROTATOR_H_E3AC50DC_B975_401B_8616_AFB80464CE95_
#define _ROTATOR_H_E3AC50DC_B975_401B_8616_AFB80464CE95_ 

#include "glm_main.h"
#include "action.h"

struct Rotator: public Action {
  glm::vec3 rotation_speed;

  Rotator(std::shared_ptr<Transformation> transform, 
          glm::vec3 rotation_speed = glm::vec3(0))
    : Action(transform) {
    this->rotation_speed = rotation_speed;
  }

  void Update() override {
    transform->Rotate(rotation_speed * GetTimer().GetTimeDelta());
  }
};


#endif // _ROTATOR_H_E3AC50DC_B975_401B_8616_AFB80464CE95_
