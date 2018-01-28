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

#include "gl_main.h"
#include "appcontext.h"
#include "framebuffer.h"
#include <string>
#include <stdexcept>
#include <iostream>


//////////////////////////////////////////////////////////////////////////////
// The glorious implementation 
//////////////////////////////////////////////////////////////////////////////
  
Layer::Layer(Type type, Permission permission, Hint hint,
             Texture::FilterMode filter) 
    : permission_(permission), 
      type_(type),
      hint_(hint),
      tex_filter_(filter),
      gl_id_(0) {
  if (type != kColor && type != kDepth) {
    throw std::logic_error("Layer::Layer() - " \
                           "Layer type " + std::to_string(type_) + 
                           " not implemented");
  }
}

Layer::~Layer() {
  if (gl_id_) {
    if (permission_ == kWrite) {
      glDeleteRenderbuffers(1, &gl_id_);
    } else {
      glDeleteTextures(1, &gl_id_);
    }
    gl_id_ = 0;
  }
}

void Layer::Init(int layer_number, int width, int height) {
  if (permission_ == kWrite) {
    InitRenderbuffer(layer_number, width, height);
  } else {
    if (hint_ == kHintTexture2D) {
      InitTexture2D(layer_number, width, height);
    } else if (hint_ == kHintCubemap) {
      InitTextureCube(layer_number, width, height);
    } else {
      throw std::logic_error("Layer::Init() - Invalid hint!");
    }
  }
}

void Layer::Bind(int slot) {
  if (permission_ == kWrite) {
    throw std::runtime_error("Layer::Bind() - Layer has write-only permission " \
                             "and cannot be binded!");
                           
  }
  if (!gl_id_) {
    throw std::runtime_error("Layer::Bind() - Layer cant be binded " \
                             "Layer::Init() must be called first!"); 
  }

  assert(slot >= 0 && slot < GL_MAX_TEXTURE_UNITS);
  glActiveTexture(GL_TEXTURE0 + slot);
  glBindTexture(GetLayerTextureType(), gl_id_);
}

void Layer::Unbind(int slot) {
  if (permission_ == kWrite) {
    throw std::runtime_error("Layer::Unbind() - Layer has write-only permission!" \
                             "and cannot be unbinded!");
                           
  }
  if (!gl_id_) {
    throw std::runtime_error("Layer::Unbind() - Layer cant be unbinded!" \
                             "Layer::Init() must be called first!"); 
  }
  assert(slot >= 0 && slot < GL_MAX_TEXTURE_UNITS);
  glActiveTexture(GL_TEXTURE0 + slot);
  glBindTexture(GetLayerTextureType(), 0);
}

GLint Layer::GetLayerFormat() const {
  switch(type_) {
    case kColor: return GL_RGBA;
    case kDepth: return GL_DEPTH_COMPONENT24;
    default:     return 0; // Not implemented
  }
}

GLenum Layer::GetLayerTextureType() const {
  switch(hint_) {
    case kHintTexture2D : return GL_TEXTURE_2D;
    case kHintCubemap   : return GL_TEXTURE_CUBE_MAP;
    default             : return 0; // not implemented
  }
}

void Layer::InitTexture2D(int layer_number, int width, int height) {
  glGenTextures(1, &gl_id_);
  glBindTexture(GL_TEXTURE_2D, gl_id_);

  GLint internal_format = GetLayerFormat();
  GLenum format = GL_RGBA;
  GLenum data_type = GL_UNSIGNED_BYTE;

  if (type_ == kDepth) {
    format = GL_DEPTH_COMPONENT;
    data_type = GL_UNSIGNED_BYTE;
  }
  glTexImage2D(GL_TEXTURE_2D, 
               0,
               internal_format, 
               width, height, 
               0, 
               format, 
               data_type, 0);
 
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, Texture::ToOpenGL(tex_filter_));
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, Texture::ToOpenGL(tex_filter_)); 
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  if (type_ == kDepth) {
    // For the shadow maps in order to use hardware PCF in sampler2DShadow
    // https://stackoverflow.com/questions/22419682/glsl-sampler2dshadow-and-shadow2d-clarification
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE, GL_COMPARE_R_TO_TEXTURE);
  }

  if (type_ == kColor) {
    glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + layer_number, gl_id_, 0);
  } else if (type_ == kDepth) {
    glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, gl_id_, 0);
  } 
  
  glBindTexture(GL_TEXTURE_2D, 0);
}

void Layer::InitTextureCube(int layer_number, int width, int height) {
  if (hint_ == kHintCubemap && type_ != kColor) {
    throw std::logic_error("Layer::InitTextureCube() - Cubemaps can only be used for color layers!");
  }
  glGenTextures(1, &gl_id_);
  glBindTexture(GL_TEXTURE_CUBE_MAP, gl_id_);

  GLint internal_format = GetLayerFormat();
  GLenum format = GL_RGBA;
  GLenum data_type = GL_UNSIGNED_BYTE;

  if (type_ == kDepth) {
    format = GL_DEPTH_COMPONENT;
    data_type = GL_UNSIGNED_BYTE;
  }

  for (int i = 0; i < 6; ++i) {
    glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 
                 0,
                 internal_format, 
                 width, height, 
                 0, 
                 format, 
                 data_type, 0);

    glFramebufferTexture2D(GL_FRAMEBUFFER, 
                           GL_COLOR_ATTACHMENT0 + layer_number, 
                           GL_TEXTURE_CUBE_MAP_POSITIVE_X + i,
                           gl_id_, 0);
  }
 
  // We're not intergating this with Texture2D class yet, so hardcoded for now   
  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, Texture::ToOpenGL(tex_filter_));
  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, Texture::ToOpenGL(tex_filter_)); 
  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

  //if (type_ == kColor) {
  //  glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + layer_number, gl_id_, 0);
  //} else if (type_ == kDepth) {
  //  glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, gl_id_, 0);
  //} 
  
  glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
}

void Layer::InitRenderbuffer(int layer_number, int width, int height) {
  // TODO cubemap support !!!!
  if (hint_ == kHintCubemap) {
    throw std::logic_error("Layer::InitRenderbuffer() - only 2D renderbuffers are supported!");
  }
  glGenRenderbuffers(1, &gl_id_);
  glBindRenderbuffer(GL_RENDERBUFFER, gl_id_);
  GLint internal_format = GetLayerFormat();
  glRenderbufferStorage(GL_RENDERBUFFER, internal_format, width, height);
    
  if (type_ == kColor) {
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + layer_number, GL_RENDERBUFFER, gl_id_);
  } else if (type_ == kDepth) {
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, gl_id_);
  } 

  glBindRenderbuffer(GL_RENDERBUFFER, 0);
}

//////////////////////////////////////////////////////////////////////////////
// Another glorious implementation 
//////////////////////////////////////////////////////////////////////////////
TextureRender::TextureRender(std::shared_ptr<Layer> layer) : layer_(layer) {
  if (!layer_) {
    throw std::logic_error("TextureRender::TextureRender() - Layer is nullptr!");
  }
}

//////////////////////////////////////////////////////////////////////////////
// Yet another glorious implementation 
//////////////////////////////////////////////////////////////////////////////

FrameBuffer::FrameBuffer(Type type, int width, int height) 
  : type_(type), 
    framebuffer_id_(0),
    width_(width),
    height_(height),
    clear_depth_value_(1.0f),
    clear_stencil_value_(0),
    clear_color_flag_(false),
    clear_depth_flag_(false),
    clear_stencil_flag_(false) {
  if (type != kScreen) {
    if (width <= 0 || height <= 0) {
      throw std::runtime_error("FrameBuffer::FrameBuffer() - Invalid width and height " + 
                               std::to_string(width) + "x" + std::to_string(height));
    }
  } else {
    Display& display = AppContext::Instance().display;
    width_ = display.GetWidth();
    height_ = display.GetHeight();
  }
}

FrameBuffer::~FrameBuffer() {
  if (framebuffer_id_) {
    glDeleteFramebuffers(1, &framebuffer_id_);
    framebuffer_id_ = 0;
  }
}

void FrameBuffer::AddLayer(Layer::Type layer_type, Layer::Permission permission, 
                           Texture::FilterMode tex_filter) {

  if (type_ == kScreen && layer_type == Layer::kColor) {
    throw std::runtime_error("FrameBuffer::AddLayer() - Can't add color layer for " \
                             "the screen framebuffer");
  }

  Layer::Hint hint = Layer::kHintTexture2D;
  switch (type_) {
    case kScreen    : hint = Layer::kHintTexture2D; break;
    case kTexture2D : hint = Layer::kHintTexture2D; break;
    case kCubemap   : hint = Layer::kHintCubemap;   break;
    default         :
      throw std::logic_error("FrameBuffer::AddLayer() - Invalid framebuffer type!");
  };

  switch (layer_type) {
    case Layer::kColor:
      color_layers_.emplace_back(
          LayerPtr(new Layer(layer_type, permission, hint, tex_filter))
          );
      break;
    case Layer::kDepth:
      if (depth_layer_) {
        throw std::runtime_error("FrameBuffer::AddLayer() - Depth layer already added");
      }
      depth_layer_.reset(
          new Layer(layer_type, permission, Layer::kHintTexture2D, tex_filter));
      SetDepthLayerClearValue(1.0f);
      break;
    default:
      throw std::logic_error("FrameBuffer::AddLayer() - " \
                             "Layer type " + std::to_string(layer_type) + 
                             " not implemented");
      break;
  }
}

void FrameBuffer::Init() {
  if (type_ == kScreen) {
    // No need to Init anything for the screen framebuffer
    // Or we can integrate it later with the window/gl initialization?
    SetDepthLayerClearValue(1);
    return;
  }

	glGenFramebuffers(1, &framebuffer_id_);
	glBindFramebuffer(GL_FRAMEBUFFER, framebuffer_id_);
  
  std::vector<GLenum> draw_buffers;
  for (int i = 0; i < color_layers_.size(); ++i) {
    color_layers_[i]->Init(i, width_, height_);
    draw_buffers.push_back(GL_COLOR_ATTACHMENT0 + i);
  }

  if (depth_layer_) {
    depth_layer_->Init(0, width_, height_);
  }

  if (stencil_layer_) {
    stencil_layer_->Init(0, width_, height_);
  }
	
  // Set the list of draw buffers.
  if (!draw_buffers.empty()) {
    glDrawBuffers(draw_buffers.size(), &draw_buffers.at(0));
  } else {
    glDrawBuffer(GL_NONE);
  }

	// Always check that our framebuffer is ok
  auto status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
  if (status != GL_FRAMEBUFFER_COMPLETE) {
    throw std::runtime_error("FrameBuffer::Init() - OpenGL error. Error="
        + std::to_string(status));
  }

  glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void FrameBuffer::Bind() {
  if (!framebuffer_id_ && type_ != kScreen) {
    throw std::runtime_error("FrameBuffer::Bind() - Cant Bind! " \
                             "FrameBuffer::Init() must be called first!");
  }
  if (type_ != kScreen) {
    glBindFramebuffer(GL_FRAMEBUFFER, framebuffer_id_);
  }
  glViewport(0, 0, width_, height_); 

  // Defaults
  glEnable(GL_CULL_FACE);
  glCullFace(GL_BACK);
  glFrontFace(GL_CW);

  // Clearing
  GLbitfield clear_mask = 0;
  if (clear_color_flag_) {
    clear_mask |= GL_COLOR_BUFFER_BIT;
    const Color& c = clear_color_value_;
    glClearColor(c.r, c.g, c.b, c.a);
  }
  if (clear_depth_flag_) {
    clear_mask |= GL_DEPTH_BUFFER_BIT;
    glClearDepth((GLdouble)clear_depth_value_);
  }
  if (clear_stencil_flag_) {
    clear_mask |= GL_STENCIL_BUFFER_BIT;
    glClearStencil(clear_stencil_value_);
  }
  if (clear_mask) {
    glClear(clear_mask);
  }
}

void FrameBuffer::Unbind() {
  if (type_ != kScreen) {
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
  }
}

std::shared_ptr<TextureRender> 
FrameBuffer::GetLayerAsTexture(int layer_number, Layer::Type layer_type) const {
  // The TextureRender will throw exception if GetLayer returns nullptr
  return std::shared_ptr<TextureRender>(
      new TextureRender(GetLayer(layer_number, layer_type)));
}

std::shared_ptr<Layer> 
FrameBuffer::GetLayer(int layer_number, Layer::Type layer_type) const {
  if (type_ == kScreen) {
    return std::shared_ptr<Layer>();
  }

  std::shared_ptr<Layer> layer;

  if (layer_type == Layer::kColor) {
    layer = color_layers_.at(layer_number);
  } else if (layer_type == Layer::kDepth) {
    layer = depth_layer_;
  } 

  if (layer && layer->GetPermission() == Layer::kWrite) {
    layer.reset();
  }

  return layer;
}
