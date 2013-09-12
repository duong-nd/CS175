uniform sampler2D uTex2;

varying vec2 vTexCoord;
varying vec3 vColor;

void main(void) {
  gl_FragColor = 0.5 * vec4(vColor.x, vColor.y, vColor.z, 1) + 0.5 * texture2D(uTex2, vTexCoord);
}
