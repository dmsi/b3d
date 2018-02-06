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

#include "texture_cube.h"
#include "common/logging.h"

TextureCube::TextureCube() : Texture(), texture_id_(0) {
  wrap_u_mode = Texture::kWrapClamp;
  wrap_v_mode = Texture::kWrapClamp;
  wrap_r_mode = Texture::kWrapClamp;
}

TextureCube::TextureCube(std::shared_ptr<Image::ColorMap> pixels[6]) {
  Defaults();
  wrap_u_mode = Texture::kWrapClamp;
  wrap_v_mode = Texture::kWrapClamp;
  wrap_r_mode = Texture::kWrapClamp;
  texture_id_ = 0;
  SetPixels(pixels);
  Apply();
}

TextureCube::~TextureCube() {
  if (texture_id_) {
    glDeleteTextures(1, &texture_id_);
    texture_id_ = 0;
  }
}

void TextureCube::SetPixels(std::shared_ptr<Image::ColorMap> pixels[6]) {
  for (int i = 0; i < 6; ++i) {
    if (pixels[0]->GetWidth() != pixels[i]->GetWidth() ||
        pixels[0]->GetHeight() != pixels[0]->GetHeight()) {
      ABORT_F("All faces should have the same size");
    }
    pixels_[i] = pixels[i];
  }
}

void TextureCube::Apply() {
  if (texture_id_) {
    glDeleteTextures(1, &texture_id_);
    texture_id_ = 0;
  }

  glGenTextures(1, &texture_id_);
  glBindTexture(GL_TEXTURE_CUBE_MAP, texture_id_);
  
  for (int i = 0; i < 6; ++i) {
    glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 
                 0, 
                 GL_RGBA, 
                 pixels_[i]->GetWidth(), 
                 pixels_[i]->GetHeight(), 
                 0, 
                 GL_RGBA, 
                 GL_FLOAT, 
                 pixels_[i]->GetArray());
  }
  // Set our texture parameters
  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, Texture::ToOpenGL(wrap_u_mode));
  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, Texture::ToOpenGL(wrap_v_mode));
  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, Texture::ToOpenGL(wrap_r_mode)); 
  // Set texture filtering
  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, Texture::ToOpenGL(filter_mode));
  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, Texture::ToOpenGL(filter_mode));
  // Create texture and generate mipmaps
  glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
}

void TextureCube::Compress() {
  ABORT_F("Not implemented");
}

void TextureCube::Bind(int slot) { 
  assert(slot >= 0 && slot < GL_MAX_TEXTURE_UNITS);
  glActiveTexture(GL_TEXTURE0 + slot);
  glBindTexture(GL_TEXTURE_CUBE_MAP, texture_id_);
}

void TextureCube::Unbind(int slot) {
  assert(slot >= 0 && slot < GL_MAX_TEXTURE_UNITS);
  glActiveTexture(GL_TEXTURE0 + slot);
  glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
}
