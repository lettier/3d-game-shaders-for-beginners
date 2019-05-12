/*
  (C) 2019 David Lettier
  lettier.com
*/

#version 140

uniform sampler2D positionTexture;
uniform sampler2D focusTexture;
uniform sampler2D outOfFocusTexture;

uniform vec2 focalLength;

uniform vec2 enabled;

out vec4 fragColor;

void main() {
  float focalLengthSharpness = 100;
  float blurRate             = 6;

  vec2 texSize  = textureSize(focusTexture, 0).xy;
  vec2 texCoord = gl_FragCoord.xy / texSize;

  vec4 focusColor      = texture(focusTexture, texCoord);
  vec4 outOfFocusColor = texture(outOfFocusTexture, texCoord);

  vec4 position = texture(positionTexture, texCoord);

  float blur =
    clamp
      (   pow
            ( blurRate
            , abs(position.y - focalLength.x)
            )
        / focalLengthSharpness
      , 0
      , 1
      );

  if (enabled.x == 1) {
    fragColor = mix(focusColor, outOfFocusColor, blur);
  } else {
    fragColor = focusColor;
  }
}
