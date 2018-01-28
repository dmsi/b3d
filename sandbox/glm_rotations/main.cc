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

#include "glm_main.h"
#include <iostream>
#include "common/debug.h"

// GLM consistency adapter
namespace cnv {
  inline glm::quat quat(const glm::vec3& yxz) {
    auto q = glm::quat(glm::vec3(yxz.y, yxz.x, yxz.z));
    std::swap(q.x, q.y);
    return q;
  }

  inline glm::mat4 toMat4(const glm::quat& q) {
    return glm::toMat4(q);
  }

  inline glm::quat lookAt(const glm::vec3& dir, const glm::vec3& up) {
    return glm::quat(0, 0, 0, 1);
  }

  inline glm::vec3 eulerAngles(const glm::quat& q) {
    glm::quat qq = glm::quat(q.w, q.y, q.x, q.z);
    glm::vec3 yxz = glm::eulerAngles(qq);
    std::swap(yxz.x, yxz.y);
    return glm::degrees(yxz);
  }
};

inline
void PrintVec3(char n, const glm::vec3& v) {
  using namespace Debug;
  std::cerr << n << " = " << Getf(v.x) << " " << Getf(v.y) << " " << Getf(v.z) << std::endl;
}

inline
void PrintQuat(char n, const glm::quat& q) {
  using namespace Debug;
  std::cerr << n << " = " << Getf(q.x) << " " << Getf(q.y) << " " << Getf(q.z) << " " << Getf(q.w) << std::endl;
}

// Makes quaternion and matrix which matches the reference for YXZ
// https://www.andre-gaschler.com/rotationconverter/
// x, y, z in degrees
void AsReferenceYXZ(float x, float y, float z) {
  std::cerr << "Euler Angles YXZ order" << std::endl;
  std::cerr << "e = " << x << " " << y << " " << z << std::endl;
  auto q = glm::quat(glm::radians(glm::vec3(y, x, z)));
  std::swap(q.x, q.y);
  std::cerr << "q = " << q.x << " " << q.y << " " << q.z << " " << q.w << std::endl;
  Debug::PrintMat4('M', glm::toMat4(q));

  std::swap(q.x, q.y);
  auto e1 = glm::degrees(eulerAngles(q));
  std::swap(e1.x, e1.y);
  std::cerr << "e1= " << e1.x << " " << e1.y << " " << e1.z << std::endl;
}

struct Tr {
  glm::quat rot;
  glm::vec3 pos;
  glm::vec3 scl;

  glm::vec3 GetLocalEulerAngles() const {
    return glm::degrees(glm::eulerAngles(rot));
  }
  void SetLocalEulerAngles(const glm::vec3& e) {
    rot = cnv::quat(glm::radians(e));
  }

  void Rotate(const glm::vec3& delta) {
    rot = cnv::quat(glm::radians(delta) + cnv::eulerAngles(rot)); 
  }
};

int main() {
  using namespace glm;

  vec3 F(0, 0, 1), 
       R(1, 0, 0),
       U(0, 1, 0);

  vec3 e(10, 20, 30);
  //AsReferenceYXZ(e.x, e.y, e.z);

  vec3 de(20, 0, 0);
  auto q = cnv::quat(glm::radians(e));
  auto dq = cnv::quat(glm::radians(de));
  //glm::quat qq = q * dq;
  //auto ee = cnv::eulerAngles(qq);
  //PrintVec3('>', ee);
  auto qqq = cnv::quat(glm::radians(e+de));
  auto ee = cnv::eulerAngles(qqq);
  PrintVec3('>', ee);
  //auto m1 = cnv::toMat4(q);
  //auto m2 = cnv::toMat4(dq);
  //auto eee = m1 * 
  glm::quat q1;
  glm::vec3 dv;
  PrintQuat('q', q1);
  PrintVec3('v', dv);

  /// THE PROBLEM!!!
  // https://stackoverflow.com/questions/12048046/quaternions-euler-angles-rotation-matrix-trouble-glm
  std::cerr << "----------" << std::endl;
  {
    glm::quat q(.5, -.5, .5, .5);
    glm::vec3 euler = glm::eulerAngles(q);// * 3.14159f / 180.f; // eulerAngleYXZ takes radians but eulerAngles returns degrees
    glm::mat4 transform1 = glm::eulerAngleYXZ(euler.y, euler.x, euler.z);
    // transform1 rotates a +Z arrow so that it points at +X
    Debug::PrintMat4('1', transform1);
  }
  std::cerr << std::endl;
  {

    glm::quat q(.5, -.5, .5, .5);
    glm::mat4 transform2 = glm::toMat4(q);
    // transform2 rotates a +Z arrow so that it points at +Y
    Debug::PrintMat4('2', transform2);
  }

  //auto r = glm::eulerAngleYXZ(e.x, e.y, e.z);
  //Debug::PrintMat4('R', r);

  //for (float i = 0; i < 90; i+=1) {
  //  std::cerr << i << std::endl;
  //  e.z = i;
  //  auto q = cnv::quat(glm::radians(e));
  //  auto ff = q * F;
  //  PrintVec3('f', ff);

  //  vec4 fff = cnv::toMat4(q) * vec4(F,1);
  //  PrintVec3('f', fff);
  //}


  //PrintQuat('q', q);
  //Debug::PrintMat4('M', cnv::toMat4(q));
  //PrintVec3('e', cnv::eulerAngles(q));
  //AsReferenceYXZ(e.x, e.y, e.z);

  return 0;
}
