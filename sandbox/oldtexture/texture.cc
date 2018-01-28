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
#include "gli_main.h"
#include "image/portablepixmap.h"

#define STB_DXT_IMPLEMENTATION
#include "image/stb_dxt.h"

#include <exception>
#include <iostream>
#include <vector>

int TextureFilterMode::ToOpenGL(TextureFilterMode::Value v) {
  switch (v) {
    case TextureFilterMode::kPoint     : return GL_NEAREST;
    case TextureFilterMode::kBilinear  : return GL_LINEAR;
    case TextureFilterMode::kTrilinear : return GL_LINEAR_MIPMAP_LINEAR;
    default: 
      throw std::runtime_error(
          "TextureFilterMode::ToOpenGL() - invalid filter mode " + std::to_string(v));
  }
}

int TextureWrapMode::ToOpenGL(TextureWrapMode::Value v) {
  switch (v) {
    case TextureWrapMode::kRepeat     : return GL_REPEAT;
    case TextureWrapMode::kClamp      : return GL_CLAMP_TO_BORDER;
    case TextureWrapMode::kMirror     : return GL_MIRRORED_REPEAT;
    case TextureWrapMode::kMirrorOnce : 
      throw std::runtime_error(
          "TextureWrapMode::ToOpenGL() - not implemented" + std::to_string(v));
    default: 
      throw std::runtime_error(
          "TextureWrapMode::ToOpenGL() - invalid wrap mode " + std::to_string(v));
  }
}

  
//////////////////////////////////////////////////////////////////////////////
// TextureOld, keeping it temporarily just for demonstration of gli and 
// PPM -> DXT1 compression
//////////////////////////////////////////////////////////////////////////////
TextureOld::TextureOld(const std::string& filename) 
  : texture_id_(0), filename_(filename) {
  
  std::string ext = filename.substr(filename.find_last_of(".") + 1);
  if (ext == "dds") {
    Load();
  } else if (ext == "ppm" || ext == "pgm") {
    LoadPixelMapCompressed();
  }
}

TextureOld::~TextureOld() {
  if (texture_id_) {
    glDeleteTextures(1, &texture_id_);
    texture_id_ = 0;
  }
}
  
void TextureOld::Bind(int slot) { 
  assert(slot >= 0 && slot < GL_MAX_TEXTURE_UNITS);
  glActiveTexture(GL_TEXTURE0 + slot);
  glBindTexture(GL_TEXTURE_2D, texture_id_);
}

void TextureOld::Unbind(int slot) {
  assert(slot >= 0 && slot < GL_MAX_TEXTURE_UNITS);
  glActiveTexture(GL_TEXTURE0 + slot);
  glBindTexture(GL_TEXTURE_2D, 0);
}
  

void TextureOld::Load() {
  if (texture_id_ != 0) {
    throw std::logic_error("TextureOld::Load() - texture is already loaded!");
  }

  gli::texture tex = gli::load(filename_.c_str());
  if (tex.empty()) {
    throw std::runtime_error("TextureOld::Load() Cant load texture from file " + filename_);
  }

  gli::gl profile(gli::gl::PROFILE_GL33);
  const gli::gl::format format = profile.translate(tex.format(), tex.swizzles()); 
  GLenum target = profile.translate(tex.target());

  glGenTextures(1, &texture_id_);
  glBindTexture(target, texture_id_);
  glTexParameteri(target, GL_TEXTURE_BASE_LEVEL, 0);
  glTexParameteri(target, GL_TEXTURE_MAX_LEVEL, static_cast<GLint>(tex.levels()-1));
  glTexParameteriv(target, GL_TEXTURE_SWIZZLE_RGBA, &format.Swizzles[0]);
  glm::tvec3<GLsizei> extent(tex.extent(0));
  glTexStorage2D(target, static_cast<GLint>(tex.levels()), format.Internal, extent.x, extent.y);
  std::cerr << "FORMAT " << format.Internal << std::endl;
  for (std::size_t level = 0; level < tex.levels(); ++level) {
    glm::tvec3<GLsizei> extent(tex.extent(level));
    glCompressedTexSubImage2D(
        target, static_cast<GLint>(level), 0, 0, extent.x, extent.y,
        format.Internal, static_cast<GLsizei>(tex.size(level)), tex.data(0, 0, level));
  }
  
  std::cerr << "Texture " << filename_ << " loaded!" << std::endl;
}

void TextureOld::LoadPixelMap() {
  // TODO Need to have more generic way of doing this
  //  1. Setup texture directly from ColorMap
  //  2. Compress texture
  //  3. Set up texture parameters like wrap, filtering, etc.
  //  Then it will be possible to separate texture operations from texture loading
  glGenTextures(1, &texture_id_);
  glBindTexture(GL_TEXTURE_2D, texture_id_);
  // Set our texture parameters
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
  // Set texture filtering
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  // Load, create texture and generate mipmaps
  std::shared_ptr<Image::ColorMap> img = Image::PortablePixMap::Read(filename_);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, img->GetWidth(), img->GetHeight(), 0, GL_RGBA, GL_FLOAT, img->GetArray());
  glGenerateMipmap(GL_TEXTURE_2D);
  glBindTexture(GL_TEXTURE_2D, 0);
}

void TextureOld::LoadPixelMapCompressed() {
  // Example of DXT1 compression
  glGenTextures(1, &texture_id_);
  glBindTexture(GL_TEXTURE_2D, texture_id_);
  // Set our texture parameters
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
  // Set texture filtering
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  // Load, create texture and generate mipmaps
  std::shared_ptr<Image::ColorMap> img_src = Image::PortablePixMap::Read(filename_);
  std::shared_ptr<Image::ColorMap32> img = Image::Convert(*img_src);
  //See usage https://github.com/Cyan4973/RygsDXTc/blob/master/README.md
  std::vector<uint8_t> dst(4 * img->GetWidth() * img->GetHeight() / 8);
  rygCompress(&dst.at(0), (unsigned char*)img->GetArray(), img->GetWidth(), img->GetHeight(), 0); 

  glCompressedTexImage2D(GL_TEXTURE_2D, 0, GL_COMPRESSED_RGBA_S3TC_DXT1_EXT, img->GetWidth(), img->GetHeight(), 0, dst.size(), &dst.at(0));
  glGenerateMipmap(GL_TEXTURE_2D);
  glBindTexture(GL_TEXTURE_2D, 0);
}


