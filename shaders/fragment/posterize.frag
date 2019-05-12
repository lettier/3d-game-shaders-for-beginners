/*
  (C) 2019 David Lettier
  lettier.com
*/

#version 140

uniform sampler2D posterizeTexture;

uniform vec2 enabled;

out vec4 fragColor;

void main() {
  float levels = 8;

  vec2 texSize  = textureSize(posterizeTexture, 0).xy;
  vec2 texCoord = gl_FragCoord.xy / texSize;

  vec4 texColor = texture(posterizeTexture, texCoord);

  if (enabled.x == 1) {
    vec3 grey  = vec3((texColor.r + texColor.g + texColor.b) / 3.0);
    vec3 grey1 = grey;

    grey = floor(grey * levels) / levels;

    texColor.rgb += (grey - grey1);
  }

  fragColor = texColor;
}
