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

#ifndef _ACTOR_POOL_H_6D07BEC4_DD24_45A8_9C23_384E4311CE5B_
#define _ACTOR_POOL_H_6D07BEC4_DD24_45A8_9C23_384E4311CE5B_ 

///////////////////////////////////////////////////////////////////////////////
// Pool of actors.
//
// In order to rapidly spawn / despawn actors with same mesh and material.
//
// Like bullets.
///////////////////////////////////////////////////////////////////////////////
template <typename TActor>
class TActorPool : public Actor {
 public:
  // TARGS
  template <typename... TArgs>
  TActorPool(const std::string& name, size_t pool_size, TArgs&&... args)
    : Actor(name), size_(pool_size) {
    assert(pool_size > 0);
    for (size_t i = 0; i < pool_size; i++) {
      auto clone = std::make_shared<TActor>(
          MakeName(i),
          std::forward<TArgs>(args)...);
      dead_.emplace_back(clone);
    }
  }

  auto Get() {
    if (!dead_.empty()) {
      auto actor = dead_.back();
      actor->SetComponent(GetComponent<MeshRenderer>());
      actor->SetComponent(GetComponent<MeshFilter>());
      glm::vec4 extra;
      GetExtra(extra.x, extra.y, extra.z, extra.w);
      actor->SetExtra(extra.x, extra.y, extra.z, extra.w);
      actor->RemoveAllActions();
      actor->transform->SetLocalPosition(0, 0, 0);
      actor->transform->SetLocalEulerAngles(0, 0, 0);
      actor->transform->SetLocalScale(1, 1, 1);
      actor->Alive();

      dead_.pop_back();
      alive_.emplace_back(actor);
      return actor;
    } 
    return std::shared_ptr<TActor>(); 
  }

  auto begin() {
    return alive_.begin();
  }

  auto end() {
    return alive_.end();
  }

  void Update() override {
    Actor::Update();

    for (auto it = alive_.begin(); it != alive_.end(); ++it) {
      auto a = *it;
      // Manage alive -> dead transitions
      if (not a->IsAlive()) {
        it = alive_.erase(it);
        dead_.emplace_back(a);
        continue;
      }
    }
  }
   
 private:
  using Ptr = std::shared_ptr<TActor>;
  using Ls = std::list<Ptr>;

  std::string MakeName(size_t n) const {
    return GetName() + "inpool" + std::to_string(n); 
  }
  
  size_t size_; // alive_.size() + dead_.size()
  Ls     alive_;
  Ls     dead_;
};

using ActorPool = TActorPool<Actor>;

#endif // _ACTOR_POOL_H_6D07BEC4_DD24_45A8_9C23_384E4311CE5B_
