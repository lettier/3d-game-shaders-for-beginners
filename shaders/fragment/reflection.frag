/*
  (C) 2019 David Lettier
  lettier.com
*/

#version 140

uniform sampler2D reflectedTexture;
uniform sampler2D materialSpecularTexture;
uniform sampler2D ssrTexture;

out vec4 fragColor;

void main() {
  vec2 texSize  = textureSize(reflectedTexture, 0).xy;
  vec2 texCoord = gl_FragCoord.xy / texSize;

  vec4 materialSpecularColor = texture(materialSpecularTexture, texCoord);
  vec4 ssrColor              = texture(ssrTexture,              texCoord);
  vec4 relectionColor        = texture(reflectedTexture,        ssrColor.xy);

  float specularValue =
    ( (0.3  * materialSpecularColor.r)
    + (0.59 * materialSpecularColor.g)
    + (0.11 * materialSpecularColor.b)
    );

  fragColor = mix(vec4(0), relectionColor, ssrColor.b * specularValue);
}
