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
  using glm::vec3;
  Scene scene;

  // Initialize application.
  AppContext::Init(1280, 720, "Atmosphere rendering [b3d]", Profile("3 3 core"));
  AppContext::Instance().display.ShowCursor(false);
  int width = AppContext::Instance().display.GetWidth();
  int height = AppContext::Instance().display.GetHeight();

  // Setup main and shadowmap rendertargets
  Cfg<RenderTarget>(scene, "rt.screen", 2000)
    . Tags("onscreen")
    . Clear(.8, .8, .8, 1)
    . Done();

  auto atmosphere_tex = Cfg<RenderTarget>(scene, "rt.atmosphere", 100)
    . Tags("atmosphere")
    . Type(FrameBuffer::kTexture2D)
    . Resolution(width, height)
    . Layer(Layer::kColor, Layer::kReadWrite)
    . Done()
    ->GetLayerAsTexture(0, Layer::kColor);

  Cfg<Actor>(scene, "actor.terrain")
    . Material("assets/materials/terrain_atmosphere.mat")
    . Texture(0, atmosphere_tex)
    . Action<TerrainGenerator>()
    . Done();

  Cfg<Actor>(scene, "actor.skydome")
    . Model("assets/models/sphere.dsm", "assets/materials/skydome_perez.mat")
    . Done();

  Cfg<Light>(scene, "light.sun", Light::kDirectional)
    . EulerAngles(20, 0, 0)
    . Action<Rotator>(glm::vec3(5, 0, 0))
    . Done();

  // Main camera
  Cfg<Camera>(scene, "camera.main")
    . Perspective(60, (float)width/height, 1, 2500)
    . Position(0, 200, 4)
    . Action<FlyingCameraController>(200)
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
