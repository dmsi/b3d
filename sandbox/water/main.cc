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
#include "my/all.h"
#include "terraingenerator_custom.h"

struct SetOverlayUniform: public Action {
  glm::vec4 color;
  bool      invert_uv;

  SetOverlayUniform(std::shared_ptr<Transformation> transform) 
    : Action(transform), color(1, 1, 1, 1), invert_uv(false) {}

  void PreDraw() override {
    if (auto mtrl = transform->GetActor().GetComponent<Material>()) {
      mtrl->SetUniform("Color", color);
      int int_invert_uv = invert_uv;
      mtrl->SetUniform("Invert_uv", int_invert_uv);
    }
  }

  void Setup(const glm::vec4& color, bool invert_uv) {
    this->color = color;
    this->invert_uv = invert_uv;
  }
};

class SetClipPlaneUniform: public Action {
 public:
  std::shared_ptr<Actor> plane;
  SetClipPlaneUniform(std::shared_ptr<Transformation> t, 
                      std::shared_ptr<Actor> p) 
   : Action(t), plane(p) {}

  void PreDraw() override {
    if (auto mtrl = transform->GetActor().GetComponent<Material>()) {
      auto pos = plane->transform->GetLocalPosition();
      glm::vec4 clip_plane(0, 1, 0, -pos.y);
      mtrl->SetUniform("clip_plane", clip_plane);
    }
  }
};

struct SetWaterSurfaceUniform: public Action {
  float wave_speed = 0.03;
  float wave_moving_factor = 0;
  float uv_tiling = 30;
  
  SetWaterSurfaceUniform(std::shared_ptr<Transformation> transform) 
    : Action(transform) {}

  void Update() override {
    wave_moving_factor += wave_speed * GetTimer().GetTimeDelta();
    if (wave_moving_factor > 1) {
      wave_moving_factor = 0;
    }
  }

  void PreDraw() override {
    if (auto mtrl = transform->GetActor().GetComponent<Material>()) {
      mtrl->SetUniform("dudv_offset", wave_moving_factor);
      mtrl->SetUniform("uv_tiling", uv_tiling);
    }
  }
};

int main(int argc, char* argv[]) {
  Scene scene;
  
  // Step 1. Initialize application.
  AppContext::Init(1280, 720, "Water [b3d]", Profile("3 3 core"));
  AppContext::Instance().display.ShowCursor(false);
  int width = AppContext::Instance().display.GetWidth();
  int height = AppContext::Instance().display.GetHeight();
  
  // Setup render targets 
  Cfg<RenderTarget>(scene, "rt.screen", 1000)
    . Tags("onscreen")
    . Clear(.4, .4, .4, 1)
    . Done();
  
  int w = width/2, h = height/2;
  auto reflection_tex = Cfg<RenderTarget>(scene, "rt.reflection", 100)
    . Camera("camera.main.reflection")
    . Tags("reflection")
    . Type(FrameBuffer::kTexture2D)
    . Resolution(w, h)
    . Layer(Layer::kColor, Layer::kReadWrite)
    . Layer(Layer::kDepth, Layer::kWrite)
    . Clear(.4, .4, .4, 1)
    . Done()
    ->GetLayerAsTexture(0, Layer::kColor);
  
  auto refraction = Cfg<RenderTarget>(scene, "rt.refraction", 99)
    . Camera("camera.main")
    . Tags("refraction")
    . Type(FrameBuffer::kTexture2D)
    . Resolution(w, h)
    . Layer(Layer::kColor, Layer::kReadWrite)
    . Layer(Layer::kDepth, Layer::kReadWrite)
    . Clear(.4, .4, .4, 1)
    . Done();
  auto refraction_tex = refraction->GetLayerAsTexture(0, Layer::kColor);
  auto refraction_depth = refraction->GetLayerAsTexture(0, Layer::kDepth);
  
  // Water surface
  auto water = Cfg<Actor>(scene, "actor.seasurface")
    . Model("Assets/plane.dsm", "Assets/water_surf.mat")
    . Position(0,  20, 0)
    . Scale   (200, 0, 200)
    . Action<SetWaterSurfaceUniform>()
    . Texture(0, reflection_tex)
    . Texture(1, refraction_tex)
    . Texture(2, refraction_depth)
    . Done();

  // Add the terrain and skydome.
  auto terrain = Cfg<Actor>(scene, "actor.terrain")
    . Action<TerrainGeneratorCustom>("Assets/terrain_flatshading_water_env.mat")
    . Action<SetClipPlaneUniform>(water)
    . Done();

  Cfg<Actor>(scene, "actor.skydome")
    . Model("Assets/sphere.dsm", "Assets/skydome_water_env.mat")
    . Done();

  // Add 'sun' light source and its debug visualisation. 
  auto sun = Cfg<Light>(scene, "actor.sun", Light::kDirectional)
    . EulerAngles(-210, 90, 0)
    . Done();
  
  
  // Configure the main camera. 
  auto main_camera = Cfg<Camera>(scene, "camera.main")
    . Perspective(60, (float)width/height, 1, 1500)
    . Position(0, 200, 20)
    . Action<FlyingCameraController>(150)
    . Done();

  Cfg<Camera>(scene, "camera.main.reflection")
    . Action<ReflectCamera>(main_camera, water->transform->GetLocalPosition().y)
    . Done();
  
  Cfg<Actor>(scene, "actor.fps_meter")
    . Action<FpsMeter>()
    . Done();
  
  // Step 6. Main loop. Press ESC to exit.
  //glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
  do {
    AppContext::BeginFrame();
    scene.Update();
    scene.Draw();
    AppContext::EndFrame();
  } while (AppContext::Running());

  // Step 7. Cleanup and close the app.
  AppContext::Close();
  return 0;
}
