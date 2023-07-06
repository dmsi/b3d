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

  // Step 1. Initialize application.
  AppContext::Init(1280, 720, "Flatshading terrain [b3d]", Profile("3 3 core"));
  AppContext::Instance().display.ShowCursor(false);
  int width = AppContext::Instance().display.GetWidth();
  int height = AppContext::Instance().display.GetHeight();

  // Step 2. Build the terrain.
  scene.Add<Actor>("actor.terrain")->AddAction<TerrainLoader>();

  // Step 3. Set directional light (the sun).
  scene.Add<Light>("light.sun", Light::kDirectional)
    ->transform->SetLocalEulerAngles(-210, 0, 0);

  // Step 4. Set main camera.
  std::shared_ptr<Camera> camera = scene.Add<Camera>("camera.main");
  camera->SetPerspective(60, 1.0f*width/height, 1, 2000);
  camera->transform->SetLocalPosition(0, 70, 3);
  camera->transform->SetLocalEulerAngles(-30, 0, 0);
  camera->AddAction<FlyingCameraController>()->moving_speed = 30;

  // Step 5. Setup FPS meter.
  scene.Add<Actor>("actor.fps_meter")->AddAction<FpsMeter>();

  // Step 6. Setup RenderTarget and FrameBuffer.
  Cfg<RenderTarget>(scene, "rt.screen", 2000)
    . Tags("onscreen")
    . Clear(.4, .4, .4, 1)
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
