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

#ifndef _TRANSFORMATION_H_27675F58_68A1_48A1_9EB9_645D29D21C8D_
#define _TRANSFORMATION_H_27675F58_68A1_48A1_9EB9_645D29D21C8D_ 

#include "glm_main.h"
#include "common/logging.h"
#include <memory>
#include <functional>
#include <stdexcept>

class Actor;

static glm::vec3 FORWARD = glm::vec3(0, 0, 1);
static glm::vec3 RIGHT   = glm::vec3(1, 0, 0);
static glm::vec3 UP      = glm::cross(FORWARD, RIGHT);

class TranslateRotateScale {
 public:
  TranslateRotateScale() : translate_(1.0f), rotate_(1.0f), scale_(1.0f) {
  }

  void Recalculate(const glm::vec3& position, const glm::vec3& euler, const glm::vec3& scale) {
    glm::mat4 identity(1.0f);
    translate_ = glm::translate(identity, position);
    //rotate_ = glm::eulerAngleXYZ(euler.x, euler.y, euler.z);  //-<<< lookat
    rotate_ = glm::eulerAngleYXZ(euler.y, euler.x, euler.z); //-<<< works ->  // yaw pitch roll
    scale_ = glm::scale(identity, scale);
  }

  void GetMatrix(glm::mat4& out_matrix) const {
    out_matrix = GetTranslationMatrix() * GetRotationMatrix() * GetScaleMatrix();
  }

  const glm::mat4& GetTranslationMatrix() const {
    return translate_;
  }

  const glm::mat4& GetRotationMatrix() const {
    return rotate_;
  }

  const glm::mat4& GetScaleMatrix() const {
    return scale_;
  }

 private:
  glm::mat4 translate_;
  glm::mat4 rotate_;
  glm::mat4 scale_;
};

//////////////////////////////////////////////////////////////////////////////
// Transformation class 
// Orientation:
//  X => pitch, Y = yaw, Z => roll 
//
// It seems there is a some inconsistency regarding euler/quat/mat4 
// conversions the results out of the glm do not match with
// https://www.andre-gaschler.com/rotationconverter
// At the moment things like Transform::GetForward(), etc as well as 
// quaternions seem to work fine only for FPS camera...
// https://stackoverflow.com/questions/12048046/quaternions-euler-angles-rotation-matrix-trouble-glm
//////////////////////////////////////////////////////////////////////////////
class Transformation {
 public:
  Transformation(Actor& actor) : 
    local_position_      (0.0f, 0.0f, 0.0f), 
    local_euler_angles_  (0.0f, 0.0f, 0.0f), 
    local_scale_         (1.0f, 1.0f, 1.0f), 
    dirty_               (true), 
    actor_               (actor) {
  }
  
  ////////////////////////////////////////////////////////////////////////////
  // Access to its actor 
  ////////////////////////////////////////////////////////////////////////////
  Actor& GetActor() { return actor_; }

  ////////////////////////////////////////////////////////////////////////////
  // Get/Set ... properties...
  ////////////////////////////////////////////////////////////////////////////
  void SetLocalPosition(const glm::vec3& position) {
    local_position_ = position;
    dirty_ = true;
  }

  const glm::vec3& GetLocalPosition() const {
    return local_position_;
  }

  void SetLocalEulerAngles(const glm::vec3& euler) {
    // TODO - normalize to range [-365 ... 0 ... +365]
    local_euler_angles_ = glm::radians(euler);
    dirty_ = true;
  }

  template <typename... T>
  void SetLocalPosition(T... args) {
    auto xyz = std::make_tuple(args...);
    SetLocalPosition(glm::vec3(std::get<0>(xyz), std::get<1>(xyz), std::get<2>(xyz)));
  }
  template <typename... T>
  void SetLocalEulerAngles(T... args) {
    auto xyz = std::make_tuple(args...);
    SetLocalEulerAngles(glm::vec3(std::get<0>(xyz), std::get<1>(xyz), std::get<2>(xyz)));
  }
  template <typename... T>
  void SetLocalScale(T... args) {
    auto xyz = std::make_tuple(args...);
    SetLocalScale(glm::vec3(std::get<0>(xyz), std::get<1>(xyz), std::get<2>(xyz)));
  }

  glm::vec3 GetLocalEulerAngles() const {
    return glm::degrees(local_euler_angles_);
  }

  void SetLocalScale(const glm::vec3& scale) {
    local_scale_ = scale;
    dirty_ = true;
  };

  const glm::vec3& GetLocalScale() const {
    return local_scale_;
  }

  glm::vec3 GetGlobalPosition() const {
    if (auto mum = parent_.lock()) {
      return mum->GetLocalPosition() + GetLocalPosition();
    } else {
      return GetLocalPosition();
    }
  }

  glm::vec3 GetGlobalEulerAngles() const {
    return glm::eulerAngles(GetGlobalQuaternion());
  }

  glm::vec3 GetGlobalScale() const {
    ABORT_F("Not implemented");
    return GetLocalScale();
  }

  glm::quat GetLocalQuaternion() const {
    // pitch, yaw, roll => YXZ
    return glm::quat(local_euler_angles_);
  }

  glm::quat GetGlobalQuaternion() const {
    if (auto mum = parent_.lock()) {
      return mum->GetGlobalQuaternion() * GetLocalQuaternion();
    } else {
      return GetLocalQuaternion();
    }
  }

  ////////////////////////////////////////////////////////////////////////////
  // Camera-like orientation
  ////////////////////////////////////////////////////////////////////////////
  void LookAt(const glm::vec3& target, const glm::vec3& up) {
    // WARNING: In case of looking into itself this will cause an invalid 
    // matrix!!!
    glm::mat4 m = glm::affineInverse(glm::lookAt(local_position_, target, up));
    m = glm::transpose(m);

    // almost working but someimes jumps for a frame or two ...
    float yaw, pitch, roll;
    if (std::abs(m[0][0] - 1.0f) < std::numeric_limits<float>::epsilon()) {
      yaw = atan2(m[0][2], m[2][3]);
      pitch = 0;
      roll = 0;
    } else if(std::abs(m[0][0] + 1.0f) < std::numeric_limits<float>::epsilon()) {
      yaw = atan2(m[0][2], m[2][3]);
      pitch = 0;
      roll = 0;
    } else {
      yaw = atan2(-m[2][0], m[0][0]);
      pitch = asin(m[1][0]);
      roll = atan2(-m[1][2], m[1][1]);
    }
    local_euler_angles_.y = (yaw);
    local_euler_angles_.z = (pitch);
    local_euler_angles_.x = (roll);

    //almost working but suddenly flips
    //auto q = quatLookAt(glm::normalize(target - local_position_), up); 
    //local_euler_angles_ = glm::vec3(pitch(q), yaw(q), roll(q));
    //auto d = glm::normalize(target - local_position_);

    dirty_ = true;
  }

  ////////////////////////////////////////////////////////////////////////////
  // Translate / Rotate / Scale
  ////////////////////////////////////////////////////////////////////////////

  void Translate(const glm::vec3& delta_position) {
    local_position_ += delta_position;
    dirty_ = true;
  }

  void Rotate(const glm::vec3& delta_euler) {
    local_euler_angles_ += glm::radians(delta_euler);
    dirty_ = true;
  }

  void Scale(const glm::vec3& delta_scale) {
    local_scale_ += delta_scale;
    dirty_ = true;
  }

  ////////////////////////////////////////////////////////////////////////////
  // Forward (Z), Right (X) and UP (Y) 
  // TODO: Not 100% sure if this is correct, need unit tests!!!
  ////////////////////////////////////////////////////////////////////////////
  glm::vec3 GetForward() const {
    return GetLocalQuaternion() * FORWARD;
  }

  glm::vec3 GetRight() const {
    return GetLocalQuaternion() * RIGHT;
  }

  glm::vec3 GetUp() const {
    return GetLocalQuaternion() * UP;
  }
  
  glm::vec3 GetGlobalForward() const {
    return GetGlobalQuaternion() * FORWARD;
  }

  glm::vec3 GetGlobalRight() const {
    return GetGlobalQuaternion() * RIGHT;
  }

  glm::vec3 GetGlobalUp() const {
    return GetGlobalQuaternion() * UP;
  }

  ////////////////////////////////////////////////////////////////////////////
  // Recalculate and get result matrix
  ////////////////////////////////////////////////////////////////////////////

  void GetMatrix(glm::mat4& out_matrix) const {
    Recalculate();
    trs_matrix_.GetMatrix(out_matrix);
    
    if (auto mum = parent_.lock()) {
      glm::mat4 mums_matrix(1.0f);
      mum->GetMatrix(mums_matrix); 
      out_matrix = mums_matrix * out_matrix;
    }
    // TODO how it behaves in case of camera?
    // Inverse the whole thing???
  }

  ////////////////////////////////////////////////////////////////////////////
  // Parent/child relations 
  // SetParent(mum1, kid);
  // SetParent(mum2, kid); -> mum1 clears kid from its childs 
  // SetParent(nullptr, kid); -> resets the parent (if any) for kid
  ////////////////////////////////////////////////////////////////////////////
  static void SetParent(std::shared_ptr<Transformation> mum,
                        std::shared_ptr<Transformation> kid) {
    assert(kid);
    std::hash<std::shared_ptr<Transformation>> hasher;
    if (auto old_mum = kid->parent_.lock()) {
      mum->childs_.erase(hasher(kid));
    }
    kid->parent_ = mum;
    if (mum) {
      mum->childs_[hasher(kid)] = kid;
    }
  }

 private:
  void Recalculate() const {
    if (dirty_) {
      trs_matrix_.Recalculate(local_position_, local_euler_angles_, local_scale_);
      dirty_ = false;
    }
  }

  glm::vec3                                local_position_;
  glm::vec3                                local_euler_angles_;
  glm::vec3                                local_scale_;

  mutable TranslateRotateScale             trs_matrix_;
  mutable bool                             dirty_;

  Actor&                                   actor_;
  std::weak_ptr<Transformation>            parent_;
  std::map<size_t, 
           std::weak_ptr<Transformation>>  childs_;
};

#endif // _TRANSFORMATION_H_27675F58_68A1_48A1_9EB9_645D29D21C8D_
