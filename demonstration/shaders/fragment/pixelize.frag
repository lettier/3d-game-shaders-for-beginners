/*
  (C) 2019 David Lettier
  lettier.com
*/

#version 150

uniform sampler2D colorTexture;
uniform sampler2D positionTexture;

uniform vec2 parameters;
uniform vec2 enabled;

out vec4 fragColor;

void main() {
  // Must be odd.
  int pixelSize = int(parameters.x);

  vec2 texSize  = textureSize(colorTexture, 0).xy;
  vec2 texCoord = gl_FragCoord.xy / texSize;

  // Avoid the background.
  vec4 position = texture(positionTexture, texCoord);

  if (enabled.x != 1 || position.a <= 0.0) {
    fragColor = texture(colorTexture, texCoord);

    return;
  }

  float x = int(gl_FragCoord.x) % pixelSize;
  float y = int(gl_FragCoord.y) % pixelSize;

        x = floor(pixelSize / 2.0) - x;
        y = floor(pixelSize / 2.0) - y;

        x = gl_FragCoord.x + x;
        y = gl_FragCoord.y + y;

  vec2 uv = vec2(x, y) / texSize;

  fragColor = texture(colorTexture, uv);
}
