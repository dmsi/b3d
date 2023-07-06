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

  LemniscateGerono(std::shared_ptr<Transformation> t, std::shared_ptr<Camera> c) 
    : Action(t), camera(c) {}

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

// Shading action. Changes light color and passes uniforms to shaders. 
struct Shading : public Action {
  using LhtPtr = std::shared_ptr<Light>;
  using CamPtr = std::shared_ptr<Camera>;

  LhtPtr       light;  // light source
  CamPtr       camera; // camera which is used for rendering to shadowmap 
  SurfMaterial surf_material; // surface material properties
    
  // A bit of 'disco' lights :)
  Color        color1 = Color(.5, .5, 1, 1);
  Color        color2 = Color(1, .5, .5, 1);
  float        color_mix_factor = 0;
  float        color_mix_speed = 1;

  Shading(std::shared_ptr<Transformation> t, LhtPtr l, CamPtr c, 
                 SurfMaterial surf = SurfMaterial())
    : Action(t), light(l), camera(c), surf_material(surf) {}

  void Update() override {
    if (color_mix_factor < 0) color_mix_speed = -color_mix_speed;
    if (color_mix_factor > 1) color_mix_speed = -color_mix_speed;

    color_mix_factor += color_mix_speed * GetTimer().GetTimeDelta();
    light->SetColor(glm::mix(color1, color2, color_mix_factor));
  }

  void PreDraw() override {
    if (auto m = transform->GetActor().GetComponent<Material>()) {
      m->SetUniform("light.position",    light->transform->GetGlobalPosition());
      m->SetUniform("light.color",       light->GetColor());
      m->SetUniform("light.attenuation", light->GetAttenuation());
      m->SetUniform("nearfar",           glm::vec2(camera->GetNear(), 
                                                   camera->GetFar()));

      m->SetUniform("surface.diffuse",   surf_material.diffuse);
      m->SetUniform("surface.ambient",   surf_material.ambient);
      m->SetUniform("surface.specular",  surf_material.specular);
      m->SetUniform("surface.shininess", surf_material.shininess);
      
      m->SetUniform("has_diffuse_texture", (int)(m->GetTexture(1) != nullptr));
    }
  }
};

int main(int argc, char* argv[]) {
  Scene scene;

  // Initialize application.
  AppContext::Init(1280, 720, "Omni shadow map [b3d]", Profile("3 3 core"));
  AppContext::Instance().display.ShowCursor(false);
  int width = AppContext::Instance().display.GetWidth();
  int height = AppContext::Instance().display.GetHeight();
  
  // Setup main and shadowmap rendertargets
  Cfg<RenderTarget>(scene, "rt.screen", 2000)
    . Tags("onscreen")
    . Clear(0, 0, 0, 1)
    . Done();
  
  // Depth only cubemap for rendering the shadowmap.
  // All actors material passes that posses tag 'shadow-caster' will 
  // be selected for rendering to this rendertarget.
  int w = 800, h = 800;
  auto shadow_map = Cfg<RenderTarget>(scene, "rt.shadowmap", 0)
    . Camera("camera.cubemap")
    . Tags("shadow-caster")
    . Type(FrameBuffer::kCubeMap)
    . Resolution(w, h)
    . Layer(Layer::kDepth, Layer::kReadWrite)
    . Done()
    ->GetLayerAsTexture(0, Layer::kDepth);
  
  // Main camera
  Cfg<Camera>(scene, "camera.main")
    . Perspective(60, (float)width/height, .1, 100)
    . Position(0, 7, 10)
    . EulerAngles(-37, 0, 0)
    . Action<FlyingCameraController>(5)
    . Done();
  
  // Shadowmap camera
  auto shadow_cam = Cfg<Camera>(scene, "camera.cubemap")
    . Perspective(90, 1, .1, 100)
    . Done();

  auto lamp = Cfg<Light>(scene, "light.pt.lamp", Light::kPoint)
    . Position(0, 3, 0)
    . Color(.8, .8, 1, 1) // overwriten in Shading 
    . Attenuation(.8, .1, 0)
    . Action<LemniscateGerono>(shadow_cam)
    . Done();

  // Light source visual
  Cfg<Actor>(scene, "actor.lamp.dbg")
    . Model("Assets/sphere.dsm", "Assets/Materials/lamp.mat")
    . Scale(.5, .5, .5)
    . Parent(lamp)
    . Done();

  // Create room, place 6 pillars, a postament in the centre and 
  // knight on the postament.

  SurfMaterial surf_room, surf_pillar, surf_postament, surf_knight;
  surf_room.specular     = Color(.6, .6, .6, 1); 
  surf_pillar.specular   = Color(.8, .8, .8, 1);
  surf_postament.diffuse = Color(.93, .92, .97, 1); 
  surf_postament.specular = Color(.9, .9, .9, 1);
  surf_postament.shininess = 10;
  surf_knight.specular   = Color(.6, .6, .6, 1); surf_knight.shininess = 150;

  std::string shadow_technique = "Assets/Materials/shadow_caster_receiver_pointlight.mat";

  Cfg<Actor>(scene, "actor.room")
    . Model("Assets/unity_cube.dsm", shadow_technique)
    . Texture(0, shadow_map)
    . Scale(10, 5, 15)
    . Position(0, 2.5, 0)
    . Action<FlipTriangles>() // turn cube to room (CCW->CW)
    . Action<Shading>(lamp, shadow_cam, surf_room)
    . Done();

  // cylinder.dsm - 1x1 size, flat side in zy plane
  Cfg<Actor>(scene, "actor.pillar1")
    . Model("Assets/cylinder.dsm", shadow_technique)
    . Texture     (0, shadow_map)
    . EulerAngles (90,  0,  0)
    . Scale       (.5, .5,  2)
    . Position    (-3,  1,  0)
    . Action<Shading>(lamp, shadow_cam, surf_pillar)
    . Done();

  Cfg<Actor>(scene, "actor.pillar2")
    . Model("Assets/cylinder.dsm", shadow_technique)
    . Texture     (0, shadow_map)
    . EulerAngles (90,  0,  0)
    . Scale       (.5, .5,  2)
    . Position    (-3,  1,  -4)
    . Action<Shading>(lamp, shadow_cam, surf_pillar)
    . Done();

  Cfg<Actor>(scene, "actor.pillar3")
    . Model("Assets/cylinder.dsm", shadow_technique)
    . Texture     (0, shadow_map)
    . EulerAngles (90,  0,  0)
    . Scale       (.5, .5,  2)
    . Position    (-3,  1,  4)
    . Action<Shading>(lamp, shadow_cam, surf_pillar)
    . Done();
  
  Cfg<Actor>(scene, "actor.pillar4")
    . Model("Assets/cylinder.dsm", shadow_technique)
    . Texture     (0, shadow_map)
    . EulerAngles (90,  0,  0)
    . Scale       (.5, .5,  2)
    . Position    (3,   1,  0)
    . Action<Shading>(lamp, shadow_cam, surf_pillar)
    . Done();

  Cfg<Actor>(scene, "actor.pillar5")
    . Model("Assets/cylinder.dsm", shadow_technique)
    . Texture     (0, shadow_map)
    . EulerAngles (90,  0,  0)
    . Scale       (.5, .5,  2)
    . Position    (3,   1, -4)
    . Action<Shading>(lamp, shadow_cam, surf_pillar)
    . Done();

  Cfg<Actor>(scene, "actor.pillar6")
    . Model("Assets/cylinder.dsm", shadow_technique)
    . Texture     (0, shadow_map)
    . EulerAngles (90,  0,  0)
    . Scale       (.5, .5,  2)
    . Position    (3,   1,  4)
    . Action<Shading>(lamp, shadow_cam, surf_pillar)
    . Done();
  
  Cfg<Actor>(scene, "actor.postament")
    . Model("Assets/cylinder.dsm", shadow_technique)
    . Texture     (0, shadow_map)
    . EulerAngles (90,  0,  0)
    . Scale       (1,   1, .5)
    . Position    (0, .25,  0)
    . Action<Shading>(lamp, shadow_cam, surf_postament)
    . Done();
  
  auto knight_tex = std::make_shared<Texture2D>(
      Image::Load("Assets/tango-128.ppm"));

  Cfg<Actor>(scene, "actor.knight")
    . Model("Assets/knight.dsm", shadow_technique)
    . Texture     (0, shadow_map)
    . Texture     (1, knight_tex)
    . Position    (0,  .5,  0)
    . Action<Shading>(lamp, shadow_cam, surf_knight)
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
