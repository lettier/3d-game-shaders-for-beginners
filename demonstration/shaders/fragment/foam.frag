/*
  (C) 2019 David Lettier
  lettier.com
*/

#version 150

uniform vec2 pi;
uniform vec2 gamma;

uniform sampler2D maskTexture;
uniform sampler2D positionFromTexture;
uniform sampler2D positionToTexture;

uniform vec2 foamDepth;
uniform vec2 sunPosition;

out vec4 fragColor;

void main() {
  vec4 foamColor = vec4(0.8, 0.85, 0.92, 1);

  foamColor.rgb  = pow(foamColor.rgb, vec3(gamma.x));
  foamColor.rgb *= max(0.4, -1 * sin(sunPosition.x * pi.y));

  vec2 texSize  = textureSize(positionFromTexture, 0).xy;
  vec2 texCoord = gl_FragCoord.xy / texSize;

  vec4 mask         = texture(maskTexture,         texCoord);
  vec4 positionFrom = texture(positionFromTexture, texCoord);
  vec4 positionTo   = texture(positionToTexture,   texCoord);

  if (mask.r <= 0 || foamDepth.x <= 0) { fragColor = vec4(0); return; }

  float depth   = (positionTo.xyz - positionFrom.xyz).y;
  float amount  = clamp(depth / foamDepth.x, 0, 1);
        amount  = 1 - amount;
        amount *= mask.r;
        // Ease in and out.
        amount  = amount * amount / (2 * (amount * amount - amount) + 1);

  fragColor = mix(vec4(0), foamColor, amount);
}
