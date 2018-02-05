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

#include <b3d.h>
#include <gtest/gtest.h>

using glm::vec3;

TEST(AttributeLayout, LayoutVec3) {
  using Layout = AttributeLayout<glm::vec3>;
  EXPECT_EQ(Layout::Attributes(), 1);
  EXPECT_EQ(Layout::Stride(), sizeof(glm::vec3));
  EXPECT_EQ(Layout::Offset<0>(), 0);
}

TEST(AttributeLayout, LayoutVec3Vec2) {
  using Layout = AttributeLayout<glm::vec3, glm::vec2>;
  EXPECT_EQ(Layout::Attributes(), 2);
  EXPECT_EQ(Layout::Stride(), sizeof(glm::vec3) + sizeof(glm::vec2));
  EXPECT_EQ(Layout::Offset<0>(), 0);
  EXPECT_EQ(Layout::Offset<1>(), sizeof(glm::vec3));
}

TEST(AttributeLayout, LayoutVec3Vec2Int) {
  using Layout = AttributeLayout<glm::vec3, glm::vec2, int>;
  EXPECT_EQ(Layout::Attributes(), 3);
  EXPECT_EQ(Layout::Stride(), sizeof(glm::vec3) + sizeof(glm::vec2) + sizeof(int));
  EXPECT_EQ(Layout::Offset<0>(), 0);
  EXPECT_EQ(Layout::Offset<1>(), sizeof(glm::vec3));
  EXPECT_EQ(Layout::Offset<2>(), sizeof(glm::vec3) + sizeof(glm::vec2));
}

TEST(AttributeLayout, FillVec3) {
  using Layout = AttributeLayout<glm::vec3, glm::vec2>;
  const size_t n_elements = 100;
  const size_t n_sz = Layout::Stride() * n_elements;
  char holder[n_sz];
  auto buf = BufferView(holder, sizeof(holder));

  for (auto i = 0; i < n_elements; i++) {
    Layout::Set<0>(buf, i, glm::vec3(i, 10-i, 30+i));
  }

  for (auto i = 0; i < n_elements; i++) {
    auto& v = Layout::Get<0>(buf, i);
    EXPECT_FLOAT_EQ(v.x, i);
    EXPECT_FLOAT_EQ(v.y, 10-i);
    EXPECT_FLOAT_EQ(v.z, 30+i);
  }
}

TEST(AttributeLayout, FillVec3Vec2) {
  using Layout = AttributeLayout<glm::vec3, glm::vec2>;
  const size_t n_elements = 100;
  const size_t n_sz = Layout::Stride() * n_elements;
  char holder[n_sz];
  auto buf = BufferView(holder, sizeof(holder));

  for (auto i = 0; i < n_elements; i++) {
    Layout::Set<0>(buf, i, glm::vec3(i, 1-i, 100+i));
    Layout::Set<1>(buf, i, glm::vec2(i/2, i-i/2));
  }

  for (auto i = 0; i < n_elements; i++) {
    auto& v3 = Layout::Get<0>(buf, i);
    auto& v2 = Layout::Get<1>(buf, i);
    EXPECT_FLOAT_EQ(v3.x, i);
    EXPECT_FLOAT_EQ(v3.y, 1-i);
    EXPECT_FLOAT_EQ(v3.z, 100+i);

    EXPECT_FLOAT_EQ(v2.x, i/2);
    EXPECT_FLOAT_EQ(v2.y, i-i/2);
  }
}

TEST(AttributeLayout, FillVec3Vec2Int) {
  using Layout = AttributeLayout<glm::vec3, glm::vec2, int>;
  const size_t n_elements = 100;
  const size_t n_sz = Layout::Stride() * n_elements;
  char holder[n_sz];
  auto buf = BufferView(holder, sizeof(holder));

  for (auto i = 0; i < n_elements; i++) {
    Layout::Set<0>(buf, i, glm::vec3(i, 1-i, 100+i));
    Layout::Set<1>(buf, i, glm::vec2(i/2, i-i/2));
    Layout::Set<2>(buf, i, i*10);
  }

  for (auto i = 0; i < n_elements; i++) {
    auto& v3 = Layout::Get<0>(buf, i);
    auto& v2 = Layout::Get<1>(buf, i);
    auto& Int = Layout::Get<2>(buf, i);
    EXPECT_FLOAT_EQ(v3.x, i);
    EXPECT_FLOAT_EQ(v3.y, 1-i);
    EXPECT_FLOAT_EQ(v3.z, 100+i);

    EXPECT_FLOAT_EQ(v2.x, i/2);
    EXPECT_FLOAT_EQ(v2.y, i-i/2);

    EXPECT_EQ(Int, i*10);
  }
}
