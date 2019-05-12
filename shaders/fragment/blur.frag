/*
  (C) 2019 David Lettier
  lettier.com
*/

#version 140

uniform sampler2D blurTexture;

uniform vec2 parameters;

out vec4 fragColor;

void main() {
  vec2 texSize  = textureSize(blurTexture, 0).xy;
  vec2 texCoord = gl_FragCoord.xy / texSize;

  int size  = int(parameters.y);
  int size2 = size * size;

  int countMax  = size + ((size - 1) / 2);
  int countMin  = size - ((size - 1) / 2);

  int xCount = countMax;
  int yCount = countMax;

  int x = 0;
  int y = 0;

  vec4 result = vec4(0);

  for (int i = 0; i < size2; ++i) {
    x = size - xCount;
    y = yCount - size;

    result +=
      texture
        ( blurTexture
        ,   texCoord
          + vec2(x * parameters.x, y * parameters.x)
        );

    xCount -= 1;
    if (xCount < countMin) { xCount = countMax; yCount -= 1; }
  }

  result = result / size2;

  fragColor = result;
}
