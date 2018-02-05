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

#ifndef _BATCH_POOL_H_E2C5F652_4D75_486D_8349_9A871FA00BFE_
#define _BATCH_POOL_H_E2C5F652_4D75_486D_8349_9A871FA00BFE_ 

#include "attributelayout.h"
#include <memory>
#include <map>

///////////////////////////////////////////////////////////////////////////////
// class BatchMemoryHeap;
//  Memory heap where all actors from the same batch keep their stuff, such as
//  model to world transformation matrix. This memory is syncronized with the 
//  VAO attributes in accordance wiht Layout specification.
//
// class ActorInBatch;
//  Actor from the batch. All actors in the same batch shared mesh and material.
//  So MeshFilter/MeshRenderer cannot be added.
//  On the other hand they have their own properties, such as position,
//  orientation and scale.
//
// class BatchRoot;
//  Main batch actor, responsible for drawing the others in the batch.
//  Can have MeshFilter, MeshRenderer and Actions.
///////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
// Memory heap where all actors in a batch keep their data.
// For instance world transformation matrix can be stored there. This memory
// will be uploaded to VAO attributes as TLayout specifies.
///////////////////////////////////////////////////////////////////////////////
template <typename TLayout>
class BatchMemoryHeap {
 public:
  // size - number of elements, not bytes!
  explicit BatchMemoryHeap(size_t size)
    :   heap_(size * TLayout::Stride())
  {}

  void* Memory(size_t index) {
    auto const offset = index * TLayout::Stride();
    return (void*)&heap_.at(offset);
  }

  void* Memory() {
    return (void*)&heap_[0];
  }

  size_t Size() const {
    return heap_.size();
  }

 private:
  std::vector<uint8_t> heap_;
};

///////////////////////////////////////////////////////////////////////////////
// ActorInBatch
//
// Actor which is used in instanced rendering. On the one hand it benefits 
// from all actor goodies, such as Actions, on the other hand all the actors
// in a batch rendering in one Draw Call.
//
// Templates:
// TLayout - concrete type of AttributeLayout<...>
// THeapStruct - Pool data structure, shall follow the spec:
//  struct THeapStruct {
//    // Pool POD data type
//    class DataType;
//    
//    // Update DataType*, TActor - is concrete Actor.
//    template <typename TActor>
//    void Update(TActor*, DataType*);
//  };
//
//  TODO
//   - Prohibit adding components such as MeshFilter and MeshRenderer
///////////////////////////////////////////////////////////////////////////////
template <typename TLayout, typename THeapStruct>
class ActorInBatch : public Actor {
 public:
  using DataType = typename THeapStruct::DataType;
  using Layout   = TLayout;
  using Heap     = BatchMemoryHeap<Layout>; 

  static_assert(TLayout::Stride() == sizeof(DataType), 
      "TLayout does not match THeapStruct!");

  template <typename... TArgs>
  explicit ActorInBatch(TArgs&&... args) 
    :   Actor(std::forward<TArgs>(args)...)
  {}

  // AssignToMemoryHeap
  void AssignToMemoryHeap(std::shared_ptr<Heap> heap, size_t index) {
    // Shall not be assigned twice.
    assert(heap);
    assert(not heap_);
    heap_ = heap;
    pool_data_ = new (heap->Memory(index)) DataType();
  }

  // UpdateMemoryHeap
  // WriteToMemoryHeap
  void UpdateMemoryHeap() {
    THeapStruct::Update(this, pool_data_);
  }
  
  /////////////////////////////////////////////////////////////////////////////
  // Shadow AddComponent in order to forbid having ActorInBatch components of 
  // MeshRenderer and MeshFilter different than batch mum.
  //
  // TODO 
  // As this is no a virtual method, the Actor::AddComponent can be accessed
  // via casting ActorInBatch to Actor pointer! Good news - it will not have
  // any impact anyway, other than keeping shared pointers to mesh and material.
  /////////////////////////////////////////////////////////////////////////////
  template <class TComponent, typename... TArgs>
  auto AddComponent(TArgs&&... args) {
    static_assert(not std::is_same<TComponent, MeshRenderer>::value,
        "MeshRenderer is prohibited for ActorInBatch");
    static_assert(not std::is_same<TComponent, MeshFilter>::value,
        "MeshFilter is prohibited for ActorInBatch");

    Actor::AddComponent<TComponent>(std::forward<TArgs>(args)...);
  }

 private:
  std::shared_ptr<Heap> heap_;
  DataType*             pool_data_ = nullptr; // to be allocated in the heap_
};

///////////////////////////////////////////////////////////////////////////////
// Batch-control actor.
///////////////////////////////////////////////////////////////////////////////
template <typename TActorInBatch>
class BatchRoot : public Actor {
 public:
  using TActor = TActorInBatch;
  using Layout = typename TActorInBatch::Layout;
  using Heap   = BatchMemoryHeap<Layout>;

  template <typename... TArgs>
  explicit BatchRoot(size_t batch_size, TArgs&&... args)
    :   Actor(std::forward<TArgs>(args)...) 
      , heap_(new Heap(batch_size))
  {}

  void Add(std::shared_ptr<TActorInBatch> actor) {
    assert(actor_map_.size() < heap_->Size() / Layout::Stride());
    actor->AssignToMemoryHeap(heap_, actor_map_.size());
    AddToMap(actor);
    if (auto mr = GetComponent<MeshRenderer>()) {
      mr->n_instances = actor_map_.size();
    }
  }

  void Remove(std::shared_ptr<TActorInBatch> actor) {
    // it is kind of problematic to remove actors from the batch... 
    // possible ways:
    //  1. Remove from list and refragment the pool to group the rest 
    //     in the beginning
    //  2. Remove from the list and put 'removed' flag to the pool
    assert(false && "not implemented!");
  }

  size_t BatchSize() const {
    return actor_map_.size();
  }

  void UploadInstances() {
    auto const n_elements = actor_map_.size();
    if (auto mf = GetComponent<MeshFilter>()) {
      for (auto& kv: actor_map_) {
        kv.second->UpdateMemoryHeap();
      }

      if (n_elements) {
        mf->template UploadPerInstance<Layout>(
            8, n_elements, heap_->Memory(), VertexArrayObject::kUsageStream); 
      }
    }
  }

 private:
  using ActorPtr = std::shared_ptr<TActorInBatch>;
  using ActorMap = std::map<size_t, ActorPtr>; 

  void AddToMap(ActorPtr actor) {
    auto key = std::hash<ActorPtr>()(actor);
    auto res = actor_map_.emplace(key, actor).second;
    (void)res;
    assert(res);
  }

  // TODO add class for pool, keep it as shared pointer 
  // and keep copy of the pointer in the actor which belong 
  // to that pool..
  //std::vector<uint8_t> heap_;
  std::shared_ptr<Heap> heap_;
  ActorMap             actor_map_;
};

#endif // _BATCH_POOL_H_E2C5F652_4D75_486D_8349_9A871FA00BFE_
