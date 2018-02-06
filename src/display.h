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

#ifndef _DISPLAY_H_6F2FA1A5_D80C_4E6A_B667_4A7CCF86094F_
#define _DISPLAY_H_6F2FA1A5_D80C_4E6A_B667_4A7CCF86094F_ 

#include <string>
#include <sstream>
#include <iterator>
#include <vector>
#include <exception>
#include <iostream>

#include "gl_main.h"
#include "common/logging.h"

//////////////////////////////////////////////////////////////////////////////
// OpenGL context 
//////////////////////////////////////////////////////////////////////////////
struct Profile {
  int                      major_version;
  int                      minore_version;
  int                      profile;
  ////////////////////////////////////////////////////////////////////////////
  // Comma-separated OpenGL version-profile
  // "4 5 core", "2 0 compatibility", "3 3 any", etc
  ////////////////////////////////////////////////////////////////////////////
  explicit Profile(const std::string& ctxt_as_str) {
    std::istringstream iss(ctxt_as_str);
    std::vector<std::string> tokens{ std::istream_iterator<std::string>{iss},
                                     std::istream_iterator<std::string>{} };
    // Since case insensitive string comparison is super cumbersome in std C++
    // and there are no one-two line solutions for this - the profile is 
    // required to be in lower case for now.
    if (tokens.size() != 3) {
      ABORT_F("Invalid profile %s", ctxt_as_str.c_str());
    }

    major_version = std::stoi(tokens[0]);
    minore_version = std::stoi(tokens[1]);
    if (tokens[2] == "core") {
      profile = GLFW_OPENGL_CORE_PROFILE;
    } else if (tokens[2] == "compatibility") {
      profile = GLFW_OPENGL_COMPAT_PROFILE;
    } else if (tokens[2] == "any") {
      profile = GLFW_OPENGL_ANY_PROFILE;
    } else {
      ABORT_F("Invalid profile in %s", ctxt_as_str.c_str());
    }
  }
};

//////////////////////////////////////////////////////////////////////////////
// Display with window 
//////////////////////////////////////////////////////////////////////////////
class Display {
 public:
  Display(int width, int height, std::string caption, const Profile& profile)
     :  width_                (width),
        height_               (height),
        caption_              (std::move(caption)),
        profile_              (profile),
        glfw_window_          (nullptr) {
  }

  void Init() {
    if (width_ <= 0 || height_ <= 0) {
      ABORT_F("Display resolution is not valid!");
    }

    if (glfw_window_) {
      ABORT_F("Display is already initialized. Call Display::Close() first.");
    }

    if (!glfwInit()) {
      ABORT_F("Failed to initialize GLFW");
    }

    glfwWindowHint(GLFW_SAMPLES,               4); 
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, profile_.major_version); 
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, profile_.minore_version);
    glfwWindowHint(GLFW_OPENGL_PROFILE,        profile_.profile); 
    // For OSX 
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);

    glfw_window_ = glfwCreateWindow(width_, height_,
                                    caption_.c_str(), nullptr, nullptr);
    if (!glfw_window_) {
      glfwTerminate();
      ABORT_F("Failed to create GLFW window");
    }

    glfwMakeContextCurrent(glfw_window_); 
    glewExperimental = true; // Needed in core profile

    if (glewInit() != GLEW_OK) {
      ABORT_F("Failed to initialize GLEW");
    }
  
    // For MAC retina display
    glfwGetFramebufferSize(glfw_window_, &width_, &height_);

    //////////////////////////////////////////////////////////////////////////
    // Print GL version for reference (hardcoded stuff)
    //////////////////////////////////////////////////////////////////////////
    LOG_F(INFO, "Version     %s", glGetString(GL_VERSION));  
    LOG_F(INFO, "Vendor      %s", glGetString(GL_VENDOR));  
    LOG_F(INFO, "Renderer    %s", glGetString(GL_RENDERER));  
  
    glfwSetInputMode(glfw_window_, GLFW_STICKY_KEYS, GL_TRUE);
    
    //////////////////////////////////////////////////////////////////////////
    // Another portion of hardcoded stuff, which is related to rendering 
    //////////////////////////////////////////////////////////////////////////
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);
  }

  void Close() {
    glfwTerminate();
  }

  void ShowCursor(bool show) {
    if (show) {
      glfwSetInputMode(GetWindow(), GLFW_CURSOR, GLFW_CURSOR_NORMAL);
    } else {
      glfwSetInputMode(GetWindow(), GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    }
  }

  //void GetFrameBufferSize(int& width, int& height) {
  //  glfwGetFramebufferSize(glfw_window_, &width, &height);
  //}

  int               GetWidth()       const { return width_; }
  int               GetHeight()      const { return height_; }
  const Profile&    GetProfile()     const { return profile_; }

  GLFWwindow*       GetWindow() { 
    if (!glfw_window_) {
      ABORT_F("Display::Init() must be called once before Display::GetWindow()");
    }
    return glfw_window_; 
  }
  
 private:
  int                     width_;
  int                     height_;
  std::string             caption_;
  Profile                 profile_;
  GLFWwindow*             glfw_window_;
};


#endif // _DISPLAY_H_6F2FA1A5_D80C_4E6A_B667_4A7CCF86094F_
