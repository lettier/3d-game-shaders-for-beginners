/*
  (C) 2019 David Lettier
  lettier.com
*/

#version 150

uniform vec2 pi;
uniform vec2 gamma;

uniform vec4 backgroundColor0;
uniform vec4 backgroundColor1;

uniform sampler2D positionTexture0;
uniform sampler2D positionTexture1;
uniform sampler2D smokeMaskTexture;

uniform vec3 origin;
uniform vec2 nearFar;
uniform vec2 sunPosition;
uniform vec2 enabled;

out vec4 fragColor;

void main() {
  float fogMin = 0.00;
  float fogMax = 0.97;

  if (enabled.x != 1) { fragColor = vec4(0); return; }

  vec2 texSize  = textureSize(positionTexture0, 0).xy;
  vec2 texCoord = gl_FragCoord.xy / texSize;

  vec4 smokeMask    = texture(smokeMaskTexture, texCoord);
  vec4 position0    = texture(positionTexture0, texCoord);
       position0.y -= origin.y;

  float near = nearFar.x;
  float far  = nearFar.y;

  vec4 position1    = texture(positionTexture1, texCoord);
       position1.y -= origin.y;
  if (position1.a <= 0) { position1.y = far; }

  vec4 position = position1;

  if        (position0.a <= 0 && smokeMask.r > 0) {
    position.y   = mix(far,           position1.y,   smokeMask.r);
  } else if (position0.a >  0 && smokeMask.r > 0) {
    position.xyz = mix(position0.xyz, position1.xyz, smokeMask.r);
  }

  float random =
    fract
      ( 10000
      * sin
          (
            ( gl_FragCoord.x
            * 104729
            + gl_FragCoord.y
            * 7639
            )
          * pi.y
          )
      );

  vec4 backgroundColor0     = backgroundColor0;
  vec4 backgroundColor1     = backgroundColor1;
       backgroundColor0.rgb = pow(backgroundColor0.rgb, vec3(gamma.x));
       backgroundColor1.rgb = pow(backgroundColor1.rgb, vec3(gamma.x));

  vec4 color =
    mix
      ( backgroundColor0
      , backgroundColor1
      , 1.0 - clamp(random * 0.1 + texCoord.y, 0.0, 1.0)
      );

  float sunPosition = max(0.2, -1 * sin(sunPosition.x * pi.y));

  color.rgb *= sunPosition;
  color.b    = mix(color.b + 0.05, color.b, sunPosition);

  float intensity =
    clamp
      (   (position.y - near)
        / (far        - near)
      , fogMin
      , fogMax
      );

  fragColor = vec4(color.rgb, intensity);
}
