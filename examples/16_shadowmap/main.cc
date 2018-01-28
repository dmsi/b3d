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

#include "b3d.h"
#include "myactions/all.h"
#include "myhelpers/all.h"

// Sets various of uniforms for shadow receivers
// The most important is bias * proj * view matrix of the shadow map camera
// in order to calculate uv coordinates for sampling the shadowmap.
struct ShadowUniform : public Action {
  using CamPtr = std::shared_ptr<Camera>;
  using LhtPtr = std::shared_ptr<Light>;

  LhtPtr light;
  CamPtr shadowmap_camera;

  ShadowUniform(std::shared_ptr<Transformation> t, LhtPtr l, CamPtr c) 
    : Action(t), light(l), shadowmap_camera(c) {}

  void PreDraw() override {
    glm::mat4 view;
    glm::mat4 proj;
    glm::mat4 bias (
      0.5, 0.0, 0.0, 0.0,
      0.0, 0.5, 0.0, 0.0,
      0.0, 0.0, 0.5, 0.0,
      0.5, 0.5, 0.5, 1.0
    );

    shadowmap_camera->GetViewMatrix(view);
    shadowmap_camera->GetProjectionMatrix(proj);

    if (auto mtrl = transform->GetActor().GetComponent<Material>()) {
      mtrl->SetUniform("depth_bias_pvm", bias * proj * view);
      mtrl->SetUniform("light.direction", light->GetDirection());
      mtrl->SetUniform("light.color", light->GetColor());
    }
  }
};

// Tracking the directional light source and updating shadowcamera position
// based on it.
struct ShadowMapCameraController : public Action {
  using LhtPtr = std::shared_ptr<Light>;

  LhtPtr sun;
  float  distance = 10.0f; // How far the sun is

  ShadowMapCameraController(std::shared_ptr<Transformation> t, LhtPtr l, float d = 10.0f)
    : Action(t), sun(l), distance(d) {}

  void Update() override {
    auto pos = -distance * sun->GetDirection();
    auto rot = sun->transform->GetLocalEulerAngles();
    rot.y = rot.y - 180;
    rot.x = -rot.x;

    transform->SetLocalPosition(pos);
    transform->SetLocalEulerAngles(rot);
  }
};

// Rotator for our sun. Does not allow it to go above the horizont.
struct SpecialRotator : public Rotator {
  SpecialRotator(std::shared_ptr<Transformation> t, const glm::vec3& rot_speed)
    : Rotator(t, rot_speed) {}

  void Update() override {
    auto e = transform->GetLocalEulerAngles();
    float bias = 10;
    if (e.x < bias || e.x > 180-bias) {
      rotation_speed.x = -rotation_speed.x;
    }
    Rotator::Update();
  }
};

int main(int argc, char* argv[]) {
  Scene scene;
  using T = std::vector<std::string>;
  using glm::vec3;

  // Initialize application.
  AppContext::Init(1280, 720, "Shadow mapping [b3d]", Profile("3 3 core"));
  AppContext::Instance().display.ShowCursor(false);
  int width = AppContext::Instance().display.GetWidth();
  int height = AppContext::Instance().display.GetHeight();
  
  // Setup main and shadowmap rendertargets
  Cfg<RenderTarget>(scene, "rt.screen", 2000)
    . Tags("onscreen")
    . Clear(.4, .4, .4, 1)
    . Done();
    
  auto shadow_tex = Cfg<RenderTarget>(scene, "rt.shadowmap", 0) 
    . Camera("camera.shadowmap")
    . Tags("shadow-caster")
    . Type(FrameBuffer::kTexture2D)
    . Resolution(width, height)
    . Layer(Layer::kDepth, Layer::kReadWrite)
    . Done()
    ->GetLayerAsTexture(0, Layer::kDepth);
  
  // Compose the scene. 
  Cfg<Camera>(scene, "camera.main")
    . Perspective(60, (float)width/height, .1, 500)
    . Position(-10, 6, 0)
    . EulerAngles(-30, -90, 0)
    . Action<FlyingCameraController>(5)
    . Done();

  auto sun = Cfg<Light>(scene, "light.sun")
    . EulerAngles(40, -225, 0)
    . Color(1, 1, 1, 1) 
    . Action<SpecialRotator>(vec3(5, 2, 0))
    . Done();

  int w = 5, h = 5, d = 10; // the shadowcube, bigger the cube => lower the quality
  auto cam_shadowmap = Cfg<Camera>(scene, "camera.shadowmap")
    . Ortho(-w, h, w, -h, -d, d*2)
    . Action<ShadowMapCameraController>(sun, 10)
    . Done();

  Cfg<Actor>(scene, "actor.floor")
    . Model("Assets/plane.dsm", "Assets/shadow_caster_receiver_dirlight.mat")
    . Texture(0, shadow_tex)
    . Action<ShadowUniform>(sun, cam_shadowmap)
    . Done();

  Cfg<Actor>(scene, "actor.1")
    . Model("Assets/torus.dsm", "Assets/shadow_caster_receiver_dirlight.mat")
    . Texture(0, shadow_tex)
    . Position(0, 1.0, 0)
    . Scale(.8, .8, .8)
    . Action<Rotator>(vec3(30, 60, 30))
    . Action<ShadowUniform>(sun, cam_shadowmap)
    . Done();
  
  Cfg<Actor>(scene, "actor.2")
    . Model("Assets/unity_cube.dsm", "Assets/shadow_caster_receiver_dirlight.mat")
    . Texture(0, shadow_tex)
    . Position(-2, 0.5, -2)
    . Action<Rotator>(vec3(0, -30, 0))
    . Action<ShadowUniform>(sun, cam_shadowmap)
    . Done();

  Cfg<Actor>(scene, "actor.3")
    . Model("Assets/knight.dsm", "Assets/shadow_caster_receiver_dirlight.mat")
    . Texture(0, shadow_tex)
    . Position(2, 0, 2)
    . Action<Rotator>(vec3(0, 45, 0))
    . Action<ShadowUniform>(sun, cam_shadowmap)
    . Done();
  
  // Overlay display for the shadowmap and FPS
  Cfg<Actor>(scene, "actor.display.shadowmap")
    . Model("Assets/screen.dsm", "Assets/overlay_texture_border.mat")
    . Tags(0, T{"onscreen"})
    . Texture(0, shadow_tex)
    . Scale   (.25, .25, 0)
    . Position(.75, .75, 0)
    . Done();

  Cfg<Actor>(scene, "actor.fps.meter")
    . Action<FpsMeter>()
    . Done();

  // Main loop. Press ESC to exit.
  do {
    AppContext::BeginFrame();
    scene.Update();
    scene.Draw();
    AppContext::EndFrame();
  } while (AppContext::Running());

  // Cleanup and close the app.
  AppContext::Close();
  return 0;
}
