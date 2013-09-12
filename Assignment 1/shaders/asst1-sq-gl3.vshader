#version 130

uniform float uVertexScale;
uniform int uWidth;
uniform int uHeight;

in vec2 aPosition;
in vec2 aTexCoord;

out vec2 vTexCoord;
out vec2 vTemp;

void main() {
  gl_Position = vec4((aPosition.x + .001 * uWidth) * uVertexScale, aPosition.y, 0, 1);
  vTexCoord = aTexCoord;
  vTemp = vec2(1, 1);
}
