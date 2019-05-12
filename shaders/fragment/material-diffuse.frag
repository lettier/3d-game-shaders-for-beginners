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
  vec3 diffuseColor = p3d_Material.diffuse.rgb;
  fragColor = vec4(diffuseColor, 1);
}
