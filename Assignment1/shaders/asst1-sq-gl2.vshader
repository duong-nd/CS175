uniform float uVertexScale;
uniform float uXCoefficient;
uniform float uYCoefficient;

attribute vec2 aPosition;
attribute vec2 aTexCoord;

varying vec2 vTexCoord;
varying vec2 vTemp;

void main() {
  /* use the coefficients passed in as uniform variables to maintain the aspect ratio of the triangle */
  gl_Position = vec4(aPosition.x * uVertexScale * uXCoefficient, aPosition.y * uYCoefficient, 0, 1);
  
  vTexCoord = aTexCoord;
  vTemp = vec2(1, 1);
}
