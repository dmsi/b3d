//
// This source file is a part of $PROJECT_NAME$
//
// Copyright (C) $COPYRIGHT$
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

#ifndef _ACTIONS_H_D42DF0DD_E1ED_45AC_821A_4840BF89B2F9_
#define _ACTIONS_H_D42DF0DD_E1ED_45AC_821A_4840BF89B2F9_ 

struct Surface {
  Color diffuse        = Color(1);
  Color ambient        = Color(.1);
  Color specular       = Color(0);
  float shininess      = 0;
  bool  has_albedo_map = false;
};

// Maintain shader uniforms
struct ShadowShader : public Action {
  using CamPtr = std::shared_ptr<Camera>;
  using LhtPtr = std::shared_ptr<Light>;

  LhtPtr light;
  CamPtr shadowmap_camera;
  Surface surface;

  ShadowShader(std::shared_ptr<Transformation> t, 
               LhtPtr l, CamPtr c, Surface s) 
    : Action(t), light(l), shadowmap_camera(c), surface(s) {}

  void PreDraw() override {
    glm::mat4 view;
    glm::mat4 proj;
    glm::mat4 bias (
      0.5, 0.0, 0.0, 0.0,
      0.0, 0.5, 0.0, 0.0,
      0.0, 0.0, 0.5, 0.0,
      0.5, 0.5, 0.5, 1.0
    );

    shadowmap_camera->GetViewMatrix(view);
    shadowmap_camera->GetProjectionMatrix(proj);

    if (auto m = transform->GetActor().GetComponent<Material>()) {
      m->SetUniform("depth_bias_pvm", bias * proj * view);
      m->SetUniform("surface.diffuse", surface.diffuse);
      m->SetUniform("surface.ambient", surface.ambient);
      m->SetUniform("surface.specular", surface.specular);
      m->SetUniform("surface.shininess", surface.shininess);
    }
  }
};

// Sets shadow camera cuboid based on the light direction
struct ShadowMapCameraController : public Action {
  using LhtPtr = std::shared_ptr<Light>;

  LhtPtr sun;
  float  distance = 10.0f; // How far the sun is

  ShadowMapCameraController(std::shared_ptr<Transformation> t, LhtPtr l, float d = 10.0f)
    : Action(t), sun(l), distance(d) {}

  void Update() override {
    auto pos = -distance * sun->GetDirection();
    auto rot = sun->transform->GetLocalEulerAngles();
    rot.y = rot.y - 180;
    rot.x = -rot.x;

    transform->SetLocalPosition(pos);
    transform->SetLocalEulerAngles(rot);
  }
};

// Draws AABB
struct BoundsVisual : public Action {
  glm::vec3 local_min;
  glm::vec3 local_max;
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
      Phys::Aabb aabb = Phys::CalculateBounds(*m);
      local_min = aabb.min;
      local_max = aabb.max;
    }

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

    vec3 local_half_extents = 0.5f * (local_max - local_min);
    local_half_extents += vec3(margin);
    vec3 local_center = 0.5f * (local_max + local_min);
    mat4 mat;
    actor->transform->GetMatrix(mat); // add method
    vec3 center = vec3(mat * vec4(local_center, 1));
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
    TransformAabb(min, max, .0);
    glm::vec3 extent = max - min; 
    glm::vec3 center = 0.5f * (max + min); 
    transform->SetLocalPosition(center);
    transform->SetLocalScale(extent);
  }
};


#endif // _ACTIONS_H_D42DF0DD_E1ED_45AC_821A_4840BF89B2F9_
