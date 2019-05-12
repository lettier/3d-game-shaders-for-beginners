/*
  (C) 2019 David Lettier
  lettier.com
*/

#version 150

uniform sampler2D colorTexture;

uniform vec2 enabled;

out vec4 fragColor;

void main() {
  int   size       = 5;
  float separation = 3;
  float threshold  = 0.4;
  float amount     = 1;

  if (enabled.x != 1 || size <= 0) { fragColor = vec4(0); return; }

  vec2 texSize = textureSize(colorTexture, 0).xy;

  float value       = 0;
  vec3  valueRatios = vec3(0.3, 0.59, 0.11);

  vec4 result = vec4(0);
  vec4 color  = vec4(0);

  for (int i = -size; i <= size; ++i) {
    for (int j = -size; j <= size; ++j) {
      color =
        texture
          ( colorTexture
          ,   ( gl_FragCoord.xy
              + (vec2(i, j) * separation)
              )
            / texSize
          );

      value = dot(color.rgb, valueRatios);
      if (value < threshold) { color = vec4(0); }

      result.rgb += color.rgb;
    }
  }

  result.rgb /= pow(size * 2 + 1, 2);
  result.a = 1;

  fragColor = mix(vec4(0), result, amount);
}
