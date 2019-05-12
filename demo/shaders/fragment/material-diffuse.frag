/*
  (C) 2019 David Lettier
  lettier.com
*/

#version 150

uniform struct
  { vec4 diffuse
  ;
  } p3d_Material;

out vec4 fragColor;

void main() {
  fragColor = p3d_Material.diffuse;
}
