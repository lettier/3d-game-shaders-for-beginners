/*
  (C) 2019 David Lettier
  lettier.com
*/

#version 150

uniform sampler2D positionTexture;
uniform sampler2D noiseTexture;
uniform sampler2D focusTexture;
uniform sampler2D outOfFocusTexture;

uniform vec2 mouseFocusPoint;
uniform vec2 nearFar;
uniform vec2 outlineEnabled;
uniform vec2 enabled;

out vec4 fragColor;

void main() {
  float minDistance = 1.0;
  float maxDistance = 3.0;
  float noiseScale  = 5.0;

  noiseScale = outlineEnabled.x == 1 ? noiseScale : 0.0;

  float far  = nearFar.y;

  vec2 texSize  = textureSize(focusTexture, 0).xy;
  vec2 texCoord = gl_FragCoord.xy / texSize;

  vec4 focusColor = texture(focusTexture, texCoord);

  fragColor = focusColor;

  if (enabled.x != 1) { return; }

  texSize  = textureSize(noiseTexture, 0).xy;
  texCoord = gl_FragCoord.xy / texSize;

  vec2 noise  = texture(noiseTexture, texCoord).rb;
       noise  = noise * 2.0 - 1.0;
       noise *= noiseScale;

  texSize  = textureSize(focusTexture, 0).xy;
  texCoord = (gl_FragCoord.xy - noise) / texSize;

  vec4 position = texture(positionTexture, texCoord);
  if (outlineEnabled.x == 1 && position.a <= 0) { position.y = far; }
  else if (position.a <= 0) { return; }

  vec4 outOfFocusColor = texture(outOfFocusTexture, texCoord);
  vec4 focusPoint      = texture(positionTexture,   mouseFocusPoint);

  float blur =
    smoothstep
      ( minDistance
      , maxDistance
      , abs(position.y - focusPoint.y)
      );

  fragColor = mix(focusColor, outOfFocusColor, blur);
}
