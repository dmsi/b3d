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

// GL+libs
#include "gl_main.h"
#include "glm_main.h"

#include "util.h"
#include "appcontext.h"
#include "scene.h"
#include "meshloader.h"
#include "actionlib/flyingcameracontroller.h"
#include "actionlib/fpsmeter.h"
#include "actionlib/rotator.h"
#include "actionlib/lightingshaderuniform.h"

#include "framebuffer.h"


#include "common/range.h"


// Counter clockwise quad with UV set
std::shared_ptr<Mesh> MakeQuadMesh(const glm::vec3& offset = glm::vec3(0,0,0)) {
  std::shared_ptr<Mesh> mesh(new Mesh(4, 6));
  mesh->vertices = {glm::vec3(0, 0, 0),  // 0 - bottom left
                    glm::vec3(1, 0, 0),  // 1 - bottom right
                    glm::vec3(1, 1, 0),  // 2 - top right
                    glm::vec3(0, 1, 0)}; // 3 - top left

  mesh->uv       = {glm::vec2(0, 0),     // 0 - bottom left
                    glm::vec2(1, 0),     // 1 - bottom right
                    glm::vec2(1, 1),     // 2 - top right
                    glm::vec2(0, 1)};    // 3 - top left

  mesh->indices = {0, 1, 2, 0, 2, 3};    // counter clockwise
  
  for (auto& v: mesh->vertices) {
    v += offset;
  }

  return mesh;
}

int main(int argc, char* argv[]) {
  // Init
  AppContext::Init(Mode(1280, 720, "borsch.3d"), Profile("3 3 core"));
  Display& display = AppContext::Instance().display;
  Input& input = AppContext::Instance().input;
  display.ShowCursor(false);

  Scene scene;

  auto SpawnActor = [&scene] (const std::string& obj, const std::string& mat) {
    static int count = 0;
    count++;
    auto actor = scene.Add<Actor>("actor." + obj + "#" + std::to_string(count));
    actor->AddComponent<MeshRenderer>()->SetMaterial(MaterialLoader::Load(mat));
    actor->AddComponent<MeshFilter>()->SetMesh(MeshLoader::Load(obj));

    Scene scene;
    return actor;
  };

  // Ground
  SpawnActor("Assets/plane.dsm", "Assets/dirlight.mat")
    ->AddAction<LightingShaderUniform>()
    ->Setup(glm::vec4(.8, .8, .8, 1),
            glm::vec4(.1, .1, .1, 1),
            glm::vec4(.9, .9, .9, 1),
            50);
  
  // Wall
  auto wall = SpawnActor("Assets/plane.dsm", "Assets/dirlight_tex.mat");
  wall->AddAction<LightingShaderUniform>()
      ->Setup(glm::vec4(.8, .8, .8, 1),
              glm::vec4(.1, .1, .1, 1),
              glm::vec4(.9, .9, .9, 1),
              50);
  wall->transform->SetLocalEulerAngles(glm::vec3(90, 0, 0));
  wall->transform->SetLocalPosition(glm::vec3(0, 5, -5));

  // An object
  auto actor = SpawnActor("Assets/torus.dsm", "Assets/dirlight.mat");
  actor->transform->SetLocalEulerAngles(glm::vec3(-60, 0, 0));
  actor->transform->SetLocalPosition(glm::vec3(0, 2, 0));
  actor->AddAction<Rotator>()->rotation_speed = glm::vec3(20, 40, 10);
  auto surface = actor->AddAction<LightingShaderUniform>();
  surface->diffuse = glm::vec4(.3, 0, 0, 1);
  surface->ambient = glm::vec4(.1, .1, .1, 1);
  surface->specular = glm::vec4(1, 1, 1, 1);
  surface->shininess = 50;

  // Main camera
  std::shared_ptr<Camera> camera = scene.Add<Camera>("camera.main");
  camera->SetPerspective(60, 1.0f*display.GetMode().width/display.GetMode().height, 0.1f, 150.0f);
  camera->transform->SetLocalPosition(glm::vec3(0, 5, 7));
  camera->transform->SetLocalEulerAngles(glm::vec3(-30, 0, 0));
  camera->AddAction<FlyingCameraController>()->moving_speed = 5;
  
  // Sun
  std::shared_ptr<Light> sun = scene.Add<Light>("light.sun");
  sun->SetColor(glm::vec4(1,1,1,1));
  sun->transform->SetLocalPosition(glm::vec3(0, 5, 5));
  sun->transform->SetLocalEulerAngles(glm::vec3(-210, 0, 0));
  
  // Debug light
  auto debug_light = SpawnActor("Assets/arrow.dsm", "Assets/lamp.mat");
  Transformation::SetParent(sun->transform, debug_light->transform);

  // FPS meter
  std::shared_ptr<Actor> fps_meter = scene.Add<Actor>("actor.fps_meter");
  fps_meter->AddAction<FpsMeter>();
  fps_meter->AddAction<FpsMeterShaderUniformUpdater>();
  
  auto quad = scene.Add<Actor>("actor.quad");
  quad->AddComponent<MeshRenderer>()->SetMaterial(MaterialLoader::Load("Assets/quad.mat"));
  quad->AddComponent<MeshFilter>()->SetMesh(MakeQuadMesh());
  
  // Render Targets
  // TODO simplify
  //auto rt = scene.AddRenderTarget("rendertarget.offscreen", 100);
  //rt->SetRange(Range<int>(100, 999));
  //auto fb = rt->SetFrameBuffer(FrameBuffer::kOffscreen, display.GetMode().width, display.GetMode().height);
  //fb->AddLayer(Layer::kColor, Layer::kReadWrite);
  //fb->AddLayer(Layer::kDepth, Layer::kWrite);
  //fb->SetColorLayerClearValue(Color(0, .4, 0, 1));
  //fb->Init(); 
  //auto texture = fb->GetLayerAsTexture(0, Layer::kColor);

  auto fb = Scene.AddRenderTarget("rt.off", 100)
    ->SetFrameBuffer(FrameBuffer::kOffscreen, width, height);
  fb->AddLayer(Layer::kColor, Layer::kReadWrite);
  fb->AddLayer(Layer::kDepth, Layer::kWrite);
  fb->SetColorLayerClearValue(Color(.4, .4, .4, 1));
  fb->Init();
  auto texture = fb->GetLayerAsTexture(0, Layer::kColor);
  quad->GetComponent<MeshRenderer>()->GetMaterial()->SetTexture(0, texture);

  //RtInfo rt = {
  //  "rt.off",
  //  1000,
  //  Ls{ 
  //     P{ Layer::kColor, Layer::kReadWrite }, 
  //     P{ Layer::kColor, Layer::kReadWrite }
  //  },
  //  Color(.4, .4, .4, 1);
  //};

  // yaml
  auto rt_info = TextLs {
    "name  : rt.off",
    "index : 1000",
    "type  : kTexture2D",
    "layer : [kColor, kReadWrite]",
    "layer : [kDepth, kReadWrite]",
    "clear : [color, .4, .4, .4, 1]"
  };
  auto fb = scene.AddRenderTarget(rt_info)->GetFrameBuffer();
  auto tex = fb->GetLayerAsTexture(0, Layer::kColor);

  rt = scene.AddRenderTarget("rendertarget.screen", 200);
  rt->SetRange(Range<int>(1000, 2000));
  fb = rt->SetFrameBuffer(FrameBuffer::kScreen, 0, 0);
  fb->AddLayer(Layer::kDepth, Layer::kWrite);
  fb->SetColorLayerClearValue(Color(0, 0, .4, 1));
  fb->Init(); 
  

  // Main loop
  //glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
  do {
    AppContext::BeginFrame();
    scene.Update();
    scene.Draw();
    AppContext::EndFrame();
  } while (AppContext::Running());

  // Cleanup
  AppContext::Close();
  return 0;
}
