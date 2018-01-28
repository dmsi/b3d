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

#ifndef _CFG_RENDERTARGET_H_7549B16A_B870_4AA0_83D4_68F648D5A67B_
#define _CFG_RENDERTARGET_H_7549B16A_B870_4AA0_83D4_68F648D5A67B_ 

#include "cfg.h"

template <>
class Cfg<RenderTarget> {
 public:
  using Ret = Cfg<RenderTarget>&;

  template <typename... TArgs>
  Cfg(Scene& scene, TArgs&&... args) {
    client_ = scene.AddRenderTarget(std::forward<TArgs>(args)...);
  }

  explicit Cfg(std::shared_ptr<RenderTarget> client)
    : client_(client) {
    if (!client_) {
      throw std::invalid_argument("Cfg<RenderTarget>::Cfg() - " \
          "RenderTarget is nullptr!");
    }
  }

  Ret Tags(std::string t) {
    tags_.emplace_back(std::move(t));
    return *this;
  }

  template <typename... TArgs>
  Ret Tags(std::string t, TArgs&&... args) {
    tags_.emplace_back(std::move(t));
    return Tags(std::forward<TArgs>(args)...);
  }

  Ret Camera(const std::string& camera) {
    camera_ = camera;
    return *this;
  }

  Ret Type(FrameBuffer::Type type) {
    type_ = type;
    return *this;
  }

  Ret Resolution(int width, int height) {
    if (width < 0 || height < 0) {
      throw std::invalid_argument("Cfg<RenderTarget>::Resolution() - " \
          "width/height is invalid!");
    }
    width_ = width;
    height_ = height;
    return *this;
  }

  Ret Layer(Layer::Type layer_type, Layer::Permission layer_permission,
            Texture::FilterMode tex_filter = Texture::kFilterBilinear) {
    layer_types_.emplace_back(layer_type);
    layer_permissions_.emplace_back(layer_permission);
    layer_tex_filters_.emplace_back(tex_filter);
    return *this;
  }

  Ret Clear(float r, float b, float g, float a) {
    clear_color_.reset(new Color(r, g, b, a));
    return *this;
  }

  auto Done() {
    auto fb = client_->SetFrameBuffer(type_, width_, height_);
    for (size_t i = 0; i < layer_types_.size(); ++i) {
      fb->AddLayer(layer_types_.at(i), layer_permissions_.at(i), 
                   layer_tex_filters_.at(i));
    }
    if (clear_color_) {
      fb->SetColorLayerClearValue(*clear_color_);
    }
    client_->SetTags(tags_);
    client_->SetCamera(camera_);
    fb->Init();
    return fb;
  }

  std::shared_ptr<RenderTarget> GetClient() {
    return client_;
  }

 private:
  std::shared_ptr<RenderTarget> client_;

  
  int                            width_ = 0;
  int                            height_ = 0;
  std::string                    camera_ = "camera.main";
  std::vector<std::string>       tags_;
  FrameBuffer::Type              type_ = FrameBuffer::kScreen;
  std::vector<Layer::Type>       layer_types_;
  std::vector<Layer::Permission> layer_permissions_;
  std::vector<Texture::FilterMode> layer_tex_filters_;
  std::unique_ptr<Color>         clear_color_;
};

#endif // _CFG_RENDERTARGET_H_7549B16A_B870_4AA0_83D4_68F648D5A67B_
