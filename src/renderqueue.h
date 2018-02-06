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

#ifndef _RENDERQUEUE_H_9F22FD78_32A1_4315_9A4D_E1D79DC422E7_
#define _RENDERQUEUE_H_9F22FD78_32A1_4315_9A4D_E1D79DC422E7_ 

#include "actor.h"
#include "material/material.h"
#include "material/pass.h"
#include "common/tags.h"
#include "common/logging.h"
#include <memory>
#include <map>

class Scene;
class Camera;

////////////////////////////////////////////////////////////////////////////
// Unites many actors under one render pass 
////////////////////////////////////////////////////////////////////////////
class RenderPassSubQueue {
 public:
  RenderPassSubQueue(std::shared_ptr<Pass> pass) : pass_(pass) {
  }
  void AddActor(std::shared_ptr<Actor> actor);
  void Draw(Scene& scene, Camera& camera);

  int GetPriority() const {
    if (!pass_) {
      ABORT_F("Render pass is nullptr");
    }
    return pass_->GetQueue();
  }

 private:
  std::shared_ptr<Pass>             pass_;
  std::shared_ptr<Material>         material_;
  std::list<std::shared_ptr<Actor>> actors_;
};

////////////////////////////////////////////////////////////////////////////
// Combines RenderPassSubQueue into a priority render queue 
////////////////////////////////////////////////////////////////////////////
class RenderQueue {
 public:
  void AddActor(std::shared_ptr<Actor> actor, const Tags& tags);
  void Clear() {the_queue_.clear();}
  void Draw(Scene& scene, Camera& camera);

 private:
  typedef std::map< std::shared_ptr<Pass>, RenderPassSubQueue > RenderPassQueue;

  RenderPassQueue& GetRenderPassQueue(std::shared_ptr<Pass> pass);
  RenderPassSubQueue& GetRenderPassSubQueue(std::shared_ptr<Pass> pass, RenderPassQueue& pass_q);

  std::map<int, RenderPassQueue> the_queue_;
};


#endif // _RENDERQUEUE_H_9F22FD78_32A1_4315_9A4D_E1D79DC422E7_
