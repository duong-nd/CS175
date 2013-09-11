#version 130

uniform float uVertexScale;
uniform sampler2D uTex0, uTex1;

in vec2 vTexCoord;
in vec2 vTemp;

out vec4 fragColor;

void main(void) {
  vec4 color = vec4(vTemp.x, vTemp.y, 0.5, 1);

  // The texture(...) calls always return a vec4. Data comes out of a texture in RGBA format
  vec4 texColor0 = texture(uTex0, vTexCoord);
  vec4 texColor1 = texture(uTex1, vTexCoord);

  // some fancy blending
  float lerper = clamp(.5 *uVertexScale, 0., 1.);
  float lerper2 = clamp(.5 * uVertexScale + 1.0, 0.0, 1.0);

  // fragColor is a vec4. The components are interpreted as red, green, blue, and alpha
  fragColor = ((lerper)*texColor1 + (1.0-lerper)*texColor0) * lerper2 + color * (1.0-lerper2);
}
