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

#include "batch_actor.h"

int main(int argc, char* argv[]) {
  Scene scene;

  // Initialize application.
  AppContext::Init(1280, 720, "Instances rendering [b3d]", Profile("3 3 core"));
  AppContext::Instance().display.ShowCursor(false);
  int width = AppContext::Instance().display.GetWidth();
  int height = AppContext::Instance().display.GetHeight();

  // 
  // First batch
  //
  auto batch = scene.Add<StdBatch::Batch>("a*.batch", 6);
  batch->AddComponent<MeshFilter>()->SetMesh(MeshLoader::Load("Assets/knight.dsm"));
  batch->AddComponent<MeshRenderer>()->SetMaterial(MaterialLoader::Load("Assets/instance_test.mat"));
  auto k1 = scene.Add<StdBatch::Actor>("a*.k1", batch);
  auto k2 = scene.Add<StdBatch::Actor>("a*.k2", batch);
  auto k3 = scene.Add<StdBatch::Actor>("a*.k3", batch);

  k2->transform->SetLocalPosition(2, 0, 0);
  k3->transform->SetLocalPosition(-2, 0, 0);
  k1->AddAction<Rotator>(glm::vec3(0, 45, 0));

  //
  // Second batch
  //
  //batch = scene.Add<StdBatch::Batch>("a*.batch1", 6);
  //batch->AddComponent<MeshFilter>()->SetMesh(MeshLoader::Load("Assets/arrow.dsm"));
  //batch->AddComponent<MeshRenderer>()->SetMaterial(MaterialLoader::Load("Assets/instance_test.mat"));
  //scene.Add<StdBatch::Actor>("a*.z1", batch)->transform->SetLocalPosition(0, 2.2, 0);
  //scene.Add<StdBatch::Actor>("a*.z2", batch)->transform->SetLocalPosition(1, 2.2, 0);
  //scene.Add<StdBatch::Actor>("a*.z3", batch)->transform->SetLocalPosition(-1, 2.2, 0);
  //scene.Get<StdBatch::Actor>("a*.z3")->AddAction<Rotator>(glm::vec3(30, 90, 10));
  //struct Foo {};
  //scene.Add<Foo>("helo blyat!");
  batch = Cfg<StdBatch::Batch>(scene, "a*.batch1", 6)
    . Model("Assets/arrow.dsm", "Assets/instance_test.mat")
    . Done();

  Cfg<StdBatch::Actor>(scene, "a*.z1", batch)
    . Position(0, 2.2, 0)
    . Done();

  Cfg<StdBatch::Actor>(scene, "a*.z2", batch)
    . Position(1, 2.2, 0)
    . Done();

  Cfg<StdBatch::Actor>(scene, "a*.z3", batch)
    . Position(-1, 2.2, 0)
    . Action<Rotator>(glm::vec3(30, 90, 10))
    . Done();

  //
  // Not in the batch, regular actor
  //
  // does it make sense to make AddComponent<Mesh> / <Material> which automatically adds MeshFilter and MeshRenderer?
  auto a = scene.Add<Actor>("a=.floor");
  a->AddComponent<MeshFilter>()->SetMesh(MeshLoader::Load("Assets/plane.dsm"));
  a->AddComponent<MeshRenderer>()->SetMaterial(MaterialLoader::Load("Assets/texture.mat"));
  
  // Setup main and shadowmap rendertargets
  Cfg<RenderTarget>(scene, "rt.screen", 2000)
    . Tags("onscreen")
    . Clear(.4, .4, .4, 1)
    . Done();
  
  // Main camera
  auto camera = Cfg<Camera>(scene, "camera.main")
    . Perspective(60, (float)width/height, .1, 100)
    . Position(0, 1, 4)
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
