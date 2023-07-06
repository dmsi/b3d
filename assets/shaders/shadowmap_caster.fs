#version 330 core

layout(location = 0) out float OUT_depth;

void main () {
  OUT_depth = gl_FragCoord.z; // no need to do this in explicit way, OpenGL does this anyway
}
