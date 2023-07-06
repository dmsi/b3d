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

#ifndef _ACTION_TERRAINLOADER_H_E4F10491_9CED_493A_9E07_D870F3300098_
#define _ACTION_TERRAINLOADER_H_E4F10491_9CED_493A_9E07_D870F3300098_ 

#include "action.h"
#include "material/material_loader.h"
#include "meshfilter.h"
#include "math_main.h"
#include "image/portablepixmap.h"
#include "noise/perlin.h"

#include <fstream>
#include <sstream>

struct TerrainLoader: public Action {
  using NoiseMap = Array2d<float>;

  std::string material = "Assets/Materials/terrain_flatshading.mat";

  TerrainLoader(std::shared_ptr<Transformation> transform) : Action(transform) {
    // TODO
    //  1. Use PAM/P7 instead of colormap.cm 
    //  2. Use PPM/P5 instead of heightmap_sampled.hm
    noise_map_ = ReadNoiseMap("Assets/heightmap_sampled.hm");
    color_map_ = ReadColorMap("Assets/colormap.cm");
  }

  void Start() override {
    // NOTE: auto actor = ... will destroy the actor => disable copy constructor!!!!!
    // use auto& actor !!!
    auto& actor = transform->GetActor();

    // Material
    actor.AddComponent<MeshRenderer>()->SetMaterial(MaterialLoader::Load(material));

    // Mesh
    auto mf = actor.AddComponent<MeshFilter>();
    std::function<float(int, int)>     sample_alt = [this](int x, int z) {
      return noise_map_->At(x % noise_map_->GetWidth(), z % noise_map_->GetHeight());
    };
    std::function<glm::vec4(int, int)> sample_col = [this](int x, int z) {
      return color_map_->At(x % color_map_->GetWidth(), z % color_map_->GetHeight());
    };
    auto mesh = Generate(sample_alt, sample_col, noise_map_->GetWidth(), noise_map_->GetHeight(), 535*2, 535*2);
    mf->SetMesh(mesh);
  }

 private: 
  // Generate xz mesh from heightmap and apply radial spherical deformation.
  std::shared_ptr<Mesh> Generate(std::function<float(int, int)> sample_altitude, 
                                 std::function<glm::vec4(int, int)> sample_color, 
                                 int x_vertices, int z_vertices, 
                                 float x_length, float z_length) { 
    assert(x_vertices > 1);
    assert(z_vertices > 1);

    std::shared_ptr<Mesh> mesh(new Mesh);
    mesh->vertices.resize(x_vertices * z_vertices);
    mesh->indices.resize((x_vertices-1) * (z_vertices-1) * 6);
    mesh->uv.resize(x_vertices * z_vertices);
    mesh->colors.resize(x_vertices * z_vertices);
    
    int triangle_index = 0;
    auto add_triangle = [mesh, &triangle_index] (int a, int b, int c) {
      mesh->indices[triangle_index] = a;
      mesh->indices[triangle_index + 1] = b;
      mesh->indices[triangle_index + 2] = c;
      triangle_index += 3;
    };

    int vertex_index = 0;

    for (int z = 0; z < z_vertices; z++) {
      float tz = (float)z / (z_vertices - 1);
      float zpos = Math::Lerp(0.5f, -0.5f, tz) * z_length;

      for (int x = 0; x < x_vertices; x++) {
        float tx = (float)x / (x_vertices - 1);
        float xpos = Math::Lerp(-0.5f,  0.5f, tx) * x_length;

        float altitude = 0;
        if (sample_altitude) {
          altitude = sample_altitude(x, z);
        }

        mesh->vertices[vertex_index] = glm::vec3(xpos, altitude, zpos);
        mesh->uv[vertex_index] = glm::vec2(tx, tz);

        float radius = (x_length + z_length)/2; 
        glm::vec3 dir = glm::normalize(mesh->vertices[vertex_index] - glm::vec3(0,-radius,0));
        mesh->vertices[vertex_index] += dir * radius;
        mesh->vertices[vertex_index].y -= radius;

        if (sample_color) {
          mesh->colors[vertex_index] = sample_color(x, z);
        }

        if (x < x_vertices - 1 && z < z_vertices - 1) {
          add_triangle(vertex_index, vertex_index + x_vertices + 1, vertex_index + x_vertices);
          add_triangle(vertex_index, vertex_index + 1, vertex_index + x_vertices + 1);
        }
        vertex_index++;
      }
    }

    mesh->RecalculateNormals();
    return mesh;
  }

  std::shared_ptr<NoiseMap> ReadNoiseMap(const std::string& filename) {
    std::ifstream in(filename, std::ios::in);
    if (!in.is_open()) {
      ABORT_F("Cant open file %s", filename.c_str());
    }
    std::string line;
    std::getline(in, line);
    std::stringstream dimensions(line);
    size_t width, height;
    dimensions >> width;
    dimensions >> height;

    std::shared_ptr<NoiseMap> noise(new NoiseMap(width, height));
    for (size_t i = 0; i < noise->GetWidth() * noise->GetHeight(); ++i) {
      std::getline(in, line);
      std::stringstream n(line);
      n >> noise->At(i);
    }

    return noise;
  }

  std::shared_ptr<Image::ColorMap> ReadColorMap(const std::string& filename) {
    std::ifstream in(filename, std::ios::in);
    if (!in.is_open()) {
      ABORT_F("Cant open file %s", filename.c_str());
    }
    std::string line;
    std::getline(in, line);
    std::stringstream dimensions(line);
    size_t width, height;
    dimensions >> width;
    dimensions >> height;

    std::shared_ptr<Image::ColorMap> cmap(new Image::ColorMap(width, height));
    for (size_t i = 0; i < cmap->GetWidth() * cmap->GetHeight(); ++i) {
      std::getline(in, line);
      std::stringstream c(line);
      c >> cmap->At(i).r; 
      c >> cmap->At(i).g;
      c >> cmap->At(i).b;
      c >> cmap->At(i).a;
    }

    return cmap;
  }

  std::shared_ptr<NoiseMap> noise_map_;
  std::shared_ptr<Image::ColorMap> color_map_;
};

#endif // _ACTION_TERRAINLOADER_H_E4F10491_9CED_493A_9E07_D870F3300098_
