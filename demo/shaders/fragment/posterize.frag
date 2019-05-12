/*
  (C) 2019 David Lettier
  lettier.com
*/

#version 150

uniform sampler2D colorTexture;
uniform sampler2D positionTexture;

uniform vec2 enabled;

out vec4 fragColor;

void main() {
  float levels = 17;

  vec2 texSize  = textureSize(colorTexture, 0).xy;
  vec2 texCoord = gl_FragCoord.xy / texSize;

  // Do not posterize the background.
  vec4 position = texture(positionTexture, texCoord);
  if (position.a <= 0) { fragColor = vec4(0);  return; }

  fragColor = texture(colorTexture, texCoord);
  if (enabled.x  != 1) { return; }

  float unquantized = dot(fragColor.rgb, vec3(0.3, 0.59, 0.11));
  float quantized   = floor(unquantized * levels) / levels;
  float adjustment  = quantized / max(unquantized, 0.0001);

  fragColor.rgb *= adjustment;
}
