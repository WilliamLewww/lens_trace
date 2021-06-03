#version 430

layout (location = 0) in vec2 texturePosition;
out vec4 outColor;

uniform sampler2D textureSampler;
uniform sampler2D textureAccumulatedSampler;
uniform uint frameCount;

void main() {
  vec4 color = texture(textureSampler, texturePosition);
  if (frameCount > 0) {
    vec4 previousColor = texture(textureAccumulatedSampler, texturePosition);
    previousColor *= frameCount;

    color += previousColor;
    color /= frameCount + 1;
  }
  outColor = color;
}