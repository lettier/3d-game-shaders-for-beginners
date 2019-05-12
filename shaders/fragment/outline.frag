/*
  (C) 2019 David Lettier
  lettier.com
*/

#version 140

uniform sampler2D materialDiffuseTexture;
uniform sampler2D positionTexture;

uniform struct p3d_FogParameters
  { vec4 color
  ; float start
  ; float end
  ;
  } p3d_Fog;

uniform vec2 enabled;
uniform vec2 fogEnabled;

out vec4 fragColor;

void main() {
  int  separation = 1;
  int  threshold  = 0;
  vec3 lineRgb    = vec3(0.012, 0.014, 0.022);

  vec2 texSize  = textureSize(materialDiffuseTexture, 0).xy;
  vec2 texCoord = gl_FragCoord.xy;

  vec4 mx = vec4(0);
  vec4 mn = vec4(1);

  int x = -1;
  int y = -1;

  for (int i = 0; i < 9; ++i) {
    vec4 color =
      texture
        ( materialDiffuseTexture
        , (texCoord + (vec2(x, y) * separation)) / texSize
        );
    mx = max(color, mx);
    mn = min(color, mn);
    x += 1;
    if (x >= 2) {
      x  = -1;
      y +=  1;
    }
  }

  float alpha = ((mx.r + mx.g + mx.b) / 3) - ((mn.r + mn.g + mn.b) / 3);
  if (alpha > threshold) { alpha = 1; }

  vec4 lineColor = vec4(lineRgb, alpha);

  if (enabled.x == 1) {
    if (fogEnabled.x == 1) {
      vec4 position = texture(positionTexture, texCoord / texSize);

      float fogIntensity =
        clamp
          (   ( p3d_Fog.end - position.y)
            / ( p3d_Fog.end - p3d_Fog.start)
          , 0
          , 1
          );
      fogIntensity = 1 - fogIntensity;

      vec4 lineWithFogColor =
        mix
          ( lineColor
          , p3d_Fog.color
          , fogIntensity
          );

      fragColor = vec4(lineWithFogColor.rgb, alpha);
    } else {
      fragColor = lineColor;
    }
  }
}
