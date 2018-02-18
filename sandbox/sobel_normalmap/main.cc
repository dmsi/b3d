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

int main(int argc, char* argv[]) {
  using glm::vec3;
  Scene scene;

  // Initialize application.
  AppContext::Init(1280, 720, "Sandbox [b3d]", Profile("3 3 core"));
  AppContext::Instance().display.ShowCursor(false);

  auto heightmap = Image::Load("Assets/geo_20x20.pgm");
  auto normalmap = std::make_shared<Image::ColorMap>(heightmap->GetWidth(), heightmap->GetHeight());

  auto sample = [&heightmap](int x, int y) {
    if (x < 0 || y < 0 || x >= heightmap->GetWidth() || y >= heightmap->GetHeight()) {
      return 0.0f; 
    }
    return Sigma(heightmap->At(x, y).r);
  };

  float normal_strength = 50;
  // Sobel operator:
  // z0   z1   z2
  // z3   h    z4
  // z5   z6   z7
  for (int iy = 0; iy < heightmap->GetHeight(); iy++) {
    for (int ix = 0; ix < heightmap->GetWidth(); ix++) {
      float z0 = sample(ix - 1, iy - 1);
      float z1 = sample(ix,     iy - 1);
      float z2 = sample(ix + 1, iy - 1);
      float z3 = sample(ix - 1, iy    );
      float z4 = sample(ix + 1, iy    );
      float z5 = sample(ix - 1, iy + 1);
      float z6 = sample(ix    , iy + 1);
      float z7 = sample(ix + 1, iy + 1);
      //normalmap->At(ix, iy) = Color(1,1,1,1) * (sample(ix, iy));
      // Sobel Filter
      glm::vec3 normal;
      normal.z = 1 / normal_strength;
      normal.x = z0 + 2*z3 + z5 - z2 - 2*z4 - z7;
      normal.y = z0 + 2*z1 + z2 - z5 - 2*z6 - z7;
      normalmap->At(ix, iy) = Color(glm::normalize(normal), 1);
    }
  }

  Image::PortablePixMap::Write("img.ppm", *normalmap);

  // Cleanup and close the app.
  AppContext::Close();
  return 0;
}
