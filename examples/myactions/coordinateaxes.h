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

#ifndef _COORDINATEAXES_H_5E7AF158_37C5_4CCC_86FE_737B8DFE42E6_
#define _COORDINATEAXES_H_5E7AF158_37C5_4CCC_86FE_737B8DFE42E6_ 

struct SetAxisUniform : public Action {
  Color color = Color(1, 1, 1, 1);

  SetAxisUniform(std::shared_ptr<Transformation> transform, 
                 Color color) 
    : Action(transform) {
    this->color = color;
  }

  void PreDraw() override {
    if (auto mtrl = transform->GetActor().GetComponent<Material>()) {
      mtrl->SetUniform("color", color);
    }
  }
};

struct CoordinateAxes : public Action {
  std::shared_ptr<Camera> track_camera;
  std::shared_ptr<Camera> own_camera;

  // index - render target index
  // size - % .percent of screen width
  CoordinateAxes(std::shared_ptr<Transformation> t, 
                 Scene& scene, 
                 int index,
                 std::shared_ptr<Camera> cam,
                 float size)
    : Action(t), track_camera(cam) {
      assert(size > 0 && size <= 1);

      std::string cam_name = "camera.dbg.axes";
      own_camera = scene.Get<Camera>(cam_name);
      if (!own_camera) {
        own_camera = Cfg<Camera>(scene, cam_name) 
          . Perspective(60, 1, .1, 100)
          . Position(0, 0, 10)
          . Done();
      }

      // Render target
      std::string ext_name = std::to_string((uint64_t)this);

      const float width_p = size;
      const float ratio = (float)GetDisplay().GetWidth() / GetDisplay().GetHeight();
      const std::string tag = "xyz" + ext_name;

      using T = std::vector<std::string>;
      int w = GetDisplay().GetWidth() * width_p;
      auto xyz_tex = Cfg<RenderTarget>(scene, "rt.dbg.xyz" + ext_name, index)
        . Camera      (cam_name)
        . Tags        (tag)
        . Type        (FrameBuffer::kTexture2D)
        . Resolution  (w, w)
        . Layer       (Layer::kColor, Layer::kReadWrite, Texture::kFilterPoint)
        . Layer       (Layer::kDepth, Layer::kWrite)
        . Clear       (.8, .4, .4, 0)
        . Done        ()
        ->GetLayerAsTexture(0, Layer::kColor);
  
      Cfg<Actor>(scene, "actor.dbg.axis.x" + ext_name)
        . Model       ("assets/models/arrow.dsm", "assets/materials/axis.mat")
        . Tags        (0, T{tag})
        . EulerAngles (0, 90, 0)
        . Action<SetAxisUniform>(Color(1, 0, 0, 1))
        . Done();

      Cfg<Actor>(scene, "actor.dbg.axis.y" + ext_name)
        . Model       ("assets/models/arrow.dsm", "assets/materials/axis.mat")
        . Tags        (0, T{tag})
        . EulerAngles (-90, 0, 0)
        . Action<SetAxisUniform>(Color(0, 1, 0, 1))
        . Done();

      Cfg<Actor>(scene, "actor.dbg.axis.z" + ext_name)
        . Model       ("assets/models/arrow.dsm", "assets/materials/axis.mat")
        . Tags        (0, T{tag})
        . Action<SetAxisUniform>(Color(0, 0, 1, 1))
        . Done();

      Cfg<Actor>(scene, "actor.dbg.axis.origin" + ext_name)
        . Model("assets/models/sphere.dsm", "assets/materials/axis.mat")
        . Tags        (0, T{tag})
        . Scale       (.3, .3, .3)
        . Action<SetAxisUniform>(Color(.8, .8, 0, 1))
        . Done();

      float sx = width_p / ratio, sy = width_p;
      Cfg<Actor>(scene, "actor.dbg.display." + ext_name)
        . Model("assets/models/screen.dsm", "assets/materials/overlay_axes.mat")
        . Tags    (0, T{"onscreen"})
        . Texture (0, xyz_tex)
        . Scale   (sx, sy, 0)
        . Position(1-sx, 1-sy, 0)
        . Done();
    }

  void Update() override {
    assert(track_camera);
    // Global!, but id does not work yet!
    auto rot = track_camera->transform->GetLocalEulerAngles(); 
    own_camera->transform->SetLocalEulerAngles(rot);
  }
};

#endif // _COORDINATEAXES_H_5E7AF158_37C5_4CCC_86FE_737B8DFE42E6_
