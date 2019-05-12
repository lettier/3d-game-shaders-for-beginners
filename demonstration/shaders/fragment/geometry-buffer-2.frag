/*
  (C) 2020 David Lettier
  lettier.com
*/

#version 150

uniform sampler2D p3d_Texture0;

uniform vec2 isSmoke;

in vec4 vertexPosition;
in vec4 vertexColor;

in vec2 diffuseCoord;

out vec4 positionOut;
out vec4 smokeMaskOut;

void main() {
  positionOut  = vertexPosition;
  smokeMaskOut = vec4(0);

  if (isSmoke.x == 1) {
    vec4 diffuseColor = texture(p3d_Texture0, diffuseCoord) * vertexColor;

    positionOut = diffuseColor.a > 0 ? vertexPosition : vec4(0);

    smokeMaskOut     = diffuseColor * vertexColor;
    smokeMaskOut.rgb = vec3(dot(smokeMaskOut.rgb, vec3(1.0 / 3.0)));
  }
}
