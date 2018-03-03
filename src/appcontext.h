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

#ifndef _APPCONTEXT_H_4B227517_B140_479C_BD2E_01283044D3DA_
#define _APPCONTEXT_H_4B227517_B140_479C_BD2E_01283044D3DA_ 

#include "gl_main.h"
#include "display.h"
#include "input.h"
#include "timer.h"
#include "common/logging.h"
#include <string>
#include <exception>


//////////////////////////////////////////////////////////////////////////////
// Bring some cumbersomness for easy access 
//////////////////////////////////////////////////////////////////////////////
class AppContext {
 public:
  Display display;
  Input   input;
  Timer   timer;

  static AppContext& Instance() {
    if (!instance) {
      ABORT_F("AppContext not initialized");
    }
    return *instance;
  }

  static void Init(int width, int height, std::string caption, const Profile& profile) {
    if (instance) {
      ABORT_F("AppContext already initialized");
    }
    instance = new AppContext(width, height, std::move(caption), profile);
    instance->input.Init();
  }

  static void Close() {
    delete instance;
    instance = nullptr;
  }

  static bool Running() {
    auto& app = AppContext::Instance();
    auto window = app.display.GetWindow();
    return !app.input.GetKey(GLFW_KEY_ESCAPE) && glfwWindowShouldClose(window) == 0;
  }

  ////////////////////////////////////////////////////////////////////////////
  // One call per frame!
  ////////////////////////////////////////////////////////////////////////////
  static void BeginFrame() {
    AppContext::Instance().timer.Tick();
    AppContext::Instance().input.Update();
  }

  static void EndFrame(bool swap = true) {
    if (swap)
      glfwSwapBuffers(AppContext::Instance().display.GetWindow());
    glfwPollEvents();
  }
 
 private:
  AppContext(int width, int height, std::string caption, const Profile& profile)
    : display     (width, height, std::move(caption), profile),
      input       () {
    display.Init();
  }

  virtual ~AppContext() {
    display.Close();
  }

  static AppContext* instance;
};
#endif // _APPCONTEXT_H_4B227517_B140_479C_BD2E_01283044D3DA_
