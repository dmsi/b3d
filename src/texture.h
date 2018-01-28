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

#ifndef _TEXTURE_H_5414E7FD_5F94_4FBC_8836_E999C5FF45F4_
#define _TEXTURE_H_5414E7FD_5F94_4FBC_8836_E999C5FF45F4_ 

#include "gl_main.h"
#include "image/colormap.h"
#include <string>
#include <memory>

//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
class Texture {
 public:
  enum FilterMode {
    kFilterPoint,
    kFilterBilinear,
    kFilterTrilinear
  };

  enum WrapMode {
    kWrapRepeat,
    kWrapClamp, // clamps to edge
    kWrapMirror,
    kWrapMirrorOnce
  };

  static int ToOpenGL(FilterMode v);
  static int ToOpenGL(WrapMode v);
    
  FilterMode filter_mode;
  WrapMode   wrap_u_mode;
  WrapMode   wrap_v_mode;
  WrapMode   wrap_r_mode;

  Texture() {Defaults();}
  virtual ~Texture() {}

  virtual void Bind(int slot) = 0;
  virtual void Unbind(int slot) = 0;

 protected:
  void Defaults() {
    filter_mode = kFilterBilinear;
    wrap_u_mode = kWrapRepeat;
    wrap_v_mode = kWrapRepeat;
    wrap_r_mode = kWrapRepeat;
  }
};

#endif // _TEXTURE_H_5414E7FD_5F94_4FBC_8836_E999C5FF45F4_
