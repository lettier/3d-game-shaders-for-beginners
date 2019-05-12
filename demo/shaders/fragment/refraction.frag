/*
  (C) 2019 David Lettier
  lettier.com
*/

#version 150

uniform sampler2D uvTexture;
uniform sampler2D maskTexture;
uniform sampler2D positionFromTexture;
uniform sampler2D positionToTexture;
uniform sampler2D backgroundColorTexture;

out vec4 fragColor;

void main() {
  vec4  tintColor = vec4(0.27, 0.58, 0.92, 0.3);
  float depthMax  = 2;

  vec2 texSize  = textureSize(backgroundColorTexture, 0).xy;
  vec2 texCoord = gl_FragCoord.xy / texSize;

  vec4 uv              = texture(uvTexture,              texCoord);
  vec4 mask            = texture(maskTexture,            texCoord);
  vec4 positionFrom    = texture(positionFromTexture,    texCoord);
  vec4 positionTo      = texture(positionToTexture,      texCoord);
  vec4 backgroundColor = texture(backgroundColorTexture, uv.xy);

  if (mask.r <= 0) { fragColor = vec4(0); return; }

  float depth   = length(positionTo.xyz - positionFrom.xyz);
  float mixture = clamp(depth / depthMax, 0, 1);

  vec3 shallowColor    = backgroundColor.rgb;
  vec3 deepColor       = mix(shallowColor, tintColor.rgb, tintColor.a);
  vec3 foregroundColor = mix(shallowColor, deepColor,     mixture);

  fragColor = mix(vec4(0), vec4(foregroundColor, 1), uv.b);
}
