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

#ifndef _FLIPTRIANGLES_H_31EF7D22_2E23_440E_861B_BD533425456C_
#define _FLIPTRIANGLES_H_31EF7D22_2E23_440E_861B_BD533425456C_ 

// CW -> CCW; CCW -> CW
struct FlipTriangles : public Action {
  int  num_attempts = 10;
  bool done         = false;

  FlipTriangles(std::shared_ptr<Transformation> t)
    : Action(t) {}

  void Update() override {
    if (done) return;

    if (auto f = transform->GetActor().GetComponent<MeshFilter>()) {
      if (auto m = f->GetMesh()) {
        if (!m->indices.empty()) {
          for (size_t i = 0; i < m->indices.size(); i+=3) {
            std::swap(m->indices.at(i), m->indices.at(i+2));
          }
          if (!m->normals.empty()) {
            for (auto& n: m->normals) {
              n = -n;
            }
          }
          f->SetMesh(m);

          done = true;
          transform->GetActor().RemoveAction<FlipTriangles>();
          return;
        }
      }
    }

    num_attempts--;
    if (num_attempts < 0 && !done) {
      transform->GetActor().RemoveAction<FlipTriangles>();
    }
  }
};

#endif // _FLIPTRIANGLES_H_31EF7D22_2E23_440E_861B_BD533425456C_
