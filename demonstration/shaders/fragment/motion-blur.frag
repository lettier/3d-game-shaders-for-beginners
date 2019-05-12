/*
  (C) 2020 David Lettier
  lettier.com
*/

#version 150

uniform sampler2D positionTexture;
uniform sampler2D colorTexture;

uniform mat4 previousViewWorldMat;
uniform mat4 worldViewMat;
uniform mat4 lensProjection;

uniform vec2 motionBlurEnabled;
uniform vec2 parameters;

out vec4 fragColor;

void main() {
  int   size       = int(parameters.x);
  float separation =     parameters.y;

  vec2 texSize  = textureSize(colorTexture, 0).xy;
  vec2 texCoord = gl_FragCoord.xy / texSize;

       fragColor = texture(colorTexture,    texCoord);
  vec4 position1 = texture(positionTexture, texCoord);

  if (size <= 0 || separation <= 0.0 || motionBlurEnabled.x != 1 || position1.a <= 0.0) { return; }

  vec4 position0 = worldViewMat * previousViewWorldMat * position1;

  position0      = lensProjection * position0;
  position0.xyz /= position0.w;
  position0.xy   = position0.xy * 0.5 + 0.5;

  position1      = lensProjection * position1;
  position1.xyz /= position1.w;
  position1.xy   = position1.xy * 0.5 + 0.5;

  vec2 direction = position1.xy - position0.xy;

  if (length(direction) <= 0.0) { return; }

  direction.xy *= separation;

  vec2 forward  = texCoord;
  vec2 backward = texCoord;

  float count = 1.0;

  for (int i = 0; i < size; ++i) {
    forward  += direction;
    backward -= direction;

    fragColor +=
      texture
        ( colorTexture
        , forward
        );
    fragColor +=
      texture
        ( colorTexture
        , backward
        );

    count += 2.0;
  }

  fragColor /= count;
}
