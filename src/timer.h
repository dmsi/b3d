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

#ifndef _TIMER_H_5D1A4D70_BA55_4E76_8E7B_ADF09BE442AD_
#define _TIMER_H_5D1A4D70_BA55_4E76_8E7B_ADF09BE442AD_ 

#include "gl_main.h"

//////////////////////////////////////////////////////////////////////////////
// Out handy timer!
//////////////////////////////////////////////////////////////////////////////
class Timer {
 public:
  Timer() : prev_time_(0), time_(0), time_delta_t(0), frame_number_(0) {}

  ////////////////////////////////////////////////////////////////////////////
  // This method  must be called once per frame!
  ////////////////////////////////////////////////////////////////////////////
  void Tick() {
    prev_time_ = time_;
    time_ = glfwGetTime(); // time since GLFW was initialized
    time_delta_t = time_ - prev_time_;
    frame_number_++;
  }

  float GetTime()        const { return time_; }
  float GetTimeDelta()   const { return time_delta_t; }
  int   GetFrameNumber() const { return frame_number_; }

 private:
  float       prev_time_;
  float       time_;
  float       time_delta_t;
  int         frame_number_;
};

#endif // _TIMER_H_5D1A4D70_BA55_4E76_8E7B_ADF09BE442AD_
