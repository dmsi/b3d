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

#ifndef _TEXTURE2D_H_U0_
#define _TEXTURE2D_H_U0_ 

#include "gl_main.h"
#include "texture.h"
#include "image/colormap.h"
#include <memory>

//////////////////////////////////////////////////////////////////////////////
// 2D Texture 
//////////////////////////////////////////////////////////////////////////////
class Texture2D : public Texture {
 public:
  Texture2D();
  explicit Texture2D(std::shared_ptr<Image::ColorMap> pixels);
  virtual ~Texture2D();

  void SetPixels(std::shared_ptr<Image::ColorMap> pixels);

  // After compression, texture will be in DXT1 format if the original texture had no alpha channel, and in DXT5 format if it had alpha channel.
  void Compress();
  void Apply();

  void Bind(int slot) override;
  void Unbind(int slot) override;

 private:
  GLuint texture_id_;
  std::shared_ptr<Image::ColorMap> pixels_;
};
#endif // _TEXTURE2D_H_U0_
