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

struct MoveCameraAround : public Action {
  glm::vec3 target = glm::vec3(0);
  glm::vec2 radius = glm::vec2(3, 3);
  float elevation = 2;
  float angle = 90;        // starting/current angle
  float rotate_speed = 10; // degrees/second

  MoveCameraAround(std::shared_ptr<Transformation> transform,
                   const glm::vec3& target = glm::vec3(0),
                   const glm::vec2& radius = glm::vec2(3, 3),
                   float elevation = 2, 
                   float rotate_speed = 10)
    : Action(transform) {
    this->target = target;
    this->radius = radius;
    this->elevation = elevation;
    this->rotate_speed = rotate_speed;
  }

  void Update() override {
    angle += rotate_speed * GetTimer().GetTimeDelta();
    auto a = glm::radians(angle);
    glm::vec3 pos(radius.x * cos(a), elevation, radius.y * sin(a)); 
    transform->SetLocalPosition(pos);
    transform->SetLocalEulerAngles(20, 90-angle, 0); 
    // LookAt does not work because of different order of rotations
    // hardcoded target for now...
  }
};

int main(int argc, char* argv[]) {
  Scene scene;

  // Step 1. Initialize application.
  AppContext::Init(1280, 720, "Skybox cubemap [b3d]", Profile("3 3 core"));
  AppContext::Instance().display.ShowCursor(false);
  int width = AppContext::Instance().display.GetWidth();
  int height = AppContext::Instance().display.GetHeight();
  
  // Step 2. Setup RenderTarget.
  Cfg<RenderTarget>(scene.AddRenderTarget("rt.screen"))
    . Tags("onscreen")
    . Done();

  // Step 3. Compose the scene. 
  Cfg<Camera>(scene, "camera.main")
    . Perspective(60, (float)width/height, 1, 500)
    . Action<MoveCameraAround>(glm::vec3(0), glm::vec2(12), 1, 30)
    . Done();

  Cfg<Actor>(scene, "actor.plane")
    .Model("Assets/plane.dsm", "Assets/texture.mat")
    .Done();
  
  Cfg<Actor>(scene, "actor.knight")
    .Model("Assets/knight.dsm", "Assets/texture.mat")
    .Done();
  
  Cfg<Actor>(scene, "actor.skybox")
    . Model("Assets/blender_cube.dsm", "Assets/skybox_cubemap.mat")
    . Done();
  
  Cfg<Actor>(scene, "actor.fps.meter")
    . Action<FpsMeter>()
    . Done();

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
