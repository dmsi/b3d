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

#ifndef _ACTION_TERRAINGENERATOR_H_E4F10491_9CED_493A_9E07_D870F3300098_
#define _ACTION_TERRAINGENERATOR_H_E4F10491_9CED_493A_9E07_D870F3300098_ 

#include "action.h"
#include "material/material_loader.h"
#include "meshfilter.h"
#include "math_main.h"
#include "noise/perlin.h"
  
struct TerrainGenerator: public Action {
  float       max_altitude = 80*2;
  float       xz_scale = 535 * 2;
  Color       low_color = Rgb(255, 248, 220);
  Color       hi_color  = Rgb(244, 164, 96);
  //Color       low_color = Rgb(46,255,87) ;
  //Color       hi_color  = Rgb(139,69,19); 

  TerrainGenerator(std::shared_ptr<Transformation> transform) : Action(transform) {
    Build();
  }

  void Build() {
    noise_map_ = GenerateNoiseMap(241, 241, 5, 1.9, .5, 50, 0, 0);

    std::function<float(int, int)>     sample_alt = [this](int x, int z) {
      int ix = x % noise_map_->GetWidth();
      int iz = z % noise_map_->GetHeight();
      float n = noise_map_->At(ix, iz);
      
      float fx = 2.0 * ix / noise_map_->GetWidth() - 1;
      float fz = 2.0 * iz / noise_map_->GetHeight() - 1;
        
      //float falloff = Sigma((1-std::abs(fx/1.3)) * (1-std::abs(fz/1.3)));
      float falloff = Sigma(std::abs(fx) * std::abs(fz));

      return max_altitude * Sigma(n) * falloff;
    };
    std::function<glm::vec4(int, int)> sample_col = [this, sample_alt](int x, int z) {
      float n = sample_alt(x, z) / max_altitude; 

      float elevation = .15;
      if (n < elevation) {
        return Rgb(255, 248, 220) * Math::Random(0.9, 1);
      }
      return glm::mix(low_color, hi_color, n);
    };
    auto mesh = Generate(sample_alt, sample_col, noise_map_->GetWidth(), noise_map_->GetHeight(), xz_scale, xz_scale);
    GetActor().AddComponent<MeshFilter>()->SetMesh(mesh);
    LOG_F(INFO, "Terrain generated   vert=%ld   triangles=%ld", mesh->vertices.size(), mesh->indices.size()/3);
  }

 private: 
  // Generate xz mesh from heightmap and [apply radial spherical deformation];
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

        //float radius = (x_length + z_length)/2; 
        //glm::vec3 dir = glm::normalize(mesh->vertices[vertex_index] - glm::vec3(0,-radius,0));
        //mesh->vertices[vertex_index] += dir * radius;
        //mesh->vertices[vertex_index].y -= radius;

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

  typedef Array2d<float> NoiseMap;
  std::shared_ptr<NoiseMap> GenerateNoiseMap(size_t width, size_t height,
                                             int octaves = 5, 
                                             float lacunarity = 2.0f, 
                                             float persistance = 0.5f,
                                             float scale = 200.0f,
                                             float offset_x = 0.0f, float offset_y = 0.0f) {
    std::shared_ptr<NoiseMap> noise_map(new NoiseMap(width, height));
    
    std::vector<glm::vec2> offset(octaves);
    for (int i = 0; i < octaves; i++) {
      float ox = Math::Random(-1000, 1000) + offset_x;
      float oy = Math::Random(-1000, 1000) + offset_y;
      offset[i] = glm::vec2(ox, oy);
    }

    float half_width = noise_map->GetWidth() / 2.0f;
    float half_height = noise_map->GetHeight() / 2.0f;
    float noise_min = std::numeric_limits<float>::max(); 
    float noise_max = std::numeric_limits<float>::min(); 

    for(int y = 0; y < (int)noise_map->GetHeight(); ++y) {     
      for(int x = 0; x < (int)noise_map->GetWidth(); ++x) {  
        float amplitude = 1;
        float frequency = 1;
        float noise_value = 0;

        for (int i = 0; i < octaves; ++i) {
          float sample_x = (x - half_width) / scale * frequency + offset[i].x;
          float sample_y = (y - half_height) / scale * frequency + offset[i].y;
          float perlin = Noise::Perlin::Get(sample_x, sample_y) * 2 - 1;
          noise_value += amplitude * perlin;

          frequency *= lacunarity;
          amplitude *= persistance;
        }
        noise_min = std::min(noise_min, noise_value);
        noise_max = std::max(noise_max, noise_value);
        noise_map->At(x, y) = noise_value;
      }
    }
    
    // Normalize
    for (int y = 0; y < (int)noise_map->GetHeight(); ++y) {
      for (int x = 0; x < (int)noise_map->GetWidth(); ++x) {
        noise_map->At(x, y) = Math::InverseLerp(noise_min, noise_max, noise_map->At(x, y)); 
        noise_map->At(x, y) = Math::Clamp(0.0f, 1.0f, noise_map->At(x, y));
      }
    }

    return noise_map;
  }

  float Sigma(float x) const {
    if (x < 0.5) {
      return 0.5 * (2*x)*(2*x)*(2*x);
    }
    return 0.5 * ( (2*x-2)*(2*x-2)*(2*x-2) + 2);
  }
  

  std::shared_ptr<NoiseMap> noise_map_;
};

#endif // _ACTION_TERRAINGENERATOR_H_E4F10491_9CED_493A_9E07_D870F3300098_
