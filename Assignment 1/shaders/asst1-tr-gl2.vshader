

uniform float uVertexScale;
uniform float uXCoefficient;
uniform float uYCoefficient;
uniform float uXOffset;
uniform float uYOffset;

attribute vec2 aPosition;
attribute vec2 aTexCoord;

varying vec2 vTexCoord;
varying vec2 vTemp;

void main() {
  gl_Position =
    vec4((aPosition.x + uXOffset) * uVertexScale * uXCoefficient, (aPosition.y + uYOffset) * uYCoefficient, 0, 1);
  vTexCoord = aTexCoord;
  vTemp = vec2(1, 1);
}
