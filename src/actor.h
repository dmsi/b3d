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

#ifndef _ACTOR_H_93C3406A_D7DA_4905_82F3_A682AA0FDCA1_
#define _ACTOR_H_93C3406A_D7DA_4905_82F3_A682AA0FDCA1_ 

#include <list>
#include <iostream>
#include <typeinfo>
#include <memory>
#include <string>
#include <algorithm>
#include <map>
#include "transformation.h"
#include "action.h"
#include "meshfilter.h"
#include "meshrenderer.h"

class Actor;

namespace Actor_cppness {
  template <class TComponent> auto GetComponent(Actor& actor);
  template <class TComponent> auto AddComponent(Actor& actor);
}

//////////////////////////////////////////////////////////////////////////////
// Unreal-Enine like Actor, or Unity3D GameObject ...
// TODO(DS) Remove Actions and Components!
//////////////////////////////////////////////////////////////////////////////
class Actor {
 public:
   // Turned it to shared ptr in order to implement child-parent relationship
   // among the Transforms and not worry too much about the lifetime.
   std::shared_ptr<Transformation> transform;

  explicit Actor(const std::string& name) : transform(new Transformation(*this)), name_(name) {
    std::cerr << "[actor " << GetName() << "] added!" << std::endl;
  }

  virtual ~Actor() {
    std::cerr << "[actor " << GetName() << "] deleted!" << std::endl;
  }

  ////////////////////////////////////////////////////////////////////////////
  // Start() not calling yet from scene. Moreover the actor should keep track
  // of recently added actions to call their start once before they're updated!
  ////////////////////////////////////////////////////////////////////////////
  virtual void Start() {
    for (auto& kv: actions_) {
      kv.second->Start();
    }
  }

  virtual void Update() {
    if (!actions_remove_queue_.empty()) {
      for (auto id: actions_remove_queue_) {
        actions_start_queue_.erase(id);
        actions_.erase(id);
      }
    }

    if (!actions_start_queue_.empty()) {
      for (auto& kv: actions_start_queue_) {
        kv.second->Start();
      }
      actions_start_queue_.clear();
    }
    for (auto& kv: actions_) {
      kv.second->Update();
    }
  }

  virtual void PreDraw() {
    for (auto& kv: actions_) {
      kv.second->PreDraw();
    }
  }

  virtual void PostDraw() {
    for (auto& kv: actions_) {
      kv.second->PostDraw();
    }
  }

  const std::string& GetName() const {
    return name_;
  }

  ////////////////////////////////////////////////////////////////////////////
  // TODO Replace it with Component storage impl (ECS-like)
  ////////////////////////////////////////////////////////////////////////////
  template <typename TAction, typename... TArgs>
  auto AddAction(TArgs&&... args) {
    auto& ti = typeid(TAction);
    auto id = ti.hash_code();
    if (actions_.find(id) == actions_.end()) {
      ActionPtr action = ActionPtr(new TAction(transform, std::forward<TArgs>(args)...));
      actions_[id] = action; 
      actions_start_queue_[id] = action;
      std::cerr << "[actor " << GetName() << "] " << "Action " << ti.name() << " added!" << std::endl;
      //return std::shared_ptr<TAction>(std::dynamic_pointer_cast<TAction>(actions_[id]));
      return std::dynamic_pointer_cast<TAction>(action);
    } else {
      throw std::logic_error("Actor::AddAction() - action is already added " + 
                             std::string(ti.name()));
    }
  }

  ////////////////////////////////////////////////////////////////////////////
  // Returns std::shared_ptr<TAction>
  ////////////////////////////////////////////////////////////////////////////
  template <class TAction>
  auto GetAction() {
    std::cerr << "Looking for action..." << std::endl;
    auto it = actions_.find(typeid(TAction).hash_code());
    if (it != actions_.end()) {
      return it->second;
    }
    return ActionPtr();
  }

  // Need a better implementation of the whole thing - can
  // keep track only of ID and mark actions disabled to avoid
  // further updates, etc calls.
  template <class TAction>
  void RemoveAction() {
    actions_remove_queue_.emplace(typeid(TAction).hash_code());
  }

  ////////////////////////////////////////////////////////////////////////////
  // Returns std::shared_ptr<TComponent>
  ////////////////////////////////////////////////////////////////////////////
  template <class TComponent>
  auto AddComponent() {
    return Actor_cppness::AddComponent<TComponent>(*this);
  }

  ////////////////////////////////////////////////////////////////////////////
  // Returns std::shared_ptr<TComponent>
  ////////////////////////////////////////////////////////////////////////////
  template <class TComponent>
  auto GetComponent() {
    return Actor_cppness::GetComponent<TComponent>(*this);
  }

 public: // Security brich, but i dont see any other simple solution
  using ActionPtr = std::shared_ptr<Action>;
  using ActionId  = std::size_t;

  std::set<ActionId>               actions_remove_queue_; 
  std::map<ActionId, ActionPtr>    actions_start_queue_;
  std::map<ActionId, ActionPtr>    actions_;
  std::string                      name_;

  std::shared_ptr<MeshFilter>      mesh_filter_;
  std::shared_ptr<MeshRenderer>    mesh_renderer_;

};

#include "actor.inl"

#endif // _ACTOR_H_93C3406A_D7DA_4905_82F3_A682AA0FDCA1_
