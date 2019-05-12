/*
  (C) 2019 David Lettier
  lettier.com
*/

#version 140

uniform sampler2D bloomTexture;

uniform vec2 enabled;

out vec4 fragColor;

void main() {
  float separation = 3;
  int   samples    = 15;
  float threshold  = 0.6465;
  float amount     = 1;

  vec2 texSize = textureSize(bloomTexture, 0).xy;

  int size  = samples;
  int size2 = size * size;

  int countMax  = size + ((size - 1) / 2);
  int countMin  = size - ((size - 1) / 2);

  int xCount = countMax;
  int yCount = countMax;

  int x = 0;
  int y = 0;

  float value = 0;

  vec4 result = vec4(0);
  vec4 color = vec4(0);

  for (int i = 0; i < size2; ++i) {
    x = size - xCount;
    y = yCount - size;

    color =
      texture
        ( bloomTexture
        ,   ( gl_FragCoord.xy
            + vec2(x * separation, y * separation)
            )
          / texSize
        );

    value = ((0.3 * color.r) + (0.59 * color.g) + (0.11 * color.b));
    if (value < threshold) { color = vec4(0); }

    result += color;

    xCount -= 1;
    if (xCount < countMin) { xCount = countMax; yCount -= 1; }
  }

  result = result / size2;

  if (enabled.x == 1) {
    result.a = 1;
    result.rgb = mix(vec3(0), result.rgb, amount);
    fragColor = result;
  } else {
    fragColor = vec4(0, 0, 0, 1);
  }
}
