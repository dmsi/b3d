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

#include "b3d.h"
#include "myactions/all.h"
#include "myhelpers/all.h"

class SineWaiver: public Action {
 public:
  float amplitude;
  float frequency;
  float phase;
  SineWaiver(std::shared_ptr<Transformation> transform) : Action(transform) {
    amplitude = 1;
    frequency = 1;
    phase = 0;
  }

  void Update() override {
    float alt = amplitude * sinf(GetTimer().GetTime() * frequency + phase);
    transform->SetLocalPosition(transform->GetUp() * alt);
  }

  void Setup(float amplitude, float frequency, float phase) {
    this->amplitude = amplitude;
    this->frequency = frequency;
    this->phase = phase;
  }
};

class SetClipPlaneUniform: public Action {
 public:
  std::shared_ptr<Actor> plane0;
  std::shared_ptr<Actor> plane1;
  SetClipPlaneUniform(std::shared_ptr<Transformation> transform) : Action(transform) {}

  void PreDraw() override {
    if (auto mtrl = transform->GetActor().GetComponent<Material>()) {

      int nr = 0;
      for (const auto& a: {plane0, plane1}) {
        if (a) {
          auto up  = a->transform->GetUp();
          auto pos = a->transform->GetLocalPosition();
          float sign = Math::Sign(glm::dot(pos, up));
          float d = sign * glm::length(a->transform->GetLocalPosition()) - 0.01;
          glm::vec4 clip_plane(up.x, up.y, up.z, -d);
          mtrl->SetUniform("clip_plane" + std::to_string(nr), clip_plane);
        }
        nr++;
      }
    }
  }

  void Setup(std::shared_ptr<Actor> plane0, std::shared_ptr<Actor> plane1) {
    this->plane0 = plane0;
    this->plane1 = plane1;
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
  AppContext::Init(1280, 720, "Clipping plane [b3d]", Profile("3 3 core"));
  AppContext::Instance().display.ShowCursor(false);
  int width = AppContext::Instance().display.GetWidth();
  int height = AppContext::Instance().display.GetHeight();
  
  // Step 2. Setup RenderTarget and FrameBuffer.
  Cfg<RenderTarget>(scene, "rt.screen")
    . Tags("onscreen")
    . Clear(.4, .4, .4, 1)
    . Done();

  // Step 3. Setup the scene

  // Step 3.1. Configure horizontal clipping plane 
  // Plane equasion Nx + Ny + Nz - D = 0, where Nx, Ny, Nz is plane normal
  // and D is offset.
  // For instance horizontal clipping plane at elevation = 2
  // (0, 1, 0, -2) => discards all below 2
  // We are using SineWaiver as an action to move the plane along its normal
  auto plane0 = SpawnActor("Assets/plane.dsm", "Assets/clip_plane_dbg.mat");
  plane0->transform->SetLocalScale(.5, 1, .5);
  plane0->AddAction<SineWaiver>();

  // Step 3.2. Configure vertical clipping plane.
  auto plane1 = SpawnActor("Assets/plane.dsm", "Assets/clip_plane_dbg.mat");
  plane1->transform->SetLocalEulerAngles(0, 0, 90);
  plane1->transform->SetLocalPosition(0.5, 0, 0);
  plane1->transform->SetLocalScale(.5, 1, .5);
  plane1->AddAction<SineWaiver>()->Setup(2, 1, 0);

  // Step 3.3. Setup our cubes which supposed to be clipped.
  // We are using SetClipPlaneUniform action here in order to pass plane
  // parameters to the vertex shader.
  auto actor = SpawnActor("Assets/unity_cube.dsm", "Assets/clip_plane_demo.mat");
  actor->transform->SetLocalPosition(-1.5, 0, 0);
  actor->AddAction<SetClipPlaneUniform>()->Setup(plane0, plane1);
  
  actor = SpawnActor("Assets/unity_cube.dsm", "Assets/clip_plane_demo.mat");
  actor->transform->SetLocalPosition(1.5, 0, 0);
  actor->AddAction<SetClipPlaneUniform>()->Setup(plane0, plane1);
  
  actor = SpawnActor("Assets/unity_cube.dsm", "Assets/clip_plane_demo.mat");
  actor->transform->SetLocalPosition(0, 0, 1.5);
  actor->AddAction<SetClipPlaneUniform>()->Setup(plane0, plane1);
  
  actor = SpawnActor("Assets/knight.dsm", "Assets/clip_plane_demo.mat");
  actor->transform->SetLocalPosition(0, .5, 1.5);
  actor->transform->SetLocalScale(.5, .5, .5);
  actor->AddAction<Rotator>()->rotation_speed = glm::vec3(0, 60, 0);
  actor->AddAction<SetClipPlaneUniform>()->Setup(plane0, plane1);

  // Step 3.4. FPS meter.
  scene.Add<Actor>("actor.fps_meter")->AddAction<FpsMeter>();
  
  // Step 4. Configure the main camera. 
  auto camera = scene.Add<Camera>("camera.main");
  camera->SetPerspective(60, 1.0f*width/height, .1, 150.0);
  camera->transform->SetLocalPosition(0, 0, 5);

  // Step 5. Main loop. Press ESC to exit.
  do {
    AppContext::BeginFrame();
    scene.Update();
    scene.Draw();
    AppContext::EndFrame();
  } while (AppContext::Running());

  // Step 6. Cleanup and close the app.
  AppContext::Close();
  return 0;
}
