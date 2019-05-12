/*
  (C) 2019 David Lettier
  lettier.com
*/

#version 150

uniform sampler2D baseTexture;
uniform sampler2D refractionTexture;
uniform sampler2D foamTexture;
uniform sampler2D reflectionTexture;
uniform sampler2D specularTexture;

out vec4 fragColor;

void main() {
  vec2 texSize  = textureSize(baseTexture, 0).xy;
  vec2 texCoord = gl_FragCoord.xy / texSize;

  vec4 base       = texture(baseTexture,       texCoord);
  vec4 refraction = texture(refractionTexture, texCoord);
  vec4 foam       = texture(foamTexture,       texCoord);
  vec4 reflection = texture(reflectionTexture, texCoord);
  vec4 specular   = texture(specularTexture,   texCoord);

  fragColor      = base;
  fragColor.rgb  = mix(fragColor.rgb, refraction.rgb, clamp(refraction.a, 0.0, 1.0));
  fragColor.rgb  = mix(fragColor.rgb, reflection.rgb, clamp(reflection.a, 0.0, 1.0));
  fragColor.rgb  = mix(fragColor.rgb, foam.rgb,       clamp(foam.a,       0.0, 1.0));
  fragColor.rgb += (specular.rgb * clamp(specular.a, 0.0, 1.0));
}
