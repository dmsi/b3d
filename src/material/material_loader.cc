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

#include "material/material_loader.h"
#include "shader.h"
#include "texture.h"
#include "texture2d.h"
#include "texture_cube.h"
#include "common/util.h"
#include "common/logging.h"
#include "yaml_main.h"
#include "image/loader.h"
#include <exception>
#include <vector>

static void LoadTextures(Material& material, YAML::Node node) {
  std::map<int, std::shared_ptr<Texture>> texture_map;
  for (auto kv: node) {
    int slot = kv.first.as<int>();
    std::shared_ptr<Texture> texture;
    if (!kv.second.IsSequence()) {
      std::string filename = kv.second.as<std::string>();
      texture = std::make_shared<Texture2D>(Image::Load(filename));
    } else {
      auto ls = kv.second.as<std::vector<std::string>>();
      std::shared_ptr<Image::ColorMap> cubemap[6] = {
        Image::Load(ls.at(0)), 
        Image::Load(ls.at(1)), 
        Image::Load(ls.at(2)), 
        Image::Load(ls.at(3)), 
        Image::Load(ls.at(4)), 
        Image::Load(ls.at(5))
      };
      texture = std::make_shared<TextureCube>(cubemap);
    }

    if (texture) {
      material.SetTexture(slot, texture);
    }
  }
}

static bool StringToBool(const std::string& s) {
  if      (s == "on")  return true;
  else if (s == "off") return false;

  ABORT_F("Bad value %s", s.c_str());
}

static std::string ReadShaderSourceCode(YAML::Node node) {
  try {
    return ReadFile(node.as<std::string>());
  } catch (std::runtime_error error) {
    return node.as<std::string>();
  }
}

static CullMode::Value StringToCullMode(const std::string& s) {
  if      (s == "cw")  return CullMode::kCw;
  else if (s == "ccw") return CullMode::kCcw;
  else if (s == "off") return CullMode::kOff;

  ABORT_F("Bad value %s", s.c_str());
}

static FillMode::Value StringToFillMode(const std::string& s) {
  if      (s == "solid")  return FillMode::kSolid;
  else if (s == "wireframe") return FillMode::kWireframe;
  else if (s == "point") return FillMode::kPoint;

  ABORT_F("Bad value %s", s.c_str());
}

static BlendFactor::Value StringToBlendFactor(const std::string& s) {
  if      (s == "one")                 return BlendFactor::kOne;
  else if (s == "zero")                return BlendFactor::kZero;
  else if (s == "src_color")           return BlendFactor::kSrcColor;
  else if (s == "src_alpha")           return BlendFactor::kSrcAlpha;
  else if (s == "dst_color")           return BlendFactor::kDstColor;
  else if (s == "dst_alpha")           return BlendFactor::kDstAlpha;
  else if (s == "one_minus_src_color") return BlendFactor::kOneMinusSrcColor;
  else if (s == "one_minus_src_alpha") return BlendFactor::kOneMinusSrcAlpha;
  else if (s == "one_minus_dst_color") return BlendFactor::kOneMinusDstColor;
  else if (s == "one_minus_dst_alpha") return BlendFactor::kOneMinusDstAlpha;

  ABORT_F("Bad value %s", s.c_str());
}

static BlendOp::Value StringToBlendOp(const std::string& s) {
  if      (s == "add")      return BlendOp::kAdd;
  else if (s == "sub")      return BlendOp::kSub;
  else if (s == "rev_sub")  return BlendOp::kRevSub;

  ABORT_F("Bad value %s", s.c_str());
}


static std::shared_ptr<Pass> LoadPass(YAML::Node node) {
  std::shared_ptr<Pass> pass(new Pass());
  std::shared_ptr<Shader> shader(new Shader());

  static const std::string kNameKey           = "name";
  static const std::string kTagsKey           = "tags";
  static const std::string kQueueKey          = "queue";
  static const std::string kZwriteKey         = "zwrite";
  static const std::string kZtestKey          = "ztest";
  static const std::string kCullKey           = "cull";
  static const std::string kBlendKey          = "blend";
  static const std::string kBlendFactorsKey   = "factors";
  static const std::string kBlendOpKey        = "op";
  static const std::string kClippingKey       = "clipping";
  static const std::string kFillKey           = "fill";
  static const std::string kVertexShaderKey   = "vertex";
  static const std::string kFragmentShaderKey = "fragment";
  static const std::string kGeometryShaderKey = "geometry";
  static const std::string kTessControlShaderKey = "tess_control";
  static const std::string kTessEvaluationShaderKey = "tess_evaluation";

  for (auto subnode: node) {
    std::string key = subnode.first.as<std::string>();

    if (key == kNameKey) {
      pass->SetName(subnode.second.as<std::string>());
    } else if (key == kQueueKey) {
      pass->SetQueue(subnode.second.as<int>());
    } else if (key == kZwriteKey) {
      pass->options.SetZwrite(StringToBool(subnode.second.as<std::string>()));
    } else if (key == kZtestKey) {
      pass->options.SetZtest(StringToBool(subnode.second.as<std::string>()));
    } else if (key == kCullKey) {
      pass->options.SetCull(StringToCullMode(subnode.second.as<std::string>()));
    } else if (key == kClippingKey) {
      pass->options.SetClippingPlanes(subnode.second.as<std::vector<int>>());
    } else if (key == kFillKey) {
      pass->options.SetFill(StringToFillMode(subnode.second.as<std::string>()));
    } else if (key == kTagsKey) {
      pass->SetTags(subnode.second.as<std::vector<std::string>>());
    } else if (key == kBlendKey) {
      YAML::Node blend = subnode.second;
      std::string op = blend[kBlendOpKey].as<std::string>();
      std::vector<std::string> factors = blend[kBlendFactorsKey].as<std::vector<std::string>>();

      pass->options.SetBlendOp(StringToBlendOp(op));
      pass->options.SetSrcBlendFactor(StringToBlendFactor(factors[0]));
      pass->options.SetDstBlendFactor(StringToBlendFactor(factors[1]));
    } else if (key == kVertexShaderKey) {
      shader->Compile(ShaderCompiler::ShaderType::kVertexShader,
                      ReadShaderSourceCode(subnode.second));
    } else if (key == kFragmentShaderKey) {
      shader->Compile(ShaderCompiler::ShaderType::kFragmentShader,
                      ReadShaderSourceCode(subnode.second));
    } else if (key == kGeometryShaderKey) {
      shader->Compile(ShaderCompiler::ShaderType::kGeometryShader, 
                      ReadShaderSourceCode(subnode.second));
    } else if (key == kTessControlShaderKey) {
      shader->Compile(ShaderCompiler::ShaderType::kTessControlShader,
                      ReadShaderSourceCode(subnode.second));
    } else if (key == kTessEvaluationShaderKey) {
      shader->Compile(ShaderCompiler::ShaderType::kTessEvaluationShader,
                      ReadShaderSourceCode(subnode.second));
    }
  }

  shader->Link();
  LOG_SCOPE_F(INFO, "Pass: %s", pass->GetName().c_str());
  LOG_F(INFO, "Queue: %d", pass->GetQueue());
  shader->PrintInfo();
  pass->SetShader(shader);
  return pass;
}

std::shared_ptr<Material> MaterialLoader::Load(const std::string& filename) {
  std::shared_ptr<Material> material(new Material());

  YAML::Node root = YAML::LoadFile(filename);

  const std::string kNameKey     = "name";
  const std::string kTexturesKey = "textures";
  const std::string kPassKey     = "pass";

  bool has_pass = false;

  for (auto node: root) {
    std::string key = node.first.as<std::string>();

    if (key == kNameKey) {
      material->SetName(node.second.as<std::string>());
    } else if (key == kTexturesKey) {
      LoadTextures(*material, node.second);
    } else if (key == kPassKey) {
      has_pass = true;
      material->AddPass(LoadPass(node.second));
    }
  }

  if (!has_pass) {
    ABORT_F("Material must have at least one pass %s", filename.c_str()); 
  }

  return material;
}
