#------------------------------------------------------------------------------
# Build Examples
#------------------------------------------------------------------------------
set(EXAMPLES
  aabb_transform
  atmosphere
  frustum_check
)

add_definitions(
  -DGLM_ENABLE_EXPERIMENTAL
  -DGLFW_INCLUDE_NONE
)

foreach(EX ${EXAMPLES})
  add_executable(${EX} ${EX}/main.cc)
  target_link_libraries(${EX} ${ALL_LIBS} ${B3D_LIBRARY})
endforeach(EX)
