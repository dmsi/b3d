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
#include "gl_main.h"

struct Instancing : public Action {
  Instancing(std::shared_ptr<Transformation> t)
    : Action(t) {}

  void Start() override {
    using glm::vec3; // shortcut
    const size_t n_instances = 50;

    if (auto mr = GetActor().GetComponent<MeshRenderer>()) {
      mr->n_instances = n_instances;
    }

    // xyz => xzy -> because we specify translations in object's local coordinate system!!!
    // and then apply the world space transformation on top on our local offset which basically
    // makes z = -y, y = z (90 degrees) => check the world matrix we set when create the actor!

    if (auto mf = GetActor().GetComponent<MeshFilter>()) {
      using Layout = AttributeLayout<glm::vec3, Color>;

      // Slots 0-7 reserved for per-vertex attributes
      // Slots 8-15 reserved for pre-instance attributes
      // We start from slot 8 here and using 2 attribute layout, 
      // so slot 8 for vec3 and slot 9 for Color.
      auto buf = mf->Map<Layout>(8, n_instances);

      for (int i = 0; i < n_instances; ++i) {
        float c = 1. *i/n_instances;
        float x = i - (int)n_instances/2;
        float a = 10.*i/2;
        float g = Math::Random();

        Layout::Set<0>(buf, i, vec3  {x, cos(a)/2, sin(a)/2});
        Layout::Set<1>(buf, i, Color {c, g, 1-c, 1});
      }

      mf->Unmap(8);
    }
  }
};

int main(int argc, char* argv[]) {
  Scene scene;

  // Initialize application.
  AppContext::Init(1280, 720, "Instances rendering (raw) [b3d]", Profile("3 3 core"));
  AppContext::Instance().display.ShowCursor(false);
  int width = AppContext::Instance().display.GetWidth();
  int height = AppContext::Instance().display.GetHeight();

  // Setup main and shadowmap rendertargets
  Cfg<RenderTarget>(scene, "rt.screen", 2000)
    . Tags("onscreen")
    . Clear(.8, .8, .8, 1)
    . Done();
  
  // Main camera
  Cfg<Camera>(scene, "camera.main")
    . Perspective(60, (float)width/height, .1, 100)
    . Position(0, 1, 25)
    . Action<FlyingCameraController>(5)
    . Done();
  
  Cfg<Actor>(scene, "actor.arrow")
    . Model("Assets/arrow.dsm", "Assets/instance_basic.mat")
    . EulerAngles(-90, 0, 0) // post pre-instance offset transform! 
    . Action<Instancing>()
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
