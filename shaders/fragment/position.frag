/*
  (C) 2019 David Lettier
  lettier.com
*/

#version 140

in vec4 vertexPosition;

out vec4 fragColor;

void main() {
  // To convert Panda3D z-up to OpenGL y-up.
  /*
  fragColor =
    vec4
      ( vertexPosition.x
      , vertexPosition.z
      , -vertexPosition.y
      , vertexPosition.w
      );
  */

  fragColor = vertexPosition;
}
