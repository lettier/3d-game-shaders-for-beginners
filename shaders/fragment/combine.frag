/*
  (C) 2019 David Lettier
  lettier.com
*/

#version 140

uniform sampler2D baseTexture;
uniform sampler2D outlineTexture;
uniform sampler2D bloomTexture;

uniform vec3 backgroundColor;

out vec4 fragColor;

void main() {
  vec2 texSize  = textureSize(baseTexture, 0).xy;
  vec2 texCoord = gl_FragCoord.xy / texSize;

  vec4 baseColor    = texture(baseTexture, texCoord);
  vec4 outlineColor = texture(outlineTexture, texCoord);
  vec4 bloomColor   = texture(bloomTexture, texCoord);

  fragColor      = mix(baseColor, outlineColor, outlineColor.a);
  fragColor.rgb += bloomColor.rgb;
  fragColor        =
    vec4
      ( mix
          ( backgroundColor
          , fragColor.rgb
          , outlineColor.a + baseColor.a
          )
      , 1
      );
}
