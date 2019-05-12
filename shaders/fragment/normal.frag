/*
  (C) 2019 David Lettier
  lettier.com
*/

#version 140

uniform sampler2D p3d_Texture0;

uniform vec2 normalMapsEnabled;

in vec3 vertexNormal;
in vec3 binormal;
in vec3 tangent;

in vec2 normalCoord;

out vec4 fragColor;

void main() {
  vec4 normalTex = texture(p3d_Texture0, normalCoord);

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
