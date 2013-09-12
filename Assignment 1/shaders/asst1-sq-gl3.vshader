#version 130

uniform float uVertexScale;
uniform int uWidth;
uniform int uHeight;

in vec2 aPosition;
in vec2 aTexCoord;

out vec2 vTexCoord;
out vec2 vTemp;

void main() {
  // TODO: MAKE THESE INTO CONSTANTS SOMEHOW!!!!
  // printf("HI FROM THE SHADER!");
  float uInitialWidth = 512.0, uInitialHeight = 512.0;

  float widthCoefficient = uWidth / uInitialWidth;
  float heightCoefficient = uHeight / uInitialHeight;
  float scaleCoefficient = min(widthCoefficient, heightCoefficient);
  gl_Position = vec4(aPosition.x * uVertexScale * uInitialWidth / uWidth, aPosition.y * uInitialHeight / uHeight, 0, 1);
  // gl_Position = vec4(0.5 - 256.0 / uWidth + aPosition.x, aPosition.y, 0, 1);
  vTexCoord = aTexCoord;
  vTemp = vec2(1, 1);
}
