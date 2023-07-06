#version 330 core

layout(location = 0) in vec3 IN_position;

uniform mat4 SU_PVM_MATRIX;

void main() {
  gl_Position = SU_PVM_MATRIX * vec4(IN_position, 1); 
}
