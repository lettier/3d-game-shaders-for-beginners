/*
  (C) 2019 David Lettier
  lettier.com
*/

#version 150

#define MAX_SHININESS 127.75

uniform struct
  { vec3 specular
  ; float shininess
  ;
  } p3d_Material;

out vec4 fragColor;

void main() {
  fragColor =
    vec4
      ( p3d_Material.specular
      , clamp(p3d_Material.shininess / MAX_SHININESS, 0.0, 1.0)
      );
}
