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
  SetClipPlaneUniform(std::shared_ptr<Transformation> transform) : Action(transform) {}

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
  int   counter = 0;
  
  // Helper function for spawning 3D actors
  auto SpawnActor = [&scene, &counter] (const std::string& obj, const std::string& mat) {
    auto actor = scene.Add<Actor>("actor." + obj + "#" + std::to_string(counter));
    actor->AddComponent<MeshRenderer>()->SetMaterial(MaterialLoader::Load(mat));
    actor->AddComponent<MeshFilter>()->SetMesh(MeshLoader::Load(obj));

    counter++;
    return actor;
  };

  // Step 1. Initialize application.
  AppContext::Init(1280, 720, "Water [b3d]", Profile("3 3 core"));
  AppContext::Instance().display.ShowCursor(false);
  int width = AppContext::Instance().display.GetWidth();
  int height = AppContext::Instance().display.GetHeight();
  

  auto water = SpawnActor("Assets/plane.dsm", "Assets/water_surf.mat");
  water->transform->SetLocalPosition(0, 20, 0);
  water->transform->SetLocalScale(200, 0, 200);
  water->AddAction<SetWaterSurfaceUniform>();

  // Step 3. Add the terrain and skydome.
  auto terrain = scene.Add<Actor>("actor.terrain");
  auto gen = terrain->AddAction<TerrainGeneratorCustom>();
  gen->material = "Assets/terrain_flatshading_water_env.mat";
  terrain->AddAction<SetClipPlaneUniform>()->plane = water;
  SpawnActor("Assets/sphere.dsm", "Assets/skydome_water_env.mat");

  // Step 4. Add 'sun' light source and its debug visualisation. 
  std::shared_ptr<Light> sun = scene.Add<Light>("light.sun");
  sun->transform->SetLocalPosition(0, 5, 5);
  sun->transform->SetLocalEulerAngles(-210, 90, 0);
  
  // Make it a child of the sun to inherit the transformations 
  auto debug_light = SpawnActor("Assets/arrow.dsm", "Assets/lamp.mat");
  debug_light->transform->SetLocalScale(3, 3, 3);
  Transformation::SetParent(sun->transform, debug_light->transform);
  
  scene.Add<Actor>("actor.fps_meter")->AddAction<FpsMeter>();
  
  // Step 5. Configure the main camera. 
  auto camera = scene.Add<Camera>("camera.main");
  camera->SetPerspective(60, 1.0f*width/height, 1, 1500.0);
  camera->transform->SetLocalPosition(0, 200, 20);
  //camera->transform->SetLocalEulerAngles(-30, 0, 0);
  //camera->AddAction<Rotator>()->rotation_speed = glm::vec3(10, 0, 0);
  camera->AddAction<FlyingCameraController>()->moving_speed = 150;
  //camera->AddAction<FlyingCameraController>()->moving_speed = 5;
  //
  //camera->transform->SetLocalPosition(0, .1, 5);
  //glm::vec3 dir = camera->transform->GetForward(); 
  //camera->transform->LookAt(dir, glm::vec3(1,1,0));
  //camera->transform->LookAt(glm::vec3(0, 0, 0), glm::vec3(0,1,0));

  auto reflected_camera = scene.Add<Camera>("camera.main.reflection");
  reflected_camera->AddAction<ReflectCamera>()
    ->Setup(camera, water->transform->GetLocalPosition().y);
  reflected_camera->CopyProjection(camera);
  
  // Step 2. Setup RenderTarget and FrameBuffer.
  // Range from 100 - to inf
  // Reserving 0 - 99 for reflections and refractions
  Cfg<RenderTarget>(scene, "rt.screen", 1000)
    . Tags("onscreen")
    . Clear(.4, .4, .4, 1)
    . Done();
  
  int w = width/2;
  int h = height/2;
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

  // Add overlay screen with reflection texture
  //scene.Add<Actor>("action.overlay1")
  //  ->AddAction<MakeOverlayArea>()
  //  ->Setup(&scene, reflection_tex, MakeOverlayArea::kTop2, true);
  
  //scene.Add<Actor>("action.overlay2")
  //  ->AddAction<MakeOverlayArea>()
  //  ->Setup(&scene, refraction_tex, MakeOverlayArea::kTop3, false);

  // Apply reflection rendered texture to the water material
  water->GetComponent<MeshRenderer>()->GetMaterial()->SetTexture(0, reflection_tex);
  water->GetComponent<MeshRenderer>()->GetMaterial()->SetTexture(1, refraction_tex);
  water->GetComponent<MeshRenderer>()->GetMaterial()->SetTexture(2, refraction_depth);
  
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
