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

#ifndef _RENDERTARGET_H_402867F5_8866_46E7_B227_CC234EC053AD_
#define _RENDERTARGET_H_402867F5_8866_46E7_B227_CC234EC053AD_ 

#include "renderqueue.h"
#include "framebuffer.h"
#include "actor.h"
#include "camera.h"
#include "common/tags.h"
#include "common/logging.h"
#include <memory>
#include <string>
#include <stdexcept>

class Scene;

////////////////////////////////////////////////////////////////////////////
// Yet another piece of ART! :D
////////////////////////////////////////////////////////////////////////////
class RenderTarget {
 public:
  explicit RenderTarget(const std::string& name);
  virtual ~RenderTarget();

  void SetCamera(const std::string& name) {
    camera_name_ = name;
  }

  // The defaults width=0 and height=0 are for the most common case of 
  // having single screen-attached render target.
  std::shared_ptr<FrameBuffer> 
  SetFrameBuffer(FrameBuffer::Type type, int width = 0, int height = 0) {
    if (framebuffer_) {
      // Dont allow to change the framebuffer
      ABORT_F("Framebuffer already set");
    }
    if (type == FrameBuffer::kCubeMap) {
      if (!cubemap_mask_.any()) {
        cubemap_mask_.flip();
        for (int i = 0; i < 6; ++i) {
          if (cubemap_mask_[i]) {
            cubemap_rt_[i].reset(new RenderTarget(name_ + ".c" + std::to_string(i)));
          }
        }
      }
    }
    framebuffer_.reset(new FrameBuffer(type, width, height));
    return framebuffer_;
  }

  std::shared_ptr<FrameBuffer> GetFrameBuffer() const {
    return framebuffer_;
  }

  const std::string& GetName() const {return name_;}

  void StartNewFrame() {
    render_queue_.Clear();
  }

  void AddActor(std::shared_ptr<Actor> actor) {
    render_queue_.AddActor(actor, tags_);
  }

  void Draw(Scene& scene);
  
  void SetTags(const std::vector<std::string>& tags) {
    tags_.Set(tags);
  }

  template <class T>
  bool CheckTag(T tag) const {
    return tags_.Check(tag);
  }

  // bit string
  void SetCubemapSides(const std::string& sides) {
    // ugly, not optimal... but hey! optimize later. 
    if (framebuffer_) {
      ABORT_F("Render targer already initialized");
    }
    cubemap_mask_ = std::bitset<6>(sides);
  }

 private:
  Tags                          tags_;
  std::string                   name_;
  std::string                   camera_name_;
  RenderQueue                   render_queue_;
  std::shared_ptr<FrameBuffer>  framebuffer_;

  std::shared_ptr<RenderTarget> cubemap_rt_[6];
  std::bitset<6>                cubemap_mask_;
};

#endif // _RENDERTARGET_H_402867F5_8866_46E7_B227_CC234EC053AD_
