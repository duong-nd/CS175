uniform vec3 uLight, uLight2, uColor;

varying vec3 vNormal;
varying vec3 vPosition;

void main() {
  vec3 normal = normalize(vNormal);

  vec3 viewDir = normalize(-vPosition);
  vec3 lightDir = normalize(uLight - vPosition);
  vec3 lightDir2 = normalize(uLight2 - vPosition);

  float nDotL = dot(normal, lightDir);
  vec3 reflection = normalize( 2.0 * normal * nDotL - lightDir);
  float rDotV = max(0.0, dot(reflection, viewDir));
  float specular = pow(rDotV, 64.0) * 0.4;
  float diffuse = max(nDotL, 0.0);

  nDotL = dot(normal, lightDir2);
  reflection = normalize( 2.0 * normal * nDotL - lightDir2);
  rDotV = max(0.0, dot(reflection, viewDir));
  specular += pow(rDotV, 64.0) * 0.4;
  diffuse += max(nDotL, 0.0);

  vec3 intensity = vec3(0.05, 0.05, 0.05) + uColor * diffuse + vec3(0.6, 0.6, 0.6) * specular;

  gl_FragColor = vec4(intensity.x, intensity.y, intensity.z, 1.0);
}
