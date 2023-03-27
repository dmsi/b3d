#------------------------------------------------------------------------------
# B3D Library target
#------------------------------------------------------------------------------
set(ALL_SOURCES
  ${DEPS_DIR}/loguru/loguru.cpp
  appcontext.cc
  input.cc
  texture.cc
  texture2d.cc
  texture_cube.cc
  meshfilter.cc
  mesh.cc
  meshloader.cc
  material/shader.cc
  material/pass.cc
  material/material.cc
  material/material_loader.cc
  scene.cc
  renderqueue.cc
  rendertarget.cc
  framebuffer.cc
  image/portablepixmap.cc
  image/loader.cc
  noise/perlin.cc
  luascript/luaaction.cc
)

add_definitions(
  -DGLM_ENABLE_EXPERIMENTAL
  -DGLFW_INCLUDE_NONE
)

add_library(${B3D_LIBRARY} STATIC ${ALL_SOURCES})
target_link_libraries(${B3D_LIBRARY} ${ALL_LIBS})

