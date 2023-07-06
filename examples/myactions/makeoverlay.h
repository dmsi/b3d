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

#ifndef _MAKEOVERLAY_H_501ADB99_BBC1_4C8C_88EE_68CE1F4F997C_
#define _MAKEOVERLAY_H_501ADB99_BBC1_4C8C_88EE_68CE1F4F997C_ 

//////////////////////////////////////////////////////////////////////////////
// Makes and overlay window and draws a texture on it.
// Position it in one of the 4x4 matrix slots:
//  kTopX    - top row of the screen, from left to right;
//  kAboveX  - one level above the center of the screen;
//  kBelowX  - one level below the center of the screen;
//  kBottomX - the bottom level of the screen.
// 16 overlays in all possible positions 100% covers the screen.
//
// Usage example:
// scene.Add<Action>("any.uniq.name")
//   ->AddAction<MakeOverlayArea>(texture, MakeOverlayArea::kBottom0);
//////////////////////////////////////////////////////////////////////////////
struct MakeOverlayArea : public Action {
  // 4x4 layout matrix
  enum {
    kTop0    = 0, kTop1,    kTop2,    kTop3,
    kAbove0,      kAbove1,  kAbove2,  kAbove3,
    kBelow0,      kBelow1,  kBelow2,  kBelow3,
    kBottom0,     kBottom1, kBottom2, kBottom3,
    kTotal
  };

  std::shared_ptr<Texture> texture;
  int                      row       = kBottom0;
  bool                     invert_uv = false;
  Color                    color     = Color(1, 1, 1, 1);

  MakeOverlayArea(std::shared_ptr<Transformation> transform, 
                  std::shared_ptr<Texture> atexture,
                  int arow = kBottom0,
                  bool ainvert_uv = false,
                  Color acolor = Color(1))
    : Action(transform),
      texture(atexture),
      row(arow),
      invert_uv(ainvert_uv),
      color(std::move(acolor)) {
    if (!texture) {
      ABORT_F("Invalid texture");
    }

    glm::vec3 offset( -.25,  -.25,  0);
    glm::vec3 scale (  .5,    .5,   1);

    // Horizontal offset
    switch (row) {
      case kTop0: 
      case kAbove0:
      case kBelow0:
      case kBottom0: 
        offset = glm::vec3( -1,  0,  0); 
        break;

      case kTop1: 
      case kAbove1:
      case kBelow1:
      case kBottom1: 
        offset = glm::vec3(-.5,  0,  0); 
        break;

      case kTop2: 
      case kAbove2:
      case kBelow2:
      case kBottom2: 
        offset = glm::vec3(  0,  0,  0); 
        break;

      case kTop3: 
      case kAbove3:
      case kBelow3:
      case kBottom3: 
        offset = glm::vec3( .5,  0,  0); 
        break;
    };

    // Vertical offset
    if        (row >= kTop0 && row <= kTop3) {
      offset += glm::vec3(0,  .5, 0);
    } else if (row >= kAbove0 && row <= kAbove3) {
      offset += glm::vec3(0,   0, 0);
    } else if (row >= kBelow0 && row <= kBelow3) {
      offset += glm::vec3(0, -.5, 0);
    } else if (row >= kBottom0 && row <= kBottom3) {
      offset += glm::vec3(0,  -1, 0);
    }

    auto& actor = transform->GetActor();
    actor.AddComponent<MeshRenderer>()
      ->SetMaterial( MaterialLoader::Load("assets/materials/core/screen_color_r2t.mat") );
    actor.AddComponent<MeshFilter>()
      ->SetMesh( MakeMesh(offset, scale) );

    if (texture) {
      actor.GetComponent<MeshRenderer>()->GetMaterial()->SetTexture(0, texture);
    }

  }

  void PreDraw() override {
    if (auto mtrl = transform->GetActor().GetComponent<Material>()) {
      mtrl->SetUniform("Color", color);
      mtrl->SetUniform("Invert_uv", (int)invert_uv);
    }
  }

 private:
  std::shared_ptr<Mesh> MakeMesh(const glm::vec3& offset = glm::vec3(0),
                                 const glm::vec3& scale = glm::vec3(1)) {
    // Counter clockwise quad with UV set
    std::shared_ptr<Mesh> mesh(new Mesh());
    mesh->vertices = { 
      glm::vec3(0, 0, 0), // 0 - bottom left
      glm::vec3(1, 0, 0), // 1 - bottom right
      glm::vec3(1, 1, 0), // 2 - top right
      glm::vec3(0, 1, 0)  // 3 - top left
    }; 

    mesh->uv = { 
      glm::vec2(0, 0),    // 0 - bottom left
      glm::vec2(1, 0),    // 1 - bottom right
      glm::vec2(1, 1),    // 2 - top right
      glm::vec2(0, 1)     // 3 - top left
    }; 

    mesh->indices = {
      0, 1, 2, 0, 2, 3    // CCW
    };  

    for (auto& v: mesh->vertices) {
      v *= scale;
      v += offset;
    }

    return mesh;
  }
};

#endif // _MAKEOVERLAY_H_501ADB99_BBC1_4C8C_88EE_68CE1F4F997C_
