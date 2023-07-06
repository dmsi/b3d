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

// Moving object across xz plane on 'infinity' trajectory.
struct LemniscateGerono : public Action {
  std::shared_ptr<Camera> camera;
  float angle = 0;
  float speed = 25; // moving speed - degrees/second in polar coordinates
  float radius = 7;
  float alt = 3;

  LemniscateGerono(std::shared_ptr<Transformation> t, 
                   std::shared_ptr<Camera> c, float a = 0) 
    : Action(t), camera(c), angle(a)  {}

  void Update() override {
    angle += speed * GetTimer().GetTimeDelta();
    float a = glm::radians(angle);
    float z = radius*cos(a);
    float x = radius*sin(a) * cos(a);
    transform->SetLocalPosition(glm::vec3(x, alt - abs(cos(a)), z));

    camera->transform->SetLocalPosition(transform->GetGlobalPosition());
  }
};

// Surface shading properties.
struct SurfMaterial {
  Color diffuse;
  Color ambient;
  Color specular;
  float shininess;

  SurfMaterial(const Color& df = Color( 1,  1,  1, 1), 
               const Color& am = Color(.1, .1, .1, 1), 
               const Color& sp = Color( 0,  0,  0, 0), float sh = 50) 
    : diffuse(df), ambient(am), specular(sp), shininess(sh) {}
};

struct LightInfo {
  std::shared_ptr<Light>   light_src;  // light source
  std::shared_ptr<Camera>  shadow_cam; // camera used for rendering to shadowmap
  std::shared_ptr<Texture> shadow_map; // cube depth map
  
  // 'Disco'
  Color color1;
  Color color2;
  float color_mix_factor = 0;
  float color_mix_speed;

  LightInfo(std::shared_ptr<Light> l, std::shared_ptr<Camera> c, 
            std::shared_ptr<Texture> s, Color c1, Color c2, float cspd)
    : light_src(l), shadow_cam(c), shadow_map(s), 
      color1(c1), color2(c2), color_mix_speed(cspd) {}

  void Update(float dt) {
    if (color_mix_factor < 0) color_mix_speed = -color_mix_speed;
    if (color_mix_factor > 1) color_mix_speed = -color_mix_speed;
    color_mix_factor += color_mix_speed * dt; 
    light_src->SetColor(glm::mix(color1, color2, color_mix_factor));
  }
};

// Shading action. Changes light color and passes uniforms to shaders. 
struct Shading : public Action {
  std::vector<LightInfo> lights;        // lights sources
  SurfMaterial           surf_material; // surface material properties
    
  Shading(std::shared_ptr<Transformation> t, 
          const std::vector<LightInfo>& alights,
          SurfMaterial surf = SurfMaterial())
    : Action(t), lights(alights), surf_material(surf) {}

  void Update() override {
    for (auto& l: lights) {
      l.Update(GetTimer().GetTimeDelta());
    }
  }

  void Start() override {
    if (auto m = transform->GetActor().GetComponent<Material>()) {
      for (size_t i = 0; i < lights.size(); ++i) {
        m->SetTexture(i + 1, lights[i].shadow_map);
      }
    }
  }

  void PreDraw() override {
    auto L = [this](std::string name, size_t idx, std::string ext="") {
      return name + "[" + std::to_string(idx)  + "]" + ext;
    };

    if (auto m = transform->GetActor().GetComponent<Material>()) {
      // Light sources
      m->SetUniform("total_lights", (int)lights.size());
      for (size_t i = 0; i < lights.size(); ++i) {
        const auto& l = lights[i].light_src;
        const auto& c = lights[i].shadow_cam;
        m->SetUniform(L("light", i, ".position"),    l->transform->GetGlobalPosition());
        m->SetUniform(L("light", i, ".color"),       l->GetColor());
        m->SetUniform(L("light", i, ".attenuation"), l->GetAttenuation());
        m->SetUniform(L("light", i, ".nearfar"),     glm::vec2(c->GetNear(), c->GetFar()));
      }

      // Surface material
      m->SetUniform("surface.diffuse",   surf_material.diffuse);
      m->SetUniform("surface.ambient",   surf_material.ambient);
      m->SetUniform("surface.specular",  surf_material.specular);
      m->SetUniform("surface.shininess", surf_material.shininess);
      
      // Albedo
      if (m->GetTexture(0)) {
        m->SetUniform("has_albedo_map", 1);
      } else {
        m->SetUniform("has_albedo_map", 0);
      }
    }
  }
};

LightInfo CreatePointLight(Scene& scene, float startpos) {
  static int counter = 0;
  counter += 1;
  std::string name = "shadowmap" + std::to_string(counter);
  int w = 800;

  // Rendertarget
  auto shadow_map = Cfg<RenderTarget>(scene, "rt." + name, counter+1)
    . Camera("camera." + name)
    . Tags("shadow-caster")
    . Type(FrameBuffer::kCubeMap)
    . Resolution(w, w)
    . Layer(Layer::kDepth, Layer::kReadWrite)
    . Done()
    ->GetLayerAsTexture(0, Layer::kDepth);

  // Shadowmap camera
  auto shadow_cam = Cfg<Camera>(scene, "camera." + name)
    . Perspective(90, 1, .1, 100)
    . Done();

  auto lamp = Cfg<Light>(scene, "light.pt.lamp" + std::to_string(counter),
                         Light::kPoint)
    . Position    (0, 3, 6.9)
    . Color       (.8, .8, 1, 1) // overwriten in Shading 
    . Attenuation (.8, .3, 0)
    . Action<LemniscateGerono>(shadow_cam, startpos)
    . Done();

  // Light source visual
  Cfg<Actor>(scene, "actor.lamp.dbg" + std::to_string(counter))
    . Model("Assets/sphere.dsm", "Assets/Materials/lamp.mat")
    . Scale(.5, .5, .5)
    . Parent(lamp)
    . Done();
  
  return LightInfo {
    lamp, 
    shadow_cam, 
    shadow_map, 
    Rgb(128, 128, 255), 
    Rgb(255, 128, 128), 
    .05
  };
}

int main(int argc, char* argv[]) {
  Scene scene;

  // Initialize application.
  AppContext::Init(1280, 720, "Multiple light sources [b3d]", Profile("3 3 core"));
  AppContext::Instance().display.ShowCursor(false);
  int width = AppContext::Instance().display.GetWidth();
  int height = AppContext::Instance().display.GetHeight();
  
  // Setup main and shadowmap rendertargets
  Cfg<RenderTarget>(scene, "rt.screen", 2000)
    . Tags("onscreen")
    . Clear(0, 0, 0, 1)
    . Done();
  
  // Main camera
  Cfg<Camera>(scene, "camera.main")
    . Perspective(60, (float)width/height, .1, 100)
    . Position(0, 7, 10)
    . EulerAngles(-37, 0, 0)
    . Action<FlyingCameraController>(5)
    . Done();
  

  std::vector<LightInfo> all_lights = {
    CreatePointLight(scene, 0),
    CreatePointLight(scene, 100),
    CreatePointLight(scene, 200)
  };

  // Create room, place 6 pillars, a postament in the centre and 
  // knight on the postament.

  SurfMaterial surf_room, surf_pillar, surf_postament, surf_knight;
  surf_room.specular     = Color(.6, .6, .6, 1); 
  surf_pillar.specular   = Color(.8, .8, .8, 1);
  surf_postament.diffuse = Color(.93, .92, .97, 1); 
  surf_postament.specular = Color(.9, .9, .9, 1);
  surf_postament.shininess = 10;
  surf_knight.specular   = Color(.6, .6, .6, 1); surf_knight.shininess = 150;

  std::string shadow_technique = "Assets/Materials/shadow_caster_receiver_multi_pointlight.mat";

  Cfg<Actor>(scene, "actor.room")
    . Model("Assets/unity_cube.dsm", shadow_technique)
    . Scale(10, 5, 15)
    . Position(0, 2.5, 0)
    . Action<FlipTriangles>() // turn cube to room (CCW->CW)
    . Action<Shading>(all_lights, surf_room)
    . Done();

  // cylinder.dsm - 1x1 size, flat side in zy plane
  Cfg<Actor>(scene, "actor.pillar1")
    . Model("Assets/cylinder.dsm", shadow_technique)
    . EulerAngles (90,  0,  0)
    . Scale       (.5, .5,  2)
    . Position    (-3,  1,  0)
    . Action<Shading>(all_lights, surf_pillar)
    . Done();

  Cfg<Actor>(scene, "actor.pillar2")
    . Model("Assets/cylinder.dsm", shadow_technique)
    . EulerAngles (90,  0,  0)
    . Scale       (.5, .5,  2)
    . Position    (-3,  1,  -4)
    . Action<Shading>(all_lights, surf_pillar)
    . Done();

  Cfg<Actor>(scene, "actor.pillar3")
    . Model("Assets/cylinder.dsm", shadow_technique)
    . EulerAngles (90,  0,  0)
    . Scale       (.5, .5,  2)
    . Position    (-3,  1,  4)
    . Action<Shading>(all_lights, surf_pillar)
    . Done();
  
  Cfg<Actor>(scene, "actor.pillar4")
    . Model("Assets/cylinder.dsm", shadow_technique)
    . EulerAngles (90,  0,  0)
    . Scale       (.5, .5,  2)
    . Position    (3,   1,  0)
    . Action<Shading>(all_lights, surf_pillar)
    . Done();

  Cfg<Actor>(scene, "actor.pillar5")
    . Model("Assets/cylinder.dsm", shadow_technique)
    . EulerAngles (90,  0,  0)
    . Scale       (.5, .5,  2)
    . Position    (3,   1, -4)
    . Action<Shading>(all_lights, surf_pillar)
    . Done();

  Cfg<Actor>(scene, "actor.pillar6")
    . Model("Assets/cylinder.dsm", shadow_technique)
    . EulerAngles (90,  0,  0)
    . Scale       (.5, .5,  2)
    . Position    (3,   1,  4)
    . Action<Shading>(all_lights, surf_pillar)
    . Done();
  
  Cfg<Actor>(scene, "actor.postament")
    . Model("Assets/cylinder.dsm", shadow_technique)
    . EulerAngles (90,  0,  0)
    . Scale       (1,   1, .5)
    . Position    (0, .25,  0)
    . Action<Shading>(all_lights, surf_postament)
    . Done();
  
  auto knight_tex = std::make_shared<Texture2D>(
      Image::Load("Assets/Textures/tango-128.ppm"));

  Cfg<Actor>(scene, "actor.knight")
    . Model("Assets/knight.dsm", shadow_technique)
    . Texture     (0, knight_tex)
    . Position    (0,  .5,  0)
    . Action<Shading>(all_lights, surf_knight)
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
