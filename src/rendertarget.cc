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

#include "rendertarget.h"
#include "scene.h"
  
RenderTarget::RenderTarget(const std::string& name) :
    name_(name),
    camera_name_("camera.main") {
  std::cerr << "[rendertarget " << name_ << "] added!" << std::endl;
}

RenderTarget::~RenderTarget() {
  std::cerr << "[rendertarget " << name_ << "] deleted!" << std::endl;
}

// Moving camera in order to capture each face of the cube
inline glm::vec3 GetCubeCameraRotation(int face) {
  using glm::vec3;
  switch(face) {
    case 0: return vec3(  0, -90, 180);  // +x 
    case 1: return vec3(  0,  90, 180);  // -x
    case 2: return vec3( 90,   0,   0);  // +y
    case 3: return vec3(-90,   0,   0);  // -y
    case 4: return vec3(  0, 180, 180);  // +z
    case 5: return vec3(  0,   0, 180);  // -z
    default: 
      throw std::logic_error("Invalid cubemap face!");
  }
}

void RenderTarget::Draw(Scene& scene) {
  auto camera = scene.Get<Camera>(camera_name_);
  if (!camera) {
    throw std::logic_error("RenderTarget::Draw() - camera " + camera_name_ + " not set!");
  }

  if (!framebuffer_) {
    throw std::logic_error("RenderTarget::Draw() - framebuffer not set!");
  }


  framebuffer_->Bind();
  if (framebuffer_->GetType() == FrameBuffer::kCubeMap) {
    for (int i = 0; i < 6; i++) {
      if (!cubemap_mask_[i]) continue;
      camera->transform->SetLocalEulerAngles(GetCubeCameraRotation(i));
      framebuffer_->BindCubemapFace(i);
      render_queue_.Draw(scene, *camera);
      framebuffer_->UnbindCubemapFace(i);
    }
  } else {
    render_queue_.Draw(scene, *camera);
  }
  framebuffer_->Unbind();

}
