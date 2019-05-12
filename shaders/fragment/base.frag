/*
  (C) 2019 David Lettier
  lettier.com
*/

#version 140

#define NUMBER_OF_LIGHTS 4

uniform mat4 p3d_ProjectionMatrixInverse;

uniform sampler2D p3d_Texture0;
uniform sampler2D p3d_Texture1;

uniform sampler2D ssaoBlurTexture;

uniform struct
  { vec4 ambient
  ; vec4 diffuse
  ; vec4 emission
  ; vec3 specular
  ; float shininess
  ;
  } p3d_Material;

uniform struct
  { vec4 ambient
  ;
  } p3d_LightModel;

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

uniform struct p3d_FogParameters
  { vec4 color
  ; float start
  ; float end
  ;
  } p3d_Fog;

uniform vec2 normalMapsEnabled;
uniform vec2 fogEnabled;

in vec4 vertexPosition;

in vec3 vertexNormal;
in vec3 binormal;
in vec3 tangent;

in vec4 vertexInShadowSpaces[NUMBER_OF_LIGHTS];

in vec2 normalCoord;
in vec2 diffuseCoord;

out vec4 fragColor;

void main() {
  vec4 normalTex   = texture(p3d_Texture0, normalCoord);
  vec4 diffuseTex  = texture(p3d_Texture1, diffuseCoord);

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

  vec4 diffuseSpecular = vec4(0.0, 0.0, 0.0, 0.0);

  for (int i = 0; i < p3d_LightSource.length(); ++i) {
    vec3 lightDirection =
        p3d_LightSource[i].position.xyz
      - vertexPosition.xyz
      * p3d_LightSource[i].position.w;

    vec3 unitLightDirection = normalize(lightDirection);
    vec3 eyeDirection       = normalize(-vertexPosition.xyz);
    vec3 reflectedDirection = normalize(-reflect(unitLightDirection, normal));

    float diffuseIntensity  = max(dot(normal, unitLightDirection), 0.0);

    if (diffuseIntensity > 0) {
      vec4 diffuse =
        vec4
          ( clamp
              (   diffuseTex.rgb
                * p3d_LightSource[i].diffuse.rgb
                * diffuseIntensity
              , 0
              , 1
              )
          , 1
          );

      diffuse.r = clamp(diffuse.r, 0, diffuseTex.r);
      diffuse.g = clamp(diffuse.g, 0, diffuseTex.g);
      diffuse.b = clamp(diffuse.b, 0, diffuseTex.b);

      vec4 specular =
        clamp
          (   vec4(p3d_Material.specular, 1)
            * p3d_LightSource[i].specular
            * pow
                ( max(dot(reflectedDirection, eyeDirection), 0)
                , p3d_Material.shininess
                )
          , 0
          , 1
          );

      float unitLightDirectionDelta =
        dot
          ( normalize(p3d_LightSource[i].spotDirection)
          , -unitLightDirection
          );

      if (unitLightDirectionDelta >= p3d_LightSource[i].spotCosCutoff) {
        diffuse *= pow(unitLightDirectionDelta, p3d_LightSource[i].spotExponent);

        diffuse.r = clamp(diffuse.r, 0, diffuseTex.r);
        diffuse.g = clamp(diffuse.g, 0, diffuseTex.g);
        diffuse.b = clamp(diffuse.b, 0, diffuseTex.b);

        float shadow =
          textureProj
            ( p3d_LightSource[i].shadowMap
            , vertexInShadowSpaces[i]
            );

        diffuse.rgb  *= shadow;
        specular.rgb *= shadow;

        float lightDistance = length(lightDirection);

        float attenuation =
            1
          / ( p3d_LightSource[i].constantAttenuation
            + p3d_LightSource[i].linearAttenuation
            * lightDistance
            + p3d_LightSource[i].quadraticAttenuation
            * (lightDistance * lightDistance)
            );

        diffuse.rgb  *= attenuation;
        specular.rgb *= attenuation;

        diffuseSpecular += (diffuse + specular);
      }
    }
  }

  vec2 ssaoBlurTexSize  = textureSize(ssaoBlurTexture, 0).xy;
  vec2 ssaoBlurTexCoord = gl_FragCoord.xy / ssaoBlurTexSize;
  float ssao            = texture(ssaoBlurTexture, ssaoBlurTexCoord).r;

  vec4 ambient = p3d_Material.ambient * p3d_LightModel.ambient * diffuseTex * ssao;

  vec4 outputColor = ambient + diffuseSpecular + p3d_Material.emission;

  if (fogEnabled.x == 1) {
    float fogIntensity =
      clamp
        (   ( p3d_Fog.end - vertexPosition.y)
          / ( p3d_Fog.end - p3d_Fog.start)
        , 0
        , 1
        );
    fogIntensity = 1 - fogIntensity;

    fragColor =
      mix
        ( outputColor
        , p3d_Fog.color
        , fogIntensity
        );
  } else {
    fragColor = outputColor;
  }
}
