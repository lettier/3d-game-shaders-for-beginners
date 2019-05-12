/*
  (C) 2019 David Lettier
  lettier.com
*/

#version 150

uniform mat4 lensProjection;

uniform sampler2D positionFromTexture;
uniform sampler2D positionToTexture;
uniform sampler2D normalFromTexture;

uniform vec2 rior;
uniform vec2 enabled;

out vec4 fragColor;

void main() {
  float maxDistance = 5;
  float resolution  = 0.3;
  int   steps       = 5;
  float thickness   = 0.5;

  vec2 texSize  = textureSize(positionFromTexture, 0).xy;
  vec2 texCoord = gl_FragCoord.xy / texSize;

  vec4 uv = vec4(texCoord.xy, 1, 1);

  vec4 positionFrom = texture(positionFromTexture, texCoord);

  if (positionFrom.w <= 0 || enabled.x != 1) { fragColor = uv; return; }

  vec3 unitPositionFrom = normalize(positionFrom.xyz);
  vec3 normalFrom       = normalize(texture(normalFromTexture, texCoord).xyz);
  vec3 pivot            = normalize(refract(unitPositionFrom, normalFrom, rior.x));

  vec4 positionTo = positionFrom;

  vec4 startView = vec4(positionFrom.xyz + (pivot *           0), 1);
  vec4 endView   = vec4(positionFrom.xyz + (pivot * maxDistance), 1);

  vec4 startFrag      = startView;
       startFrag      = lensProjection * startFrag;
       startFrag.xyz /= startFrag.w;
       startFrag.xy   = startFrag.xy * 0.5 + 0.5;
       startFrag.xy  *= texSize;

  vec4 endFrag      = endView;
       endFrag      = lensProjection * endFrag;
       endFrag.xyz /= endFrag.w;
       endFrag.xy   = endFrag.xy * 0.5 + 0.5;
       endFrag.xy  *= texSize;

  vec2 frag  = startFrag.xy;
       uv.xy = frag / texSize;

  float deltaX    = endFrag.x - startFrag.x;
  float deltaY    = endFrag.y - startFrag.y;
  float useX      = abs(deltaX) >= abs(deltaY) ? 1 : 0;
  float delta     = mix(abs(deltaY), abs(deltaX), useX) * clamp(resolution, 0, 1);
  vec2  increment = vec2(deltaX, deltaY) / max(delta, 0.001);

  float search0 = 0;
  float search1 = 0;

  int hit0 = 0;
  int hit1 = 0;

  float viewDistance = startView.y;
  float depth        = thickness;

  float i = 0;

  for (i = 0; i < int(delta); ++i) {
    frag      += increment;
    uv.xy      = frag / texSize;
    positionTo = texture(positionToTexture, uv.xy);

    search1 =
      mix
        ( (frag.y - startFrag.y) / deltaY
        , (frag.x - startFrag.x) / deltaX
        , useX
        );

    search1 = clamp(search1, 0, 1);

    viewDistance = (startView.y * endView.y) / mix(endView.y, startView.y, search1);
    depth        = viewDistance - positionTo.y;

    if (depth > 0 && depth < thickness) {
      hit0 = 1;
      break;
    } else {
      search0 = search1;
    }
  }

  search1 = search0 + ((search1 - search0) / 2);

  steps *= hit0;

  for (i = 0; i < steps; ++i) {
    frag       = mix(startFrag.xy, endFrag.xy, search1);
    uv.xy      = frag / texSize;
    positionTo = texture(positionToTexture, uv.xy);

    viewDistance = (startView.y * endView.y) / mix(endView.y, startView.y, search1);
    depth        = viewDistance - positionTo.y;

    if (depth > 0 && depth < thickness) {
      hit1 = 1;
      search1 = search0 + ((search1 - search0) / 2);
    } else {
      float temp = search1;
      search1 = search1 + ((search1 - search0) / 2);
      search0 = temp;
    }
  }

  float visibility =
      hit1
    * positionTo.w
    * ( 1
      - max
         ( dot(-unitPositionFrom, pivot)
         , 0
         )
      )
    * (uv.x < 0 || uv.x > 1 ? 0 : 1)
    * (uv.y < 0 || uv.y > 1 ? 0 : 1);

  visibility = clamp(visibility, 0, 1);

  fragColor = vec4(mix(texCoord.xy, uv.xy, visibility), 1, 1);
}
