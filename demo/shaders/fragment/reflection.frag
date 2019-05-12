/*
  (C) 2019 David Lettier
  lettier.com
*/

#version 150

uniform sampler2D colorTexture;
uniform sampler2D colorBlurTexture;
uniform sampler2D specularTexture;

out vec4 fragColor;

void main() {
  vec2 texSize  = textureSize(colorTexture, 0).xy;
  vec2 texCoord = gl_FragCoord.xy / texSize;

  vec4 specular  = texture(specularTexture,  texCoord);
  vec4 color     = texture(colorTexture,     texCoord);
  vec4 colorBlur = texture(colorBlurTexture, texCoord);

  float specularAmount = dot(specular.rgb, vec3(1)) / 3;

  if (specularAmount <= 0) { fragColor = vec4(0); return; }

  float roughness = 1 - min(specular.a, 1);

  fragColor = mix(color, colorBlur, roughness) * specularAmount;
}
