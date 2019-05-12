/*
  (C) 2019 David Lettier
  lettier.com
*/

#version 140

#define NUMBER_OF_LIGHTS 4

uniform mat4 p3d_ModelMatrix;
uniform mat4 p3d_ViewMatrix;
uniform mat4 p3d_ModelViewMatrix;
uniform mat4 p3d_ModelViewProjectionMatrix;
uniform mat3 p3d_NormalMatrix;

uniform struct p3d_LightSourceParameters
  { vec4 color

  ; vec4 ambient
  ; vec4 diffuse
  ; vec4 specular

  ; vec4 position

  ; vec3  spotDirection
  ; float spotExponent
  ; float spotCutoff
  ; float spotCosCutoff

  ; float constantAttenuation
  ; float linearAttenuation
  ; float quadraticAttenuation

  ; vec3 attenuation

  ; sampler2DShadow shadowMap

  ; mat4 shadowViewMatrix
  ;
  } p3d_LightSource[NUMBER_OF_LIGHTS];

in vec4 p3d_Vertex;
in vec3 p3d_Normal;

in vec2 p3d_MultiTexCoord0;
in vec2 p3d_MultiTexCoord1;

in vec3 p3d_Binormal;
in vec3 p3d_Tangent;

out vec4 vertexPosition;

out vec3 vertexNormal;
out vec3 binormal;
out vec3 tangent;

out vec2 normalCoord;
out vec2 diffuseCoord;

out vec4 vertexInShadowSpaces[NUMBER_OF_LIGHTS];

void main() {
  vertexPosition = p3d_ModelViewMatrix * p3d_Vertex;

  vertexNormal = normalize(p3d_NormalMatrix * p3d_Normal);
  binormal     = normalize(p3d_NormalMatrix * p3d_Binormal);
  tangent      = normalize(p3d_NormalMatrix * p3d_Tangent);

  normalCoord   = p3d_MultiTexCoord0;
  diffuseCoord  = p3d_MultiTexCoord1;

  for (int i = 0; i < p3d_LightSource.length(); ++i) {
    vertexInShadowSpaces[i] = p3d_LightSource[i].shadowViewMatrix * vertexPosition;
  }

  gl_Position = p3d_ModelViewProjectionMatrix * p3d_Vertex;
}
