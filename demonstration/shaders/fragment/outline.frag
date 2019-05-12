/*
  (C) 2019 David Lettier
  lettier.com
*/

#version 150

uniform vec2 gamma;

uniform sampler2D positionTexture;
uniform sampler2D colorTexture;
uniform sampler2D noiseTexture;
uniform sampler2D depthOfFieldTexture;
uniform sampler2D fogTexture;

uniform vec2 nearFar;
uniform vec2 enabled;

out vec4 fragColor;

void main() {
  float minSeparation = 1.0;
  float maxSeparation = 1.0;
  float minDistance   = 1.5;
  float maxDistance   = 2.0;
  float noiseScale    = 1.0;
  int   size          = 1;
  vec3  colorModifier = vec3(0.522, 0.431, 0.349);

  colorModifier = pow(colorModifier, vec3(gamma.x));

  float near = nearFar.x;
  float far  = nearFar.y;

  vec2 fragCoord = gl_FragCoord.xy;

  vec2 texSize   = textureSize(colorTexture, 0).xy;
  vec2 texCoord  = fragCoord / texSize;

  vec4  color        = texture(colorTexture,        texCoord);
  float depthOfField = texture(depthOfFieldTexture, texCoord).r;
  float fog          = texture(fogTexture,          texCoord).a;

  if (enabled.x != 1) { fragColor = color; return; }

  fragColor = vec4(0.0);

  vec2 noise  = texture(noiseTexture, fragCoord / textureSize(noiseTexture, 0).xy).rb;
       noise  = noise * 2.0 - 1.0;
       noise *= noiseScale;

  texCoord = (fragCoord - noise) / texSize;

  vec4 position     = texture(positionTexture, texCoord);
  vec4 positionTemp = position;

  if (position.a <= 0.0) { position.y = far; }

  float depth =
    clamp
      (   1.0
        - ( (far - position.y)
          / (far - near)
          )
      , 0.0
      , 1.0
      );

  float separation = mix(maxSeparation, minSeparation, depth);
  float count      = 1.0;
  float mx         = 0.0;

  for (int i = -size; i <= size; ++i) {
    for (int j = -size; j <= size; ++j) {
      texCoord =
          (vec2(i, j) * separation + (fragCoord + noise))
        / texSize;

      positionTemp =
        texture
          ( positionTexture
          , texCoord
          );

      if (positionTemp.y <= 0.0) { positionTemp.y = far; }

      mx = max(mx, abs(position.y - positionTemp.y));

      depthOfField =
        max
          ( texture
              ( depthOfFieldTexture
              , texCoord
              ).r
          , depthOfField
          );

      fog +=
        texture
          ( fogTexture
          , texCoord
          ).a;

      count += 1.0;
    }
  }

        depthOfField = 1.0 - clamp(depthOfField, 0.0, 1.0);
        fog          = 1.0 - clamp(fog / count,  0.0, 1.0);
  float diff         = smoothstep(minDistance, maxDistance, mx) * depthOfField * fog;

  texCoord = fragCoord / texSize;

  vec3 lineColor  = texture(colorTexture, texCoord).rgb;
       lineColor *= colorModifier;

  fragColor.rgb = mix(color.rgb, lineColor, clamp(diff, 0.0, 1.0));
  fragColor.a   = 1.0;
}
