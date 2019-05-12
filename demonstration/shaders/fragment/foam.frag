/*
  (C) 2019 David Lettier
  lettier.com
*/

#version 150

uniform vec2 pi;
uniform vec2 gamma;

uniform mat4 viewWorldMat;

uniform sampler2D maskTexture;
uniform sampler2D positionFromTexture;
uniform sampler2D positionToTexture;

uniform vec2 foamDepth;
uniform vec2 sunPosition;

out vec4 fragColor;

void main() {
  vec4 foamColor = vec4(0.8, 0.85, 0.92, 0.8);

  vec2 texSize  = textureSize(positionFromTexture, 0).xy;
  vec2 texCoord = gl_FragCoord.xy / texSize;

  vec4 mask = texture(maskTexture, texCoord);

  if (mask.r <= 0.0 || foamDepth.x <= 0.0) { fragColor = vec4(0.0); return; }

  foamColor.rgb  = pow(foamColor.rgb, vec3(gamma.x));
  foamColor.rgb *= max(0.4, -1 * sin(sunPosition.x * pi.y));

  vec4 positionFrom = texture(positionFromTexture, texCoord);
  vec4 positionTo   = texture(positionToTexture,   texCoord);

  positionFrom = viewWorldMat * positionFrom;
  positionTo   = viewWorldMat * positionTo;

  float depth   = length(positionTo.xyz - positionFrom.xyz);
  float amount  = clamp(depth / foamDepth.x, 0.0, 1.0);
        amount  = 1.0 - amount;
        amount *= mask.r;
        // Ease in and out.
        amount  =   (amount * amount)
                  / (2.0 * (amount * amount - amount) + 1.0);

  fragColor = vec4(foamColor.rgb, amount * foamColor.a);
}
