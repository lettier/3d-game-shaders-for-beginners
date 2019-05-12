/*
  (C) 2019 David Lettier
  lettier.com
*/

#version 140

#define NUM_SAMPLES 64
#define NUM_NOISE 16

uniform mat4 lensProjection;

uniform vec3 samples[NUM_SAMPLES];
uniform vec3 noise[NUM_NOISE];

uniform sampler2D positionTexture;
uniform sampler2D normalTexture;

uniform vec2 enabled;

out vec4 fragColor;

void main() {
  float radius    = 1;
  float bias      = 0.01;
  float magnitude = 1.5;
  float contrast  = 1.5;

  if (enabled.x != 1) { fragColor = vec4(1); return; }

  vec2 texSize  = textureSize(positionTexture, 0).xy;
  vec2 texCoord = gl_FragCoord.xy / texSize;

  vec4 position = texture(positionTexture, texCoord);
  vec3 normal   = texture(normalTexture, texCoord).xyz;

  int  noiseS = int(sqrt(NUM_NOISE));
  int  noiseX = int(gl_FragCoord.x - 0.5) % noiseS;
  int  noiseY = int(gl_FragCoord.y - 0.5) % noiseS;
  vec3 random = noise[noiseX + (noiseY * noiseS)];

  vec3 tangent  = normalize(random - normal * dot(random, normal));
  vec3 binormal = cross(normal, tangent);
  mat3 tbn      = mat3(tangent, binormal, normal);

  float occlusion = NUM_SAMPLES;

  for (int i = 0; i < NUM_SAMPLES; ++i) {
    vec3 sample = tbn * samples[i];
         sample = position.xyz + sample * radius;

    vec4 offset      = vec4(sample, 1);
         offset      = lensProjection * offset;
         offset.xyz /= offset.w;
         offset.xy   = offset.xy * 0.5 + 0.5;

    // Config.prc
    // gl-coordinate-system default
    // textures-auto-power-2 1
    // textures-power-2 down

    vec4 offsetPosition = texture(positionTexture, offset.xy);

    float occluded = 0;
    if (sample.y + bias <= offsetPosition.y) { occluded = 0; } else { occluded = 1; }

    float intensity =
      smoothstep
        ( 0
        , 1
        ,   radius
          / abs(position.y - offsetPosition.y)
        );
    occluded *= intensity;

    occlusion -= occluded;
  }

  occlusion /= NUM_SAMPLES;
  occlusion = pow(occlusion, magnitude);
  occlusion = contrast * (occlusion - 0.5) + 0.5;

  fragColor = vec4(vec3(occlusion), position.a);
}
