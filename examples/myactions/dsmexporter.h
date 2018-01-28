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

#ifndef _DSMEXPORTER_H_445FC0BC_F5FF_462D_8996_4A93C444557B_
#define _DSMEXPORTER_H_445FC0BC_F5FF_462D_8996_4A93C444557B_ 

struct DsmExporter : public Action {
  std::string filename;
  int         num_attempts = 10;
  bool        exported = false;

  DsmExporter(std::shared_ptr<Transformation> transform,
              const std::string& afilename) 
    : Action(transform), filename(afilename) {}

  void Update() override {
    if (!exported && auto f = transform->GetActor().GetComponent<MeshFilter>()) {
      if (auto m = f->GetMesh()) {
        MeshLoader::ExportDsm(m, filename);
        exported = true;
        transform->GetActor().RemoveAction<DsmExporter>();
        return;
      }
    }

    num_attempts--;
    if (num_attempts < 0 && !exported) {
      transform->GetActor().RemoveAction<DsmExporter>();
    }
  }
};

#endif // _DSMEXPORTER_H_445FC0BC_F5FF_462D_8996_4A93C444557B_
