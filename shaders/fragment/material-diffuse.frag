/*
  (C) 2019 David Lettier
  lettier.com
*/

#version 140

uniform struct
  { vec4 diffuse
  ;
  } p3d_Material;

out vec4 fragColor;

void main() {
  fragColor = vec4(p3d_Material.diffuse.rgb, 1);
}
