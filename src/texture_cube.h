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

#ifndef _TEXTURE_CUBE_H_2AA3E281_E899_4995_B4BF_E9022382CFD6_
#define _TEXTURE_CUBE_H_2AA3E281_E899_4995_B4BF_E9022382CFD6_ 

#include "gl_main.h"
#include "image/colormap.h"
#include "texture.h"
#include <memory>

//////////////////////////////////////////////////////////////////////////////
// Cube Texture (cubemap) 
//////////////////////////////////////////////////////////////////////////////
class TextureCube : public Texture {
 public:
  TextureCube();
  explicit TextureCube(std::shared_ptr<Image::ColorMap> pixels[6]);
  virtual ~TextureCube();

  // From inside
  // 0 - positive x - right face
  // 1 - negative x - left face
  // 2 - positive y - top face
  // 3 - negative y - bottom face
  // 4 - positive z - back face
  // 5 - negative z - front face
  void SetPixels(std::shared_ptr<Image::ColorMap> pixels[6]);

  // After compression, texture will be in DXT1 format if the original texture had no alpha channel, and in DXT5 format if it had alpha channel.
  void Compress();
  void Apply();

  void Bind(int slot) override;
  void Unbind(int slot) override;

 private:
  GLuint texture_id_;
  std::shared_ptr<Image::ColorMap> pixels_[6];
};

#endif // _TEXTURE_CUBE_H_2AA3E281_E899_4995_B4BF_E9022382CFD6_
