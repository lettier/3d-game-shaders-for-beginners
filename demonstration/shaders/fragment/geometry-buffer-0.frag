/*
  (C) 2020 David Lettier
  lettier.com
*/

#version 150

uniform sampler2D p3d_Texture1;

uniform vec2 normalMapsEnabled;

in vec4 vertexPosition;

in vec3 vertexNormal;
in vec3 binormal;
in vec3 tangent;

in vec2 normalCoord;

out vec4 positionOut;
out vec4 normalOut;

void main() {
  vec4 normalTex =
    texture
      ( p3d_Texture1
      , normalCoord
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

  positionOut = vertexPosition;
  normalOut   = vec4(normal, 1);
}
