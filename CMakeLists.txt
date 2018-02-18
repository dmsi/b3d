cmake_minimum_required (VERSION 3.1)
project (borsch3d)

find_package(OpenGL REQUIRED)
find_package(GLEW REQUIRED)
find_package(GLFW3 REQUIRED)

include_directories (
  ${PROJECT_SOURCE_DIR}/submodules/glm
  ${PROJECT_SOURCE_DIR}/submodules/loguru
  ${PROJECT_SOURCE_DIR}/submodules/yaml-cpp/include
  ${PROJECT_SOURCE_DIR}/src
  ${PROJECT_SOURCE_DIR}/examples
)

link_directories (
  ${PROJECT_SOURCE_DIR}/submodules/yaml-cpp/build
)

set(ALL_LIBS
	${OPENGL_LIBRARY}
  ${GLEW_LIBRARY}
  glfw 
  yaml-cpp
  pthread 
)

# gprof support
#set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pg")
#set (CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -pg")

set(CMAKE_CXX_STANDARD 17)

# Debug
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -Wall -O0 -g")
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -Wall")

set(ALL_SOURCES
  src/appcontext.cc
  src/input.cc
  src/texture.cc
  src/texture2d.cc
  src/texture_cube.cc
  src/meshfilter.cc
  src/meshloader.cc
  src/material/shader.cc
  src/material/pass.cc
  src/material/material.cc
  src/material/material_loader.cc
  src/scene.cc
  src/renderqueue.cc
  src/rendertarget.cc
  src/framebuffer.cc
  src/image/portablepixmap.cc
  src/image/loader.cc
  src/noise/perlin.cc
)

add_definitions(
  -DGLM_ENABLE_EXPERIMENTAL
)

add_library(b3d STATIC ${ALL_SOURCES})
target_link_libraries(b3d ${ALL_LIBS})

set(EXAMPLES
  01_helloworld
  02_rotating_model
  04_ortho_camera
  05_cubemap_skybox
  06_clipping_plane
  07_render_to_texture
  08_dudv_distortion
  09_render_to_cubemap
  10_flatshading_terrain
  11_directional_light
  12_deferred_rendering
  13_horizontal_reflections
  14_water
  15_skyfog
  16_shadowmap
  17_omni_shadowmap
  18_omni_shadowmap_multi
  20_multiple_color_attachments
  21_multicam_frustum_test
  25_instancing_raw
  26_instancing_batch
  27_tri_tesselation
  30_postprocess_gauss_blur
  31_postprocess_bloom
  32_postprocess_radialshafts
  40_quadtree_visual
  41_quadtree_tesselation
)

set(SANDBOX
  #aabb_transform
  #atmosphere
  #frustum_check
  #hmap
  #objectpool
  #profiling_nonbatch
  #quad_tesselation
  #radialshafts
  #sandbox
  #simulation
  #skyfog_bloom
  #sobel_normalmap
  #water
)

foreach(EX ${EXAMPLES})
  add_executable(${EX} examples/${EX}/main.cc)
  target_link_libraries(${EX} ${ALL_LIBS} b3d)
endforeach(EX)
  
foreach(EX ${SANDBOX})
  add_executable(${EX} sandbox/${EX}/main.cc)
  target_link_libraries(${EX} ${ALL_LIBS} b3d)
endforeach(EX)
