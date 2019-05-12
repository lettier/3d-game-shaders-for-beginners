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

uniform sampler2D p3d_Texture0;
uniform sampler2D flowTexture;
uniform sampler2D foamPatternTexture;

uniform vec2 normalMapsEnabled;
uniform vec2 flowMapsEnabled;
uniform vec2 isParticle;

in vec4 vertexPosition;
in vec4 vertexColor;

in vec3 vertexNormal;
in vec3 binormal;
in vec3 tangent;

in vec2 diffuseCoord;
in vec2 normalCoord;

out vec4 positionOut;
out vec4 normalOut;
out vec4 materialSpecularOut;
out vec4 foamMaskOut;

void main() {
  vec2 flow   = texture(flowTexture,  normalCoord).xy;
       flow   = (flow - 0.5) * 2;
       flow.x = abs(flow.x) <= 0.02 ? 0 : flow.x;
       flow.y = abs(flow.y) <= 0.02 ? 0 : flow.y;

  vec3 normal;

  if (normalMapsEnabled.x == 1) {

    vec4 normalTex =
      texture
        ( p3d_Texture0
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

  vec4 materialSpecular =
    vec4
      ( p3d_Material.specular
      , clamp(p3d_Material.shininess / MAX_SHININESS, 0, 1)
      );

  vec4 foamPattern =
    texture
      ( foamPatternTexture
      , vec2
          ( diffuseCoord.x + flowMapsEnabled.x * flow.x * osg_FrameTime
          , diffuseCoord.y + flowMapsEnabled.y * flow.y * osg_FrameTime
          )
      );

  positionOut         = vertexPosition;
  normalOut           = vec4(normal, 1);
  materialSpecularOut = materialSpecular;
  foamMaskOut         = foamPattern;
}
