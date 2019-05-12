/*
  (C) 2019 David Lettier
  lettier.com
*/

#version 140

uniform sampler2D baseTexture;
uniform sampler2D materialSpecularTexture;
uniform sampler2D reflectionTexture;
uniform sampler2D reflectionBlurTexture;
uniform sampler2D outlineTexture;
uniform sampler2D bloomTexture;

uniform vec3 backgroundColor;

out vec4 fragColor;

void main() {
  vec2 texSize  = textureSize(baseTexture, 0).xy;
  vec2 texCoord = gl_FragCoord.xy / texSize;

  vec4 baseColor             = texture(baseTexture,             texCoord);
  vec4 materialSpecularColor = texture(materialSpecularTexture, texCoord);
  vec4 relectionColor        = texture(reflectionTexture,       texCoord);
  vec4 relectionBlurColor    = texture(reflectionBlurTexture,   texCoord);
  vec4 outlineColor          = texture(outlineTexture,          texCoord);
  vec4 bloomColor            = texture(bloomTexture,            texCoord);

  float roughness = 1 - materialSpecularColor.a;

  vec3  relection       = mix(relectionColor.rgb, relectionBlurColor.rgb, roughness);
  float reflectionAlpha = mix(relectionColor.a,   relectionBlurColor.a,   roughness);

  fragColor      = baseColor;
  fragColor.rgb  = mix(baseColor.xyz, relection, reflectionAlpha);
  fragColor      = mix(fragColor, outlineColor, outlineColor.a);
  fragColor.rgb += bloomColor.rgb;
  fragColor      =
    vec4
      ( mix
          ( backgroundColor
          , fragColor.rgb
          , outlineColor.a + baseColor.a
          )
      , 1
      );
}
