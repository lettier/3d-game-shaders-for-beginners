/*
  (C) 2019 David Lettier
  lettier.com
*/

#version 140

uniform sampler2D pixelizeTexture;

uniform vec2 enabled;

out vec4 fragColor;

void main() {
  // Must be odd.
  int pixelSize = 5;

  vec2 texSize  = textureSize(pixelizeTexture, 0).xy;
  vec2 texCoord = gl_FragCoord.xy / texSize;

  float x = int(gl_FragCoord.x) % pixelSize;
  float y = int(gl_FragCoord.y) % pixelSize;

  x = floor(pixelSize / 2.0) - x;
  y = floor(pixelSize / 2.0) - y;

  x = gl_FragCoord.x + x;
  y = gl_FragCoord.y + y;

  if (enabled.x == 1) {
    fragColor = texture(pixelizeTexture, vec2(x, y) / texSize);
  } else {
    fragColor = texture(pixelizeTexture, texCoord);
  }
}
