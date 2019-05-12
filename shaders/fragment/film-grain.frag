/*
  (C) 2019 David Lettier
  lettier.com
*/

#version 140

uniform float osg_FrameTime;

uniform sampler2D filmGrainTexture;

uniform vec2 enabled;

out vec4 fragColor;

void main() {
  float amount = 0.1;

  vec2 texSize  = textureSize(filmGrainTexture, 0).xy;
  vec2 texCoord = gl_FragCoord.xy / texSize;

  vec4 color = texture(filmGrainTexture, texCoord);

  float toRadians = 3.14 / 180;

  if (enabled.x == 1) {
    float randomIntensity =
      fract
        ( 10000
        * sin
            (
              ( gl_FragCoord.x
              + gl_FragCoord.y
              * osg_FrameTime
              )
            * toRadians
            )
        );

    amount *= randomIntensity;

    color.rgb += amount;
  }

  fragColor = color;
}
