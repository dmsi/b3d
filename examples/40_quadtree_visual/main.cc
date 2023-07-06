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
      2, 1, 0, 3, 2, 0
    };
  }
  return mesh;
}

struct Tint : public Action {
  Color color;
  Tint(std::shared_ptr<Transformation> t, const Color& c)
    : Action(t), color(c) {}

  void PreDraw() override {
    if (auto m = GetActor().GetComponent<Material>()) {
      m->SetUniform("tint", color);
    }
  }
};

// Quick and dirty quadtree
// nw -x+z
// ne +x+z
// se +x-z
// nw -x-z
struct QuadTreeNode {
  QuadTreeNode* parent;
  QuadTreeNode* childs[4];

  glm::vec3     position;
  float         size;

  size_t        level;

  enum Dir {
    kNw = 0, kNe, kSe, kSw
  };

  ~QuadTreeNode() {
    if (childs[0]) delete childs[0];
    if (childs[1]) delete childs[1];
    if (childs[2]) delete childs[2];
    if (childs[3]) delete childs[3];
  }

  bool IsLeaf() const {
    return !childs[0] && !childs[1] && !childs[2] && !childs[3];
  }

  // Offset for child position
  glm::vec3 offset(Dir dir) const {
    switch(dir) {
      case kNw: return glm::vec3(-size/2,  0,  size/2);
      case kNe: return glm::vec3( size/2,  0,  size/2);
      case kSe: return glm::vec3( size/2,  0, -size/2);
      case kSw: return glm::vec3(-size/2,  0, -size/2);
      default: ABORT_F("Invalid child direction");
    }
  }

  void Subdivide(size_t level) {
    this->level = level;
    if (level == 0) return;
    for (size_t i = 0; i < 4; ++i) {
      childs[i] = new QuadTreeNode();
      childs[i]->position = position + offset((Dir)i); 
      childs[i]->size = size/2;
      childs[i]->Subdivide(level - 1);
    }
  }

  // Total number of elements, including root
  // 4^0 + 4^1 + 4^2 + ... + 4^N, where N - number of subdivisions
  size_t Count() {
    size_t count = 0;
    for(size_t i = 0; i < 4; ++i) {
      if (childs[i]) count += childs[i]->Count();
    }
    return count + 1;
  }
  
  // walker returns: true - go for the childs, false - dont
  void Traverse(std::function<bool(QuadTreeNode* node)> walker) {
    if (walker(this)) {
      for (size_t i = 0; i < 4; ++i) {
        if (childs[i]) childs[i]->Traverse(walker);
      }
    }
  }
};

struct QuadTree {
  QuadTreeNode* root;
  size_t n_subdivisions;

  QuadTree(size_t n_subdivisions, size_t size) {
    this->n_subdivisions = n_subdivisions;
    root = new QuadTreeNode();
    root->position = glm::vec3(0, 0, 0);
    root->size = size;

    root->Subdivide(n_subdivisions);
  }

  ~QuadTree() {
    if (root) {
      delete root;
      root = 0;
    }
  }
};

struct QuadTreeVisualiser : public Action {
  std::shared_ptr<QuadTree> tree;
  std::shared_ptr<Camera> camera;

  QuadTreeVisualiser(std::shared_ptr<Transformation> t, 
                     std::shared_ptr<Camera> cam,
                     size_t levels, float size)
    : Action(t), camera(cam) {
    // separator
    if (dynamic_cast<ActorPool*>(&GetActor()) == nullptr) {
      ABORT_F("QuadTreeVisualiser must be attached to an ActorPool");
    }
    tree = std::make_shared<QuadTree>(levels, size);
    std::cerr << "QuadTree (" <<
                 "C: " << tree->root->Count() << ", "
                 "L: " << tree->root->level << ", "
                 "S: " << tree->root->size << ")" << std::endl;
  }

  ActorPool* GetPool() {
    return (ActorPool*)&GetActor();
  }
  
  void Update() override {
    auto pool = GetPool();
    pool->Clear();

    auto walker = [this, &pool](QuadTreeNode* node) -> bool {
      assert(node);
      auto& frustum = camera->GetFrustum();
      glm::vec3 min = node->position - node->size;
      glm::vec3 max = node->position + node->size;
      bool test = frustum.TestAabb(min, max);

      if (!test) {
        // A bit of profiling and optimization. Not delete actions, just modify
        // them (to avoid memory allocations). 
        // And shared pointers... they are cool but expensive.
        auto a = pool->Get(false);
        if (a) {
          a->transform->SetLocalPosition(node->position);
          a->transform->SetLocalScale(glm::vec3(node->size, 1, node->size));
          auto tint = a->GetAction<Tint>();
          if (!tint) a->AddAction<Tint>(Color(1,0,0,1));
          else       tint->color = Color(1,0,0,1);
        }
        return false;
      }

      if (node->IsLeaf()) {
        auto a = pool->Get(false);
        if (a) {
          a->transform->SetLocalPosition(node->position);
          a->transform->SetLocalScale(glm::vec3(node->size, 1, node->size));
          auto tint = a->GetAction<Tint>();
          if (!tint) a->AddAction<Tint>(Color(1,1,0,1));
          else       tint->color = Color(1,1,0,1);
        }
      }
      return true;
    };

    // Rebuild every frame
    tree->root->Traverse(walker);
  }
};

int main(int argc, char* argv[]) {
  using glm::vec3;
  using T = std::vector<std::string>;
  Scene scene;

  //loguru::g_stderr_verbosity = loguru::Verbosity_WARNING;
  //loguru::add_file("out.log", loguru::Truncate, loguru::Verbosity_MAX);

  // Initialize application.
  AppContext::Init(1280, 720, "Quadtree visual [b3d]", Profile("3 3 core"));
  AppContext::Instance().display.ShowCursor(false);
  int width = AppContext::Instance().display.GetWidth();
  int height = AppContext::Instance().display.GetHeight();

  // Setup render targets 
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
    . Clear(0, 0, 0, 1)
    . Done()
    ->GetLayerAsTexture(0, Layer::kColor);

  // Main camera
  auto maincam = Cfg<Camera>(scene, "camera.main")
    . Perspective(60, (float)width/height, .1, 300) 
    . Position(0, 1, 5)
    . Action<FlyingCameraController>(5)
    . Done();

  // Topview camera
  Cfg<Camera>(scene, "camera.topview")
    . Perspective(60, (float)width/height, 1, 500) 
    . Position(0, 10, 30)
    . EulerAngles(-30, 0, 0)
    . Parent(maincam)
    . Done();
 
  // Display to show the visualised quadtree 
  Cfg<Actor>(scene, "actor.screen.topview")
    . Model("Assets/screen.dsm", "Assets/Materials/overlay_texture.mat")
    . Texture(0, topview_tex)
    . Scale(.25, .25, 1)
    . Position(0.75, 0.75, 0)
    . Done();

  // Put knight to the world origin, just for orientation 
  Cfg<Actor>(scene, "actor.knight")
    . Model("Assets/knight.dsm", "Assets/Materials/texture.mat")
    . Tags(0, T{"onscreen", "topview"})
    . Done();

  // TODO: Use batch + instancing instead
  auto pool = Cfg<ActorPool>(scene, "actor.terr.patch", 1400)
    . Mesh(Patch(true))
    . Material("Assets/Materials/xzplane_border.mat")
    . Tags(0, T{"onscreen", "topview"})
    . Action<QuadTreeVisualiser>(maincam, 5, 20)
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
