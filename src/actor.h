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
#include <type_traits>
#include "transformation.h"
#include "action.h"
#include "meshfilter.h"
#include "meshrenderer.h"
#include "common/util.h"
#include "common/logging.h"


//////////////////////////////////////////////////////////////////////////////
// Unreal-Enine like Actor, or Unity3D GameObject ...
//////////////////////////////////////////////////////////////////////////////
class Actor {
 public:
  // Turned it to shared ptr in order to implement child-parent relationship
  // among the Transforms and not worry too much about the lifetime.
  std::shared_ptr<Transformation> transform;

  explicit Actor(const std::string& name) : 
    transform(new Transformation(*this)), name_(name), alive_(true) {
    LOG_F(INFO, "Actor added: %s", GetName().c_str());
  }

  virtual ~Actor() {
    LOG_F(INFO, "Actor deleted: %s", GetName().c_str());
  }

  ////////////////////////////////////////////////////////////////////////////
  // Actor states
  //  IsVisible() == false => being updated, but not drawn 
  //  IsActive() == false => not being updated nor drawn
  //  IsAlive() == false => not being updated nor drawn, shall be removed.
  //
  //  States are recursive inclusive, i.e. 
  //   IsAlive() == false includes IsActive() == false
  //   IsActive() == false means IsVisible() == false
  //
  //  IMPL
  //   std::bitset<...> with states kUpdateFlag, kDrawFlag, etc.?
  //
  //  NOTE
  //   Due to optimization reasons in gourps like batch object and object
  //   pools this behaviour can be optimized. I.e. IsDead() might not lead to
  //   physically removing the actor immidiately, but it guarantess no updates
  //   and draws.
  ////////////////////////////////////////////////////////////////////////////

  bool IsVisible() const;
  bool IsActive() const;
  bool IsAlive() const {return alive_;}
  bool IsStatic() const;
  bool IsSuperStatic() const;

  void Die() {
    alive_ = false;
  }
  void Alive() {
    alive_ = true;
  }

  ////////////////////////////////////////////////////////////////////////////
  // Called once after adding, during the current or the next frame.
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

  // Arbitrary extra information about the Actor. To fill last coloumt
  // of 4x4 transformation matrix for StdBatch. 
  // Did not think of anything better....
  void SetExtra(float x, float y, float z, float w) {
    extra_.x = x;
    extra_.y = y;
    extra_.z = z;
    extra_.w = w;
  }

  void GetExtra(float& x, float& y, float& z, float& w) const {
    x = extra_.x;
    y = extra_.y;
    z = extra_.z;
    w = extra_.w;
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
      //ActionPtr action = ActionPtr(new TAction(transform, args...));
      actions_[id] = action; 
      actions_start_queue_[id] = action;
      //LOG_F(INFO, "Action added: %s   %s", GetName().c_str(), ti.name());
      //return std::shared_ptr<TAction>(std::dynamic_pointer_cast<TAction>(actions_[id]));
      return std::dynamic_pointer_cast<TAction>(action);
    } else {
      ABORT_F("Action already added %s", ti.name());
    }
  }

  ////////////////////////////////////////////////////////////////////////////
  // Returns std::shared_ptr<TAction>
  ////////////////////////////////////////////////////////////////////////////
  template <class TAction>
  auto GetAction() {
    auto it = actions_.find(typeid(TAction).hash_code());
    if (it != actions_.end()) {
      //return it->second;
      return std::static_pointer_cast<TAction>(it->second);
    }
    return std::shared_ptr<TAction>(); 
  }

  // Need a better implementation of the whole thing - can
  // keep track only of ID and mark actions disabled to avoid
  // further updates, etc calls.
  template <class TAction>
  void RemoveAction() {
    actions_remove_queue_.emplace(typeid(TAction).hash_code());
  }

  void RemoveAllActions() {
    actions_remove_queue_.clear();
    actions_start_queue_.clear();
    actions_.clear();
  }

  ////////////////////////////////////////////////////////////////////////////
  // Creates component and returns shared pointer to it, args bypassed to 
  // TComponent constructor.
  ////////////////////////////////////////////////////////////////////////////
  template <class TComponent, typename... TArgs>
  auto AddComponent(TArgs&&... args); 

  ////////////////////////////////////////////////////////////////////////////
  // Returns shared pointer to component
  ////////////////////////////////////////////////////////////////////////////
  template <class TComponent>
  auto GetComponent();

  ////////////////////////////////////////////////////////////////////////////
  // Replaces component
  ////////////////////////////////////////////////////////////////////////////
  template <class TComponent>
  void SetComponent(std::shared_ptr<TComponent> component);


 private: 
  using ActionPtr = std::shared_ptr<Action>;
  using ActionId  = std::size_t;

  std::set<ActionId>               actions_remove_queue_; 
  std::map<ActionId, ActionPtr>    actions_start_queue_;
  std::map<ActionId, ActionPtr>    actions_;
  std::string                      name_;

  std::shared_ptr<MeshFilterBase>  mesh_filter_;
  std::shared_ptr<MeshRenderer>    mesh_renderer_;

  glm::vec4                        extra_;

  bool                             alive_;
};

#include "actor.inl"

#endif // _ACTOR_H_93C3406A_D7DA_4905_82F3_A682AA0FDCA1_
