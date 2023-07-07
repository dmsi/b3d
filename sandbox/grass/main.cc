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

// 4 vertex patch -1 +1 in xy plane
auto Patch(float w, float h) {
  using glm::vec3;
  auto mesh = std::make_shared<Mesh>();

  mesh->vertices = {
    vec3(-0.5 * w,  0,  0),
    vec3( 0.5 * w,  0,  0),
    vec3(-0.5 * w,  h,  0),
    vec3( 0.5 * w,  h,  0),
  };

  return mesh;
}

int main(int argc, char* argv[]) {
  using glm::vec3;
  Scene scene;

  // Initialize application.
  AppContext::Init(1280, 720, "Sandbox [b3d]", Profile("4 0 core"));
  AppContext::Instance().display.ShowCursor(false);
  int width = AppContext::Instance().display.GetWidth();
  int height = AppContext::Instance().display.GetHeight();

  // Setup main and shadowmap rendertargets
  Cfg<RenderTarget>(scene, "rt.screen", 2000)
    . Tags("onscreen")
    . Clear(.8, .8, .8, 1)
    //. Clear(0, 0, 0, 1)
    . Done();

  //Cfg<Actor>(scene, "actor.terrain")
  //  . Model("Assets/screen.dsm", "Assets/texture.mat")
  //  . EulerAngles(-90, 0, 0)
  //  . Done();

  for (int i = 0; i < 400; i++) {
    Cfg<Actor>(scene, "actor.grassnode" + std::to_string(i))
      . Mesh(Patch(Math::Random(.02, .04), Math::Random(.2, .6)))
      . Position(Math::Random()/2, 0, Math::Random()/2)
      . EulerAngles(0, Math::Random()*360, 0)
      . Material("sandbox/grass/grasspatch.mat")
      . Done()
      ->GetComponent<MeshRenderer>()
      ->primitive = MeshRenderer::kPtPatches;
  }

  //scene.Get<Actor>("actor.grassnode")
  //  ->GetComponent<MeshRenderer>

  Cfg<Actor>(scene, "actor.knight")
    . Model("Assets/knight.dsm", "Assets/texture.mat")
    . Position(0, 0, -1)
    . Done();

  // Main camera
  auto cam = Cfg<Camera>(scene, "camera.main")
    . Perspective(60, (float)width/height, .1, 200)
    . Position(0, 0, 2)
    . Action<FlyingCameraController>(1)
    . Done();

  // Fps meter.
  Cfg<Actor>(scene, "actor.fps.meter")
    . Action<FpsMeter>()
    . Done();

  Cfg<Actor>(scene, "actor.axes")
    . Action<CoordinateAxes>(scene, 0, cam, .15)
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
