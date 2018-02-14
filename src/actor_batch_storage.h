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

#ifndef _BATCH_ACTOR_H_4D4014F1_04C0_4D55_AD48_02F3579C23E7_
#define _BATCH_ACTOR_H_4D4014F1_04C0_4D55_AD48_02F3579C23E7_ 

#include "actor_batch.h"
#include "common/util.h"
#include "common/debug.h"

///////////////////////////////////////////////////////////////////////////////
// BatchStorage, stores Batches and Actors which share same memory layout.
///////////////////////////////////////////////////////////////////////////////
//template <typename TLayout, typename TActor, typename TBatch>
template <typename TBatch>
struct BatchStorage {
  template <typename T> using Ptr = std::shared_ptr<T>;
  using TLayout = typename TBatch::Layout;
  using TActor  = typename TBatch::TActor;

  template <typename... TArgs>
  auto AddActor(const std::string& name, Ptr<TBatch> batch, TArgs&&... args) {
    assert(batch);
    assert(batches_.find(batch->GetName()) != batches_.end());
    assert(actors_.find(name) == actors_.end());
    auto actor = std::make_shared<TActor>(name, std::forward<TArgs>(args)...);
    batch->Add(actor);
    return actors_.emplace(name, actor).first->second; // like ... for real?
  }

  template <typename... TArgs>
  auto AddBatch(const std::string& name, TArgs&&... args) {
    assert(batches_.find(name) == batches_.end());
    auto batch = std::make_shared<TBatch>(std::forward<TArgs>(args)..., name);
    return batches_.emplace(name, batch).first->second;
  }

  // TODO I am using std::string as a key/id left in right. This is not friendly
  // with performance in general.
  // It makes sense to replace std::string Name with a custom class.
  // class Name or class HashString, which basically uses integer hashes as keys.
  // But has int/string interface for construction. It will greatly help in
  // optimization area.
  Ptr<TActor> GetActor(const std::string& name) {
    return cppness
      ::MapForSharedPtr<decltype(actors_)>
      ::JustGetFromMap(name, actors_);
  }

  Ptr<TBatch> GetBatch(const std::string& name) {
    return cppness
      ::MapForSharedPtr<decltype(batches_)>
      ::JustGetFromMap(name, batches_);
  }

  // TODO
  // Accessed from the scene directly for now (for simplicity). 
  std::map<std::string, Ptr<TActor>> actors_;
  std::map<std::string, Ptr<TBatch>> batches_;
};

///////////////////////////////////////////////////////////////////////////////
// Pretty standard batch which stores glm::mat4 in the common memory.
///////////////////////////////////////////////////////////////////////////////
namespace StdBatch {
  // class HeapMat4::DataType;
  // static void HeapMat4::Update(TActor*, DataType*)
  struct HeapMat4 {
    using DataType = glm::mat4;
    DataType world;

    template <typename TActor>
      static void Update(TActor* a, DataType* data) {
        assert(a && data);
        a->transform->GetMatrix(*data);
        // Pack extra information to the bottom row, shader will
        // replace it for the matrix by 0 0 0 1
        a->GetExtra( (*data)[0][3], 
                     (*data)[1][3], 
                     (*data)[2][3], 
                     (*data)[3][3] );
      }
  };
  using Layout  = AttributeLayout<glm::vec4, glm::vec4, glm::vec4, glm::vec4>;
  using Actor   = ActorInBatch<Layout, HeapMat4>;
  using Batch   = BatchRoot<Actor>;
  using Storage = BatchStorage<Batch>;
};

#endif // _BATCH_ACTOR_H_4D4014F1_04C0_4D55_AD48_02F3579C23E7_
