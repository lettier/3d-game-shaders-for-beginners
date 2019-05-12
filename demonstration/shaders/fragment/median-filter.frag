/*
  (C) 2019 David Lettier
  lettier.com
*/

#version 150

#define MAX_SIZE        4
#define MAX_KERNEL_SIZE ((MAX_SIZE * 2 + 1) * (MAX_SIZE * 2 + 1))
#define MAX_BINS_SIZE   10

uniform sampler2D colorTexture;

uniform vec2 parameters;

out vec4 fragColor;

void main() {
  vec2 texSize  = textureSize(colorTexture, 0).xy;
  vec2 texCoord = gl_FragCoord.xy / texSize;

  int size = int(parameters.x);
  if (size <= 0) { fragColor = texture(colorTexture, texCoord); return; }
  if (size > MAX_SIZE) { size = MAX_SIZE; }
  int kernelSize = int(pow(size * 2 + 1, 2));

  int binsSize = int(parameters.y);
      binsSize = clamp(binsSize, 1, MAX_BINS_SIZE);

  int i        = 0;
  int j        = 0;
  int count    = 0;
  int binIndex = 0;

  vec4  colors[MAX_KERNEL_SIZE];
  float bins[MAX_BINS_SIZE];
  int   binIndexes[colors.length()];

  float total = 0;
  float limit = floor(float(kernelSize) / 2) + 1;

  float value       = 0;
  vec3  valueRatios = vec3(0.3, 0.59, 0.11);

  for (i = -size; i <= size; ++i) {
    for (j = -size; j <= size; ++j) {
      colors[count] =
        texture
          ( colorTexture
          ,   ( gl_FragCoord.xy
              + vec2(i, j)
              )
            / texSize
          );
      count += 1;
    }
  }

  for (i = 0; i < binsSize; ++i) {
    bins[i] = 0;
  }

  for (i = 0; i < kernelSize; ++i) {
    value           = dot(colors[i].rgb, valueRatios);
    binIndex        = int(floor(value * binsSize));
    binIndex        = clamp(binIndex, 0, binsSize - 1);
    bins[binIndex] += 1;
    binIndexes[i]   = binIndex;
  }

  binIndex = 0;

  for (i = 0; i < binsSize; ++i) {
    total += bins[i];
    if (total >= limit) {
      binIndex = i;
      break;
    }
  }

  fragColor = colors[0];

  for (i = 0; i < kernelSize; ++i) {
    if (binIndexes[i] == binIndex) {
      fragColor = colors[i];
      break;
    }
  }
}
