/*
  (C) 2020 David Lettier
  lettier.com
*/

#version 150

uniform sampler2D colorTexture;

uniform vec2 parameters;

out vec4 fragColor;

void main() {
  int   size         = int(parameters.x);
  float separation   =     parameters.y;
  float minThreshold = 0.2;
  float maxThreshold = 0.5;

  vec2 texSize   = textureSize(colorTexture, 0).xy;
  vec2 fragCoord = gl_FragCoord.xy;

  fragColor = texture(colorTexture, fragCoord / texSize);

  if (size <= 0) { return; }

  float  mx = 0.0;
  vec4  cmx = fragColor;

  for (int i = -size; i <= size; ++i) {
    for (int j = -size; j <= size; ++j) {
      // For a rectangular shape.
      //if (false);

      // For a diamond shape;
      //if (!(abs(i) <= size - abs(j))) { continue; }

      // For a circular shape.
      if (!(distance(vec2(i, j), vec2(0, 0)) <= size)) { continue; }

      vec4 c =
        texture
          ( colorTexture
          ,   ( gl_FragCoord.xy
              + (vec2(i, j) * separation)
              )
            / texSize
          );

      float mxt = dot(c.rgb, vec3(0.3, 0.59, 0.11));

      if (mxt > mx) {
         mx = mxt;
        cmx = c;
      }
    }
  }

  fragColor.rgb =
    mix
      ( fragColor.rgb
      , cmx.rgb
      , smoothstep(minThreshold, maxThreshold, mx)
      );
}
