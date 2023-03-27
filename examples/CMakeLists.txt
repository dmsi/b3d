#------------------------------------------------------------------------------
# Build Examples
#------------------------------------------------------------------------------
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

add_definitions(
  -DGLM_ENABLE_EXPERIMENTAL
  -DGLFW_INCLUDE_NONE
)

foreach(EX ${EXAMPLES})
  add_executable(${EX} ${EX}/main.cc)
  target_link_libraries(${EX} ${ALL_LIBS} ${B3D_LIBRARY})
endforeach(EX)
