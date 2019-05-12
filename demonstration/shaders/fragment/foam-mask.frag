/*
  (C) 2019 David Lettier
  lettier.com
*/

#version 150

uniform float osg_FrameTime;

uniform sampler2D foamPatternTexture;
uniform sampler2D flowTexture;

uniform vec2 flowMapsEnabled;

in vec2 diffuseCoord;

out vec4 fragColor;

void main() {
  vec2 flow   = texture(flowTexture, diffuseCoord).xy;
       flow   = (flow - 0.5) * 2;
       flow.x = abs(flow.x) <= 0.02 ? 0 : flow.x;
       flow.y = abs(flow.y) <= 0.02 ? 0 : flow.y;

  vec4 foamPattern =
    texture
      ( foamPatternTexture
      , vec2
          ( diffuseCoord.x + flowMapsEnabled.x * flow.x * osg_FrameTime
          , diffuseCoord.y + flowMapsEnabled.y * flow.y * osg_FrameTime
          )
      );

  fragColor = vec4(vec3(dot(foamPattern.rgb, vec3(1)) / 3), 1);
}
