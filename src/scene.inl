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

template <typename TComponent, typename... TArgs>
inline
auto Scene::Add(const std::string& name, TArgs&&... args) {
  if constexpr (std::is_same<TComponent, Actor>::value) {
    // 
    // Actor
    //
    auto thing = std::make_shared<TComponent>(name, std::forward<TArgs>(args)...);
    return cppness
      ::MapForSharedPtr<decltype(actors_)>
      ::JustPutToMap(name, thing, actors_);
  } else
  if constexpr (std::is_same<TComponent, Light>::value) {
    // 
    // Light 
    //
    auto thing = std::make_shared<TComponent>(name, std::forward<TArgs>(args)...);
    return cppness
      ::MapForSharedPtr<decltype(lights_)>
      ::JustPutToMap(name, thing, lights_);
  } else
  if constexpr (std::is_same<TComponent, Camera>::value) {
    // 
    // Camera 
    //
    auto thing = std::make_shared<TComponent>(name, std::forward<TArgs>(args)...);
    return cppness
      ::MapForSharedPtr<decltype(cameras_)>
      ::JustPutToMap(name, thing, cameras_);
  } else 
  if constexpr (std::is_same<TComponent, RenderTarget>::value) {
    // 
    // RenderTarget 
    //
    auto t = std::make_tuple(std::forward<TArgs>(args)...);
    auto key = std::get<0>(t);
    auto thing = std::make_shared<TComponent>(name);
    return cppness
      ::MapForSharedPtr<decltype(render_targets_)>
      ::JustPutToMap(key, thing, render_targets_);
  } else 
  if constexpr (std::is_same<TComponent, StdBatch::Batch>::value) {
    // 
    // StdBatch::Batch 
    //
    auto thing = std_batch_.AddBatch(name, std::forward<TArgs>(args)...);
    return thing;
  } else 
  if constexpr (std::is_same<TComponent, StdBatch::Actor>::value) {
    // 
    // StdBatch::Actor 
    //
    auto thing = std_batch_.AddActor(name, std::forward<TArgs>(args)...);
    return thing;
  } else {
    // 
    // None of above 
    //
    static_assert(cppness::dependent_false<TComponent>::value,
        "TComponent not recognized");
  }
}

template <typename TComponent, typename... TArgs>
inline
auto Scene::Get(TArgs&&... args) {
  if constexpr (std::is_same<TComponent, Actor>::value) {
    // 
    // Actor
    //
    auto t = std::make_tuple(std::forward<TArgs>(args)...);
    auto key = std::get<0>(t);
    return cppness
      ::MapForSharedPtr<decltype(actors_)>
      ::JustGetFromMap(key, actors_);
  } else
  if constexpr (std::is_same<TComponent, Light>::value) {
    // 
    // Light 
    //
    auto t = std::make_tuple(std::forward<TArgs>(args)...);
    auto key = std::get<0>(t);
    return cppness
      ::MapForSharedPtr<decltype(lights_)>
      ::JustGetFromMap(key, lights_);
  } else
  if constexpr (std::is_same<TComponent, Camera>::value) {
    // 
    // Camera 
    //
    auto t = std::make_tuple(std::forward<TArgs>(args)...);
    auto key = std::get<0>(t);
    return cppness
      ::MapForSharedPtr<decltype(cameras_)>
      ::JustGetFromMap(key, cameras_);
  } else 
  if constexpr (std::is_same<TComponent, RenderTarget>::value) {
    // 
    // RenderTarget 
    //
    auto t = std::make_tuple(std::forward<TArgs>(args)...);
    auto key = std::get<0>(t);
    return cppness
      ::MapForSharedPtr<decltype(render_targets_)>
      ::JustGetFromMap(key, render_targets_);
  } else 
  if constexpr (std::is_same<TComponent, StdBatch::Batch>::value) {
    // 
    // StdBatch::Batch 
    //
    auto t = std::make_tuple(std::forward<TArgs>(args)...);
    auto key = std::get<0>(t);
    return std_batch_.GetBatch(key);
  } else 
  if constexpr (std::is_same<TComponent, StdBatch::Actor>::value) {
    // 
    // StdBatch::Actor 
    //
    auto t = std::make_tuple(std::forward<TArgs>(args)...);
    auto key = std::get<0>(t);
    return std_batch_.GetActor(key);
  } else {
    // 
    // None of above 
    //
    static_assert(cppness::dependent_false<TComponent>::value,
        "TComponent not recognized");
  }
}
