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

// No short string optimization, new/delete ... alot
//  => ~10% overall
struct SomeHeavyActor : public Action {
  SomeHeavyActor(std::shared_ptr<Transformation> t) : Action(t) {}

  void PreDraw() override {
    if (auto m = GetActor().GetComponent<Material>()) {
      m->SetUniform("long-long-very-long-uniform-name0", 0);
      m->SetUniform("long-long-very-long-uniform-name1", 1.0f);
      m->SetUniform("long-long-very-long-uniform-name2", glm::vec3(1));
      m->SetUniform("long-long-very-long-uniform-name3", glm::vec4(1));
      m->SetUniform("long-long-very-long-uniform-name4", glm::mat4(1));
    }
  }
};

// Pre allocate uniform names...
//  => 2.2 % overall, mostly std::map lookup for uniform location
struct SomeHeavyActorOptimized : public Action {
  const std::string name0 = "long-long-very-long-uniform-name0";
  const std::string name1 = "long-long-very-long-uniform-name1";
  const std::string name2 = "long-long-very-long-uniform-name2";
  const std::string name3 = "long-long-very-long-uniform-name3";
  const std::string name4 = "long-long-very-long-uniform-name4";

  SomeHeavyActorOptimized(std::shared_ptr<Transformation> t) : Action(t) {}

  void PreDraw() override {
    if (auto m = GetActor().GetComponent<Material>()) {
      m->SetUniform(name0, 0);
      m->SetUniform(name1, 1.0f);
      m->SetUniform(name2, glm::vec3(1));
      m->SetUniform(name3, glm::vec4(1));
      m->SetUniform(name4, glm::mat4(1));
    }
  }
};

int main(int argc, char* argv[]) {
  using glm::vec3;
  Scene scene;

  // Initialize application.
  AppContext::Init(1280, 720, "Non batch profiling test [b3d]", Profile("3 3 core"));
  AppContext::Instance().display.ShowCursor(false);
  int width = AppContext::Instance().display.GetWidth();
  int height = AppContext::Instance().display.GetHeight();

  // Setup main and shadowmap rendertargets
  Cfg<RenderTarget>(scene, "rt.screen", 2000)
    . Tags("onscreen")
    . Clear(.8, .8, .8, 1)
    . Done();

  auto mesh = MeshLoader::Load("Assets/knight.dsm");
  auto mtrl = MaterialLoader::Load("Assets/texture.mat");
  constexpr size_t N_OBJECTS = 1000;

  for (size_t i = 0; i < N_OBJECTS; ++i) { 
    auto a = Cfg<Actor>(scene, "actor.obj" + std::to_string(i));
    a . Model(mesh, mtrl)
      . Position(Math::Random(-40, 40), 0 , Math::Random(-40, 40))
      . Action<SomeHeavyActorOptimized>();
      a . Action<Rotator>(vec3(0, 10+Math::Random()*120, 0));

    a.Done();
  }

  // Main camera
  Cfg<Camera>(scene, "camera.main")
    . Perspective(60, (float)width/height, .1, 500)
    . Position(0, 1, 4)
    . Action<FlyingCameraController>(20)
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
