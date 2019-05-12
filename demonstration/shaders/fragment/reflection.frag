/*
  (C) 2019 David Lettier
  lettier.com
*/

#version 150

uniform sampler2D colorTexture;
uniform sampler2D colorBlurTexture;
uniform sampler2D maskTexture;

out vec4 fragColor;

void main() {
  vec2 texSize  = textureSize(colorTexture, 0).xy;
  vec2 texCoord = gl_FragCoord.xy / texSize;

  vec4 mask      = texture(maskTexture,      texCoord);
  vec4 color     = texture(colorTexture,     texCoord);
  vec4 colorBlur = texture(colorBlurTexture, texCoord);

  float amount = clamp(mask.r, 0.0, 1.0);

  if (amount <= 0.0) { fragColor = vec4(0.0); return; }

  float roughness = clamp(mask.g, 0.0, 1.0);

  fragColor = mix(color, colorBlur, roughness) * amount;
}
