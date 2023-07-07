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

struct SurfaceMaterial {
  Color diffuse    = Color(1);
  Color ambient    = Color(.2, .2, .2, 1);
  Color specular   = Color(.5);
  float shininess  = 50;
  bool  has_albedo = false;
};

struct DeferredShader : public Action {
  SurfaceMaterial surf;

  DeferredShader(std::shared_ptr<Transformation> t,
                 const SurfaceMaterial& s) 
    :   Action(t), surf(s)
  {}

  void PreDraw() override {
    if (auto m = GetActor().GetComponent<Material>()) {
      m->SetUniform("surface.diffuse",    surf.diffuse);
      m->SetUniform("surface.ambient",    surf.ambient);
      m->SetUniform("surface.specular",   surf.specular);
      m->SetUniform("surface.shininess",  surf.shininess);
      m->SetUniform("surface.has_albedo", (int)surf.has_albedo);
    }
  }
};

int main(int argc, char* argv[]) {
  using glm::vec3;
  Scene scene;

  // Initialize application.
  AppContext::Init(1280, 720, "Deferred rendering [b3d]", Profile("3 3 core"));
  AppContext::Instance().display.ShowCursor(false);
  int width = AppContext::Instance().display.GetWidth();
  int height = AppContext::Instance().display.GetHeight();

  // Setup main and shadowmap rendertargets
  Cfg<RenderTarget>(scene, "rt.screen", 2000)
    . Tags("onscreen")
    . Done();
  
  // Deferred pass
  auto deferred_rt = Cfg<RenderTarget>(scene, "rt.deferred", 0) 
    . Tags("deferred")
    . Type(FrameBuffer::kTexture2D)
    . Resolution(width, height) 
    . Layer(Layer::kColor, Layer::kReadWrite)
    . Layer(Layer::kColor, Layer::kReadWrite)
    . Layer(Layer::kColor, Layer::kReadWrite)
    . Layer(Layer::kColor, Layer::kReadWrite)
    . Layer(Layer::kDepth, Layer::kWrite)
    . Clear(.4, .4, .4, 0) // a=0 to discard
    . Done();

  // Deferred pass result placeholders
  auto albedo_map = deferred_rt->GetLayerAsTexture(0, Layer::kColor);
  auto normal_map = deferred_rt->GetLayerAsTexture(1, Layer::kColor);
  auto specular_map = deferred_rt->GetLayerAsTexture(2, Layer::kColor);
  auto eye_map = deferred_rt->GetLayerAsTexture(3, Layer::kColor);
    
  // Compose the scene
  SurfaceMaterial knt_srf {Color(1), Color(.2), Color(.5), 50, true};
  SurfaceMaterial flr_srf {Color(.8,.4,.4, 1), Color(.2), Color(0), 50, false};
  SurfaceMaterial shr_srf {Color(.4, .8, .4, 1), Color(.2), Color(.7), 100, false};
  SurfaceMaterial cyl_srf {Color(.4, .4, .8, 1), Color(.2), Color(.7), 100, false};

  Cfg<Actor>(scene, "actor.floor")
    . Model("assets/models/plane.dsm", "assets/materials/deferred_basic.mat")
    . Action<DeferredShader>(flr_srf)
    . Done(); 

  Cfg<Actor>(scene, "actor.knight")
    . Model("assets/models/knight.dsm", "assets/materials/deferred_basic.mat")
    . Action<Rotator>(vec3(0, 90, 0))
    . Action<DeferredShader>(knt_srf)
    . Done();

  Cfg<Actor>(scene, "actor.sphere")
    . Model("assets/models/sphere.dsm", "assets/materials/deferred_basic.mat")
    . Position(-2, .5, 0)
    . Action<DeferredShader>(shr_srf)
    . Done();

  Cfg<Actor>(scene, "actor.pillar")
    . Model("assets/models/cylinder.dsm", "assets/materials/deferred_basic.mat")
    . Position(2, 1, 0)
    . EulerAngles(90, 0, 0)
    . Scale(.5, .5, 2)
    . Action<DeferredShader>(cyl_srf)
    . Done();

  Cfg<Light>(scene, "light.sun", Light::kDirectional)
    . EulerAngles(-210, 0, 0)
    . Done();

  // Main screen
  Cfg<Actor>(scene, "actor.display.main")
    . Model("assets/models/screen.dsm", "assets/materials/deferred_combine.mat")
    . Texture(0, albedo_map)
    . Texture(1, normal_map)
    . Texture(2, specular_map)
    . Texture(3, eye_map)
    . Done();
  
  // Side displays shows deferred steps
  Cfg<Actor>(scene, "actor.display.albedo")
    . Model("assets/models/screen.dsm", "assets/materials/overlay_texture.mat")
    . Texture(0, albedo_map)
    . Scale(.25, .25, 1) 
    . Position(1-.25, 1-.25, 0)
    . Done();

  Cfg<Actor>(scene, "actor.display.normal")
    . Model("assets/models/screen.dsm", "assets/materials/overlay_texture.mat")
    . Texture(0, normal_map)
    . Scale(.25, .25, 1) 
    . Position(1-.25, .25, 0)
    . Done();
  
  Cfg<Actor>(scene, "actor.display.specular")
    . Model("assets/models/screen.dsm", "assets/materials/overlay_texture.mat")
    . Texture(0, specular_map)
    . Scale(.25, .25, 1) 
    . Position(1-.25, -.25, 0)
    . Done();

  Cfg<Actor>(scene, "actor.display.eye")
    . Model("assets/models/screen.dsm", "assets/materials/overlay_texture.mat")
    . Texture(0, eye_map)
    . Scale(.25, .25, 1) 
    . Position(1-.25, -.75, 0)
    . Done();
  
  // Main camera
  Cfg<Camera>(scene, "camera.main")
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
