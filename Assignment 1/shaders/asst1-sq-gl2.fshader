uniform float uVertexScale;
uniform sampler2D uTex0, uTex1;

varying vec2 vTexCoord;
varying vec2 vTemp;

void main(void) {
  vec4 color = vec4(vTemp.x, vTemp.y, 0.5, 1);

  vec4 texColor0 = texture2D(uTex0, vTexCoord);
  vec4 texColor1 = texture2D(uTex1, vTexCoord);

  /* blending */
  float lerper = clamp(.5 *uVertexScale, 0., 1.);
  float lerper2 = clamp(.5 * uVertexScale + 1.0, 0.0, 1.0);

  gl_FragColor = ((lerper)*texColor1 + (1.0-lerper)*texColor0) * lerper2 + color * (1.0-lerper2);
}
