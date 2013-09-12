#version 130

uniform float uVertexScale;
uniform sampler2D uTex2;

in vec2 vTexCoord;
in vec2 vColor;

out vec4 fragColor;

void main(void) {
  vec4 color = vec4(vColor.x, vColor.y, vColor.z, 1);
  vec4 texColor2 = texture(uTex2, vTexCoord);

  /* blending */
  float lerper = clamp(.5 *uVertexScale, 0., 1.);
  float lerper2 = clamp(.5 * uVertexScale + 1.0, 0.0, 1.0);

  fragColor = ((lerper)*color + (1.0-lerper)*texColor2) * lerper2 + color * (1.0-lerper2);
}
