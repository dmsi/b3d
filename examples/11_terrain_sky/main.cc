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
  AppContext::Init(1280, 720, "Terrain and skydome [b3d]", Profile("3 3 core"));
  AppContext::Instance().display.ShowCursor(false);
  int width = AppContext::Instance().display.GetWidth();
  int height = AppContext::Instance().display.GetHeight();
  
  // Step 2. Setup RenderTarget and FrameBuffer.
  Cfg<RenderTarget>(scene, "rt.screen", 2000)
    . Tags("onscreen")
    . Clear(.4, .4, .4, 1)
    . Done();

  // Step 3. Add the terrain and skydome.
  scene.Add<Actor>("actor.terrain")->AddAction<TerrainLoader>();
  SpawnActor("Assets/sphere.dsm", "Assets/skydome.mat");
  
  // Step 4. Add 'sun' light source and its debug visualisation. 
  std::shared_ptr<Light> sun = scene.Add<Light>("light.sun", Light::kDirectional);
  sun->transform->SetLocalPosition(0, 5, 5);
  sun->transform->SetLocalEulerAngles(210, 90, 0);
  sun->AddAction<Rotator>()->rotation_speed = glm::vec3(5, 0, 0);
  
  // Make it a child of the sun to inherit the transformations 
  auto debug_light = SpawnActor("Assets/arrow.dsm", "Assets/lamp.mat");
  debug_light->transform->SetLocalScale(3, 3, 3);
  Transformation::SetParent(sun->transform, debug_light->transform);
  
  // Step 5. Configure the main camera. 
  auto camera = scene.Add<Camera>("camera.main");
  camera->SetPerspective(60, 1.0f*width/height, 1, 1500.0);
  camera->transform->SetLocalPosition(0, 9, 20);
  camera->transform->SetLocalEulerAngles(-30, 0, 0);
  camera->AddAction<FlyingCameraController>()->moving_speed = 5;
  
  // Step 6. Main loop. Press ESC to exit.
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
