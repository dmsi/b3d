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

#ifndef _CAMERA_H_7E6A64CD_EB55_4011_9E91_F26738B56FF9_
#define _CAMERA_H_7E6A64CD_EB55_4011_9E91_F26738B56FF9_ 

#include "actor.h"
#include "glm_main.h"

class Camera : public Actor {
 public:
  enum Type {kPerspective, kOrtho};
  Camera(const std::string& name) : 
    Actor        (name),
    type_        (kPerspective),
    fov_         (0),
    ratio_       (0),
    near_        (0),
    far_         (0),
    left_        (0),
    right_       (0),
    top_         (0),
    bottom_      (0) {}

  void SetPerspective(float fov, float ratio, float near, float far) {
    type_ = kPerspective;
    fov_ = glm::radians(fov);
    ratio_ = ratio;
    near_ = near;
    far_ = far;
  }

  void SetOrtho(float left, float top, float right, float bottom, float near, float far) {
    type_ = kOrtho;
    top_ = top;
    left_ = left;
    right_ = right;
    bottom_ = bottom;
    near_ = near;
    far_ = far;
  }

  // As far as this inline method... maybe just return the value???
  void GetProjectionMatrix(glm::mat4& proj) const {
    if (type_ == kOrtho) {
      proj = glm::ortho(left_, right_, bottom_, top_, near_, far_);
    } else {
      proj = glm::perspective(fov_, ratio_, near_, far_);
    }
  }

  void GetViewMatrix(glm::mat4& view) const {
    transform->GetMatrix(view);
    view = glm::affineInverse(view);
  }

  void CopyProjection(std::shared_ptr<Camera> from) {
    type_   = from->type_;
    fov_    = from->fov_;
    ratio_  = from->ratio_;
    near_   = from->near_;
    far_    = from->far_;
    left_   = from->left_;
    right_  = from->right_;
    top_    = from->top_;
    bottom_ = from->bottom_;
  }

  float GetNear() const {
    return near_;
  }

  float GetFar() const {
    return  far_;
  }

 private:
  Type  type_;

  //////////////////////////////////////////////////////////////////////////////
  // Perspective
  //////////////////////////////////////////////////////////////////////////////
  float fov_;
  float ratio_;
  float near_;
  float far_;

  //////////////////////////////////////////////////////////////////////////////
  // Ortho 
  //////////////////////////////////////////////////////////////////////////////
  float left_;
  float right_;
  float top_;
  float bottom_;
};

#endif // _CAMERA_H_7E6A64CD_EB55_4011_9E91_F26738B56FF9_
