/*
  (C) 2019 David Lettier
  lettier.com
*/

#version 150

uniform sampler2D colorTexture;

uniform vec2 enabled;

out vec4 fragColor;

void main() {
  int   size       = 3;
  float separation = 4.0;
  float threshold  = 0.6;
  float amount     = 0.6;

  if (enabled.x != 1 || size <= 0) { fragColor = vec4(0); return; }

  vec2 texSize = textureSize(colorTexture, 0).xy;

  vec4 result = vec4(0.0);
  vec4 color  = vec4(0.0);

  float value = 0.0;
  float count = 0.0;

  for (int i = -size; i <= size; ++i) {
    for (int j = -size; j <= size; ++j) {
      color =
        texture
          ( colorTexture
          ,   (vec2(i, j) * separation + gl_FragCoord.xy)
            / texSize
          );

      value = max(color.r, max(color.g, color.b));
      if (value < threshold) { color = vec4(0.0); }

      result += color;
      count  += 1.0;
    }
  }

  result /= count;

  fragColor = mix(vec4(0.0), result, amount);
}
