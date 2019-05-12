/*
  (C) 2019 David Lettier
  lettier.com
*/

#version 150

uniform vec2 pi;

uniform float osg_FrameTime;

uniform sampler2D colorTexture;

uniform vec2 enabled;

out vec4 fragColor;

void main() {
  float amount = 0.01;

  vec2 texSize  = textureSize(colorTexture, 0).xy;
  vec2 texCoord = gl_FragCoord.xy / texSize;

  vec4 color = texture(colorTexture, texCoord);

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
            * pi.y
            )
        );

    amount *= randomIntensity;

    color.rgb += amount;
  }

  fragColor = color;
}
