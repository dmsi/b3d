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

#include "renderqueue.h"
#include "scene.h"

////////////////////////////////////////////////////////////////////////////
// RenderPassSubQueue 
////////////////////////////////////////////////////////////////////////////
void RenderPassSubQueue::AddActor(std::shared_ptr<Actor> actor) {
  auto mesh_renderer = actor->GetComponent<MeshRenderer>();
  if (mesh_renderer) {
    auto material = mesh_renderer->GetMaterial();
    if (!material_) {
      material_ = material;
    } else if (material != material_) {
      ABORT_F("Material mistmatch");
    }
    actors_.push_back(actor);
  }
}

void RenderPassSubQueue::Draw(Scene& scene, Camera& camera) {
  glm::mat4 pvm_matrix(1.0f);
  glm::mat4 model_matrix(1.0f);
  glm::mat4 view_matrix(1.0f);
  glm::mat4 proj_matrix(1.0f);
  material_->Bind();
  pass_->Bind();

  for (auto& actor: actors_) {
    std::shared_ptr<MeshRenderer> mesh_renderer = actor->GetComponent<MeshRenderer>();
    std::shared_ptr<MeshFilter>   mesh_filter = actor->GetComponent<MeshFilter>();

    if (mesh_renderer && mesh_filter) {
      actor->PreDraw();

      scene.SetSceneUniforms(*pass_, camera);

      // TODO is there a better way of doing this without getting view matrix each time?
      camera.GetViewMatrix(view_matrix);
      camera.GetProjectionMatrix(proj_matrix);
      actor->transform->GetMatrix(model_matrix);
      pvm_matrix = proj_matrix * view_matrix * model_matrix;
      pass_->SuPvmMatrix(pvm_matrix);
      pass_->SuMMatrix(model_matrix);
      pass_->SuVMatrix(view_matrix);
      pass_->SuPMatrix(proj_matrix);

      mesh_renderer->DrawCall(*mesh_filter);

      actor->PostDraw();
    } 
  }
  pass_->Unbind();
  material_->Unbind();
}

////////////////////////////////////////////////////////////////////////////
// RenderQueue 
////////////////////////////////////////////////////////////////////////////
void RenderQueue::AddActor(std::shared_ptr<Actor> actor, const Tags& tags) {
  if (auto mrend = actor->GetComponent<MeshRenderer>()) {
    if (auto mtrl = mrend->GetMaterial()) {
      for (auto it = mtrl->begin(); it != mtrl->end(); ++it) {
        auto pass = *it;
        if (pass->CheckTags(tags)) {
          RenderPassQueue& pass_q = GetRenderPassQueue(pass);
          RenderPassSubQueue& pass_sq = GetRenderPassSubQueue(pass, pass_q);
          pass_sq.AddActor(actor);
        }
      }
    }
  }
}
  
void RenderQueue::Draw(Scene& scene, Camera& camera) {
  // The order should be OK, from min priority to max
  for (auto& kv: the_queue_) {
    RenderPassQueue& pass_q = kv.second;
    for (auto& sub_kv: pass_q) {
      RenderPassSubQueue& pass_sq = sub_kv.second;
      pass_sq.Draw(scene, camera);
    }
  }
}
  
RenderQueue::RenderPassQueue& 
RenderQueue::GetRenderPassQueue(std::shared_ptr<Pass> pass) {
  RenderPassQueue& pass_q = the_queue_[pass->GetQueue()];
  return pass_q;
}

RenderPassSubQueue& RenderQueue::GetRenderPassSubQueue(std::shared_ptr<Pass> pass, RenderPassQueue& pass_q) {
  auto it = pass_q.find(pass);
  if (it == pass_q.end()) {
    auto result = pass_q.emplace(pass, RenderPassSubQueue(pass));
    if (!result.second) {
      ABORT_F("Cant emplace RenderQueue");
    }
    return result.first->second;
  }
  return it->second;
}
