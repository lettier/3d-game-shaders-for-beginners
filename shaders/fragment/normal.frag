/*
  (C) 2019 David Lettier
  lettier.com
*/

#version 140

in vec3 vertexNormal;

out vec4 fragColor;

void main() {
  vec3 normal = normalize(vertexNormal);

  // To convert Panda3D z-up to OpenGL y-up.
  // fragColor = vec4(normal.x, normal.z, -normal.y, 1);

  fragColor = vec4(normal, 1);
}
