/*
  (C) 2019 David Lettier
  lettier.com
*/

#version 150

uniform sampler2D colorTexture;

uniform vec2 parameters;

out vec4 fragColor;

void main() {
  vec2 texSize  = textureSize(colorTexture, 0).xy;
  vec2 texCoord = gl_FragCoord.xy / texSize;

  int size  = int(parameters.x);
  if (size <= 0) { fragColor = texture(colorTexture, texCoord); return; }

  float separation = parameters.y;
        separation = max(separation, 1);

  fragColor = vec4(0);

  for (int i = -size; i <= size; ++i) {
    for (int j = -size; j <= size; ++j) {
      fragColor +=
        texture
          ( colorTexture
          ,   ( gl_FragCoord.xy
              + (vec2(i, j) * separation)
              )
            / texSize
          );
    }
  }

  fragColor /= pow(size * 2 + 1, 2);
}
