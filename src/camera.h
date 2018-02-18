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

///////////////////////////////////////////////////////////////////////////////
//
///////////////////////////////////////////////////////////////////////////////
class Frustum {
 public:
  /////////////////////////////////////////////////////////////////////////////
  // m - projection * view matrix
  // view = glm::inverse(camera->transform->GetMatrix())
  /////////////////////////////////////////////////////////////////////////////
  void Calculate(const glm::mat4& pv_matrix) {
    // http://gamedevs.org/uploads/fast-extraction-viewing-frustum-planes-from-world-view-projection-matrix.pdf
    // had to "transpose" the matrix
    //#define MTX(a, b) m[b-1][a-1]
    auto MTX = [&pv_matrix](int a, int b) {
      return pv_matrix[b-1][a-1];
    };

    // Left clipping plane
    planes_[kLeft].x = MTX(4, 1) + MTX(1, 1);
    planes_[kLeft].y = MTX(4, 2) + MTX(1, 2);
    planes_[kLeft].z = MTX(4, 3) + MTX(1, 3);
    planes_[kLeft].w = MTX(4, 4) + MTX(1, 4);
    // Right clipping plane
    planes_[kRight].x = MTX(4, 1) - MTX(1, 1);
    planes_[kRight].y = MTX(4, 2) - MTX(1, 2);
    planes_[kRight].z = MTX(4, 3) - MTX(1, 3);
    planes_[kRight].w = MTX(4, 4) - MTX(1, 4);
    // Top clipping plane
    planes_[kTop].x = MTX(4, 1) - MTX(2, 1);
    planes_[kTop].y = MTX(4, 2) - MTX(2, 2);
    planes_[kTop].z = MTX(4, 3) - MTX(2, 3);
    planes_[kTop].w = MTX(4, 4) - MTX(2, 4);
    // Bottom clipping plane
    planes_[kBottom].x = MTX(4, 1) + MTX(2, 1);
    planes_[kBottom].y = MTX(4, 2) + MTX(2, 2);
    planes_[kBottom].z = MTX(4, 3) + MTX(2, 3);
    planes_[kBottom].w = MTX(4, 4) + MTX(2, 4);
    // Near clipping plane
    planes_[kNear].x = MTX(4, 1) + MTX(3, 1);
    planes_[kNear].y = MTX(4, 2) + MTX(3, 2);
    planes_[kNear].z = MTX(4, 3) + MTX(3, 3);
    planes_[kNear].w = MTX(4, 4) + MTX(3, 4);
    // Far clipping plane
    planes_[kFar].x = MTX(4, 1) - MTX(3, 1);
    planes_[kFar].y = MTX(4, 2) - MTX(3, 2);
    planes_[kFar].z = MTX(4, 3) - MTX(3, 3);
    planes_[kFar].w = MTX(4, 4) - MTX(3, 4);

    for (int i = 0; i < 6; i++) {
      planes_[i] = glm::normalize(planes_[i]);
    }
  }

  /////////////////////////////////////////////////////////////////////////////
  // Returns true if point p is inside the frustum.
  /////////////////////////////////////////////////////////////////////////////
  bool TestPoint(const glm::vec3& p) const {
    for (int i = 0; i < 6; ++i) {
      glm::vec3 n(planes_[i]);
      // test half space
      if (glm::dot(n, p) + planes_[i].w < 0) {
        return false; // outside
      }
    }
    return true;
  }

  /////////////////////////////////////////////////////////////////////////////
  // Helper function that transforms NDC cube vertex to frustrum.
  // Can be used for CPU-based frustum visualisation.
  // inv_pv = glm::inverse(projection * view)
  /////////////////////////////////////////////////////////////////////////////
  glm::vec3 NdcToWorld(const glm::mat4& inv_pv, const glm::vec3& ndc) const {
    glm::vec4 world = inv_pv * glm::vec4(ndc, 1); 
    world = world / world.w;
    return (glm::vec3)world;
  }

  // Returns: false - outside, true - inside or intersects
  // https://www.gamedev.net/forums/topic/512123-fast--and-correct-frustum---aabb-intersection/
  // had to invert plane normals
  bool TestAabb(const glm::vec3 &mins, const glm::vec3 &maxs) const {
    glm::vec3 vmin, vmax;

    for(int i = 0; i < 6; ++i) {
      const glm::vec4& plane = planes_[i];
      glm::vec3 n(plane);
      // X axis
      if(n.x < 0) {
         vmin.x = mins.x;
         vmax.x = maxs.x;
      } else {
         vmin.x = maxs.x;
         vmax.x = mins.x;
      }
      // Y axis
      if(n.y < 0) {
         vmin.y = mins.y;
         vmax.y = maxs.y;
      } else {
         vmin.y = maxs.y;
         vmax.y = mins.y;
      }
      // Z axis
      if(n.z < 0) {
         vmin.z = mins.z;
         vmax.z = maxs.z;
      } else {
         vmin.z = maxs.z;
         vmax.z = mins.z;
      }
      if (glm::dot(n, vmin) + plane.w < 0) {
        return false; // outside
      }
    }
    return true; // inside, or intersetcs
  }

 private:
  enum {
    kNear = 0,
    kFar,
    kTop,
    kBottom,
    kLeft,
    kRight
  };

  glm::vec4 planes_[6];
};

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

  void Update() override {
    Actor::Update();
    glm::mat4 p, v;
    GetProjectionMatrix(p);
    GetViewMatrix(v);
    frustum_.Calculate(p*v);
  }

  const Frustum& GetFrustum() const {
    return frustum_;
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

  Frustum frustum_;
};

#endif // _CAMERA_H_7E6A64CD_EB55_4011_9E91_F26738B56FF9_
