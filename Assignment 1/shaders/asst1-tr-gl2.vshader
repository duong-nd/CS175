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
  gl_Position =
    vec4((aPosition.x + uXOffset) * uXCoefficient, (aPosition.y + uYOffset) * uYCoefficient, 0, 1);
  vTexCoord = aTexCoord;
  vColor = aColor;
}
