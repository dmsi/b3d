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
// Replaced by Texture2D class.
// Keeping it temporarily just for demonstration of gli and 
// PPM -> DXT1 compression
//////////////////////////////////////////////////////////////////////////////
class TextureOld {
 public:
  explicit TextureOld(const std::string& filename);
  explicit TextureOld(GLuint id) : texture_id_(id) {}
  virtual ~TextureOld();

  uint32_t GetTextureId() const { return texture_id_; }

  void Bind(int slot);
  void Unbind(int slot);
  
 private:
  void Load();
  void LoadPixelMap();
  void LoadPixelMapCompressed();

  GLuint              texture_id_;
  const std::string   filename_;
};


struct TextureFilterMode {
  enum Value {
    kPoint,
    kBilinear,
    kTrilinear
  };

  static int ToOpenGL(Value v);
};

struct TextureWrapMode {
  enum Value {
    kRepeat,
    kClamp,
    kMirror,
    kMirrorOnce
  };
  
  static int ToOpenGL(Value v);
};


#endif // _TEXTURE_H_5414E7FD_5F94_4FBC_8836_E999C5FF45F4_
