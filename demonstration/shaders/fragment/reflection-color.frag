/*
  (C) 2019 David Lettier
  lettier.com
*/

#version 150

uniform sampler2D uvTexture;
uniform sampler2D colorTexture;

out vec4 fragColor;

void main() {
  vec2 texSize  = textureSize(uvTexture, 0).xy;
  vec2 texCoord = gl_FragCoord.xy / texSize;

  vec4 uv = texture(uvTexture, texCoord);
  if (uv.b <= 0) { fragColor = vec4(0); return;}

  vec4 color = texture(colorTexture, uv.xy);

  float alpha = clamp(uv.b, 0, 1);

  fragColor = vec4(mix(vec3(0), color.rgb, alpha), alpha);
}
