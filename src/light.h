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

#ifndef _LIGHT_H_BC7A3B80_18DF_4A94_A39E_E78F83CE810A_
#define _LIGHT_H_BC7A3B80_18DF_4A94_A39E_E78F83CE810A_ 

#include "glm_main.h"
#include "actor.h"
#include "common/debug.h"

//////////////////////////////////////////////////////////////////////////////
// Light source, sir!
// In case of directional light the default untransformed direction is z-axis
// i.e. (0,0,1)
// TODO does it make sense to set the default direction to (0, 0, -1) in order
// to align it with the default camera position?
//////////////////////////////////////////////////////////////////////////////
class Light: public Actor {
 public:
  enum Type {kSpot, kDirectional, kPoint};

  Light(const std::string& name, Type type) 
    : Actor(name), 
      type_(type), 
      color_(1,1,1,1), 
      intencity_(1), 
      range_(1),
      spot_angle_(10) {}

  void SetColor(const glm::vec4& color) {
    color_ = color;
  }

  Type GetType() const {return type_;}
  const glm::vec4& GetColor() const {return color_;}

  // Not the most optimal way, but will do for now
  glm::vec3 GetDirection() const {
    // Based on the quaternion and does not work out of the box.
    // https://stackoverflow.com/questions/12048046/quaternions-euler-angles-rotation-matrix-trouble-glm
    //return transform->GetForward();
    // There is discrepancy in glm conversions between angles - matrices and quat.
    // so dont use quat until the issue fixed...
    glm::mat4 m(1.0f);
    transform->GetMatrix(m); // scale???
    m[3][0] = 0; m[3][1] = 0; m[3][2] = 0;
    return glm::normalize(glm::vec3(m * glm::vec4(0, 0, 1, 1)));
    //auto e = glm::radians(transform->GetLocalEulerAngles()); // local or global??? local for now
    //auto r = glm::eulerAngleYXZ(e.y, e.x, e.z);
    //return glm::normalize(glm::vec3(r * glm::vec4(0, 0, 1, 1)));
  }

  glm::vec3 GetPosition() const {
    return transform->GetGlobalPosition();
  }

  float GetRange() const {return range_;}
  float GetIntensity() const {return intencity_;}
  float GetSpotAngle() const {return spot_angle_;}

 private:
  Type                   type_;
  glm::vec4              color_;
  float                  intencity_;
  float                  range_;
  float                  spot_angle_;
};


#endif // _LIGHT_H_BC7A3B80_18DF_4A94_A39E_E78F83CE810A_
