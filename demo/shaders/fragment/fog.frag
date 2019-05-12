/*
  (C) 2019 David Lettier
  lettier.com
*/

#version 150

uniform vec4 backgroundColor0;
uniform vec4 backgroundColor1;

uniform sampler2D positionTexture;

uniform vec2 nearFar;

uniform vec2 enabled;

out vec4 fragColor;

void main() {
  float fogMin = 0.00;
  float fogMax = 0.97;

  vec2 texSize  = textureSize(positionTexture, 0).xy;
  vec2 texCoord = gl_FragCoord.xy / texSize;

  vec4 color =
    mix
      ( backgroundColor0
      , backgroundColor1
      , 1 - texCoord.y
      );

  vec4 position = texture(positionTexture, texCoord);

  if (position.a <= 0 || enabled.x != 1) { fragColor = vec4(0); return; }

  float near = nearFar.x;
  float far  = nearFar.y;

  float intensity =
    clamp
      (   (position.y - near)
        / (far        - near)
      , fogMin
      , fogMax
      );

  fragColor =
    mix
      ( vec4(0)
      , color
      , intensity
      );
}
