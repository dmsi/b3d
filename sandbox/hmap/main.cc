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

#include "b3d.h"
#include "my/all.h"

struct Rot : public Action {
  glm::vec3 dr;
  Rot(std::shared_ptr<Transformation> t, float x, float y, float z)
    : Action(t), dr(x, y, z) {}

  void Update() override {
    auto r = transform->GetLocalEulerAngles();
    if (r.x < 20) {
      //dr.x = 20;
      dr.x = abs(dr.x);
    }

    if (r.x > 160) {
      //dr.x = 160;
      dr.x = -abs(dr.x);
    }

    transform->Rotate(dr * GetTimer().GetTimeDelta());
  }
};

struct TerrainHmap: public Action {
  struct Params {
    float max_altitude;
    float xz_scale;
  };
  Color       low_color = Rgb(255, 248, 220);
  Color       hi_color  = Rgb(244, 164, 96);
  Params params;
  std::shared_ptr<Image::ColorMap> hmap;

  TerrainHmap(std::shared_ptr<Transformation> t, Params p) 
    : Action(t), params(p) {
    Build();
  }

  void Build() {
    //hmap = Image::Load("Assets/sfo_8x8_hmap.pgm");
    hmap = Image::Load("Assets/geo_20x20.pgm");
    //hmap = Image::Load("img.ppm");
    //hmap = Image::Load("Assets/mv_8x8.pgm");
    std::function<float(int, int)>     sample_alt = [this](int x, int z) {
      int ix = x % hmap->GetWidth();
      int iz = z % hmap->GetHeight();
      float h = hmap->At(ix, iz).r;
      return params.max_altitude * Sigma(h);
      //return h*params.max_altitude;
    };
    std::function<Color(int, int)> sample_col = [this, sample_alt](int x, int z) {
      //return Color(1); 
      float n = sample_alt(x, z) / params.max_altitude; 

      float elevation = .15;
      if (n < elevation) {
        return Rgb(255, 248, 220) * Math::Random(0.9, 1);
      }
      return glm::mix(low_color, hi_color, n);
    };

    auto mesh = Grid(
        sample_alt, sample_col, 
        hmap->GetWidth(),
        params.xz_scale);

    GetActor().AddComponent<MeshFilter>()->SetMesh(mesh);

    LOG_F(INFO, "Terrain generated   vert=%ld   triangles=%ld", 
          mesh->vertices.size(), mesh->indices.size()/3);
  }

 private: 
  std::shared_ptr<Mesh> Grid(std::function<float(int, int)> sample_altitude, 
                             std::function<Color(int, int)> sample_color, 
                             int resolution,  // n_vertices per row = . res x res
                             float scale) {
    assert(resolution > 1);

    auto mesh = std::make_shared<Mesh>();

    auto n_vertices = resolution * resolution;
    auto n_indices = (resolution - 1) * (resolution - 1) * 6;

    mesh->vertices.resize(n_vertices);
    mesh->uv.resize(n_vertices);
    mesh->colors.resize(n_vertices);
    mesh->indices.resize(n_indices);
    
    int triangle_index = 0;
    auto add_triangle = [mesh, &triangle_index] (int a, int b, int c) {
      mesh->indices[triangle_index] = a;
      mesh->indices[triangle_index + 1] = b;
      mesh->indices[triangle_index + 2] = c;
      triangle_index += 3;
    };

    int vertex_index = 0;

    for (int z = 0; z < resolution; z++) {
      float v = (float)z / (resolution - 1);
      float zpos = Math::Lerp(-0.5f, 0.5f, v) * scale;

      for (int x = 0; x < resolution; x++) {
        float u = (float)x / (resolution - 1);
        float xpos = Math::Lerp(-0.5f,  0.5f, u) * scale;

        float altitude = 0;
        if (sample_altitude) {
          altitude = sample_altitude(x, z);
        }

        /*
        if (x < resolution - 3 && z < resolution - 3) {
          altitude = 0;
          int i = 0;
          for (int xx=x; xx <= x+2; ++xx) 
            for (int zz=z; zz <= z+2; ++zz) {
              altitude += sample_altitude(xx, zz);
              i++;
            }
          altitude /= (float)i;
        }*/

        mesh->vertices[vertex_index] = glm::vec3(xpos, altitude, zpos);
        mesh->uv[vertex_index] = glm::vec2(u, v);

        if (sample_color) {
          mesh->colors[vertex_index] = sample_color(x, z);
        }

        if (x < resolution - 1 && z < resolution - 1) {
          //add_triangle(vertex_index, vertex_index + resolution + 1, vertex_index + resolution);
          //add_triangle(vertex_index, vertex_index + 1, vertex_index + resolution + 1);
          add_triangle(vertex_index, vertex_index + resolution, vertex_index + resolution + 1);
          add_triangle(vertex_index, vertex_index + resolution + 1, vertex_index + 1);
        }
        vertex_index++;
      }
    }

    mesh->RecalculateNormals();
    return mesh;
  }

  float Sigma(float x) const {
    if (x < 0.5) {
      return 0.5 * (2*x)*(2*x)*(2*x);
    }
    return 0.5 * ( (2*x-2)*(2*x-2)*(2*x-2) + 2);
  }
};


int main(int argc, char* argv[]) {
  using glm::vec3;
  using T = std::vector<std::string>;
  Scene scene;

  LOG_SCOPE_F(INFO, "Helo blyat!");

  // Initialize application.
  AppContext::Init(1280, 720, "Sandbox [b3d]", Profile("3 3 core"));
  AppContext::Instance().display.ShowCursor(false);
  int width = AppContext::Instance().display.GetWidth();
  int height = AppContext::Instance().display.GetHeight();

  // Setup main and shadowmap rendertargets
  Cfg<RenderTarget>(scene, "rt.screen", 2000)
    . Tags("onscreen")
    . Clear(.8, .8, .8, 1)
    . Done();
  
  auto atmosphere_tex = Cfg<RenderTarget>(scene, "rt.atmosphere", 100)
    . Tags("atmosphere")
    . Type(FrameBuffer::kTexture2D)
    . Resolution(width, height)
    . Layer(Layer::kColor, Layer::kReadWrite)
    . Done()
    ->GetLayerAsTexture(0, Layer::kColor);
  
  //TerrainHmap::Params p {700, 4000};
  TerrainHmap::Params p {1000, 8000};
  auto t = Cfg<Actor>(scene, "actor.terrain")
    . Material("sandbox/hmap/terrain.mat")
    . Texture(0, atmosphere_tex)
    . Action<TerrainHmap>(p)
    . Done();

  /*
  if (auto m = t->GetComponent<Mesh>()) {
    int size = sqrt(m->vertices.size());
    auto nmap = std::make_shared<Image::ColorMap>(size, size);
    for (size_t iz = 0; iz < size; ++iz) {
      for (size_t ix = 0; ix < size; ++ix) {
        auto n = m->normals[iz * size + ix];
        nmap->At(ix, iz) = Color(glm::normalize(n) * 0.5f + 0.5f, 1);
      }
    }
    Image::PortablePixMap::Write("nmap.ppm", *nmap);
  }*/

  Cfg<Actor>(scene, "actor.skydome")
    . Model("Assets/sphere.dsm", "sandbox/hmap/skydome_perez.mat")
    //. Model("Assets/sphere.dsm", "Assets/skybox_cubemap.mat")
    . Tags(0, T{"onscreen", "atmosphere"})
    . Done();

  Cfg<Light>(scene, "light.sun", Light::kDirectional)
    . EulerAngles(20, 180, 0)
    //. Action<Rot>(10, 0, 0)
    . Done();

  // Main camera
  auto cam = Cfg<Camera>(scene, "camera.main")
    . Perspective(60, (float)width/height, 5, 12000)
    . Position(0, 100, 4)
    . Action<FlyingCameraController>(200) 
    . Done();

  //Cfg<Actor>(scene, "knight")
  //  . Model("Assets/knight.dsm", "Assets/texture.mat")
  //  . Position(0, 0, -6)
  //  . Parent(cam)
  //  . Done();
  
  // Fps meter.
  Cfg<Actor>(scene, "actor.fps.meter")
    . Action<FpsMeter>()
    . Done();
  
  // Main loop. Press ESC to exit.
  //glPolygonMode( GL_FRONT_AND_BACK, GL_LINE );
  do {
    AppContext::BeginFrame();
    scene.Update();
    scene.Draw();
    AppContext::EndFrame();
  } while (AppContext::Running());

  // Cleanup and close the app.
  AppContext::Close();
  return 0;
}
