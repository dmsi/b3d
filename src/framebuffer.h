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

#ifndef _FRAMEBUFFER_H_E22E7451_EEE4_4B6A_B7CF_31CC1A6C4E7D_
#define _FRAMEBUFFER_H_E22E7451_EEE4_4B6A_B7CF_31CC1A6C4E7D_ 

#include "image/colormap.h"
#include "texture.h"
#include <vector>
#include <memory>

// namespace Render::Framebuffer ??

//////////////////////////////////////////////////////////////////////////////
// Framebuffer object layer such as Color, depth or stencil 
//
// Valid combinations: 
// kColor
//  - 2D texture [R+RW]
//  - Cubemap [RW]
//
// kDepth
//  - 2D texture [R+RW]
//
// kDepthAndStencil
//  - Not supported
// 
// kStencil
//  - Not supported
//
// TODO
// Namespace it? Having just Layer in global namespace is not 100% comfortable
// So there are two options - rename or namespace, i.e.
// FrameBufferLayer or FrameBuffer::Layer or Render::Fb::Layer
//
// TODO
// Integrate it with Texture2D and TextureCube classes. This will help to 
// avoid -> InitTexture2D and InitTextureCube...
//////////////////////////////////////////////////////////////////////////////
class Layer {
 public:
  enum Permission {
    kWrite,    // Not good for sampling, i.e. GL_RENDERBUFFER, sometimes faster.
               // For instance can be used for depth buffer which is not going
               // to be sampled.
               
    kReadWrite // Good for sampling, i.e. can be used as texture.  
  };

  enum Type {
    kColor,
    kDepth,
    kDepthAndStencil,
    kStencil
  };

  enum Hint {
    kHintTexture2D,
    kHintCubeMap
  };

  Layer(Type type, Permission permission, Hint hint, 
        Texture::FilterMode filter = Texture::kFilterBilinear);
  virtual ~Layer();
  void Init(int layer_number, int width, int height);
  void Bind(int slot);
  void Unbind(int slot);
  Permission GetPermission() const { return permission_; }

 private:
  GLint GetLayerFormat() const;
  GLenum GetLayerTextureType() const ;
  void InitTexture2D(int layer_number, int width, int height);
  void InitTextureCube(int layer_number, int width, int height);
  void InitRenderbuffer(int layer_number, int width, int height);

  friend class FrameBuffer;

  Permission               permission_;
  Type                     type_;
  Hint                     hint_;
  Texture::FilterMode      tex_filter_;
  GLuint                   gl_id_;
};

//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
class TextureRender : public Texture {
 public:
  explicit TextureRender(std::shared_ptr<Layer> layer);
  virtual ~TextureRender() {}

  void Bind(int slot) override { layer_->Bind(slot); }
  void Unbind(int slot) override { layer_->Unbind(slot); }
 private:
  std::shared_ptr<Layer> layer_;
};

//////////////////////////////////////////////////////////////////////////////
// Onscreen and offscreen framebuffer, for rendering to screen or texture.
//
// Valid types:
//  kScreen
//   No color layers allowed. In order to have depth buffer a  depth layer 
//   can be added or just set depth clear value instead.
//   Width and height are ignored and can be set to 0. The actual values
//   will be taken from the actual display framebuffer.
//   UNDEFINED!!! - adding depth layer with RW permission and the get it as 
//   texture.
// 
//  kTexture2D
//   Rendering to 2D texture. Color and Depth layer are allowed, both support
//   R+RW modes.
//
//  kCubeMap:
//   Rendering to a cubemap. Color and Depth layers are allowed. Color in RW
//   mode, depth in R+RW.
//
// Can have up to several color layers and up to one depth layer.
// The rest is not supported now.
//
// Usage:
//  kScreen:
//   auto fb = new FrameBuffer(FrameBuffer::kScreen, 0, 0);
//   fb->dLayer(Layer::kDepth, Layer::kWrite);
//   fb->SetColorLayerClearValue(Color(1, 0, 0, 1));
//   >>>fb is ready, no Init required.
//   (...)
//   fb->Bind();
//   ...draw...
//   fb->Unbind();
//   (...)
//
//  kTexture2D
//   auto fb = new FrameBuffer(FrameBuffer::kTexture2D, 200, 200);
//   fb->AddLayer(Layer::kColor, Layer::kReadWrite); 
//   fb->AddLayer(Layer::kDepth, Layer::kReadWrite); 
//   fb->SetColorLayerClearValue(Color(1,1,1,1));
//   fb->Init();
//   >>>fb is ready now, so can get the texture placeholders:
//   auto color_texture = fb->GetLayerAsTexture(0, Layer::kColor);
//   auto depth_texture = fb->GetLayerAsTexture(0, Layer::kDepth);
//   >>>these textures can be applied to a material.
//   >>>these method can only be called after Init! The first argument makes
//      sense only for color layers. It indicates number of color layer in case
//      when MRT is in use. Ignored for other layers.
//      Any inconsistency, like when layer number exceeds the actual amount of
//      color layers added, or not having read permission on the layer will 
//      cause exception.
//   (...)
//   fb->Bind();
//   ...draw...
//   fb->Unbind();
//
//  kCubeMap
//   auto fb = new FrameBuffer(FrameBuffer::kCubeMap, 200, 200);
//   fb->AddLayer(Layer::kColor, Layer::kReadWrite); // the only right way!
//   fb->AddLayer(Layer::kDepth, Layer::kWrite);     // can be R+RW.
//   fb->SetColorLayerClearValue(Color(1,1,1,1));
//   fb->Init();
//   auto color_texture = fb->GetLayerAsTexture(0, Layer::kColor);
//   (...)
//   fb->Bind();
//   ...The order +x-x +y-y +z-z 
//   for (int i = 0; i < 6; ++i) {
//    fb->BindCubemapFace(i);
//    ...Camera is supposed to have 90 FOV and 1:1 aspect ratio...
//    ...Rotate the camera to face i direction...
//    ...Draw...
//    fb->UnbindCubemapFace(i);
//   } 
//   fb->Unbind();
//////////////////////////////////////////////////////////////////////////////
class FrameBuffer {
 public:
  enum Type {
    kScreen,           // Screen framebuffer, linked with the screen
    kTexture2D,        // Texture2D framebuffer  
    kCubeMap,          // Cubemap framebuffer
  };

  // In case of kScreen width and height can be set to 0
  explicit FrameBuffer(Type type, int width, int height);
  virtual ~FrameBuffer();

  void AddLayer(Layer::Type layer_type, Layer::Permission permission,
                Texture::FilterMode tex_filter = Texture::kFilterBilinear);
  void Init();

  // Need to be called manually, not happens by default after AddLayer
  void SetColorLayerClearValue(const Color& color) {
    clear_color_value_ = color;
    clear_color_flag_ = true;
  }

  // Automatically called when depth layer added via AddLayer
  // the initial value is 1
  void SetDepthLayerClearValue(float depth) {
    clear_depth_value_ = depth;
    clear_depth_flag_ = true;
  }

  // Automatically called when stencil layer added via AddLayer
  // the initial value is 0
  void SetStencilLayerClearValue(int stencil) {
    clear_stencil_value_ = stencil;
    clear_stencil_flag_ = true;
  }

  Type GetType() const {
    return type_;
  }

  void Bind();
  void Unbind();

  // +x -x +y -y +z -z => 0 1 2 3 4 5
  void BindCubemapFace(int face) {
    assert(type_ == kCubeMap);
    assert(face >= 0 && face <= 5);
    for (int i = 0; i < color_layers_.size(); ++i) {
      assert(color_layers_[i]->GetPermission() == Layer::kReadWrite);
      auto id = color_layers_[i]->gl_id_;

      glFramebufferTexture2D(
          GL_FRAMEBUFFER, 
          GL_COLOR_ATTACHMENT0 + i, 
          GL_TEXTURE_CUBE_MAP_POSITIVE_X + face,
          id, 0);

      // Seems like I need to clear for each cubeface AFTER glFramebufferTexture2D
      GLbitfield clear_mask = 0;
      if (clear_color_flag_) {
        clear_mask |= GL_COLOR_BUFFER_BIT;
        const Color& c = clear_color_value_;
        glClearColor(c.r, c.g, c.b, c.a);
      }
      //if (clear_depth_flag_) {
      //  clear_mask |= GL_DEPTH_BUFFER_BIT;
      //  glClearDepth((GLdouble)clear_depth_value_);
      //}
      if (clear_stencil_flag_) {
        clear_mask |= GL_STENCIL_BUFFER_BIT;
        glClearStencil(clear_stencil_value_);
      }
      if (clear_mask) {
        glClear(clear_mask);
      }
    } // for

    // depth cubemap 
    if (depth_layer_) {
      // bind depth cubeface if this is a depth cubemap
      if (depth_layer_->hint_ == Layer::kHintCubeMap) {
        glFramebufferTexture2D(
            GL_FRAMEBUFFER, 
            GL_DEPTH_ATTACHMENT,
            GL_TEXTURE_CUBE_MAP_POSITIVE_X + face,
            depth_layer_->gl_id_, 0);
      }

      // clear depth buffer for each face if we need to
      if (clear_depth_flag_) {
        glClearDepth((GLdouble)clear_depth_value_);
        glClear(GL_DEPTH_BUFFER_BIT);
      }
    }
  }

  void UnbindCubemapFace(int face) {
    // Just for consistancy
  }

  // For color layers
  std::shared_ptr<TextureRender> 
  GetLayerAsTexture(int layer_number, Layer::Type layer_type) const;

  // For non color layers
  std::shared_ptr<TextureRender>
  GetLayerAsTexture(Layer::Type layer_type) const;

 private:
  std::shared_ptr<Layer> GetLayer(int layer_number, Layer::Type layer_type) const;
  Type                  type_;
  GLuint                framebuffer_id_;

  int                   width_;
  int                   height_;

  using LayerPtr = std::shared_ptr<Layer>;
  std::vector<LayerPtr> color_layers_;
  LayerPtr              depth_layer_;
  LayerPtr              stencil_layer_;

  Color                 clear_color_value_;
  float                 clear_depth_value_;
  int                   clear_stencil_value_;

  bool                  clear_color_flag_;
  bool                  clear_depth_flag_;
  bool                  clear_stencil_flag_;
};

#endif // _FRAMEBUFFER_H_E22E7451_EEE4_4B6A_B7CF_31CC1A6C4E7D_
