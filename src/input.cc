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

#include <exception>

#include "input.h"
#include "appcontext.h"
#include "common/logging.h"

//////////////////////////////////////////////////////////////////////////////
// Configurable in the future
//////////////////////////////////////////////////////////////////////////////
static const std::string AXIS_HORIZONTAL = "Horizontal";
static const std::string AXIS_VERTICAL   = "Vertical";
static const std::string AXIS_MOUSE_X    = "Mouse X";
static const std::string AXIS_MOUSE_Y    = "Mouse Y";

void Input::Init() {
  //double xpos, ypos;
  //glfwGetCursorPos(GetWindow(), &xpos, &ypos);
  //mouse_position_.x = xpos;
  //mouse_position_.y = ypos;
  //old_mouse_position_ = mouse_position_;
}

float Input::GetAxis(const std::string& axis) const {
  glm::vec2 dmouse = GetMouseDelta();
  if (axis == AXIS_HORIZONTAL) {
    int key_left = GetKey(GLFW_KEY_A);
    int key_right = GetKey(GLFW_KEY_D);
    return -1.0f*key_left + 1.0f*key_right;
  } else if (axis == AXIS_VERTICAL) {
    int key_up = GetKey(GLFW_KEY_W);
    int key_down = GetKey(GLFW_KEY_S);
    return -1.0f*key_down + 1.0f*key_up;
  } else if (axis == AXIS_MOUSE_X) {
    return dmouse.x;
  } else if (axis == AXIS_MOUSE_Y) {
    return dmouse.y;
  }

  return 0;
}

bool Input::GetMouseButtonDown(int button) const {
  return glfwGetMouseButton(GetWindow(), button);
}

bool Input::GetKey(int key) const {
  return glfwGetKey(GetWindow(), key) == GLFW_PRESS;
}

bool Input::GetKeyDown(int key) const {
  ABORT_F("Not implemented");
  return false;
}

bool Input::GetKeyUp(int key) const {
  ABORT_F("Not implemented");
  return false;
}

void Input::Update() {
  double xpos, ypos;
  glfwGetCursorPos(GetWindow(), &xpos, &ypos);

  old_mouse_position_ = mouse_position_;
  mouse_position_.x = xpos;
  mouse_position_.y = ypos;
}

GLFWwindow* Input::GetWindow() const {
  return AppContext::Instance().display.GetWindow();
}

glm::vec2 Input::GetMouseDelta() const {
  static int test = 0; // workaround 
  float half_width = AppContext::Instance().display.GetWidth() / 2.0f;
  float half_height = AppContext::Instance().display.GetHeight() / 2.0f;
  glm::vec2 dmouse = old_mouse_position_ - mouse_position_;
  dmouse.x /= half_width;
  dmouse.y /= half_height;
  if (test < 60) {
    test++;
    return glm::vec2(0, 0);
  }
  return dmouse;
}
