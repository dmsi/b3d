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

#include <iostream>
#include <memory>
#include <array>
#include <stdexcept>
#include <cassert>
#include <set>

#include <chrono>
#include <thread>
  

template <class TComponent>
class ComponentStorage {
 public:
  using TypeID = std::size_t; 
  enum {kMaxActorComponents = 32};

  template <typename T> 
  bool Has() const {
    return lookup_storage_[GetTypeID<T>()] != nullptr;
  }

  template <typename T, typename... TArgs>
  auto Add(TArgs&&... args) {
    if (Has<T>()) {
      Remove<T>();
    }
    auto id = GetTypeID<T>();
    std::shared_ptr<T> ptr(new T(std::forward<TArgs>(args)...));
    lookup_storage_[id] = ptr; 
    iterable_storage_.emplace(ptr);
    return ptr;
  }

  template <typename T> 
  auto Get() {
    return GetByID(GetTypeID<T>());
  }

  auto GetByID(TypeID id) {
    assert(id < kMaxActorComponents);
    auto ptr = lookup_storage_[id];
    if (ptr) {
      return ptr;
    }
    throw std::logic_error("ComponentStorage::GetByID() - Component not found");
  }

  template <typename T>
  void Remove() {
    RemoveByID(GetTypeID<T>());
  }
  
  void RemoveByID(TypeID id) {
    assert(id < kMaxActorComponents);
    auto& ptr = lookup_storage_[id];
    if (ptr) {
      iterable_storage_.erase(ptr);
      ptr.reset();
    } else {
      throw std::logic_error("ComponentStorage::RemoveByID() - Component not found");
    }
  }
  
  template <typename T>
  auto GetTypeID() const {
    return ID<T>();
  }

  template <typename T>
  static TypeID ID() noexcept {
    static_assert (std::is_base_of<TComponent, T>::value, 
                   "Must be baed on TComponent");
    static TypeID idx = NewID();
    assert(idx < kMaxActorComponents);
    return idx;
  }

  auto begin() {
    return iterable_storage_.begin();
  }

  auto end() {
    return iterable_storage_.end();
  }

 private:
  using ComponentPtr = std::shared_ptr<TComponent>;
  using InternalStorage = std::array<ComponentPtr, kMaxActorComponents>;

  static TypeID NewID() {
    static std::atomic<TypeID> a_idx = 0;
    TypeID idx = a_idx++;
    return idx;
  }

  InternalStorage lookup_storage_; // lookup
  std::set<ComponentPtr> iterable_storage_;
};

bool Test() {
  class B0 {};
    class D0_B0 : public B0 {};
    class D1_B0 : public B0 {};
  class B1 {};
    class D0_B1 : public B1 {};
    class D1_B1 : public B1 {};

  using B0Storage = ComponentStorage<B0>;
  using B1Storage = ComponentStorage<B1>;

  B0Storage s_b0;
  B1Storage s0_b1, s1_b1;

  s_b0.Add<D0_B0>();
  s_b0.Add<D1_B0>();
  s0_b1.Add<D0_B1>();
  s0_b1.Add<D1_B1>();

  if (s_b0.GetTypeID<D1_B0>() != 1 || B0Storage::ID<D1_B0>() != 1) {
    return false;
  }
  if (s_b0.GetTypeID<D0_B0>() != 0 || B0Storage::ID<D0_B0>() != 0) {
    return false;
  }
  if (s1_b1.GetTypeID<D1_B1>() != 1 || B1Storage::ID<D1_B1>() != 1) {
    return false;
  }
  if (s1_b1.GetTypeID<D0_B1>() != 0 || B1Storage::ID<D0_B1>() != 0) {
    return false;
  }

  return true;
}

struct Action {
  virtual void Start()  {}
  virtual void Update() {}
};

struct Actor {
  using AStorage = ComponentStorage<Action>;

  void Update() {
    // Check rm Q
    if (rm_q_.any()) {
      for (int i = 0; i < rm_q_.size(); ++i) {
        if (rm_q_[i]) {
          actions_.RemoveByID(i);
          rm_q_[i] = false;
          start_q_[i] = false;
        }
      }
    }

    // Check start Q
    if (start_q_.any()) {
      for (int i = 0; i < start_q_.size(); ++i) {
        if (start_q_[i] == true) {
          actions_.GetByID(i)->Start();
          start_q_[i] = false;
        }
      }
    }
    for (auto& a: actions_) {
      a->Update();
    }
  }

  template <typename T, typename... TArgs>
  auto AddAction(TArgs&&... args) {
    auto a = actions_.Add<T>(std::forward<TArgs>(args)...);
    start_q_[actions_.GetTypeID<T>()] = true;
    return a;
  }

  template <typename T>
  auto RemoveAction() {
    auto id = actions_.GetTypeID<T>();
    rm_q_[id] = true;
  }

  template <typename T>
  bool HasAction() {
    return actions_.Has<T>();
  }

  AStorage actions_;
  std::bitset<AStorage::kMaxActorComponents> start_q_;
  std::bitset<AStorage::kMaxActorComponents> rm_q_;
};

#define PRNT(cls, adr, fn) std::cerr << "[" << adr << "] " << #fn << " - " << #cls << std::endl

struct Do : public Action {
  explicit Do()          { PRNT(Do, this, New); }
  virtual ~Do()          { PRNT(Do, this, Del); }
  void Start() override  { PRNT(Do, this, Str); } 
  void Update() override { PRNT(Do, this, Upd); } 
};

struct Undo: public Action {
  explicit Undo()        { PRNT(Undo, this, New); }
  virtual ~Undo()        { PRNT(Undo, this, Del); }
  void Start() override  { PRNT(Undo, this, Str); } 
  void Update() override { PRNT(Undo, this, Upd); } 
};

struct Tricky : public Action {
  Actor* owner;
  Tricky(Actor* owner) {this->owner = owner; PRNT(Tricky, this, New); } 
  virtual ~Tricky() { PRNT(Tricky, this, Del); }
  void Start() override { owner->AddAction<Undo>(); PRNT(Tricky, this, Str); } 
  void Update() override { 
    if (owner->HasAction<Do>()) owner->RemoveAction<Do>(); 
    else                        owner->AddAction<Do>();
    PRNT(Tricky, this, Upd);
  }
};

int main() {
  std::cerr << "TEST => " << (Test() ? "PASS" : "FAIL") << std::endl;

  Actor actor;
  actor.AddAction<Do>();
  actor.AddAction<Tricky>(&actor);
  actor.AddAction<Undo>();

  while(true) {
    std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    std::cerr << "------ tick..." << std::endl;
    actor.Update();
  }

  return 0;
}
