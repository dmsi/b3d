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
  auto SpawnActor = [&scene, &counter] () {
    const std::string obj = "Assets/unity_cube.dsm";
    const std::string mat = "Assets/texture.mat";
    auto actor = scene.Add<Actor>("actor." + obj + "#" + std::to_string(counter));
    actor->AddComponent<MeshRenderer>()->SetMaterial(MaterialLoader::Load(mat));
    actor->AddComponent<MeshFilter>()->SetMesh(MeshLoader::Load(obj));
    counter++;
    return actor;
  };

  // Step 1. Initialize application.
  AppContext::Init(1280, 720, "Orthographic camera [b3d]", Profile("3 3 core"));
  AppContext::Instance().display.ShowCursor(false);

  // Step 2. Setup RenderTarget and FrameBuffer.
  Cfg<RenderTarget>(scene, "rt.screen")
    . Tags("onscreen")
    . Clear(.4, .4, .4, 1)
    . Done();

  // Step 3. Conjure our cubes. 
  SpawnActor()->transform->SetLocalPosition(-1.5, 0,   0);
  SpawnActor()->transform->SetLocalPosition( 1.5, 0,   0);
  SpawnActor()->transform->SetLocalPosition(   0, 0, 1.5);
  
  auto actor = SpawnActor();
  actor->transform->SetLocalPosition(0, 1, 1.5);
  actor->transform->SetLocalScale(.5,1,.5);
  actor->AddAction<Rotator>()->rotation_speed = glm::vec3(0, 60, 0);

  // Step 4. FPS meter.
  scene.Add<Actor>("actor.fps_meter")->AddAction<FpsMeter>();
  
  // Step 6. Configure the main camera. 
  float w = 4, h = 4;
  auto camera = scene.Add<Camera>("camera.main");
  camera->SetOrtho(-w, h, w, -h, .1, 150.0);
  camera->transform->SetLocalPosition(0, 0, 5);

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
