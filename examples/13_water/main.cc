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

struct SetTerrainUniform: public Action {
  std::shared_ptr<Actor> plane;

  SetTerrainUniform(std::shared_ptr<Transformation> transform) 
    : Action(transform) {}

  void PreDraw() override {
    if (auto mtrl = transform->GetActor().GetComponent<Material>()) {
      auto pos = plane->transform->GetLocalPosition();
      glm::vec4 clip_plane(0, 1, 0, -pos.y);
      mtrl->SetUniform("clip_plane", clip_plane);
    }
  }
};

struct SetWaterSurfaceUniform: public Action {
  float dudv_speed = 0.03;
  float dudv_offset = 0;
  
  SetWaterSurfaceUniform(std::shared_ptr<Transformation> transform) 
    : Action(transform) {}

  void Update() override {
    dudv_offset += dudv_speed * GetTimer().GetTimeDelta();
    if (dudv_offset > 1) {
      dudv_offset = 0;
    }
  }

  void PreDraw() override {
    if (auto mtrl = transform->GetActor().GetComponent<Material>()) {
      mtrl->SetUniform("dudv_offset", dudv_offset);
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
  
  // Step 2. Setup screen render target 
  Cfg<RenderTarget>(scene, "rt.screen", 2)
    . Tags("onscreen")
    . Clear(.4, .4, .4, 1)
    . Done();
  
  // Step 2.1. Setup render target for reflections
  auto reflection_rt = Cfg<RenderTarget>(scene, "rt.reflection", 1) 
    . Camera("camera.main.reflection")
    . Tags("reflection")
    . Type(FrameBuffer::kTexture2D)
    . Resolution(width/2, height/2)
    . Layer(Layer::kColor, Layer::kReadWrite)
    . Layer(Layer::kDepth, Layer::kWrite)
    . Clear(.4, .4, .4, 1)
    . Done();
  
  // Step 2.2. Setup render target for reflections
  auto refraction_rt = Cfg<RenderTarget>(scene, "rt.refraction", 0) 
    . Camera("camera.main")
    . Tags("refraction")
    . Type(FrameBuffer::kTexture2D)
    . Resolution(width/2, height/2)
    . Layer(Layer::kColor, Layer::kReadWrite)
    . Layer(Layer::kDepth, Layer::kReadWrite)
    . Clear(.4, .4, .4, 1)
    . Done();

  // Step 2.3. Getting placeholders for reflection and refraction render
  // textures.
  auto reflection_tex = reflection_rt->GetLayerAsTexture(0, Layer::kColor);
  auto refraction_tex = refraction_rt->GetLayerAsTexture(0, Layer::kColor);
  auto refraction_depth = refraction_rt->GetLayerAsTexture(0, Layer::kDepth);

  auto water = scene.Add<Actor>("actor.water_surface");
  water->AddAction<SetWaterSurfaceUniform>();
  water->AddAction<LoadModel>()
    ->Setup(&scene, "Assets/plane.dsm", "Assets/water_surf.mat")
    ->Position(0, 20, 0)
    ->Scale(200, 1, 200)
    ->Done();
  if (auto m = water->GetComponent<MeshRenderer>()->GetMaterial()) {
    m->SetTexture(0, reflection_tex);
    m->SetTexture(1, refraction_tex);
    m->SetTexture(2, refraction_depth);
  }

  // Step 3. Building terrain. 
  auto terrain = scene.Add<Actor>("actor.terrain");
  terrain->AddAction<TerrainGeneratorCustom>()
    ->material = "Assets/terrain_flatshading_water_env.mat";
  terrain->AddAction<SetTerrainUniform>()->plane = water;

  // Step 3.1. Adding skydome.
  scene.Add<Actor>("actor.skydome")
    ->AddAction<LoadModel>()
    ->Setup(&scene, "Assets/sphere.dsm", "Assets/skydome_water_env.mat");

  // Step 4. Add 'sun' light source.
  std::shared_ptr<Light> sun = scene.Add<Light>("light.sun", Light::kDirectional);
  sun->transform->SetLocalPosition(0, 5, 5);
  sun->transform->SetLocalEulerAngles(-210, 90, 0);
  
  // Step 5. FPS meter
  scene.Add<Actor>("actor.fps_meter")->AddAction<FpsMeter>();
  
  // Step 5.2. Add overlay screen with reflection texture.
  scene.Add<Actor>("actor.overlay.reflections")
    ->AddAction<MakeOverlayArea>(reflection_tex, 
                                 MakeOverlayArea::kTop2,
                                 true);
  
  // Step 5.3. Add overlay screen with reflection texture.
  scene.Add<Actor>("actor.overlay.refractions")
    ->AddAction<MakeOverlayArea>(refraction_tex, 
                                 MakeOverlayArea::kTop3,
                                 false);
  
  // Step 6. Configure the main camera. 
  auto camera = scene.Add<Camera>("camera.main");
  camera->SetPerspective(60, 1.0f*width/height, 1, 1500.0);
  camera->transform->SetLocalPosition(0, 200, 20);
  camera->AddAction<FlyingCameraController>()->moving_speed = 150;

  // Step 6.1. Configure camera for reflections
  scene.Add<Camera>("camera.main.reflection")
    ->AddAction<ReflectCamera>()
    ->Setup(camera, water->transform->GetLocalPosition().y);
  
  // Step 7. Main loop. Press ESC to exit.
  do {
    AppContext::BeginFrame();
    scene.Update();
    scene.Draw();
    AppContext::EndFrame();
  } while (AppContext::Running());

  // Step 8. Cleanup and close the app.
  AppContext::Close();
  return 0;
}
