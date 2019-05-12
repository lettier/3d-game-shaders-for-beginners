/*
  (C) 2021 David Lettier
  lettier.com
*/

#version 150

uniform sampler2D colorTexture;

uniform vec2 enabled;
uniform vec2 mouseFocusPoint;

out vec4 fragColor;

void main() {
  float redOffset   =  0.009;
  float greenOffset =  0.006;
  float blueOffset  = -0.006;

  vec2 texSize  = textureSize(colorTexture, 0).xy;
  vec2 texCoord = gl_FragCoord.xy / texSize;

  vec2 direction = texCoord - mouseFocusPoint;

  fragColor = texture(colorTexture, texCoord);

  if (enabled.x != 1) { return; }

  fragColor.r = texture(colorTexture, texCoord + (direction * vec2(redOffset  ))).r;
  fragColor.g = texture(colorTexture, texCoord + (direction * vec2(greenOffset))).g;
  fragColor.b = texture(colorTexture, texCoord + (direction * vec2(blueOffset ))).b;
}
