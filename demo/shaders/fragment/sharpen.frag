/*
  (C) 2019 David Lettier
  lettier.com
*/

#version 150

uniform sampler2D colorTexture;

uniform vec2 enabled;

out vec4 fragColor;

void main() {
  float amount = 0.6;

  vec2 texSize  = textureSize(colorTexture, 0).xy;
  vec2 texCoord = gl_FragCoord.xy / texSize;

  float neighbor = amount * -1;
  float center   = amount * 4 + 1;

  vec3 color =
        texture(colorTexture, vec2(gl_FragCoord.x + 0, gl_FragCoord.y + 1) / texSize).rgb
      * neighbor

      + texture(colorTexture, vec2(gl_FragCoord.x - 1, gl_FragCoord.y + 0) / texSize).rgb
      * neighbor
      + texture(colorTexture, vec2(gl_FragCoord.x + 0, gl_FragCoord.y + 0) / texSize).rgb
      * center
      + texture(colorTexture, vec2(gl_FragCoord.x + 1, gl_FragCoord.y + 0) / texSize).rgb
      * neighbor

      + texture(colorTexture, vec2(gl_FragCoord.x + 0, gl_FragCoord.y - 1) / texSize).rgb
      * neighbor
      ;

  if (enabled.x == 1) {
    fragColor = vec4(color, texture(colorTexture, texCoord).a);
  } else {
    fragColor = texture(colorTexture, texCoord);
  }
}
