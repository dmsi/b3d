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

#include <iostream>
#include <memory>
#include <limits>

#include "b3d.h"
#include "myactions/all.h"
#include "myhelpers/all.h"

struct Logger: public Action {
  Logger(std::shared_ptr<Transformation> t) : Action(t) {}
  void Update() override {
  }
};

struct ShadorReceiverUniform: public Action {
  using CamPtr = std::shared_ptr<Camera>;

  CamPtr shadowmap_camera;

  ShadorReceiverUniform(std::shared_ptr<Transformation> t, CamPtr c) 
    : Action(t), shadowmap_camera(c) {}

  void PreDraw() override {
    glm::mat4 view;
    glm::mat4 proj;
    glm::mat4 bias (
      0.5, 0.0, 0.0, 0.0,
      0.0, 0.5, 0.0, 0.0,
      0.0, 0.0, 0.5, 0.0,
      0.5, 0.5, 0.5, 1.0
    );

    // unclear moment, what about the shadowmap_camera model matrix????
    // is is already included to the view? - yes... ?
    shadowmap_camera->GetViewMatrix(view);
    shadowmap_camera->GetProjectionMatrix(proj);

    if (auto mtrl = transform->GetActor().GetComponent<Material>()) {
      mtrl->SetUniform("depth_bias_pvm", bias * proj * view);
    }
  }
};


int main(int argc, char* argv[]) {
  Scene scene;
  //using TexLs = std::vector<std::shared_ptr<Texture>>;
  using T = std::vector<std::string>;
  using glm::vec2;
  using glm::vec3;

  // Step 1. Initialize application.
  AppContext::Init(1280, 720, "Shadow mapping [b3d]", Profile("3 3 core"));
  AppContext::Instance().display.ShowCursor(false);
  int width = AppContext::Instance().display.GetWidth();
  int height = AppContext::Instance().display.GetHeight();
  
  // Step 2. Setup RenderTarget.
  Cfg<RenderTarget>(scene, "rt.screen", 2000)
    . Tags("onscreen")
    . Clear(.8, .4, .4, 1)
    . Done();
    
  auto shadow_map = Cfg<RenderTarget>(scene, "rt.shadowmap", 0) 
      . Camera("camera.shadowmap")
      . Tags("shadow-caster")
      . Type(FrameBuffer::kTexture2D)
      . Resolution(width, height)
      . Layer(Layer::kColor, Layer::kReadWrite)
      . Layer(Layer::kDepth, Layer::kReadWrite, Texture::kFilterPoint)
      . Clear(.4, .4, .4, 1)
      . Done()
      ->GetLayerAsTexture(0, Layer::kDepth);
  
  Cfg<Actor>(scene, "actor.display.shadowmap")
    . Model("Assets/screen.dsm", "sandbox/shadowmap/overlay_shadowmap.mat")
    . Tags(0, T{"onscreen"})
    . Texture(0, shadow_map)
    . Scale   (.25, .25, 0)
    . Position(.75, .75, 0)
    . Done();

  // Step 3. Compose the scene. 
  Cfg<Camera>(scene, "camera.main")
    . Perspective(60, (float)width/height, .1, 500)
    . Position(0, 2, 4)
    . EulerAngles(-30, 0, 0)
    . Action<FlyingCameraController>(5)
    . Done();


  auto sun = Cfg<Light>(scene, "light.sun")
    . Position(-5, 1, -5)
    . EulerAngles(20, 225, 0)
    . Color(1, 1, 1, 1) 
    . Action<Logger>()
    . Done();

  auto sun_pos = -10.0f * sun->GetDirection();
  auto sun_rot = sun->transform->GetLocalEulerAngles();
  sun_rot.y = sun_rot.y - 180;
  sun_rot.x = -sun_rot.x;
  int w = 7, h = 7, d = 10;
  auto cam_shadowmap = Cfg<Camera>(scene, "camera.shadowmap")
    //. Perspective(60, (float)width/height, .1, 500)
    . Ortho(-w, h, w, -h, -d, d*2)
    . Position(sun_pos)
    . EulerAngles(sun_rot)
    . Done();

  auto arrow = Cfg<Actor>(scene, "actor.arrow")
    . Model("Assets/arrow.dsm", "Assets/lamp2.mat")
    . Parent(sun)
    . Done();

  Cfg<Actor>(scene, "actor.floor")
    . Model("Assets/plane.dsm", "sandbox/shadowmap/shadow_receiver.mat")
    . Texture(0, shadow_map)
    . Action<ShadorReceiverUniform>(cam_shadowmap)
    . Done();

  Cfg<Actor>(scene, "actor.caster1")
    . Model("Assets/sphere.dsm", "sandbox/shadowmap/shadow_caster.mat")
    . Position(0, .5, 0)
    . Done();

  Cfg<Actor>(scene, "actor.skydome")
    . Model("Assets/sphere.dsm", "Assets/skydome.mat")
    . Done();

  Cfg<Actor>(scene, "actor.fps.meter")
    . Action<FpsMeter>()
    . Done();

  // Step 4. Main loop. Press ESC to exit.
  do {
    AppContext::BeginFrame();
    scene.Update();
    scene.Draw();
    AppContext::EndFrame();
  } while (AppContext::Running());

  // Step 5. Cleanup and close the app.
  AppContext::Close();
  return 0;
}
