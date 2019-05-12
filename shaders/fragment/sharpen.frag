/*
  (C) 2019 David Lettier
  lettier.com
*/

#version 140

uniform sampler2D sharpenTexture;

uniform vec2 enabled;

out vec4 fragColor;

void main() {
  float amount = 0.8;

  vec2 texSize  = textureSize(sharpenTexture, 0).xy;
  vec2 texCoord = gl_FragCoord.xy / texSize;

  float neighbor = amount * -1;
  float center   = amount * 4 + 1;

  vec3 color =
        texture(sharpenTexture, vec2(gl_FragCoord.x + 0, gl_FragCoord.y + 1) / texSize).rgb
      * neighbor

      + texture(sharpenTexture, vec2(gl_FragCoord.x - 1, gl_FragCoord.y + 0) / texSize).rgb
      * neighbor
      + texture(sharpenTexture, vec2(gl_FragCoord.x + 0, gl_FragCoord.y + 0) / texSize).rgb
      * center
      + texture(sharpenTexture, vec2(gl_FragCoord.x + 1, gl_FragCoord.y + 0) / texSize).rgb
      * neighbor

      + texture(sharpenTexture, vec2(gl_FragCoord.x + 0, gl_FragCoord.y - 1) / texSize).rgb
      * neighbor
      ;

  if (enabled.x == 1) {
    fragColor = vec4(color, texture(sharpenTexture, texCoord).a);
  } else {
    fragColor = texture(sharpenTexture, texCoord);
  }
}
