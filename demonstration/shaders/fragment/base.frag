/*
  (C) 2019 David Lettier
  lettier.com
*/

#version 150

#define NUMBER_OF_LIGHTS 4

uniform float osg_FrameTime;

uniform vec2 pi;
uniform vec2 gamma;

uniform mat4 p3d_ProjectionMatrixInverse;
uniform mat4 trans_world_to_view;
uniform mat4 trans_view_to_world;

uniform sampler2D p3d_Texture0;
uniform sampler2D p3d_Texture1;
uniform sampler2D flowTexture;
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

uniform vec2 normalMapsEnabled;
uniform vec2 blinnPhongEnabled;
uniform vec2 celShadingEnabled;
uniform vec2 flowMapsEnabled;
uniform vec2 specularOnly;
uniform vec2 isParticle;
uniform vec2 sunPosition;

in vec4 vertexColor;

in vec4 vertexInShadowSpaces[NUMBER_OF_LIGHTS];

in vec4 vertexPosition;

in vec3 vertexNormal;
in vec3 binormal;
in vec3 tangent;

in vec2 normalCoord;
in vec2 diffuseCoord;

out vec4 fragColor;

void main() {
  vec2 flow   = texture(flowTexture, normalCoord).xy;
       flow   = (flow - 0.5) * 2;
       flow.x = abs(flow.x) <= 0.02 ? 0 : flow.x;
       flow.y = abs(flow.y) <= 0.02 ? 0 : flow.y;

  vec4 normalTex =
    texture
      ( p3d_Texture0
      , vec2
          ( normalCoord.x + flowMapsEnabled.x * flow.x * osg_FrameTime
          , normalCoord.y + flowMapsEnabled.y * flow.y * osg_FrameTime
          )
      );

  vec4 diffuseColor;
  if (isParticle.x == 1) {
    diffuseColor   = texture(p3d_Texture0, diffuseCoord) * vertexColor;
  } else {
    diffuseColor   = texture(p3d_Texture1, diffuseCoord);
  }
  diffuseColor.rgb = pow(diffuseColor.rgb, vec3(gamma.x));
  diffuseColor.rgb = mix(diffuseColor.rgb, p3d_Material.diffuse.rgb, celShadingEnabled.x * 0.0);

  vec3 materialSpecularColor = p3d_Material.specular;

  vec3 normal;

  if (isParticle.x == 1) {
    normal = normalize((trans_world_to_view * vec4(0.0, 0.0, 1.0, 0.0)).xyz);
  } else if (normalMapsEnabled.x == 1) {
    vec3 normalRaw =
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
        * normalRaw
        );
  } else {
    normal =
      normalize(vertexNormal);
  }

  vec4 diffuse  = vec4(0.0, 0.0, 0.0, diffuseColor.a);
  vec4 specular = vec4(0.0, 0.0, 0.0, diffuseColor.a);

  for (int i = 0; i < p3d_LightSource.length(); ++i) {
    vec3 lightDirection =
        p3d_LightSource[i].position.xyz
      - vertexPosition.xyz
      * p3d_LightSource[i].position.w;

    vec3 unitLightDirection = normalize(lightDirection);
    vec3 eyeDirection       = normalize(-vertexPosition.xyz);
    vec3 reflectedDirection = normalize(-reflect(unitLightDirection, normal));
    vec3 halfwayDirection   = normalize(unitLightDirection + eyeDirection);

    float levels = 2.0;

    float diffuseIntensity = max(dot(normal, unitLightDirection), 0.0);
          diffuseIntensity =
              celShadingEnabled.x == 1
            ? smoothstep(0.1, 0.11, diffuseIntensity)
            : diffuseIntensity;

    if (diffuseIntensity > 0) {
      vec4 lightDiffuseColor     = p3d_LightSource[i].diffuse;
           lightDiffuseColor.rgb = pow(lightDiffuseColor.rgb, vec3(gamma.x));

      vec4 diffuseTemp =
        vec4
          ( clamp
              (   diffuseColor.rgb
                * lightDiffuseColor.rgb
                * diffuseIntensity
              , 0
              , 1
              )
          , diffuseColor.a
          );
      diffuseTemp   = clamp(diffuseTemp, vec4(0), diffuseColor);

      float specularIntensity =
          blinnPhongEnabled.x == 1
        ? clamp(dot(normal,       halfwayDirection),   0.0, 1.0)
        : clamp(dot(eyeDirection, reflectedDirection), 0.0, 1.0);

      specularIntensity =
          celShadingEnabled.x == 1
        ? smoothstep(0.98, 0.99, specularIntensity)
        : specularIntensity;

      vec4 lightSpecularColor     = p3d_LightSource[i].specular;
           lightSpecularColor.rgb = pow(lightSpecularColor.rgb, vec3(gamma.x));

      vec4 specularTemp =
        clamp
          (   vec4(materialSpecularColor, 1.0)
            * lightSpecularColor
            * pow
                ( specularIntensity
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
        float spotExponent = p3d_LightSource[i].spotExponent;

        diffuseTemp.rgb *= (spotExponent <= 0.0 ? 1.0 : pow(unitLightDirectionDelta, spotExponent));
        diffuseTemp      = clamp(diffuseTemp, vec4(0), diffuseColor);

        float shadow =
          textureProj
            ( p3d_LightSource[i].shadowMap
            , vertexInShadowSpaces[i]
            );

        diffuseTemp.rgb  *= mix(shadow, 1.0, isParticle.x);
        specularTemp.rgb *= mix(shadow, 1.0, isParticle.x);

        float lightDistance = length(lightDirection);

        float attenuation =
            1
          / ( p3d_LightSource[i].constantAttenuation
            + p3d_LightSource[i].linearAttenuation
            * lightDistance
            + p3d_LightSource[i].quadraticAttenuation
            * (lightDistance * lightDistance)
            );

        diffuseTemp.rgb  *= attenuation;
        specularTemp.rgb *= attenuation;

        diffuse  += diffuseTemp;
        specular += specularTemp;
      }
    }
  }

  if (specularOnly.x == 1) { fragColor = specular; return; }

  vec2  ssaoBlurTexSize  = textureSize(ssaoBlurTexture, 0).xy;
  vec2  ssaoBlurTexCoord = gl_FragCoord.xy / ssaoBlurTexSize;
  float ssao             = texture(ssaoBlurTexture, ssaoBlurTexCoord).r;

  float sunPosition = sin(sunPosition.x * pi.y);

  vec3 groundLight = pow(vec3(0.711, 0.283, 0.350), vec3(gamma.x)) * max(0.4, -1 * sunPosition);
  vec3 skyLight    = pow(vec3(0.822, 0.558, 0.507), vec3(gamma.x)) * max(0.2, -1 * sunPosition);

  vec3 worldNormal = normalize((trans_view_to_world * vec4(normal, 0.0)).xyz);

  vec3 ambientLight =
    mix
      ( groundLight
      , skyLight
      , 0.5 * (1.0 + dot(worldNormal, vec3(0, 0, 1)))
      );

  vec3 ambient =
      (celShadingEnabled.x == 1 ? groundLight : ambientLight)
    * diffuseColor.rgb
    * ssao;

  sunPosition = max(0.05, smoothstep(0.0, 0.15, sunPosition));

  vec3 emission  = p3d_Material.emission.rgb * sunPosition;
       emission *= ceil(p3d_Material.emission.r);

  vec4 outputColor     = vec4(0);
       outputColor.rgb =
           ambient.rgb
        +  diffuse.rgb
        + specular.rgb
        + emission.rgb;
       outputColor.a   = diffuse.a;

  fragColor = outputColor;
}
