/*
  (C) 2019 David Lettier
  lettier.com
*/

#version 150

uniform sampler2D materialDiffuseTexture;
uniform sampler2D fogTexture;

uniform vec2 enabled;

out vec4 fragColor;

void main() {
  int  separation = 1;
  int  threshold  = 0;
  vec3 color      = vec3(0.012, 0.014, 0.022);

  if (enabled.x != 1) { fragColor = vec4(0); return; }

  vec2 texSize   = textureSize(materialDiffuseTexture, 0).xy;
  vec2 fragCoord = gl_FragCoord.xy;

  vec4 fogColor = vec4(0);

  vec4 mx = vec4(0);
  vec4 mn = vec4(1);

  int x = -1;
  int y = -1;

  for (int i = 0; i < 9; ++i) {
    vec2 texCoord = (fragCoord + (vec2(x, y) * separation)) / texSize;

    vec4 fogColorTemp =
      texture
        ( fogTexture
        , texCoord
        );

    if (fogColorTemp.a > 0) { fogColor = fogColorTemp; }

    vec4 diffuse =
      texture
        ( materialDiffuseTexture
        , texCoord
        );

    mx = max(diffuse, mx);
    mn = min(diffuse, mn);

    x += 1;

    if (x >= 2) {
      x  = -1;
      y +=  1;
    }
  }

  float alpha = ((mx.r + mx.g + mx.b) / 3) - ((mn.r + mn.g + mn.b) / 3);
  if (alpha > threshold) { alpha = 1; }

  fragColor = mix(vec4(color, alpha), fogColor, fogColor.a);
}
