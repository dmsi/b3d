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

#ifndef _LUAACTION_HPP_57834C34_FA34_4BEC_B8F6_FC28920A60D7_
#define _LUAACTION_HPP_57834C34_FA34_4BEC_B8F6_FC28920A60D7_ 

#include "action.h"
#include "luascript.h"
#include "glm_main.h"

#include <set>
#include <string>
#include <memory>

///////////////////////////////////////////////////////////////////////////////
// Provides interface to execute Action written in LUA
//
// Supported Action methods:
//  - Start()
//  - Update()
//  - PreDraw()
//  - ! PostDraw() not implemented
///////////////////////////////////////////////////////////////////////////////
class LuaAction : public Action {
 public:
  using List = std::set<std::string>;

  static constexpr const char* REGISTER_FN   = "RegisterModule";
  static constexpr const char* DEREGISTER_FN = "DeregisterModule";
  static constexpr const char* STARTONE_FN   = "StartModule";
  static constexpr const char* STARTALL_FN   = "Start";
  static constexpr const char* UPDATE_FN     = "Update";
  static constexpr const char* PREDRAW_FN    = "PreDraw";

  LuaAction(std::shared_ptr<Transformation> t, List ls) 
    : Action(t), scripts_(ls) {
    // Compile
    executor_.CompileCode(ReadFile("scripts/action_main.lua"));
    // Init the stack
    executor_.Execute();
    // Register modules - actions
    for (auto& scr : scripts_) {
      executor_.Call(REGISTER_FN, scr.c_str());
    }

    Bind();

    // Push globals
    auto* L = executor_.L;
    lua_getglobal(L, "globals");
    lua_pushstring(L, "_transform");
    luabridge::push(L, transform.get());
    lua_settable(L, -3);
    
    executor_.Call("Bind");

    // Debug - print modules info
    DiscoverModules();
  }

  void AddScript(const std::string& script) {
    if (!scripts_.emplace(script).second) {
      ABORT_F("Lua script %s has been already added", 
          script.c_str());
    }
    executor_.Call(REGISTER_FN, script.c_str());
    DiscoverModules();
    scripts_start_q_.emplace(script);
  }

  void RemoveScript(const std::string& script) {
    if (scripts_.erase(script) == 0) {
      ABORT_F("Lua script %s not found",
          script.c_str());
    }
    scripts_start_q_.erase(script);
    executor_.Call(DEREGISTER_FN, script.c_str());
    DiscoverModules();
  }

  void Start() override {
    executor_.Call(STARTALL_FN);
  }

  void Update() override {
    if (not scripts_start_q_.empty()) {
      for (auto& scr : scripts_start_q_) {
        executor_.Call(STARTONE_FN, scr.c_str()); 
      }
      scripts_start_q_.clear();
    }

    executor_.Call(UPDATE_FN);
  }

  void PreDraw() override {
    executor_.Call(PREDRAW_FN);
  }

 private:
  void DiscoverModules() {
    // For debug purposes
    executor_.Call("DiscoverModules");
  }

  void Bind();

  List      scripts_;
  List      scripts_start_q_;
  LuaScript executor_;
};

#endif // _LUAACTION_HPP_57834C34_FA34_4BEC_B8F6_FC28920A60D7_
