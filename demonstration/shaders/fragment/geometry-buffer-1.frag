/*
  (C) 2020 David Lettier
  lettier.com
*/

#version 150

#define MAX_SHININESS 127.75

uniform float osg_FrameTime;

uniform struct
  { vec3 specular
  ; float shininess
  ;
  } p3d_Material;

uniform mat4 p3d_ProjectionMatrix;

uniform sampler2D p3d_Texture1;
uniform sampler2D p3d_Texture3;
uniform sampler2D p3d_Texture4;
uniform sampler2D flowTexture;
uniform sampler2D foamPatternTexture;

uniform vec2 normalMapsEnabled;
uniform vec2 flowMapsEnabled;

in vec4 vertexPosition;
in vec4 vertexColor;

in vec3 vertexNormal;
in vec3 binormal;
in vec3 tangent;

in vec2 diffuseCoord;
in vec2 normalCoord;

out vec4 positionOut;
out vec4 normalOut;
out vec4 reflectionMaskOut;
out vec4 refractionMaskOut;
out vec4 foamMaskOut;

void main() {
  vec2 flow   = texture(flowTexture,  normalCoord).xy;
       flow   = (flow - 0.5) * 2.0;
       flow.x = abs(flow.x) <= 0.02 ? 0.0 : flow.x;
       flow.y = abs(flow.y) <= 0.02 ? 0.0 : flow.y;

  vec3 normal;

  if (normalMapsEnabled.x == 1) {
    vec4 normalTex =
      texture
        ( p3d_Texture1
        , vec2
            ( normalCoord.x + flowMapsEnabled.x * flow.x * osg_FrameTime
            , normalCoord.y + flowMapsEnabled.y * flow.y * osg_FrameTime
            )
        );

    normal =
      normalize
        ( normalTex.rgb
        * 2.0
        - 1.0
        );
    normal =
      normalize
        ( mat3
            ( tangent
            , binormal
            , vertexNormal
            )
        * normal
        );
  } else {
    normal = normalize(vertexNormal);
  }

  vec4 reflectionMask = texture(p3d_Texture3, diffuseCoord);
  vec4 refractionMask = texture(p3d_Texture4, diffuseCoord);

  vec2 foamPatternTextureSize = textureSize(foamPatternTexture, 0).xy;

  vec4 foamUvOffset           = p3d_ProjectionMatrix * vec4(normalize(normal), 1.0);
       foamUvOffset.xyz      /= foamUvOffset.w;
       foamUvOffset.xy        = foamUvOffset.xy * 0.5 + 0.5;
       foamUvOffset.xy       /= foamPatternTextureSize;
       foamUvOffset.xy       *= 0.3;

  vec2 foamUv                 = diffuseCoord.xy + foamUvOffset.xy;
       foamUv                 = foamUv + flowMapsEnabled * flow * 0.5 * osg_FrameTime;
       foamUv                *= 0.5;

  vec4 foamPattern            = texture(foamPatternTexture, foamUv);

  positionOut         = vertexPosition;
  normalOut           = vec4(normal, 1.0);
  reflectionMaskOut   = reflectionMask;
  refractionMaskOut   = refractionMask;
  foamMaskOut         = foamPattern;
}
