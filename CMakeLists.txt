cmake_minimum_required (VERSION 3.10)

project(B3D VERSION 0.1.0 LANGUAGES C CXX)

#------------------------------------------------------------------------------
# Options & Vars
#------------------------------------------------------------------------------
option(B3D_PROFILING "Builds with profiling information" OFF)
option(B3D_BUILD_EXAMPLES "Builds with examples" ON)
option(B3D_BUILD_SANDBOX "Builds with sandbox examples (for internal use)" ON)
option(B3D_BUILD_GLFW "Builds GLFW along with B3D" ON)
option(B3D_BUILD_TESTS "Builds B3D tests" OFF)

# TODO does not work...
option (FORCE_COLORED_OUTPUT "Always produce ANSI-colored output (GNU/Clang only)." TRUE)
if (${FORCE_COLORED_OUTPUT})
    if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
       add_compile_options (-fdiagnostics-color=always)
    elseif ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
       add_compile_options (-fcolor-diagnostics)
    endif ()
endif ()


set(B3D_LIBRARY b3d)
set(DEPS_DIR ${PROJECT_SOURCE_DIR}/deps)

if (NOT CMAKE_BUILD_TYPE)
  message(STATUS "CMAKE_BUILD_TYPE not set: Release is used for the current build")
  set(CMAKE_BUILD_TYPE Release)
endif()

if (CMAKE_BUILD_TYPE STREQUAL Debug)
  message(STATUS "Debug mode activated")
  set(_B3D_DEBUG ON)
elseif (CMAKE_BUILD_TYPE STREQUAL Release)
  message(STATUS "Release mode activated")
  set(_B3D_DEBUG OFF)
endif()

#------------------------------------------------------------------------------
# Packages for all platforms
#------------------------------------------------------------------------------
find_package(OpenGL REQUIRED)
find_package(Threads REQUIRED)
find_package(PkgConfig REQUIRED)

#------------------------------------------------------------------------------
# Platform-specific packages
#------------------------------------------------------------------------------
if (APPLE)
  find_library(COCOA_LIBRARY Cocoa)
  find_library(COREVIDEO_LIBRARY CoreVideo)
  find_library(IOKIT_LIBRARY IOKit)
endif()

#------------------------------------------------------------------------------
# Include dirs
#------------------------------------------------------------------------------
include_directories (
  ${DEPS_DIR}/glad/include
  ${DEPS_DIR}/yaml-cpp/include
  ${DEPS_DIR}/glm
  ${DEPS_DIR}/entt/single_include
  ${PROJECT_SOURCE_DIR}/src
  ${PROJECT_SOURCE_DIR}/examples
)

#------------------------------------------------------------------------------
# LIB dirs
#------------------------------------------------------------------------------
link_directories (
)

set(ALL_LIBS
  ${OPENGL_LIBRARY}
  ${CMAKE_DL_LIBS}
  ${CMAKE_THREAD_LIBS_INIT}
  yaml-cpp
)

#------------------------------------------------------------------------------
# Add GLFW library
#------------------------------------------------------------------------------
if (B3D_BUILD_GLFW)
  # https://www.glfw.org/docs/3.3/build.html
  list (APPEND ALL_LIBS glfw)
else()
  pkg_search_module(GLFW REQUIRED glfw3)
  include_directories(${GLFW_INCLUDE_DIRS})
  link_directories(${GLFW_LIBRARY_DIRS})
  list (APPEND ALL_LIBS ${GLFW_LIBRARIES})
endif()

#------------------------------------------------------------------------------
# Add platform-specific libs
#------------------------------------------------------------------------------
if (APPLE)
  list (APPEND ALL_LIBS
    ${COCOA_LIBRARY}
    ${COREVIDEO_LIBRARY}
    ${IOKIT_LIBRARY}
  )
elseif (UNIX AND NOT APPLE)
  list (APPEND ALL_LIBS
    ${X11_LIBRARIES}
  )
endif()

#------------------------------------------------------------------------------
# Set compiler flags
# TODO: Set debug flags and release flags
#------------------------------------------------------------------------------
set(CMAKE_CXX_STANDARD 17)

# TODO this does not work well with MSVS compiler
if (_B3D_DEBUG)
  # different tools available like sanitize thread or memory
  #set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -Wall -O0 -g -fsanitize=thread")
  set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -Wall -O0 -g" )
else()
  set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -O3 -g")
endif()

if (APPLE)
  if (B3D_PROFILING)
    # TODO: This is not for profiling, it is required for LuaJIT/FFI (witout FFI works fine)
    #set(CMAKE_EXE_LINKER_FLAGS  "${CMAKE_EXE_LINKER_FLAGS}  -pagezero_size 10000 -image_base 100000000")
  endif()
elseif (UNIX AND NOT APPLE)
  if (B3D_PROFILING)
    set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pg")
    set (CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -pg")
  endif()
endif()

#------------------------------------------------------------------------------
# Build targets
#------------------------------------------------------------------------------

set(GLAD_ALL_EXTENSIONS ON CACHE BOOL "" FORCE)
set(GLAD_REPRODUCIBLE ON CACHE BOOL "" FORCE)
add_subdirectory(${DEPS_DIR}/glad)
list(APPEND ALL_LIBS ${GLAD_LIBRARIES})

set(YAML_CPP_BUILD_TESTS OFF CACHE BOOL "" FORCE)
add_subdirectory(${DEPS_DIR}/yaml-cpp)

if (B3D_BUILD_GLFW)
  set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
  set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
  set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
  add_subdirectory(${DEPS_DIR}/glfw)
endif()

add_subdirectory(src)

if (B3D_BUILD_EXAMPLES)
  add_subdirectory(examples)
endif()

if (B3D_BUILD_SANDBOX)
  add_subdirectory(sandbox)
endif()

#------------------------------------------------------------------------------
# Build tests
#------------------------------------------------------------------------------
if (B3D_BUILD_TESTS)
  enable_testing()
  add_subdirectory(${DEPS_DIR}/googletest)
  add_subdirectory(tests)
endif()
