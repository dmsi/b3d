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

#ifndef _ATTRIBUTELAYOUT_H_4E8EAD20_302D_4791_B4FB_F7D9188DCE0C_
#define _ATTRIBUTELAYOUT_H_4E8EAD20_302D_4791_B4FB_F7D9188DCE0C_ 

#include "gl_main.h"
#include <cassert>
#include <vector>
#include <tuple>

#include <iostream>

//https://stackoverflow.com/questions/37602057/why-isnt-a-for-loop-a-compile-time-expression
namespace never_easy {
  template<std::size_t N>
  struct num { static const constexpr auto value = N; };

  template <class F, std::size_t... Is>
  void for_(F func, std::index_sequence<Is...>) {
    using expander = int[];
    (void)expander{0, ((void)func(num<Is>{}), 0)...};
  }

  template <std::size_t N, typename F>
  void for_(F func) {
    for_(func, std::make_index_sequence<N>());
  }
}

template <typename... T>
struct AttributeLayout {
  using TTuple = std::tuple<T...>;

  template <size_t N>
  using TAttribute = typename std::tuple_element<N, TTuple>::type;

  static constexpr auto Attributes() { return std::tuple_size<TTuple>::value; }
  
  template <size_t N>
  static constexpr auto Offset() {
    // TODO This does not make it as constexpr
    size_t sum = 0;
    never_easy::for_<N>([&] (auto i) {
      sum += sizeof(TAttribute<i.value>);
    });
    return sum;
  }

  static constexpr auto Stride() {
    return (0 + ... + sizeof(T));
  }

  template <size_t N, typename Y, typename TBuffer>
  static
  void Set(TBuffer& dst, int pos, const Y& val) {
    static_assert(std::is_same<Y, TAttribute<N>>::value, "");
    dst.Write(pos * Stride() + Offset<N>(), (void*)&val, sizeof(val));
  }

  template <size_t N, typename TBuffer>
  static
  const auto& Get(TBuffer& src, int pos) {
    // TODO add size for checking bounds
    void *ptr = src.Read(pos * Stride() + Offset<N>());
    return *(TAttribute<N>*)ptr;
  }
};

struct BufferView {
  BufferView(void *ptr, size_t sz) : ptr_(ptr), sz_(sz) {}

  void Write(size_t offset, void* what, size_t sz) {
    assert(ptr_);
    assert(what);
    assert(offset + sz <= sz_);
    (void)sz_;
    memcpy((uint8_t*)ptr_ + offset, what, sz);
  }

  void* Read(size_t offset) {
    assert(offset < sz_);
    return (uint8_t*)ptr_ + offset;
  }

 private:
  void* ptr_;
  size_t sz_;
};

//template <typename... T>
//using Layout = AttributeLayout<T...>;

template <typename T> struct AttributeTraits;
template <> struct AttributeTraits<int> {
  static constexpr auto n_components = 1;
  static constexpr auto type = GL_INT; 
};
template <> struct AttributeTraits<float> {
  static constexpr auto n_components = 1;
  static constexpr auto type = GL_FLOAT; 
};
template <> struct AttributeTraits<glm::vec2> {
  static constexpr auto n_components = 2;
  static constexpr auto type = GL_FLOAT; 
};
template <> struct AttributeTraits<glm::vec3> {
  static constexpr auto n_components = 3;
  static constexpr auto type = GL_FLOAT; 
};
template <> struct AttributeTraits<glm::vec4> {
  static constexpr auto n_components = 4;
  static constexpr auto type = GL_FLOAT; 
};

#endif // _ATTRIBUTELAYOUT_H_4E8EAD20_302D_4791_B4FB_F7D9188DCE0C_
