/*
  (C) 2019 David Lettier
  lettier.com
*/

#version 150

uniform sampler2D baseTexture;
uniform sampler2D fogTexture;
uniform sampler2D bloomTexture;
uniform sampler2D outlineTexture;

uniform vec4 backgroundColor0;
uniform vec4 backgroundColor1;

out vec4 fragColor;

void main() {
  vec2 texSize  = textureSize(baseTexture, 0).xy;
  vec2 texCoord = gl_FragCoord.xy / texSize;

  vec4 backgroundColor =
    mix
      ( backgroundColor0
      , backgroundColor1
      , 1 - texCoord.y
      );

  vec4 baseColor    = texture(baseTexture,    texCoord);
  vec4 bloomColor   = texture(bloomTexture,   texCoord);
  vec4 outlineColor = texture(outlineTexture, texCoord);
  vec4 fogColor     = texture(fogTexture,     texCoord);

  fragColor = baseColor;
  fragColor = clamp(fragColor + bloomColor, vec4(0), fragColor + 0.2);
  fragColor = mix(fragColor, outlineColor, min(outlineColor.a, 1));
  fragColor = mix(fragColor, fogColor,     min(fogColor.a,     1));
  fragColor =
    vec4
      ( mix
          ( backgroundColor.rgb
          , fragColor.rgb
          , min(baseColor.a + outlineColor.a + fogColor.a, 1)
          )
      , 1
      );
}
