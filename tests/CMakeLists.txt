cmake_minimum_required (VERSION 3.1)
project (borsch3d)

find_package(OpenGL REQUIRED)
find_package(GLEW REQUIRED)
find_package(GLFW3 REQUIRED)

include_directories (
  ${PROJECT_SOURCE_DIR}/../submodules/glm
  ${PROJECT_SOURCE_DIR}/../submodules/yaml-cpp/include
  ${PROJECT_SOURCE_DIR}/../src
  ${PROJECT_SOURCE_DIR}/../examples
  ${PROJECT_SOURCE_DIR}/../submodules/googletest/googletest/include
)

link_directories (
  ${PROJECT_SOURCE_DIR}/../submodules/yaml-cpp/build
  ${PROJECT_SOURCE_DIR}/../submodules/googletest/build/googlemock/gtest
  ${PROJECT_SOURCE_DIR}/../build
)

set(ALL_LIBS
	${OPENGL_LIBRARY}
  ${GLEW_LIBRARY}
  b3d
  glfw 
  yaml-cpp
  gtest_main
  gtest
)

set(CMAKE_CXX_STANDARD 17)

# Debug
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -Wall -O0 -g")
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -Wall")

add_definitions(
  -DGLM_ENABLE_EXPERIMENTAL
)

set(TESTS
  test_transform
  test_camera
  test_attributelayout
)

foreach(EX ${TESTS})
  add_executable(${EX} ${EX}.cc)
  target_link_libraries(${EX} ${ALL_LIBS})
endforeach(EX)
