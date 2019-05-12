/*
  (C) 2019 David Lettier
  lettier.com
*/

#version 150

uniform vec2 gamma;

uniform sampler2D positionTexture;
uniform sampler2D colorTexture;
uniform sampler2D noiseTexture;

uniform vec2 nearFar;
uniform vec2 depthOfFieldEnabled;
uniform vec2 enabled;

out vec4 fragColor;

void main() {
  float minSeparation = 1.0;
  float maxSeparation = 2.0;
  float minDistance   = 1.5;
  float maxDistance   = 2.0;
  float noiseScale    = 5.0;
  int   size          = 1;
  vec3  colorModifier = vec3(0.324, 0.063, 0.099);

  colorModifier.rgb = pow(colorModifier.rgb, vec3(gamma.x));

  float near = nearFar.x;
  float far  = nearFar.y;

  fragColor = vec4(0.0);

  if (enabled.x != 1) { return; }

  vec2 fragCoord = gl_FragCoord.xy;
  vec2 texSize   = textureSize(noiseTexture, 0).xy;
  vec2 texCoord  = fragCoord/ texSize;

  vec2 noise  = texture(noiseTexture, texCoord).rb;
       noise  = noise * 2.0 - 1.0;
       noise *= noiseScale;

  texSize  = textureSize(colorTexture, 0).xy;
  texCoord = (fragCoord - noise) / texSize;

  vec4 position = texture(positionTexture, texCoord);
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

  float mx = 0.0;

  for (int i = -size; i <= size; ++i) {
    for (int j = -size; j <= size; ++j) {
      texCoord =
          (vec2(i, j) * separation + fragCoord + noise)
        / texSize;

      vec4 positionTemp =
        texture
          ( positionTexture
          , texCoord
          );
      if (positionTemp.y <= 0.0) { positionTemp.y = far; }
      if (depthOfFieldEnabled.x == 1 && positionTemp.y <= position.y) { continue; }

      mx = max(mx, abs(position.y - positionTemp.y));
    }
  }

  float diff = smoothstep(minDistance, maxDistance, mx);

  texCoord = fragCoord / texSize;

  vec3 lineColor = texture(colorTexture, texCoord).rgb * colorModifier;

  fragColor.rgb = lineColor;
  fragColor.a   = diff;
}
