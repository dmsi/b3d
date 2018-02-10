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

#ifndef _PHYS_H_82BF7FBE_D775_4DF8_9E24_6F20DB5D20A0_
#define _PHYS_H_82BF7FBE_D775_4DF8_9E24_6F20DB5D20A0_ 

namespace Phys {

struct Aabb {
  glm::vec3 min;
  glm::vec3 max;
};

template <typename T, typename U>
bool CheckCollision(const T&, const U&);

template <>
bool CheckCollision<Aabb, Aabb>(const Aabb& a, const Aabb& b) {
  // Separathing Axis Theorem (SAT)
  
  // Exit with no intersection if found separated along an axis
  if(a.max.x < b.min.x or a.min.x > b.max.x) return false;
  if(a.max.y < b.min.y or a.min.y > b.max.y) return false;
  if(a.max.z < b.min.z or a.min.z > b.max.z) return false;

  // No separating axis found, therefor there is at least one overlapping axis
  return true;
}

// add actor Lifetime<> which removes actor when timer expires
// pre-load material and mesh for debug collision object and 
// spawn object on every collision
struct ColliderBox {
  glm::vec3 vertex[8];
};

struct RigidBody3D {
  int id;
};

Aabb CalculateBounds(const Mesh& mesh) {
  Aabb bounds = {glm::vec3(std::numeric_limits<float>::max()), 
                 glm::vec3(std::numeric_limits<float>::lowest())};

  for (auto& v : mesh.vertices) {
    if (bounds.min.x > v.x) bounds.min.x = v.x;
    if (bounds.min.y > v.y) bounds.min.y = v.y;
    if (bounds.min.z > v.z) bounds.min.z = v.z;
    
    if (bounds.max.x < v.x) bounds.max.x = v.x;
    if (bounds.max.y < v.y) bounds.max.y = v.y;
    if (bounds.max.z < v.z) bounds.max.z = v.z;
  }

  return bounds;
}

class SimulationPipeline {
 public:
  void SimulationStep() {
    UpdateWorldAabb();
    BroadPhase();
    NarrowPhase();
    ResolveContacts();
    Integrate();
  }

  // Recalculate AABBs to world
  void UpdateWorldAabb() {}

  // Select pairs whose AABBs intersetcs
  void BroadPhase() {}

  // Calculates collisions information for each pair
  // such as normal, penetration, etc
  void NarrowPhase() {}

  // Resolve the collisions
  void ResolveContacts() {}

  // Integrate, i.e. pos += dt * velocity... 
  void Integrate() {}

 private:
  std::vector<RigidBody3D> body_ls_;
  std::vector<Aabb>        world_aabb_ls_;
};

} // namespace Phys::

#endif // _PHYS_H_82BF7FBE_D775_4DF8_9E24_6F20DB5D20A0_
