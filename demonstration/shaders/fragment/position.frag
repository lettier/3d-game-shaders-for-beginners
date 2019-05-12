/*
  (C) 2019 David Lettier
  lettier.com
*/

#version 150

in vec4 vertexPosition;

out vec4 fragColor;

void main() {
  // To convert Panda3D z-up to OpenGL y-up.
  /*
  fragColor = vertexPosition.xzyw;
  */

  fragColor = vertexPosition;
}
