//
// This source file is a part of $PROJECT_NAME$
//
// Copyright (C) $COPYRIGHT$
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

#include <map>
#include "luaaction.h"

static void BindTransformation(lua_State* L, Transformation* t) {
  using glm::vec3;

  // This is c++! Simple and beautiful, 10 out of 10! 
  auto SetLocalPosition 
    = static_cast<void (Transformation::*)(const glm::vec3&)>(&Transformation::SetLocalPosition);
  auto SetLocalEulerAngles
    = static_cast<void (Transformation::*)(const glm::vec3&)>(&Transformation::SetLocalEulerAngles);
  auto SetLocalScale
    = static_cast<void (Transformation::*)(const glm::vec3&)>(&Transformation::SetLocalScale);

  luabridge::getGlobalNamespace(L)
  .beginNamespace("b3d")
    .beginClass <Transformation> ("Transformation")
      .addFunction("SetLocalPosition", SetLocalPosition) 
      .addFunction("GetLocalPosition", &Transformation::GetLocalPosition)
      .addFunction("SetLocalEulerAngles", SetLocalEulerAngles) 
      .addFunction("GetLocalEulerAngles", &Transformation::GetLocalEulerAngles)
      .addFunction("SetLocalScale", SetLocalScale) 
      .addFunction("GetLocalScale", &Transformation::GetLocalScale)
      .addFunction("GetGlobalPosition", &Transformation::GetGlobalPosition)
      .addFunction("GetGlobalEulerAngles", &Transformation::GetGlobalEulerAngles)
      .addFunction("GetGlobalScale", &Transformation::GetGlobalScale)
      .addFunction("Rotate", &Transformation::Rotate)
      .addFunction("Translate", &Transformation::Translate)
      .addFunction("Scale", &Transformation::Scale)
    .endClass()
  .endNamespace();
}

static void BindTimer(lua_State* L) {
  struct TimerGetSet {
    static float GetTimeDelta() {
      return AppContext::Instance().timer.GetTimeDelta();
    }

    static float GetTime() {
      return AppContext::Instance().timer.GetTime();
    }

    static void SetFloat(float) {
    }
  };

  luabridge::getGlobalNamespace(L)
  .beginNamespace("globals")
    .beginNamespace("Timer")
      .addProperty("dt", &TimerGetSet::GetTimeDelta, &TimerGetSet::SetFloat)
      .addProperty("time", &TimerGetSet::GetTime, &TimerGetSet::SetFloat)
      .addFunction("GetTimeDelta", &TimerGetSet::GetTimeDelta)
      .addFunction("GetTime", &TimerGetSet::GetTime)
    .endNamespace()
  .endNamespace();
}

static void BindVec3(lua_State* L) {
  luabridge::getGlobalNamespace(L)
  .beginNamespace("b3d")
    .beginClass <glm::vec3> ("vec3")
      .addConstructor<void(*)(float, float, float)>()
      .addData("x", &glm::vec3::x)
      .addData("y", &glm::vec3::y)
      .addData("z", &glm::vec3::z)
    .endClass()
  .endNamespace();
}

void LuaAction::Bind() {
  auto* L = executor_.L;
  BindTransformation(L, transform.get());
  BindTimer(L);
  BindVec3(L);
}
