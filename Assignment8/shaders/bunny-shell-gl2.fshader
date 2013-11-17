uniform sampler2D uTexShell;

uniform vec3 uLight;

uniform float uAlphaExponent;

varying vec3 vNormal;
varying vec3 vPosition;
varying vec2 vTexCoord;

void main() {
  vec3 normal = normalize(vNormal);
  vec3 toLight = normalize(uLight - vPosition);

  vec3 toP = -normalize(vPosition);

  vec3 h = normalize(toP + toLight);

  float u = dot(normal, toLight);
  float v = dot(normal, toP);
  u = 1.0 - u*u;
  v = pow(1.0 - v*v, 16.0);

  float r = 0.1 + 0.6 * u + 0.3 * v;
  float g = 0.1 + 0.3 * u + 0.3 * v;
  float b = 0.1 + 0.1 * u + 0.3 * v;

  float alpha = pow(texture2D(uTexShell, vTexCoord).r, uAlphaExponent);

  gl_FragColor = vec4(r, g, b, alpha);
}
