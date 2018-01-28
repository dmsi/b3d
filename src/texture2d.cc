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

#include "texture2d.h"

Texture2D::Texture2D() : Texture(), texture_id_(0) {
}

Texture2D::Texture2D(std::shared_ptr<Image::ColorMap> pixels) {
  Defaults();
  texture_id_ = 0;
  SetPixels(pixels);
  Apply();
}

Texture2D::~Texture2D() {
  if (texture_id_) {
    glDeleteTextures(1, &texture_id_);
    texture_id_ = 0;
  }
}

void Texture2D::SetPixels(std::shared_ptr<Image::ColorMap> pixels) {
  pixels_ = pixels;
}

void Texture2D::Apply() {
  if (texture_id_) {
    glDeleteTextures(1, &texture_id_);
    texture_id_ = 0;
  }

  glGenTextures(1, &texture_id_);
  glBindTexture(GL_TEXTURE_2D, texture_id_);
  // Set our texture parameters
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, Texture::ToOpenGL(wrap_u_mode));
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, Texture::ToOpenGL(wrap_v_mode));
  // Set texture filtering
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, Texture::ToOpenGL(filter_mode));
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, Texture::ToOpenGL(filter_mode));
  // Create texture and generate mipmaps
  glTexImage2D(GL_TEXTURE_2D, 
               0, 
               GL_RGBA, 
               pixels_->GetWidth(), 
               pixels_->GetHeight(), 
               0, 
               GL_RGBA, 
               GL_FLOAT, 
               pixels_->GetArray());
  glGenerateMipmap(GL_TEXTURE_2D);
  glBindTexture(GL_TEXTURE_2D, 0);
}

void Texture2D::Compress() {
  throw std::runtime_error("Texture::Compress() - not implemented!");
}

void Texture2D::Bind(int slot) { 
  assert(slot >= 0 && slot < GL_MAX_TEXTURE_UNITS);
  glActiveTexture(GL_TEXTURE0 + slot);
  glBindTexture(GL_TEXTURE_2D, texture_id_);
}

void Texture2D::Unbind(int slot) {
  assert(slot >= 0 && slot < GL_MAX_TEXTURE_UNITS);
  glActiveTexture(GL_TEXTURE0 + slot);
  glBindTexture(GL_TEXTURE_2D, 0);
}
