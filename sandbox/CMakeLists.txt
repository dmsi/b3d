#------------------------------------------------------------------------------
# Build Examples
#------------------------------------------------------------------------------
set(EXAMPLES
  aabb_transform
  atmosphere
  frustum_check
  grass
  hmap
  layoutmesh
  objectpool
  profiling_nonbatch
  quad_tesselation
  radialshafts
  simulation
  skyfog_bloom
  sobel_normalmap
  uniformstorage
  water
)

add_definitions(
  -DGLM_ENABLE_EXPERIMENTAL
  -DGLFW_INCLUDE_NONE
)

foreach(EX ${EXAMPLES})
  add_executable(${EX} ${EX}/main.cc)
  target_link_libraries(${EX} ${ALL_LIBS} ${B3D_LIBRARY})
endforeach(EX)
