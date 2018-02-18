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

void CalculateBounds(const Mesh& mesh, glm::vec3& min, glm::vec3& max) {
  min = glm::vec3(std::numeric_limits<float>::max());
  max = glm::vec3(std::numeric_limits<float>::lowest());

  for (auto& v : mesh.vertices) {
    if (min.x > v.x) min.x = v.x;
    if (min.y > v.y) min.y = v.y;
    if (min.z > v.z) min.z = v.z;
    
    if (max.x < v.x) max.x = v.x;
    if (max.y < v.y) max.y = v.y;
    if (max.z < v.z) max.z = v.z;
  }
}


struct FrustumTest : public Action {
  std::shared_ptr<Camera> camera;
  glm::vec3 min, max; // bounds
  bool calculated = false;

  FrustumTest(std::shared_ptr<Transformation> t, std::shared_ptr<Camera> c)
    : Action(t), camera(c) {
    assert(camera);

  }

  void PreDraw() override {
    if (!calculated) {
      if (auto m = GetActor().GetComponent<Mesh>()) {
        CalculateBounds(*m, min, max);
        min *= 5; max *= 5; // because we scaled it *5 .......
        std::cerr << min.x << " " << min.y << " " << min.z << std::endl;
        std::cerr << max.x << " " << max.y << " " << max.z << std::endl;
        calculated = true;
      }
    }
    if (auto m = GetActor().GetComponent<Material>()) {
      auto& frustum = camera->GetFrustum();
      //auto pos = transform->GetGlobalPosition();
      //bool test = frustum.TestPoint(pos);
      bool test = frustum.TestAabb(min, max);

      Color c((float)test, 0, (float)test, 1);
      m->SetUniform("tint", c);
    }
  }
};


struct BoundsVisual : public Action {
  glm::vec3              local_min;
  glm::vec3              local_max;
  std::shared_ptr<Actor> actor;

  BoundsVisual(std::shared_ptr<Transformation> t, 
               std::shared_ptr<Actor> a)
    :  Action(t)
    ,  actor(a)
  {
    Build();
  }

  void Build() {
    if (auto m = actor->GetComponent<Mesh>()) {
      CalculateBounds(*m, local_min, local_max);
    }

    // 1x1x1 cube
    // https://en.wikibooks.org/wiki/OpenGL_Programming/Bounding_box
    std::shared_ptr<Mesh> m = std::make_shared<Mesh>();
    m->vertices = {
      glm::vec3(-0.5, -0.5, -0.5),
      glm::vec3(0.5, -0.5, -0.5),
      glm::vec3(0.5,  0.5, -0.5),
      glm::vec3(-0.5,  0.5, -0.5),
      glm::vec3(-0.5, -0.5,  0.5),
      glm::vec3(0.5, -0.5,  0.5),
      glm::vec3(0.5,  0.5,  0.5),
      glm::vec3(-0.5,  0.5,  0.5)
    };

    m->indices = {
      0, 1, 2, 3, 
      4, 5, 6, 7, 
      0, 4, 1, 5, 
      2, 6, 3, 7,
      0, 3, 1, 2, 
      4, 7, 5, 6
    };

    auto mf = GetActor().AddComponent<MeshFilter>();
    mf->SetMesh(m);

    auto mr = GetActor().AddComponent<MeshRenderer>();
    mr->SetMaterial(MaterialLoader::Load("Assets/bound_lines.mat"));

    mr->primitive = MeshRenderer::kPtLines;
  }

  // Transforms local_min and local_max with actor->transform to 
  // min, max
  void TransformAabb(glm::vec3& min, glm::vec3& max, float margin) {
    using glm::mat4;
    using glm::vec3;
    using glm::vec4;

    // TODO optimization required

    vec3 local_half_extents = 0.5f * (local_max - local_min);
    local_half_extents += vec3(margin);
    vec3 local_center = 0.5f * (local_max + local_min);
    mat4 mat;
    actor->transform->GetMatrix(mat); // TODO add method
    vec3 center = vec3(mat * vec4(local_center, 1));

    // Get idea from bullet3
    // https://github.com/bulletphysics/bullet3/blob/master/src/Bullet3Collision/BroadPhaseCollision/shared/b3Aabb.h
    // I had to transpose the matrix since glm and OpenGL uses coloumn-major
    // matrix order while in bulled they use row-major.
    mat = glm::transpose(mat);
    vec3 extent {
      glm::dot(local_half_extents, (vec3)glm::abs(mat[0])),
      glm::dot(local_half_extents, (vec3)glm::abs(mat[1])),
      glm::dot(local_half_extents, (vec3)glm::abs(mat[2])),
    };

    min = center - extent;
    max = center + extent;
  }

  void Update() override {
    glm::vec3 min, max;
    TransformAabb(min, max, 0);
    glm::vec3 extent = max - min; 
    glm::vec3 center = 0.5f * (max + min); 
    transform->SetLocalPosition(center);
    transform->SetLocalScale(extent);
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

    auto mf = GetActor().GetComponent<MeshFilter>();
    auto mr = GetActor().GetComponent<MeshRenderer>();
    if (!mf) mf = GetActor().AddComponent<MeshFilter>();
    if (!mr) mr = GetActor().AddComponent<MeshRenderer>();
    mf->SetMesh(mesh);
    mr->primitive = MeshRenderer::kPtLines;
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
  using T = std::vector<std::string>;
  Scene scene;

  loguru::g_stderr_verbosity = loguru::Verbosity_OFF;
  loguru::add_file("out.log", loguru::Truncate, loguru::Verbosity_MAX);
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
  
  auto topview_tex = Cfg<RenderTarget>(scene, "rt.topview", 0) 
    . Camera("camera.topview")
    . Tags("topview")
    . Type(FrameBuffer::kTexture2D)
    . Resolution(width, height) 
    . Layer(Layer::kColor, Layer::kReadWrite)
    . Layer(Layer::kDepth, Layer::kWrite)
    . Clear(.0, .0, .0, 1)
    . Done()
    ->GetLayerAsTexture(0, Layer::kColor);

  // Main camera
  auto maincam = Cfg<Camera>(scene, "camera.main")
    . Perspective(60, (float)width/height, .1, 20) 
    . Position(0, 1, 5)
    . Action<FlyingCameraController>(5)
    . Done();

  auto testcam = Cfg<Camera>(scene, "camera.topview")
    . Perspective(60, (float)width/height, 1, 500) 
    . Position(0, 10, 30)
    . EulerAngles(-30, 0, 0)
    . Parent(maincam)
    . Done();
 
  Cfg<Actor>(scene, "testcam.origin.visual")
    . Model("Assets/arrow.dsm", "Assets/arrow.mat")
    . Tags(0, T{"topview"})
    . EulerAngles(0, 180, 0)
    . Parent(maincam)
    . Done();

  Cfg<Actor>(scene, "testcam.visual")
    . Material("sandbox/frustum_check/frustum.mat")
    . Tags(0, T{"topview"})
    . Action<FrustumVisual>(maincam)
    . Done();
 
  Cfg<Actor>(scene, "actor.screen.topview")
    . Model("Assets/screen.dsm", "Assets/overlay_texture.mat")
    . Texture(0, topview_tex)
    . Scale(.25, .25, 1)
    . Position(0.75, 0.75, 0)
    . Done();

  //Cfg<Actor>(scene, "actor.knight")
  //  . Model("Assets/knight.dsm", "Assets/texture.mat")
  //  . Tags(0, T{"onscreen", "topview"})
  //  . Done();

  auto aaa = Cfg<Actor>(scene, "actor.yyall")
    . Model("Assets/sphere.dsm", "Assets/tint.mat")
    . Scale(5, 5, 5)
    . Action<FrustumTest>(maincam)
    . Tags(0, T{"onscreen", "topview"})
    . Done();

  Cfg<Actor>(scene, "atror.dbg")
    . Tags(0, T{"onscreen", "topview"})
    . Action<BoundsVisual>(aaa)
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
