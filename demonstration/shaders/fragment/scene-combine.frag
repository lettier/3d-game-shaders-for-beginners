/*
  (C) 2019 David Lettier
  lettier.com
*/

#version 150

uniform vec2 pi;
uniform vec2 gamma;

uniform sampler2D baseTexture;
uniform sampler2D bloomTexture;
uniform sampler2D fogTexture;

uniform vec4 backgroundColor0;
uniform vec4 backgroundColor1;

uniform vec2 sunPosition;

out vec4 fragColor;

void main() {
  vec2 texSize  = textureSize(baseTexture, 0).xy;
  vec2 texCoord = gl_FragCoord.xy / texSize;

  vec4 backgroundColor0     = backgroundColor0;
  vec4 backgroundColor1     = backgroundColor1;
       backgroundColor0.rgb = pow(backgroundColor0.rgb, vec3(gamma.x));
       backgroundColor1.rgb = pow(backgroundColor1.rgb, vec3(gamma.x));

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

  float sunPosition = sin(sunPosition.x * pi.y);
        sunPosition = max(0.2, -1 * sunPosition);

  vec4 backgroundColor =
    mix
      ( backgroundColor0
      , backgroundColor1
      , 1.0 - clamp(random * 0.1 + texCoord.y, 0.0, 1.0)
      );

  backgroundColor.rgb *= sunPosition;
  backgroundColor.b    = mix(backgroundColor.b + 0.05, backgroundColor.b, sunPosition);

  vec4 baseColor  = texture(baseTexture,  texCoord);
  vec4 bloomColor = texture(bloomTexture, texCoord);
  vec4 fogColor   = texture(fogTexture,   texCoord);

  fragColor     = baseColor;
  fragColor     = fragColor + bloomColor;
  fragColor     = mix(fragColor, fogColor, min(fogColor.a, 1));
  fragColor     =
    vec4
      ( mix
          ( backgroundColor.rgb
          , fragColor.rgb
          , min(baseColor.a + fogColor.a, 1)
          )
      , 1
      );
}
