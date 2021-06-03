#version 430

layout (location = 0) in vec2 a_vertexPosition;
layout (location = 1) in vec2 a_texturePosition;
layout (location = 0) out vec2 texturePosition;

void main() {
  gl_Position = vec4(a_vertexPosition, 0.0, 1.0);
  texturePosition = vec2(a_texturePosition.x, a_texturePosition.y);
}