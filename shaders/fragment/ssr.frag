/*
  (C) 2019 David Lettier
  lettier.com
*/

#version 140

uniform mat4 lensProjection;

uniform sampler2D positionTexture;
uniform sampler2D normalTexture;

uniform vec2 enabled;

out vec4 fragColor;

void main() {
  float steps       = 100;
  float maxDistance = 15;
  float thickness   = 1;

  vec4 uv = vec4(0);

  if (enabled.x != 1) { fragColor = uv; return; }

  vec2 texSize  = textureSize(positionTexture, 0).xy;
  vec2 texCoord = gl_FragCoord.xy / texSize;

  vec4 position   = texture(positionTexture, texCoord);
  vec3 uPosition  = normalize(position.xyz);
  vec3 normal     = normalize(texture(normalTexture, texCoord).xyz);
  vec3 reflection = normalize(reflect(uPosition, normal));

  float stepSize = maxDistance / steps;
  float count    = 1;

  for (float i = 0; i < steps; ++i) {
    float distance = count * stepSize;
    if (distance > maxDistance) { break; }

    vec3 sample = position.xyz + (reflection * distance);

    vec4 offset      = vec4(sample, 1);
         offset      = lensProjection * offset;
         offset.xyz /= offset.w;
         offset.xy   = offset.xy * 0.5 + 0.5;

    if
      (  offset.x <  0
      || offset.y <  0
      || offset.z < -1
      || offset.x >  1
      || offset.y >  1
      || offset.z >  1
      ) { break; }

    // Config.prc
    // gl-coordinate-system default
    // textures-auto-power-2 1
    // textures-power-2 down

    vec4 offsetPosition = texture(positionTexture, offset.xy);

    if
      (  sample.y >= offsetPosition.y
      && sample.y <= offsetPosition.y + thickness
      ) {
      float visibility = 1 - max(dot(reflection, -uPosition), 0);

      uv = vec4(offset.x, offset.y, visibility, 1);

      count                 -= 1;
      maxDistance            = distance;
      float previousDistance = count * stepSize;
      float stepsLeft        = max(steps - i, 1);
      stepSize               = (maxDistance - previousDistance) / stepsLeft;
    } else {
      count      += 1;
    }
  }

  fragColor = uv;
}
