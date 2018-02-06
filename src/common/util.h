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

#ifndef _UTIL_H_B32DF744_3672_4AE3_A5FB_9CB1F681FFA0_
#define _UTIL_H_B32DF744_3672_4AE3_A5FB_9CB1F681FFA0_ 

#include "common/logging.h"
#include <string>
#include <fstream> 
#include <exception>

//////////////////////////////////////////////////////////////////////////////
// Reads text file line by line and returns content as a string. 
//////////////////////////////////////////////////////////////////////////////
inline std::string ReadFile(const std::string& filename) {
  std::string content;
  std::ifstream in(filename, std::ios::in);
  if (in.is_open()) {
    std::string line = "";
    while(std::getline(in, line)) {
      content += "\n" + line;
    }
    in.close();
    return content;
  } else {
    throw std::runtime_error("Cant read file " + filename);
  }
}
  
namespace cppness { 
  template<class T> struct dependent_false : std::false_type {};

  // is_shared_ptr trait
  template<typename T> struct is_shared_ptr : std::false_type {};
  template<typename T> struct is_shared_ptr<std::shared_ptr<T>> : std::true_type {};

  ///////////////////////////////////////////////////////////////////////////////
  // Common operations with map, where value is shared_ptr
  ///////////////////////////////////////////////////////////////////////////////
  template <typename TMap>
  struct MapForSharedPtr {
    using TKey      = typename TMap::key_type;
    using TValuePtr = typename TMap::mapped_type;
    using TIterator = typename TMap::iterator;

    static_assert(is_shared_ptr<TValuePtr>::value,
        "TValuePtr must be std::shared_ptr!");

    static inline
    TIterator GetHint(const TKey& key, TMap& map) {
      auto it = map.lower_bound(key);
      if (it != map.end() && it->first == key) {
        ABORT_F("Component already added");
      }
      return it;
    }

    static inline
    TValuePtr AddToMap(const TKey& key, TValuePtr value, TIterator hint, 
                       TMap& map) {
      return map.emplace_hint(hint, key, value)->second;
    }

    static inline
    TValuePtr JustPutToMap(const TKey& key, TValuePtr value, TMap& map) {
      auto hint = GetHint(key, map);
      return AddToMap(key, value, hint, map);
    }

    // Return TValuePtr or nullptr
    static inline
    TValuePtr JustGetFromMap(const TKey& key, TMap& map) {
      auto it = map.find(key);
      if (it != map.end()) {
        assert(it->second);
        return it->second;
      }
      // TODO put warning here
      return TValuePtr();
    }
  };
}

#endif // _UTIL_H_B32DF744_3672_4AE3_A5FB_9CB1F681FFA0_
