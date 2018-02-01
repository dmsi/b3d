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
#include "myhelpers/all.h"
#include "myactions/all.h"

int main(int argc, char* argv[]) {
  //using T = std::vector<std::string>;
  Scene scene;

  // Step 1. Initialize application.
  AppContext::Init(1280, 720, "Water [b3d]", Profile("3 3 core"));
  AppContext::Instance().display.ShowCursor(false);
  int width = AppContext::Instance().display.GetWidth();
  int height = AppContext::Instance().display.GetHeight();
  
  
  // Cfg<Camera> ... Cfg<Light> ???
  // Step 5. Configure the main camera. 
  auto camera = scene.Add<Camera>("camera.main");
  camera->SetPerspective(60, 1.0f*width/height, .1, 150);
  //camera->SetPerspective(60, 1, .1, 150);
  camera->transform->SetLocalPosition(0, 0, 5);
  //camera->AddAction<FlyingCameraController>()->moving_speed = 150;
  camera->AddAction<FlyingCameraController>()->moving_speed = 5;

  // Step 2. Setup RenderTarget and FrameBuffer.
  Cfg<RenderTarget>(scene.AddRenderTarget("rt.screen", 1000))
    . Tags("onscreen")
    . Clear(.8, .8, .8, .8)
    . Done();

  Cfg<Actor>(scene, "actor...axes")
    . Action<CoordinateAxes>(scene, 10, camera, .35)
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
