/*
  (C) 2019 David Lettier
  lettier.com
*/

#version 150

uniform float osg_FrameTime;

uniform sampler2D p3d_Texture1;
uniform sampler2D flowTexture;

uniform vec2 normalMapsEnabled;
uniform vec2 flowMapsEnabled;

in vec3 vertexNormal;
in vec3 binormal;
in vec3 tangent;

in vec2 normalCoord;

out vec4 fragColor;

void main() {
  vec2 flow   = texture(flowTexture,  normalCoord).xy;
       flow   = (flow - 0.5) * 2;
       flow.x = abs(flow.x) <= 0.02 ? 0 : flow.x;
       flow.y = abs(flow.y) <= 0.02 ? 0 : flow.y;

  vec4 normalTex =
    texture
      ( p3d_Texture1
      , vec2
          ( normalCoord.x + flowMapsEnabled.x * flow.x * osg_FrameTime
          , normalCoord.y + flowMapsEnabled.y * flow.y * osg_FrameTime
          )
      );

  vec3 normal;
  if (normalMapsEnabled.x == 1) {
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

  // To convert Panda3D z-up to OpenGL y-up.
  // fragColor = vec4(normal.x, normal.z, -normal.y, 1);

  fragColor = vec4(normal, 1);
}
