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

#ifndef _MESHRENDERER_H_5B91DDA6_5485_41DC_A13B_6E5F7B334E05_
#define _MESHRENDERER_H_5B91DDA6_5485_41DC_A13B_6E5F7B334E05_ 

#include "gl_main.h"
#include "meshfilter.h"
#include "material/material.h"
#include <memory>

//////////////////////////////////////////////////////////////////////////////
// Even more Unity3D-ness 
//////////////////////////////////////////////////////////////////////////////
class MeshRenderer {
 public:
  void SetMaterial(std::shared_ptr<Material> material) {
    material_ = material;
  }
  
  std::shared_ptr<Material> GetMaterial() {
    return material_;
  }

  ////////////////////////////////////////////////////////////////////////////
  // TODO(DS) add support for other types that GL_TRIANGLES
  ////////////////////////////////////////////////////////////////////////////
  void DrawCall(MeshFilter& mesh_filter) {
    if (auto mesh = mesh_filter.GetMesh()) {
      mesh_filter.Bind();
      if (mesh_filter.GetSlot(MeshFilter::Slot::kIndices)) {
        glDrawElements(GL_TRIANGLES, mesh->indices.size(), GL_UNSIGNED_SHORT, nullptr);
      } else {
        glDrawArrays(GL_TRIANGLES, 0, mesh->vertices.size());
      }
      mesh_filter.Unbind();
    }
  }

 private:
  std::shared_ptr<Material> material_;
};


#endif // _MESHRENDERER_H_5B91DDA6_5485_41DC_A13B_6E5F7B334E05_
