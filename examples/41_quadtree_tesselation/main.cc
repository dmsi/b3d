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

// 4 vertex patch -1 +1 in xz plane
auto Patch() {
  using glm::vec3;
  auto mesh = std::make_shared<Mesh>();

  mesh->vertices = {
    vec3(-1, 0, -1),
    vec3( 1, 0, -1),
    vec3(-1, 0,  1),
    vec3( 1, 0,  1),
  };

  return mesh;
}

// Quick and dirty quadtree
// nw -x+z
// ne +x+z
// se +x-z
// nw -x-z
struct QuadTreeNode {
  QuadTreeNode* childs[4] = {nullptr, nullptr, nullptr, nullptr};

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

  // Subdivide 'level' times
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
  size_t NodeCount() {
    size_t count = 0;
    for(size_t i = 0; i < 4; ++i) {
      if (childs[i]) count += childs[i]->NodeCount();
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
  size_t        n_subdivisions;

  QuadTree(size_t n_subdivisions, size_t size) {
    this->n_subdivisions = n_subdivisions;
    root = new QuadTreeNode();
    root->position = glm::vec3(0, 0, 0);
    root->size = size;

    if (n_subdivisions) {
      root->Subdivide(n_subdivisions);
    }
  }

  ~QuadTree() {
    if (root) {
      delete root;
      root = 0;
    }
  }
};

// Instanced quad-tree renderer 
struct QuadTreeRenderer : public Action {
  std::shared_ptr<QuadTree> tree;
  std::shared_ptr<Camera>   camera;
  size_t                    max_nodes;
  float                     max_elevation;
  
  // vec4 xyz - position, w - size
  // vec4 - lod color 
  using Layout = AttributeLayout<true, glm::vec4, glm::vec4>;
  std::vector<uint8_t> node_buffer;

  std::shared_ptr<MeshFilter> mesh_filter;
  std::shared_ptr<MeshRenderer> mesh_renderer;

  QuadTreeRenderer(std::shared_ptr<Transformation> t, 
                   std::shared_ptr<Camera> cam,
                   size_t levels, float size, float elevation)
    : Action(t), camera(cam) {
    // separator
    tree = std::make_shared<QuadTree>(levels, size);
    max_nodes = tree->root->NodeCount();
    node_buffer.resize(Layout::Stride() * max_nodes);
    max_elevation = elevation;

    LOG_SCOPE_F(INFO, "QuadTree");
    LOG_F(INFO, "Node count       : %zu", tree->root->NodeCount());
    LOG_F(INFO, "Tree levels      : %zu", tree->root->level);
    LOG_F(INFO, "Root node scale  : %.0f", tree->root->size);
    LOG_F(INFO, "Node vRAM        : %.1f MB", node_buffer.size() / (1024 * 1024.));
  }

  void Start() override {
    mesh_filter = GetActor().GetComponent<MeshFilter>();
    mesh_renderer = GetActor().GetComponent<MeshRenderer>();
    if (!mesh_filter || !mesh_renderer) {
      ABORT_F("MeshFilter/MeshRenderer not found");
    }
    
    mesh_renderer->primitive = MeshRenderer::kPtPatches;
    mesh_renderer->patch_size = mesh_filter->GetMesh()->vertices.size(); 

    // Adjust height map parameters
    if (auto mtrl = mesh_renderer->GetMaterial()) {
      auto hmap_tex = std::dynamic_pointer_cast<Texture2D>(mtrl->GetTexture(0));
      if (hmap_tex) {
        hmap_tex->wrap_u_mode = Texture::kWrapClamp;
        hmap_tex->wrap_v_mode = Texture::kWrapClamp;
        hmap_tex->filter_mode = Texture::kFilterBilinear;
        hmap_tex->Apply();
      }
    }
  }

  void Update() override {
    if (!mesh_filter || !mesh_renderer) {
      ABORT_F("MeshFilter/MeshRenderer not found");
    }
    mesh_renderer->n_instances = 0;
    Build(camera);
  }

  // Compare node diagonal vs distance to camera
  // Used for QuadTree subdivision LOD 
  bool LodDiagonal(QuadTreeNode* node) {
    glm::vec3 p = node->position; 
    glm::vec3 c = camera->transform->GetGlobalPosition(); 
    p.y = 0; c.y = 0; // xz plane only
    auto dc = glm::distance(p, c);
    // more the multiplier, higher the details overall...
    auto d = 8 * glm::distance(p + node->offset(QuadTreeNode::kNw),
                               p + node->offset(QuadTreeNode::kSe));
    return dc > d;
  }

  void PlacePatch(QuadTreeNode* node, const Color& color) {
    BufferView buff(&node_buffer[0], node_buffer.size());

    Layout::Set<0>(buff, mesh_renderer->n_instances, glm::vec4(node->position, node->size));
    Layout::Set<1>(buff, mesh_renderer->n_instances, color);

    mesh_renderer->n_instances++;
  }
  
  void Build(std::shared_ptr<Camera>& camera) {
    auto walker = [this, &camera](QuadTreeNode* node) -> bool {
      assert(node);
      auto& frustum = camera->GetFrustum();
      glm::vec3 min = node->position - node->size;
      glm::vec3 max = node->position + node->size;
      max.y = max_elevation; 
      bool test = frustum.TestAabb(min, max);

      if (!test) {
        // Frustum culling failed, dont go deeper 
        return false;
      }

      if (LodDiagonal(node) || node->level == 0) {
        auto c = Color(1, (tree->root->level-node->level) / (float)tree->root->level, 0, 1);
        PlacePatch(node, c);
        return false;
      }

      // Keep going deeper
      return true;
    };

    // Rebuild every frame
    tree->root->Traverse(walker);

    mesh_filter->
      Upload<Layout>(
          8, mesh_renderer->n_instances, max_nodes, &node_buffer[0], 
          VertexArrayObject::kUsageStream);
  }

  void PreDraw() override {
    if (auto m = GetActor().GetComponent<Material>()) {
      m->SetUniform("terrain_half_size", tree->root->size);
      m->SetUniform("max_elevation", max_elevation);
      m->SetUniform("tesselation_min_distance", 50.0f);
      m->SetUniform("tesselation_max_distance", 200.0f);
    }
  }
};

int main(int argc, char* argv[]) {
  using glm::vec3;
  Scene scene;

  // Initialize application.
  AppContext::Init(1280, 720, "Quadtree tesselation [b3d]", Profile("4 0 core"));
  AppContext::Instance().display.ShowCursor(false);
  int width = AppContext::Instance().display.GetWidth();
  int height = AppContext::Instance().display.GetHeight();

  // Setup render targets 
  Cfg<RenderTarget>(scene, "rt.screen", 2000)
    . Tags("onscreen")
    . Clear(.8, .8, .8, 1)
    . Done();
  
  // Main camera
  auto maincam = Cfg<Camera>(scene, "camera.main")
    . Perspective(60, (float)width/height, 1, 9000) 
    . Position(0, 100, 5)
    . Action<FlyingCameraController>(200)
    . Done();

  // Quadtree terrain
  Cfg<Actor>(scene, "actor.terrain")
    . Mesh(Patch())
    . Material("assets/materials/quadtree_tesselation.mat")
    . Action<QuadTreeRenderer>(maincam, 8, 4000, 1000)
    . Done();

  // Sun
  Cfg<Light>(scene, "light.sun", Light::kDirectional) 
    . EulerAngles(20, 180, 0)
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
