[:arrow_backward:](screen-space-refraction.md)
[:arrow_double_up:](../README.md)
[:arrow_up_small:](#)
[:arrow_down_small:](#copyright)
[:arrow_forward:](depth-of-field.md)

# 3D Game Shaders For Beginners

## Flow Mapping

![Flow Mapping](https://i.imgur.com/ns6IxE4.gif)

Flow mapping is useful when you need to animate some fluid material.
Much like diffuse maps map UV coordinates to diffuse colors and normal maps map UV coordinates to normals,
flow maps map UV coordinates to 2D translations or flows.

![Flow Map](https://i.imgur.com/b9Vw94N.png)

Here you see a flow map that maps UV coordinates to translations in the positive y-axis direction.
Flow maps use the red and green channels to store translations in the x and y direction.
The red channel is for the x-axis and the green channel is the y-axis.
Both range from zero to one which translates to flows that range from `(-1, -1)` to `(1, 1)`.
This flow map is all one color consisting of 0.5 red and 0.6 green.

```c
[r, g, b] =
  [r * 2 - 1, g * 2 - 1, b * 2 - 1] =
    [ x, y, z]
```

Recall how the colors in a normal map are converted to actual normals.
There is a similar process for flow maps.

```c
// ...

uniform sampler2D flowTexture;

  vec2 flow   = texture(flowTexture, uv).xy;
       flow   = (flow - 0.5) * 2;

  // ...
```

To convert a flow map color to a flow,
you minus 0.5 from the channel (red and green) and multiply by two.

```c
(r, g) =
 ( (r - 0.5) * 2
 , (g - 0.5) * 2
 ) =
  ( (0.5 - 0.5) * 2
  , (0.6 - 0.5) * 2
  ) =
    (x, y) =
      (0, 0.2)
```

The flow map above maps each UV coordinate to the flow `(0, 0.2)`.
This indicates zero movement in the x direction and a movement of 0.2 in the y direction.

The flows can be used to translate all sorts of things but they're typically used to
offset the UV coordinates of a another texture.

![Foam Mask](https://i.imgur.com/N6TWBw8.gif)

```c
  // ...

  vec2 flow   = texture(flowTexture, diffuseCoord).xy;
       flow   = (flow - 0.5) * 2;

  vec4 foamPattern =
    texture
      ( foamPatternTexture
      , vec2
          ( diffuseCoord.x + flow.x * osg_FrameTime
          , diffuseCoord.y + flow.y * osg_FrameTime
          )
      );

  // ...
```

For example, the demo program uses a flow map to animate the water.
Here you see the flow map being used to animate the
[foam mask](foam.md#mask).
This continuously moves the diffuse UV coordinates directly up,
giving the foam mask the appearance of moving down stream.

```c
          // ...

          ( diffuseCoord.x + flow.x * osg_FrameTime
          , diffuseCoord.y + flow.y * osg_FrameTime

          // ...
```

You'll need how many seconds have passed since the first frame
in order to animate the UV coordinates in the direction indicated by the flow.
`osg_FrameTime` is
[provided](https://github.com/panda3d/panda3d/blob/daa57733cb9b4ccdb23e28153585e8e20b5ccdb5/panda/src/display/graphicsStateGuardian.cxx#L930)
by Panda3D.
It is a timestamp of how many seconds have passed since the first frame.

### Source

- [main.cxx](../demo/src/main.cxx)
- [base.vert](../demo/shaders/vertex/base.vert)
- [basic.vert](../demo/shaders/vertex/basic.vert)
- [normal.frag](../demo/shaders/fragment/normal.frag)

## Copyright

(C) 2019 David Lettier
<br>
[lettier.com](https://www.lettier.com)

[:arrow_backward:](screen-space-refraction.md)
[:arrow_double_up:](../README.md)
[:arrow_up_small:](#)
[:arrow_down_small:](#copyright)
[:arrow_forward:](depth-of-field.md)
