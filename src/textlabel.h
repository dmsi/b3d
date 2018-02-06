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

#ifndef _TEXTLABEL_H_5490E62F_1E32_469C_9ED0_4887039530E4_
#define _TEXTLABEL_H_5490E62F_1E32_469C_9ED0_4887039530E4_ 

#include "glm_main.h"
#include "material/material.h"
#include "meshrenderer.h"
#include "meshfilter.h"
#include "common/logging.h"
#include <iostream>
#include <memory>
#include <fstream>

//////////////////////////////////////////////////////////////////////////////
// TODO(DS) Make it a proper component of Actor!
// 1. Add TextRenderer which is going to render the text
// 2. Add Layers system which is going to be rendered last
//////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////
// x, y, width, height -> defines quad widht and height and u,v texcoords
// next.pos.left = cur.pos.left + cur.xadvance + next.xoffset
// next.pos.top = cur.pos.top + next.yoffset
//////////////////////////////////////////////////////////////////////////////
struct GlyphInfo {
  int  id;
  int  x, y;
  int  width, height;
  int  xoffset, yoffset;
  int  xadvance;
  int  page;
  int  chnl;

  GlyphInfo(int _id, 
            int _x, int _y, 
            int _width, int _height, 
            int _xoffset, int _yoffset, 
            int _xadvance, int _page, int _chnl)
    : id(_id), 
      x(_x), y(_y), 
      width(_width), height(_height), 
      xoffset(_xoffset), yoffset(_yoffset), 
      xadvance(_xadvance), page(_page), chnl(_chnl) 
  {
    // This is the only case when I want to move bracket to the next line
  }

  void Print() const {
    LOG_SCOPE_F(INFO, "FontInfo"); 
    LOG_F(INFO, "id         : %d", id);
    LOG_F(INFO, "x          : %d", x);
    LOG_F(INFO, "y          : %d", y);
    LOG_F(INFO, "width      : %d", width);
    LOG_F(INFO, "height     : %d", height);
    LOG_F(INFO, "xoffset    : %d", xoffset);
    LOG_F(INFO, "yoffset    : %d", yoffset);
    LOG_F(INFO, "xadvance   : %d", xadvance);
    LOG_F(INFO, "page       : %d", page);
    LOG_F(INFO, "chnl       : %d", chnl);
  }
};

//////////////////////////////////////////////////////////////////////////////
// The font from bitmap.
//////////////////////////////////////////////////////////////////////////////
class BitmapFont {
 public:
  BitmapFont(const std::string& filename) : bitmap_width_(0), bitmap_height_(0) {
    std::string content;
    std::ifstream in(filename, std::ios::in);
    if (in.is_open()) {
      std::string line = "";
      while(std::getline(in, line)) {
        ParseLine(line);
      }
    } else {
      ABORT_F("Cant read file %s", filename.c_str());
    }
  }

  const GlyphInfo& GetGlyph(int id) const {
    const auto& it = glyph_map_.find(id);
    if (it == glyph_map_.end()) {
      ABORT_F("Character not found %d", id); 
    }
    return it->second;
  }

  int GetBitmapWidth() const { return bitmap_width_; }
  int GetBitmapHeight() const { return bitmap_height_; }
  const std::string GetBitmapFilename() const { return bitmap_filename_; }
  
  ////////////////////////////////////////////////////////////////////////////
  // Parsing sugar, yopta!
  ////////////////////////////////////////////////////////////////////////////
  static int IntFromToken(const std::string& token) {
    auto pos = token.find('=');
    if (pos != std::string::npos && pos+1 < token.length()) {
      return std::stoi(token.substr(pos+1));
    } else {
      ABORT_F("Invalid token %s", token.c_str());
    }
  }

  ////////////////////////////////////////////////////////////////////////////
  // Yee... not a python..
  ////////////////////////////////////////////////////////////////////////////
  static std::string StringFromToken(const std::string& token) { 
    auto pos = token.find('=');
    if (pos != std::string::npos && pos+1 < token.length()) {
      return token.substr(pos+1);
    } else {
      ABORT_F("Invalid token %s", token.c_str());
    }
  }

 private:
  ////////////////////////////////////////////////////////////////////////////
  // The whole parsing thing is very not optimized, but it supposed not to be
  // a real-time operation...
  //
  // YAML would be better here, but the tool only converts to this format and
  // to the XML. 
  ////////////////////////////////////////////////////////////////////////////
  void ParseLine(const std::string& line) {
    // Very-very dumb implementation of parsing :)
    // It's a real real pain to do such kind of things in C++ after using 
    // Python for a while..
    //
    // Not the most effecient solution, but this is not a real-time function,
    // so it does not matter! ... On PC at least :/
    // Tokenize
    std::istringstream iss(line);
    std::vector<std::string> tokens {std::istream_iterator<std::string>{iss},
                                     std::istream_iterator<std::string>{}};
    if (tokens.size() < 2) {
      return;
    }

    // common lineHeight=32 base=25 scaleW=256 scaleH=256 pages=1 packed=0 alphaChnl=1 redChnl=0 greenChnl=0 blueChnl=0
    if (tokens[0] == "common") {
      ParseLineCommon(line, tokens);
    }

    // page id=0 file="consolas_32_0.dds"
    else if (tokens[0] == "page") {
      ParseLinePage(line, tokens);
    }

    // char id=32   x=246   y=78    width=9     height=7     xoffset=-4    yoffset=28    xadvance=15    page=0  chnl=15
    else if (tokens[0] == "char") {
      ParseLineChar(line, tokens);
    }
  }

  ////////////////////////////////////////////////////////////////////////////
  ////////////////////////////////////////////////////////////////////////////
  void ParseLineCommon(const std::string& line, 
                       const std::vector<std::string>& tokens) {
    if (tokens.size() <= 5) {
      ABORT_F("Invalid line %s", line.c_str());
    }
    bitmap_width_ = IntFromToken(tokens[3]);
    bitmap_height_ = IntFromToken(tokens[4]);
  }

  ////////////////////////////////////////////////////////////////////////////
  ////////////////////////////////////////////////////////////////////////////
  void ParseLinePage(const std::string& line,
                     const std::vector<std::string>& tokens) {
    if (tokens.size() != 3) {
      ABORT_F("Invalid line %s", line.c_str());
    }
    bitmap_filename_ = StringFromToken(tokens[2]);
  }

  ////////////////////////////////////////////////////////////////////////////
  ////////////////////////////////////////////////////////////////////////////
  void ParseLineChar(const std::string& line, 
                     const std::vector<std::string>& tokens) {
    if (tokens[0] == "char") {
      if (tokens.size() != 11) {
        ABORT_F("Invalid line %s", line.c_str());
      }
      GlyphInfo g(
          IntFromToken(tokens[1]),                          // id
          IntFromToken(tokens[2]), IntFromToken(tokens[3]), // x, y
          IntFromToken(tokens[4]), IntFromToken(tokens[5]), // width, height
          IntFromToken(tokens[6]), IntFromToken(tokens[7]), // xoffset, yoffset
          IntFromToken(tokens[8]),                          // xadvance
          IntFromToken(tokens[9]),                          // page
          IntFromToken(tokens[10])                          // chnl
          );

      glyph_map_.insert(std::make_pair(g.id, g));
    }
  }

  int bitmap_width_;
  int bitmap_height_;
  std::string bitmap_filename_;
  std::map<int, GlyphInfo> glyph_map_;
};

//////////////////////////////////////////////////////////////////////////////
// Maintains the mesh for the text
//////////////////////////////////////////////////////////////////////////////
class TextLabel {
 public:

  ////////////////////////////////////////////////////////////////////////////
  // pos - in display pixel coordinates: (0, 0) - topleft 
  ////////////////////////////////////////////////////////////////////////////
  TextLabel(const glm::vec2& scale, const glm::vec2& pos, 
            std::shared_ptr<MeshFilter> mesh_filter = nullptr,
            std::shared_ptr<MeshRenderer> mesh_renderer = nullptr)
    : mesh_     (new Mesh()), 
      scale_    (scale),
      padding_  (3.0f) {
    const size_t kNumCharactersToReserve = 100;
    mesh_->vertices.reserve(kNumCharactersToReserve * 4);
    mesh_->uv.reserve(kNumCharactersToReserve * 4);
    mesh_->indices.reserve(kNumCharactersToReserve * 4 * 6);

    if (mesh_filter) {
      mesh_filter_ = mesh_filter;
    } else {
      mesh_filter_.reset(new MeshFilter());
    }
  
    if (mesh_renderer) {
      mesh_renderer_ = mesh_renderer;
    } else {
      mesh_renderer_.reset(new MeshRenderer());
    }
    
    mesh_filter_->SetMode(MeshFilter::BakeMode::kDynamic);
    mesh_filter_->SetSlot(MeshFilter::Slot::kUv);

    float screen_width = (float)AppContext::Instance().display.GetWidth();
    float screen_height = (float)AppContext::Instance().display.GetHeight();
    offset_ = glm::vec2(pos.x / screen_width - 1.0f, -pos.y / screen_height + 1.0f);
  }

  void SetText(const BitmapFont& font, const std::string& text) {
    if (!mesh_ || !mesh_filter_ || !mesh_renderer_) {
      ABORT_F("Mesh and material not configured");
    }

    mesh_->Clear();
  
    // A lot of chars to get display width and height...
    Display& display = AppContext::Instance().display;
    float screen_width = (float)display.GetWidth();
    float screen_height = (float)display.GetHeight();

    float left = 0;
    float top = 0;
    for (auto c: text) {
      GlyphInfo g = font.GetGlyph(c);
      left += g.xoffset;
      top = -g.yoffset;

      AddQuad(left, top, font, g, screen_width, screen_height);
      
      left += g.xadvance + padding_;
    }

    mesh_filter_->SetMesh(mesh_);
  }

  void Draw(Material& material, const glm::vec4& color = glm::vec4(1,1,0,1)) {
    glEnable    (GL_BLEND);
    glBlendFunc (GL_SRC_ALPHA, GL_ONE);
    glDisable   (GL_DEPTH_TEST);
    Pass& pass = *(*material.begin());
    material.Bind();
    pass.Bind();
    //pass.SetUniform("TextColor", color);
    mesh_renderer_->DrawCall(*mesh_filter_);
    pass.Unbind();
    material.Unbind();
    glDisable  (GL_BLEND);
    glEnable   (GL_DEPTH_TEST);
  }

  std::shared_ptr<Mesh> GetMesh() {return mesh_;}
  std::shared_ptr<MeshFilter> GetMeshFilter() {return mesh_filter_;}
  std::shared_ptr<MeshRenderer> GetMeshRenderer() {return mesh_renderer_;}

  // bind meshRenderer()

 private:
  ////////////////////////////////////////////////////////////////////////////
  // Add clockwise quad with glyph to the mesh.
  ////////////////////////////////////////////////////////////////////////////
  void AddQuad(float left, float top, 
               const BitmapFont& font, const GlyphInfo& glyph,
               float screen_width, float screen_height) {
    Mesh& out_mesh = *mesh_;

    // Add 4 vertices cw
    float glyph_left   = scale_.x*(left / screen_width) + offset_.x;
    float glyph_right  = scale_.x*((left + glyph.width) / screen_width) + offset_.x;
    float glyph_top    = scale_.y*(top / screen_height) + offset_.y;
    float glyph_bottom = scale_.y*((top - glyph.height) / screen_height) + offset_.y;
    out_mesh.vertices.push_back(glm::vec3(glyph_left,  glyph_top,    0.0f));
    out_mesh.vertices.push_back(glm::vec3(glyph_right, glyph_top,    0.0f));
    out_mesh.vertices.push_back(glm::vec3(glyph_right, glyph_bottom, 0.0f));
    out_mesh.vertices.push_back(glm::vec3(glyph_left,  glyph_bottom, 0.0f));

    // Add 4 uv cw
    float u_left = (float)glyph.x / font.GetBitmapWidth();
    float v_top = (float)glyph.y / font.GetBitmapHeight();
    float u_right = u_left + (float)glyph.width / font.GetBitmapWidth();
    float v_bottom = v_top + (float)glyph.height / font.GetBitmapHeight(); 
    out_mesh.uv.push_back(glm::vec2(u_left,  v_top));
    out_mesh.uv.push_back(glm::vec2(u_right, v_top));
    out_mesh.uv.push_back(glm::vec2(u_right, v_bottom));
    out_mesh.uv.push_back(glm::vec2(u_left,  v_bottom));

    // Add 6 indices ccw
    // The default glFrontFace(GL_CCW) in OpenGL. Make it default CW for the borsch
    // or go along with default opengl CCW?
    uint16_t lt = (uint16_t)out_mesh.vertices.size() - 4;
    uint16_t rt = (uint16_t)out_mesh.vertices.size() - 3;
    uint16_t rb = (uint16_t)out_mesh.vertices.size() - 2;
    uint16_t lb = (uint16_t)out_mesh.vertices.size() - 1;
    out_mesh.indices.push_back(lt);
    out_mesh.indices.push_back(rb);
    out_mesh.indices.push_back(lb);

    out_mesh.indices.push_back(lt);
    out_mesh.indices.push_back(rt);
    out_mesh.indices.push_back(rb);
  }

  std::shared_ptr<Mesh>         mesh_;
  std::shared_ptr<MeshFilter>   mesh_filter_;
  std::shared_ptr<MeshRenderer> mesh_renderer_;
  glm::vec2                     scale_;
  glm::vec2                     offset_;
  float                         padding_;
};
#endif // _TEXTLABEL_H_5490E62F_1E32_469C_9ED0_4887039530E4_
