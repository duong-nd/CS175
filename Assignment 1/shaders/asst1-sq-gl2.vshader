uniform float uVertexScale;
uniform float uXCoefficient;
uniform float uYCoefficient;

attribute vec2 aPosition;
attribute vec2 aTexCoord;

varying vec2 vTexCoord;
varying vec2 vTemp;

void main() {
  gl_Position =
    vec4(aPosition.x * uVertexScale * uXCoefficient, aPosition.y * uYCoefficient, 0, 1);
  vTexCoord = aTexCoord;
  vTemp = vec2(1, 1);
}
