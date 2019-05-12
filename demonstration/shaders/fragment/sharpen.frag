/*
  (C) 2019 David Lettier
  lettier.com
*/

#version 150

uniform sampler2D colorTexture;

uniform vec2 enabled;

out vec4 fragColor;

void main() {
  float amount = 0.3;

  vec2 texSize   = textureSize(colorTexture, 0).xy;
  vec2 fragCoord = gl_FragCoord.xy;
  vec2 texCoord  = fragCoord / texSize;

  if (enabled.x != 1) { fragColor = texture(colorTexture, texCoord); return; }

  float neighbor = amount * -1.0;
  float center   = amount *  4.0 + 1.0;

  vec3 color =
        texture(colorTexture, (fragCoord + vec2( 0,  1)) / texSize).rgb
      * neighbor

      + texture(colorTexture, (fragCoord + vec2(-1,  0)) / texSize).rgb
      * neighbor
      + texture(colorTexture, (fragCoord + vec2( 0,  0)) / texSize).rgb
      * center
      + texture(colorTexture, (fragCoord + vec2( 1,  0)) / texSize).rgb
      * neighbor

      + texture(colorTexture, (fragCoord + vec2( 0, -1)) / texSize).rgb
      * neighbor
      ;

  fragColor = vec4(color, texture(colorTexture, texCoord).a);
}
