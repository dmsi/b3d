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

#include "texture.h"
#include "image/portablepixmap.h"

#include <exception>
#include <iostream>
#include <vector>

//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////

int Texture::ToOpenGL(Texture::FilterMode v) {
  switch (v) {
    case Texture::kFilterPoint     : return GL_NEAREST;
    case Texture::kFilterBilinear  : return GL_LINEAR;
    case Texture::kFilterTrilinear : return GL_LINEAR_MIPMAP_LINEAR;
    default: 
      throw std::runtime_error(
          "Texture::ToOpenGL() - invalid filter mode " + std::to_string(v));
  }
}

int Texture::ToOpenGL(Texture::WrapMode v) {
  switch (v) {
    case Texture::kWrapRepeat     : return GL_REPEAT;
    case Texture::kWrapClamp      : return GL_CLAMP_TO_EDGE;
    case Texture::kWrapMirror     : return GL_MIRRORED_REPEAT;
    case Texture::kWrapMirrorOnce : 
      throw std::runtime_error(
          "Texture::ToOpenGL() - not implemented" + std::to_string(v));
    default: 
      throw std::runtime_error(
          "Texture::ToOpenGL() - invalid wrap mode " + std::to_string(v));
  }
}

