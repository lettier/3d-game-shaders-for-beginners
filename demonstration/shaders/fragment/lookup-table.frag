/*
  (C) 2020 David Lettier
  lettier.com
*/

#version 150

uniform vec2 pi;
uniform vec2 gamma;

uniform sampler2D colorTexture;
uniform sampler2D lookupTableTexture0;
uniform sampler2D lookupTableTexture1;

uniform vec2 sunPosition;
uniform vec2 enabled;

out vec4 fragColor;

void main() {
  vec2 texSize = textureSize(colorTexture, 0).xy;

  vec4 color = texture(colorTexture, gl_FragCoord.xy / texSize);

  if (enabled.x != 1) { fragColor = color; return; }

  color.rgb = pow(color.rgb, vec3(gamma.y));

  float u  =  floor(color.b * 15.0) / 15.0 * 240.0;
        u  = (floor(color.r * 15.0) / 15.0 *  15.0) + u;
        u /= 255.0;
  float v  = 1.0 - (floor(color.g * 15.0) / 15.0);

  vec3 left0 = texture(lookupTableTexture0, vec2(u, v)).rgb;
  vec3 left1 = texture(lookupTableTexture1, vec2(u, v)).rgb;

  u  =  ceil(color.b * 15.0) / 15.0 * 240.0;
  u  = (ceil(color.r * 15.0) / 15.0 *  15.0) + u;
  u /= 255.0;
  v  = 1.0 - (ceil(color.g * 15.0) / 15.0);

  vec3 right0 = texture(lookupTableTexture0, vec2(u, v)).rgb;
  vec3 right1 = texture(lookupTableTexture1, vec2(u, v)).rgb;

  float sunPosition = sin(sunPosition.x * pi.y);
        sunPosition = 0.5 * (sunPosition + 1);

  vec3 left  = mix(left0,  left1,  sunPosition);
  vec3 right = mix(right0, right1, sunPosition);

  color.r = mix(left.r, right.r, fract(color.r * 15.0));
  color.g = mix(left.g, right.g, fract(color.g * 15.0));
  color.b = mix(left.b, right.b, fract(color.b * 15.0));

  color.rgb = pow(color.rgb, vec3(gamma.x));

  fragColor = color;
}
