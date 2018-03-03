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

#ifndef _LUASCRIPT_H_4F289F82_6E00_4868_BEED_4246F3C8A611_
#define _LUASCRIPT_H_4F289F82_6E00_4868_BEED_4246F3C8A611_ 

#include "common/logging.h"
#include "common/util.h"

extern "C" {
#include <lua.h>
#include <lualib.h>
#include <lauxlib.h>
}
#include <LuaBridge.h>

#include <string>

struct LuaScript {
  LuaScript();
  virtual ~LuaScript();

  // Noncopyable
  LuaScript(const LuaScript&) = delete;
  LuaScript& operator=(LuaScript&) = delete;

  void CompileCode(const std::string& code);

  void Execute();

  // TODO replace it with template method + variadic params
  // use luabridge::push<> and iterate over tuple in order to
  // call the functions
  void Call(const char* fn, const char* arg);
  void Call(const char* fn);

  // Clear stack
  void Clear();

  lua_State* L = nullptr;
};

inline 
LuaScript::LuaScript() {
  L = luaL_newstate();
  if (!L) ABORT_F("Cant allocate Lua state");
  luaL_openlibs(L);
}

inline 
LuaScript::~LuaScript() {
  if (L) {
    lua_close(L);
    L = nullptr;
  }
}

inline
void LuaScript::CompileCode(const std::string& code) {
  // The error message is on top of the stack.
  if (luaL_loadstring(L, code.c_str())) {
    ABORT_F("Lua compilation error: %s", lua_tostring(L, -1));
  }
}

inline
void LuaScript::Clear() {
  int n = lua_gettop(L);
  lua_pop(L, n);
}

inline
void LuaScript::Execute() {
  // What would be the proper action in case of runtime error?
  if (lua_pcall(L, 0, 0, 0)) {
    ABORT_F("Lua runtime error: %s", lua_tostring(L, -1));
  }
}

inline
void LuaScript::Call(const char* fn, const char* arg) {
  //What would be faster?
  //luabridge::LuaRef f = luabridge::getGlobal(L, fn);
  //f(arg);
  assert(fn && arg);
  assert(L);
  lua_getglobal(L, fn);
  lua_pushstring(L, arg);
  if (lua_pcall(L, 1, 0, 0)) {
    ABORT_F("Lua runtime error: %s", lua_tostring(L, -1));
  }
}

inline
void LuaScript::Call(const char* fn) {
  assert(L);
  lua_getglobal(L, fn);
  if (lua_pcall(L, 0, 0, 0)) {
    ABORT_F("Lua runtime error: %s", lua_tostring(L, -1));
  }
}

#endif // _LUASCRIPT_H_4F289F82_6E00_4868_BEED_4246F3C8A611_
