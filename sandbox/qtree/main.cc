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

auto Patch(bool indices) {
  using glm::vec3;
  auto mesh = std::make_shared<Mesh>();
  mesh->vertices = {
    vec3(-1, 0, -1),
    vec3( 1, 0, -1),
    vec3( 1, 0,  1),
    vec3(-1, 0,  1)
  };

  if (indices) { // ccw
    mesh->indices = {
      //0, 1, 2, 0, 2, 3
      2, 1, 0, 3, 2, 0
    };
  }
  return mesh;
}

struct FrustrumTest : public Action {
  std::shared_ptr<Camera> camera;

  FrustrumTest(std::shared_ptr<Transformation> t, std::shared_ptr<Camera> c) 
    : Action(t), camera(c) {
    assert(camera);
  }

  void PreDraw() override {
    if (auto m = GetActor().GetComponent<Material>()) {
      auto& frustum = camera->GetFrustum();
      if (frustum.CheckPoint(transform->GetLocalPosition())) {
        m->SetUniform("tint", Color(1, 0, 0, 1));
      } else {
        m->SetUniform("tint", Color(0, 0, 0, 1));
      }
    }
  }
};

struct FrustumVisual : public Action {
  std::shared_ptr<Camera> camera;
  std::shared_ptr<Mesh> mesh;

  FrustumVisual(std::shared_ptr<Transformation> t, std::shared_ptr<Camera> c) 
    : Action(t), camera(c) {
    mesh = std::make_shared<Mesh>();
    mesh->vertices = {
      2.0f * glm::vec3(-0.5, -0.5, -0.5),
      2.0f * glm::vec3(0.5, -0.5, -0.5),
      2.0f * glm::vec3(0.5,  0.5, -0.5),
      2.0f * glm::vec3(-0.5,  0.5, -0.5),
      2.0f * glm::vec3(-0.5, -0.5,  0.5),
      2.0f * glm::vec3(0.5, -0.5,  0.5),
      2.0f * glm::vec3(0.5,  0.5,  0.5),
      2.0f * glm::vec3(-0.5,  0.5,  0.5)
    };
   
    mesh->indices = {
      0, 1, 2, 3, 
      4, 5, 6, 7, 
      0, 4, 1, 5, 
      2, 6, 3, 7,
      0, 3, 1, 2, 
      4, 7, 5, 6
    };

    auto m = std::make_shared<Mesh>();
    m->vertices = mesh->vertices;
    m->indices = mesh->indices;

    auto mf = GetActor().GetComponent<MeshFilter>();
    auto mr = GetActor().GetComponent<MeshRenderer>();
    if (!mf) mf = GetActor().AddComponent<MeshFilter>();
    if (!mr) mr = GetActor().AddComponent<MeshRenderer>();
    mf->SetMode(MeshFilter::kDynamic);
    mf->SetMesh(m);
    mr->primitive = MeshRenderer::kPtLines;
  }

  void Recalculate() {
    assert(camera);
    glm::mat4 p, v, inv_pv;
    camera->GetProjectionMatrix(p);
    camera->GetViewMatrix(v);
    inv_pv = glm::inverse(p * v);
    auto& frustum = camera->GetFrustum();
    if (auto m = GetActor().GetComponent<Mesh>()) {
      for (size_t i = 0; i < m->vertices.size(); ++i) {
        m->vertices[i] = frustum.NdcToWorld(inv_pv, mesh->vertices[i]);
      }
      GetActor().GetComponent<MeshFilter>()->SetMesh(m);
    }
  }

  void Update() override {
    //Recalculate();
  }

  void PreDraw() override {
    assert(camera);
    glm::mat4 p, v;
    camera->GetProjectionMatrix(p);
    camera->GetViewMatrix(v);
    if (auto m = GetActor().GetComponent<Material>()) {
      m->SetUniform("target_projection", p);
      m->SetUniform("target_view", v);
    }
  }

};

int main(int argc, char* argv[]) {
  using glm::vec3;
  Scene scene;

  LOG_SCOPE_F(INFO, "Helo blyat!");

  // Initialize application.
  AppContext::Init(1280, 720, "Sandbox [b3d]", Profile("3 3 core"));
  AppContext::Instance().display.ShowCursor(false);
  int width = AppContext::Instance().display.GetWidth();
  int height = AppContext::Instance().display.GetHeight();

  // Setup main and shadowmap rendertargets
  Cfg<RenderTarget>(scene, "rt.screen", 2000)
    . Tags("onscreen")
    . Clear(.8, .8, .8, 1)
    . Done();

  auto pool = Cfg<ActorPool>(scene, "actor.terr.patch", 10)
    . Mesh(Patch(true))
    . Material("sandbox/qtree/patch.mat")
    . Done();

  /*
  float sx = 20, sz = 20;
  auto a = pool->Get();
  a->transform->SetLocalPosition(-sx, 0, -sz);
  a->transform->SetLocalScale(sx, 0, sz);
  
  a = pool->Get();
  a->transform->SetLocalPosition(sx, 0, -sz);
  a->transform->SetLocalScale(sx, 0, sz);
  
  a = pool->Get();
  a->transform->SetLocalPosition(sx, 0, sz);
  a->transform->SetLocalScale(sx, 0, sz);
  
  a = pool->Get();
  a->transform->SetLocalPosition(-sx, 0, sz);
  a->transform->SetLocalScale(sx, 0, sz);
  */

  // Main camera
  Cfg<Camera>(scene, "camera.main")
    . Perspective(60, (float)width/height, .1, 80) 
    . Position(0, 0, 5)
    . Action<FlyingCameraController>(5)
    . Done();

  float w = 1, h = 1;
  auto testcam = Cfg<Camera>(scene, "testcam")
    . Perspective(20, (float)width/height, 1, 10) 
    //. Ortho(-w, h, w, -h, 1, 10)
    . Position(0, 0, 4)
    . EulerAngles(0, 20, 0)
    . Action<Rotator>(vec3(0, 10, 0))
    . Done();
 
  Cfg<Actor>(scene, "testcam.origin.visual")
    . Model("Assets/arrow.dsm", "Assets/arrow.mat")
    . EulerAngles(0, 180, 0)
    . Parent(testcam)
    . Done();

  Cfg<Actor>(scene, "testcam.visual")
    . Material("Assets/frustum.mat")
    . Action<FrustumVisual>(testcam)
    . Done();
 
  Cfg<Actor>(scene, "actor.obj")
    . Model("Assets/sphere.dsm", "Assets/tint.mat")
    . Scale(.2, .2, .2)
    . Action<FrustrumTest>(testcam)
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
