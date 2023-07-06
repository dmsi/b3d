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

struct SetTerrainUniform: public Action {
  std::shared_ptr<Actor> plane;

  SetTerrainUniform(std::shared_ptr<Transformation> t, 
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

  // Step 3. Configuring water surface
  auto water = Cfg<Actor>(scene, "actor.water_surface")
    . Model("Assets/plane.dsm", "Assets/Materials/water_surf.mat")
    . Position(0,  20,   0)
    . Scale   (200, 1, 200)
    . Texture (0, reflection_tex)
    . Texture (1, refraction_tex)
    . Texture (2, refraction_depth)
    . Action<SetWaterSurfaceUniform>()
    . Done();

  // Step 3.1. Building terrain. 
  Cfg<Actor>(scene, "actor.terrain")
    . Material("Assets/Materials/terrain_flatshading_water_env.mat")
    . Action<TerrainGeneratorCustom>()
    . Action<SetTerrainUniform>(water)
    . Done();

  // Step 3.2. Adding skydome.
  Cfg<Actor>(scene, "actor.skydome")
    . Model("Assets/sphere.dsm", "Assets/Materials/skydome_water_env.mat")
    . Done();

  // Step 4. Add 'sun' light source.
  Cfg<Light>(scene, "light.sun", Light::kDirectional)
    . EulerAngles(-210, 90, 0)
    . Done();
  
  // Step 5. FPS meter
  Cfg<Actor>(scene, "actor.fps_meter")
    . Action<FpsMeter>()
    . Done();
  
  // Step 5.2. Add overlay screen with reflection texture.
  Cfg<Actor>(scene, "actor.overlay.reflections")
    . Action<MakeOverlayArea>(reflection_tex, MakeOverlayArea::kTop2, true)
    . Done();
  
  // Step 5.3. Add overlay screen with reflection texture.
  Cfg<Actor>(scene, "actor.overlay.refractions")
    . Action<MakeOverlayArea>(refraction_tex, MakeOverlayArea::kTop3, false)
    . Done();
  
  // Step 6. Configure the main camera. 
  auto camera = Cfg<Camera>(scene, "camera.main")
    . Perspective(60, (float)width/height, 1, 1500)
    . Position(0, 200, 20)
    . Action<FlyingCameraController>(150)
    . Done();

  // Step 6.1. Configure camera for reflections
  Cfg<Camera>(scene, "camera.main.reflection")
    . Action<ReflectCamera>(camera, water->transform->GetLocalPosition().y)
    . Done();
  
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
