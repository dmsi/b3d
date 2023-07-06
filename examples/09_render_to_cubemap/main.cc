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

// 1. Moves on 'Lemniscate of Gerono' trajectory in xz plane, keeps y
// 2. Updates camera position which is used for rendering to cubemap
// 3. Set shader varialbles 
struct MirrorHandler: public Action {
  std::shared_ptr<Camera> camera;
  float angle = 0;
  float speed = 10; // moving speed - degrees / second in polar coordinates
  float radius = 4;
  Color surface_tint = Color(.9, .9, 1, 1);
  float surface_rr = 0.5; // 0 - reflections, 1 - refractions
  float refractive_index = 1.0/1.1; // https://en.wikipedia.org/wiki/List_of_refractive_indices

  MirrorHandler(std::shared_ptr<Transformation> transform, 
                std::shared_ptr<Camera> acamera)
    : Action(transform), camera(acamera) {}

  void Update() override {
    // Lemniscate of Gerono
    angle += speed * GetTimer().GetTimeDelta();
    auto p = transform->GetLocalPosition();
    float a = glm::radians(angle);
    float x = radius*cos(a);
    float z = radius*sin(a) * cos(a);
    transform->SetLocalPosition(glm::vec3(x, p.y, z));
    
    // 2. Sync camera position
    camera->transform->SetLocalPosition(transform->GetLocalPosition());
  }

  void PreDraw() override {
    // 3. Uniforms
    if (auto m = transform->GetActor().GetComponent<Material>()) {
      m->SetUniform("tint", surface_tint);
      m->SetUniform("rr_factor", surface_rr);
      m->SetUniform("refractive_index", refractive_index);
    }
  }
};

int main(int argc, char* argv[]) {
  using T = std::vector<std::string>;
  using glm::vec3;
  Scene scene;

  // Step 1. Initialize application.
  AppContext::Init(1280, 720, "Render to cubemap [b3d]", Profile("3 3 core"));
  AppContext::Instance().display.ShowCursor(false);
  int width = AppContext::Instance().display.GetWidth();
  int height = AppContext::Instance().display.GetHeight();
  
  // Step 2. Setup onscreen and cubeenv rendertargets
  Cfg<RenderTarget>(scene, "rt.screen", 1)
    . Tags("onscreen")
    . Done();
  
  int w = 100, h = 100;
  auto cube_tex = Cfg<RenderTarget>(scene, "rt.cubeenv", 0)
    . Camera("camera.cubemap")
    . Tags("cubeenv")
    . Type(FrameBuffer::kCubeMap)
    . Resolution(w, h)
    . Layer(Layer::kColor, Layer::kReadWrite)
    . Layer(Layer::kDepth, Layer::kWrite)
    . Done()
    ->GetLayerAsTexture(0, Layer::kColor);

  // Step 3. Compose the scene. 
  Cfg<Camera>(scene, "camera.main")
    . Perspective(60, (float)width/height, .1, 500)
    . Position(7, 6, 7)
    . EulerAngles(-30, 45, 0)
    . Action<FlyingCameraController>(5)
    . Done();

  // For the cubemap camera FOV=90 and ratio 1:1. 
  // The rotation is controlled by the RenderTarget where it is 
  // assigned to. The position is controlled by an action.
  auto cubecam = Cfg<Camera>(scene, "camera.cubemap")
    . Perspective(90, 1, .1, 500)
    . Done();

  Cfg<Actor>(scene, "actor.skybox")
    . Model("Assets/sphere.dsm", "Assets/Materials/skybox_cubemap.mat")
    . Tags(0, T{"onscreen", "cubeenv"}) 
    . Done();
  
  Cfg<Actor>(scene, "actor.mirror")
    . Model("Assets/sphere.dsm", "Assets/Materials/cubemap_rr_surface.mat")
    . Tags(0, T{"onscreen"})  // onscreen tag comes from material, overwrite it just for consistancy 
    . Texture(0, cube_tex)
    . Position(0, 1, 0)
    . Scale(2, 2, 2)
    . Action<MirrorHandler>(cubecam)
    . Done();

  auto floor = Cfg<Actor>(scene, "actor.floor")
    . Model("Assets/plane.dsm", "Assets/Materials/texture.mat")
    . Tags(0, T{"onscreen", "cubeenv"})
    . Action<Rotator>(vec3(0, 30, 0))
    . Done();
  
  auto k1 = Cfg<Actor>(scene, "actor.k1")
    . Model("Assets/knight.dsm", "Assets/Materials/texture.mat")
    . Tags(0, T{"onscreen", "cubeenv"})
    . Position(-4, 0, -4)
    . EulerAngles(0, 45, 0)
    . Done();
  
  auto k2 = Cfg<Actor>(scene, "actor.k2")
    . Model("Assets/knight.dsm", "Assets/Materials/texture.mat")
    . Tags(0, T{"onscreen", "cubeenv"})
    . Position( 4, 0, -4)
    . EulerAngles(0, -45, 0)
    . Done();
  
  auto k3 = Cfg<Actor>(scene, "actor.k3")
    . Model("Assets/knight.dsm", "Assets/Materials/texture.mat")
    . Tags(0, T{"onscreen", "cubeenv"})
    . Position( 4, 0,  4)
    . EulerAngles(0, -135, 0)
    . Done();
  
  auto k4 = Cfg<Actor>(scene, "actor.k4")
    . Model("Assets/knight.dsm", "Assets/Materials/texture.mat")
    . Tags(0, T{"onscreen", "cubeenv"})
    . Position(-4, 0,  4)
    . EulerAngles(0, 135, 0)
    . Done();

  // Set transformation relationships in order to 'stick' knights
  // to the floor and rotate then when rotate the floor.
  Transformation::SetParent(floor->transform, k1->transform);
  Transformation::SetParent(floor->transform, k2->transform);
  Transformation::SetParent(floor->transform, k3->transform);
  Transformation::SetParent(floor->transform, k4->transform);

  Cfg<Actor>(scene, "actor.fps.meter")
    . Action<FpsMeter>()
    . Done();

  // Step 4. Main loop. Press ESC to exit.
  do {
    AppContext::BeginFrame();
    scene.Update();
    scene.Draw();
    AppContext::EndFrame();
  } while (AppContext::Running());

  // Step 5. Cleanup and close the app.
  AppContext::Close();
  return 0;
}
