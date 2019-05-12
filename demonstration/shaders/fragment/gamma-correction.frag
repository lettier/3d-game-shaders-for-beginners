/*
  (C) 2019 David Lettier
  lettier.com
*/

#version 150

uniform vec2 gamma;

uniform sampler2D colorTexture;

out vec4 fragColor;

void main() {
  vec2 texSize  = textureSize(colorTexture, 0).xy;
  vec2 texCoord = gl_FragCoord.xy / texSize;

  fragColor     = texture(colorTexture, texCoord);
  fragColor.rgb = pow(fragColor.rgb, vec3(gamma.y));
}
