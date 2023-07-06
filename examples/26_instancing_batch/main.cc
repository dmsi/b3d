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

  // Initialize application.
  AppContext::Init(1280, 720, "Batching actors - one draw call [b3d]", Profile("3 3 core"));
  AppContext::Instance().display.ShowCursor(false);
  int width = AppContext::Instance().display.GetWidth();
  int height = AppContext::Instance().display.GetHeight();

  // Setup main and shadowmap rendertargets
  Cfg<RenderTarget>(scene, "rt.screen", 2000)
    . Tags("onscreen")
    . Clear(.8, .8, .8, 1)
    . Done();
  
  // 
  // Batch, all in one draw call. 
  //
  int batch_size = 1000;
  auto batch = Cfg<StdBatch::Batch>(scene, "actor.batch", batch_size)
    . Model("Assets/arrow.dsm", "Assets/Materials/instance_batch.mat")
    . Done();

  for (int i = 0; i < batch_size; ++i) {
    float c = 1. *i/batch_size;
    float x = i - (int)batch_size/2;
    float a = 10.*i/2;
    float g = Math::Random();

    Cfg<StdBatch::Actor>(scene, "actor_in_batch." + std::to_string(i), batch)
      . EulerAngles (-90, 0, 0)
      . Position    (x, cos(a)/2, sin(a)/2)
      . Extra       (c, g, 1-c, 1) // using as color in the shader
      . Action<Rotator>(glm::vec3(Math::Random()*90, 0, 0))
      . Done();
  }
  
  // Main camera
  Cfg<Camera>(scene, "camera.main")
    . Perspective(60, (float)width/height, .1, 1000)
    . Position(0, 1, 25)
    . Action<FlyingCameraController>(5)
    . Done();
  
  // Fps meter.
  Cfg<Actor>(scene, "actor.fps.meter")
    . Action<FpsMeter>()
    . Done();
  
  // Main loop. Press ESC to exit.
  do {
    AppContext::BeginFrame();
    scene.Update();
    scene.Draw();
    AppContext::EndFrame();
  } while (AppContext::Running());

  // Cleanup and close the app.
  AppContext::Close();
  return 0;
}
