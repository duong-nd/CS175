uniform float uXCoefficient;
uniform float uYCoefficient;
uniform float uXOffset;
uniform float uYOffset;

attribute vec2 aPosition;
attribute vec2 aTexCoord;
attribute vec3 aColor;

varying vec2 vTexCoord;
varying vec3 vColor;

void main() {
  /* use the coefficients passed in as uniform variables to maintain the aspect ratio of the triangle */
  gl_Position = vec4((aPosition.x + uXOffset) * uXCoefficient, (aPosition.y + uYOffset) * uYCoefficient, 0, 1);

  vTexCoord = aTexCoord;
  vColor = aColor;
}
