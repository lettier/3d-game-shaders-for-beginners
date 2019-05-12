/*
  (C) 2019 David Lettier
  lettier.com
*/

#version 150

uniform vec2 gamma;

uniform sampler2D colorTexture;
uniform sampler2D positionTexture;

uniform vec2 enabled;

out vec4 fragColor;

void main() {
  float levels = 6.0;

  vec2 texSize  = textureSize(colorTexture, 0).xy;
  vec2 texCoord = gl_FragCoord.xy / texSize;

  // Avoid the background.
  vec4 position = texture(positionTexture, texCoord);
  if (position.a <= 0) { fragColor = vec4(0); return; }

  fragColor = texture(colorTexture, texCoord);

  if (enabled.x != 1) { return; }

  fragColor.rgb = pow(fragColor.rgb, vec3(gamma.y));

  float greyscale = max(fragColor.r, max(fragColor.g, fragColor.b));

  float lower     = floor(greyscale * levels) / levels;
  float lowerDiff = abs(greyscale - lower);

  float upper     = ceil(greyscale * levels) / levels;
  float upperDiff = abs(upper - greyscale);

  float level      = lowerDiff <= upperDiff ? lower : upper;
  float adjustment = level / greyscale;

  fragColor.rgb = fragColor.rgb * adjustment;

  fragColor.rgb = pow(fragColor.rgb, vec3(gamma.x));
}
