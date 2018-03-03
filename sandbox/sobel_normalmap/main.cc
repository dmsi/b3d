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

float Sigma(float x) {
  if (x < 0.5) {
    return 0.5 * (2*x)*(2*x)*(2*x);
  }
  return 0.5 * ( (2*x-2)*(2*x-2)*(2*x-2) + 2);
}

void Voronoi(std::vector<float> koeff, size_t resolution) {
  using glm::vec2;
  auto cmap = std::make_shared<Image::ColorMap>(resolution, resolution);

  auto sample = [&cmap](float u, float v) {
    int iu = (int)(u * cmap->GetWidth()) % cmap->GetWidth();
    int iv = (int)(v * cmap->GetHeight()) % cmap->GetHeight();
    return cmap->At(iu, iv);
  };

  auto linear_cobmo = [](std::vector<float>& koeff, std::vector<float> dist) {
    float sum = 0;
    for (size_t i = 0; i < koeff.size(); ++i) {
      sum += koeff.at(i) * dist.at(i);
    }
    return sum;
  };

  std::vector<vec2> points(koeff.size());
  for (size_t i = 0; i < points.size(); ++i) {
    points.at(i) = vec2(Math::Random(), Math::Random());
    std::cerr << points.at(i).x << " " << points.at(i).y << std::endl;
  }

  std::vector<float> dist(koeff.size());

  for (int iv = 0; iv < cmap->GetHeight(); iv++) {
    for (int iu = 0; iu < cmap->GetWidth(); iu++) {
      float u = (float)iu / cmap->GetWidth();
      float v = (float)iv / cmap->GetHeight();

      for (size_t k = 0; k < points.size(); ++k) {
        dist.at(k) = glm::distance(vec2(u, v), points.at(k));
      }

      cmap->At(iu, iv) = Color(1) * linear_cobmo(koeff, dist);
    }
  }

  Image::PortablePixMap::Write("voronoi.ppm", *cmap);
}

int main(int argc, char* argv[]) {
  using glm::vec3;
  Scene scene;

  // Initialize application.
  AppContext::Init(1280, 720, "Sandbox [b3d]", Profile("3 3 core"));
  AppContext::Instance().display.ShowCursor(false);

  auto heightmap = Image::Load("Assets/sfo_8x8_hmap.pgm");
  auto normalmap = std::make_shared<Image::ColorMap>(heightmap->GetWidth(), heightmap->GetHeight());

  auto sample = [&heightmap](int x, int y) {
    if (x < 0 || y < 0 || x >= heightmap->GetWidth() || y >= heightmap->GetHeight()) {
      return 0.0f; 
    }
    return Sigma(heightmap->At(x, y).r);
    //return heightmap->At(x, y).r;
  };

  float normal_strength = 50;
  // Sobel operator:
  // z0   z1   z2
  // z3   h    z4
  // z5   z6   z7
  for (int iy = 0; iy < heightmap->GetHeight(); iy++) {
    for (int ix = 0; ix < heightmap->GetWidth(); ix++) {
      //float z0 = sample(ix - 1, iy - 1);
      //float z1 = sample(ix,     iy - 1);
      //float z2 = sample(ix + 1, iy - 1);
      //float z3 = sample(ix - 1, iy    );
      //float z4 = sample(ix + 1, iy    );
      //float z5 = sample(ix - 1, iy + 1);
      //float z6 = sample(ix    , iy + 1);
      //float z7 = sample(ix + 1, iy + 1);
      //// Sobel Filter
      //glm::vec3 normal;
      //normal.z = 1 / normal_strength;
      //normal.x = z0 + 2*z3 + z5 - z2 - 2*z4 - z7;
      //normal.y = z0 + 2*z1 + z2 - z5 - 2*z6 - z7;
      //normalmap->At(ix, iy) = Color(glm::normalize(normal), 1);

      //     z2
      // z1  z0   z3
      //     z4
      float z0 = sample(ix,     iy    );
      float z1 = sample(ix - 1, iy    );
      float z2 = sample(ix    , iy - 1);
      float z3 = sample(ix + 1, iy    );
      float z4 = sample(ix    , iy + 1);

      float s = std::max(
          std::max( abs(z0 - z1), abs(z0 - z3) ),
          std::max( abs(z0 - z2), abs(z0 - z4) ) );
      normalmap->At(ix, iy) = Color(1) * s * 10.0f;
    }
  }

  Image::PortablePixMap::Write("img.ppm", *normalmap);

  //Voronoi({-1, 1, 3, 4, 5, 8, 10, 11, 8, .2}, 512);
  //Voronoi({-1, 1, 0, 0, 0, 0, .1, .1, .2, .1}, 512);
  Voronoi({-1, 1}, 512);

  // Cleanup and close the app.
  AppContext::Close();
  return 0;
}
