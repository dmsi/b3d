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

auto GetCamera() {
  return std::shared_ptr<Camera> (new Camera("test.camera"));
}

TEST(Camera, Position) {
  auto c = GetCamera(); auto t = c->transform;
  EXPECT_FLOAT_EQ(t->GetLocalPosition().x, 0);
  EXPECT_FLOAT_EQ(t->GetLocalPosition().y, 0);
  EXPECT_FLOAT_EQ(t->GetLocalPosition().z, 0);
}
TEST(Transform, Orientation) {
  auto c = GetCamera(); auto t = c->transform;
  EXPECT_FLOAT_EQ(t->GetLocalEulerAngles().x, 0);
  EXPECT_FLOAT_EQ(t->GetLocalEulerAngles().y, 0);
  EXPECT_FLOAT_EQ(t->GetLocalEulerAngles().z, 0);
}
TEST(Transform, Scale) {
  auto c = GetCamera(); auto t = c->transform;
  EXPECT_FLOAT_EQ(t->GetLocalScale().x, 1);
  EXPECT_FLOAT_EQ(t->GetLocalScale().y, 1);
  EXPECT_FLOAT_EQ(t->GetLocalScale().z, 1);
}
