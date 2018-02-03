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
  using Layout = TAttributeLayout<glm::vec3>;
  Layout l;
  EXPECT_EQ(l.attributes(), 1);
  EXPECT_EQ(l.stride(), sizeof(glm::vec3));
  EXPECT_EQ(l.offset<0>(), 0);

}

TEST(AttributeLayout, LayoutVec3Vec2) {
  using Layout = TAttributeLayout<glm::vec3, glm::vec2>;
  Layout l;
  EXPECT_EQ(l.attributes(), 2);
  EXPECT_EQ(l.stride(), sizeof(glm::vec3) + sizeof(glm::vec2));
  EXPECT_EQ(l.offset<0>(), 0);
  EXPECT_EQ(l.offset<1>(), sizeof(glm::vec3));
}

TEST(AttributeLayout, FillVec3) {
  using Layout = TAttributeLayout<glm::vec3, glm::vec2>;
  Layout l;
  const size_t n_elements = 100;
  const size_t n_sz = Layout::stride() * n_elements;
  char holder[n_sz];
  auto buf = BufferView(holder, sizeof(holder));

  for (auto i = 0; i < n_elements; i++) {
    l.Set<0>(i, glm::vec3(i, 10-i, 30+i), buf);
  }

  for (auto i = 0; i < n_elements; i++) {
    auto& v = l.Get<0>(i, buf);
    EXPECT_FLOAT_EQ(v.x, i);
    EXPECT_FLOAT_EQ(v.y, 10-i);
    EXPECT_FLOAT_EQ(v.z, 30+i);
  }
}

TEST(AttributeLayout, FillVec3Vec2) {
  using Layout = TAttributeLayout<glm::vec3, glm::vec2>;
  Layout l;
  const size_t n_elements = 100;
  const size_t n_sz = Layout::stride() * n_elements;
  char holder[n_sz];
  auto buf = BufferView(holder, sizeof(holder));

  for (auto i = 0; i < n_elements; i++) {
    l.Set<0>(i, glm::vec3(i, 1-i, 100+i), buf);
    l.Set<1>(i, glm::vec2(i/2, i-i/2), buf);
  }

  for (auto i = 0; i < n_elements; i++) {
    auto& v3 = l.Get<0>(i, buf);
    auto& v2 = l.Get<1>(i, buf);
    EXPECT_FLOAT_EQ(v3.x, i);
    EXPECT_FLOAT_EQ(v3.y, 1-i);
    EXPECT_FLOAT_EQ(v3.z, 100+i);

    EXPECT_FLOAT_EQ(v2.x, i/2);
    EXPECT_FLOAT_EQ(v2.y, i-i/2);
  }
}
