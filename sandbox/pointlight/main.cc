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
#include "myactions/all.h"
#include "myhelpers/all.h"

// CW -> CCW; CCW -> CW
struct FlipTriangles : public Action {
  int  num_attempts = 10;
  bool done         = false;

  FlipTriangles(std::shared_ptr<Transformation> t)
    : Action(t) {}

  void Update() override {
    if (done) return;

    if (auto f = transform->GetActor().GetComponent<MeshFilter>()) {
      if (auto m = f->GetMesh()) {
        if (!m->indices.empty()) {
          for (size_t i = 0; i < m->indices.size(); i+=3) {
            std::swap(m->indices.at(i), m->indices.at(i+2));
          }
          if (!m->normals.empty()) {
            for (auto& n: m->normals) {
              n = -n;
            }
          }
          f->SetMesh(m);

          done = true;
          transform->GetActor().RemoveAction<FlipTriangles>();
          return;
        }
      }
    }

    num_attempts--;
    if (num_attempts < 0 && !done) {
      transform->GetActor().RemoveAction<FlipTriangles>();
    }
  }
};

struct LemniscateGerono : public Action {
  std::shared_ptr<Camera> camera;
  float angle = 0;
  float speed = 15; // moving speed - degrees / second in polar coordinates
  float radius = 7;

  LemniscateGerono(std::shared_ptr<Transformation> t) 
    : Action(t) {}

  void Update() override {
    angle += speed * GetTimer().GetTimeDelta();
    auto p = transform->GetLocalPosition();
    float a = glm::radians(angle);
    float z = radius*cos(a);
    float x = radius*sin(a) * cos(a);
    transform->SetLocalPosition(glm::vec3(x, p.y, z));
  }
};

struct PtLightUniform : public Action {
  std::shared_ptr<Light> light;

  PtLightUniform(std::shared_ptr<Transformation> t, std::shared_ptr<Light> l)
    : Action(t), light(l) {}

  void PreDraw() override {
    if (auto m = transform->GetActor().GetComponent<Material>()) {
      m->SetUniform("light.position", light->transform->GetGlobalPosition());
      // more to come...
    }
  }
};

int main(int argc, char* argv[]) {
  Scene scene;
  //using T = std::vector<std::string>;
  using glm::vec3;

  // Initialize application.
  AppContext::Init(1280, 720, "Point light [b3d]", Profile("3 3 core"));
  AppContext::Instance().display.ShowCursor(false);
  int width = AppContext::Instance().display.GetWidth();
  int height = AppContext::Instance().display.GetHeight();
  
  // Setup main and shadowmap rendertargets
  Cfg<RenderTarget>(scene, "rt.screen", 2000)
    . Tags("onscreen")
    . Clear(.4, .4, .4, 1)
    . Done();
    
  // Compose the scene. 
  Cfg<Camera>(scene, "camera.main")
    . Perspective(60, (float)width/height, .1, 500)
    . Position(0, 3, 6)
    . EulerAngles(-10, 0, 0)
    . Action<FlyingCameraController>(5)
    . Done();

  auto lamp = Cfg<Light>(scene, "light.pt.lamp")
    . Position(0, 4, 0)
    . Action<LemniscateGerono>()
    . Done();

  Cfg<Actor>(scene, "actor.lamp.dbg")
    . Model("Assets/sphere.dsm", "Assets/lamp.mat")
    . Scale(.5, .5, .5)
    . Parent(lamp)
    . Done();

  Cfg<Actor>(scene, "actor.room")
    . Model("Assets/unity_cube.dsm", "Assets/pointlight.mat")
    . Scale(10, 5, 15)
    . Position(0, 2.5, 0)
    . Action<FlipTriangles>()
    . Action<PtLightUniform>(lamp)
    . Done();

  Cfg<Actor>(scene, "actor.k")
    . Model("Assets/knight.dsm", "Assets/pointlight.mat")
    . Action<PtLightUniform>(lamp)
    . Done();

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
