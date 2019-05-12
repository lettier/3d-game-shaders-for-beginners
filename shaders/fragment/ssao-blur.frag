/*
  (C) 2019 David Lettier
  lettier.com
*/

#version 140

uniform sampler2D ssaoTexture;

out vec4 fragColor;

void main() {
  float separation = 0.001;
  int samples      = 3;

  vec2 texSize  = textureSize(ssaoTexture, 0).xy;
  vec2 texCoord = gl_FragCoord.xy / texSize;

  int size  = samples;
  int size2 = size * size;

  int countMax  = size + ((size - 1) / 2);
  int countMin  = size - ((size - 1) / 2);

  int xCount = countMax;
  int yCount = countMax;

  int x = 0;
  int y = 0;

  vec3 result = vec3(0);

  for (int i = 0; i < size2; ++i) {
    x = size - xCount;
    y = yCount - size;

    result +=
      texture
        ( ssaoTexture
        ,   texCoord
          + vec2(x * separation, y * separation)
        ).rgb;

    xCount -= 1;
    if (xCount < countMin) { xCount = countMax; yCount -= 1; }
  }

  result = result / size2;

  fragColor = vec4(result, texture(ssaoTexture, texCoord).a);
}
