/*
  (C) 2019 David Lettier
  lettier.com
*/

#version 150

#define NUMBER_OF_LIGHTS    4
#define MAX_SHININESS     127.75
#define MAX_FRESNEL_POWER   5.0

uniform float osg_FrameTime;

uniform vec2 pi;
uniform vec2 gamma;

uniform mat4 trans_world_to_view;
uniform mat4 trans_view_to_world;

uniform sampler2D p3d_Texture0;
uniform sampler2D p3d_Texture1;
uniform sampler2D p3d_Texture2;
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
uniform vec2 fresnelEnabled;
uniform vec2 rimLightEnabled;
uniform vec2 blinnPhongEnabled;
uniform vec2 celShadingEnabled;
uniform vec2 flowMapsEnabled;
uniform vec2 specularOnly;
uniform vec2 isParticle;
uniform vec2 isWater;
uniform vec2 sunPosition;

in vec4 vertexColor;

in vec4 vertexInShadowSpaces[NUMBER_OF_LIGHTS];

in vec4 vertexPosition;

in vec3 vertexNormal;
in vec3 binormal;
in vec3 tangent;

in vec2 diffuseCoord;
in vec2 normalCoord;

out vec4 out0;
out vec4 out1;

void main() {
  vec3  shadowColor   = pow(vec3(0.149, 0.220, 0.227), vec3(gamma.x));
  int   shadowSamples = 2;

  vec4 diffuseColor;
  if (isParticle.x == 1) {
    diffuseColor   = texture(p3d_Texture0, diffuseCoord) * vertexColor;
  } else {
    diffuseColor   = texture(p3d_Texture0, diffuseCoord);
  }
  diffuseColor.rgb = pow(diffuseColor.rgb, vec3(gamma.x));

  vec3 materialSpecularColor = p3d_Material.specular;

  vec2 flow   = texture(flowTexture, normalCoord).xy;
       flow   = (flow - 0.5) * 2.0;
       flow.x = abs(flow.x) <= 0.02 ? 0.0 : flow.x;
       flow.y = abs(flow.y) <= 0.02 ? 0.0 : flow.y;

  vec4 normalTex =
    texture
      ( p3d_Texture1
      , vec2
          ( normalCoord.x + flowMapsEnabled.x * flow.x * osg_FrameTime
          , normalCoord.y + flowMapsEnabled.y * flow.y * osg_FrameTime
          )
      );

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

  vec4 specularMap = texture(p3d_Texture2, diffuseCoord);

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

    float lightDistance = length(lightDirection);

    float attenuation =
        1.0
      / ( p3d_LightSource[i].constantAttenuation
        + p3d_LightSource[i].linearAttenuation
        * lightDistance
        + p3d_LightSource[i].quadraticAttenuation
        * (lightDistance * lightDistance)
        );

    if (attenuation <= 0.0) { continue; }

    float diffuseIntensity = dot(normal, unitLightDirection);

    if (diffuseIntensity < 0.0) { continue; }

    diffuseIntensity =
        celShadingEnabled.x == 1
      ? smoothstep(0.1, 0.2, diffuseIntensity)
      : diffuseIntensity;

    vec4 lightDiffuseColor     = p3d_LightSource[i].diffuse;
         lightDiffuseColor.rgb = pow(lightDiffuseColor.rgb, vec3(gamma.x));

    vec4 diffuseTemp =
      vec4
        ( clamp
            (   diffuseColor.rgb
              * lightDiffuseColor.rgb
              * diffuseIntensity
            , 0.0
            , 1.0
            )
        , diffuseColor.a
        );

    float specularIntensity =
      ( blinnPhongEnabled.x == 1
      ? clamp(dot(normal,       halfwayDirection),   0.0, 1.0)
      : clamp(dot(eyeDirection, reflectedDirection), 0.0, 1.0)
      );

    specularIntensity =
      ( celShadingEnabled.x == 1
      ? smoothstep(0.9, 1.0, specularIntensity)
      : specularIntensity
      );

    vec4  lightSpecularColor     = p3d_LightSource[i].specular;
          lightSpecularColor.rgb = pow(lightSpecularColor.rgb, vec3(gamma.x));

    vec4 materialSpecularColor        = vec4(vec3(specularMap.r), diffuseColor.a);
    if (fresnelEnabled.x == 1) {
      float fresnelFactor             = dot((blinnPhongEnabled.x == 1 ? halfwayDirection : normal), eyeDirection);
            fresnelFactor             = max(fresnelFactor, 0.0);
            fresnelFactor             = 1.0 - fresnelFactor;
            fresnelFactor             = pow(fresnelFactor, specularMap.b * MAX_FRESNEL_POWER);
            materialSpecularColor.rgb = mix(materialSpecularColor.rgb, vec3(1.0), clamp(fresnelFactor, 0.0, 1.0));
    }

    vec4 specularTemp      = vec4(vec3(0.0), diffuseColor.a);
         specularTemp.rgb  = lightSpecularColor.rgb * pow(specularIntensity, specularMap.g * MAX_SHININESS);
         specularTemp.rgb *= materialSpecularColor.rgb;
         specularTemp.rgb *= (1 - isParticle.x);
         specularTemp.rgb  = clamp(specularTemp.rgb, 0.0, 1.0);

    float unitLightDirectionDelta =
      dot
        ( normalize(p3d_LightSource[i].spotDirection)
        , -unitLightDirection
        );

    if (unitLightDirectionDelta < p3d_LightSource[i].spotCosCutoff) { continue; }

    float spotExponent = p3d_LightSource[i].spotExponent;

    diffuseTemp.rgb *= (spotExponent <= 0.0 ? 1.0 : pow(unitLightDirectionDelta, spotExponent));

    vec2  shadowMapSize = textureSize(p3d_LightSource[i].shadowMap, 0);
    float inShadow      = 0.0;
    float count         = 0.0;

    for (  int si = -shadowSamples; si <= shadowSamples; ++si) {
      for (int sj = -shadowSamples; sj <= shadowSamples; ++sj) {
        inShadow +=
          ( 1.0
          - textureProj
              ( p3d_LightSource[i].shadowMap
              , vertexInShadowSpaces[i] + vec4(vec2(si, sj) / shadowMapSize, vec2(0.0))
              )
          );

        count += 1.0;
      }
    }

    inShadow /= count;

    vec3 shadow =
      mix
        ( vec3(1.0)
        , shadowColor
        , inShadow
        );

    diffuseTemp.rgb  *= mix(shadow, vec3(1.0), isParticle.x);
    specularTemp.rgb *= mix(shadow, vec3(1.0), isParticle.x);

    diffuseTemp.rgb  *= attenuation;
    specularTemp.rgb *= attenuation;

    diffuse.rgb  += diffuseTemp.rgb;
    specular.rgb += specularTemp.rgb;
  }

  vec4 rimLight = vec4(vec3(0.0), diffuseColor.a);
  if (rimLightEnabled.x == 1) {
       rimLight.rgb =
        vec3
          ( 1.0
          - max
              ( 0.0
              , dot(normalize(-vertexPosition.xyz), normalize(normal))
              )
          );
       rimLight.rgb =
          ( celShadingEnabled.x == 1
          ? smoothstep(0.3, 0.4, rimLight.rgb)
          : pow(rimLight.rgb, vec3(2.0)) * 1.2
          );
       rimLight.rgb *= diffuse.rgb;
  }

  vec2 ssaoBlurTexSize  = textureSize(ssaoBlurTexture, 0).xy;
  vec2 ssaoBlurTexCoord = gl_FragCoord.xy / ssaoBlurTexSize;
  vec3 ssao             = texture(ssaoBlurTexture, ssaoBlurTexCoord).rgb;
       ssao             = mix(shadowColor, vec3(1.0), clamp(ssao.r, 0.0, 1.0));

  float sunPosition  = sin(sunPosition.x * pi.y);
  float sunMixFactor = 1.0 - (sunPosition / 2.0 + 0.5);

  vec3 ambientCool = pow(vec3(0.302, 0.451, 0.471), vec3(gamma.x)) * max(0.5, sunMixFactor);
  vec3 ambientWarm = pow(vec3(0.765, 0.573, 0.400), vec3(gamma.x)) * max(0.5, sunMixFactor);

  vec3 skyLight    = mix(ambientCool, ambientWarm, sunMixFactor);
  vec3 groundLight = mix(ambientWarm, ambientCool, sunMixFactor);

  vec3 worldNormal = normalize((trans_view_to_world * vec4(normal, 0.0)).xyz);

  vec3 ambientLight =
    mix
      ( groundLight
      , skyLight
      , 0.5 * (1.0 + dot(worldNormal, vec3(0, 0, 1)))
      );

  vec3 ambient =
      ambientLight.rgb
    * diffuseColor.rgb
    * ssao;

  vec3 emission = p3d_Material.emission.rgb * max(0.1, pow(sunPosition, 0.4));

  out0.a   =   diffuseColor.a;
  out0.rgb =   ambient.rgb
             + diffuse.rgb
             + rimLight.rgb
             + emission.rgb;

  if (isWater.x == 1) { out0.a = 0.0; }

  out1.a   = diffuseColor.a;
  out1.rgb = specular.rgb;

  if (isParticle.x == 1) { out1.rgb = vec3(0.0); }
}
