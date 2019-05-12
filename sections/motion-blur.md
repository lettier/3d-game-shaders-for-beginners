[:arrow_backward:](ssao.md)
[:arrow_double_up:](../README.md)
[:arrow_up_small:](#)
[:arrow_down_small:](#copyright)
[:arrow_forward:](screen-space-reflection.md)

# 3D Game Shaders For Beginners

## Motion Blur

<p align="center">
<img src="https://i.imgur.com/eTnhpLr.gif" alt="Motion Blur" title="Motion Blur">
</p>

To really sell the illusion of speed, you can do no better than motion blur.
From high speed car chases to moving at warp speed,
motion blur greatly improves the look and feel of fast moving objects.

There are a few ways to implement motion blur as a screen space technique.
The less involved implementation will only blur the scene in relation to the camera's movements
while the more involved version will blur any moving objects even with the camera remaining still.
The less involved technique is described below but the principle is the same.

### Textures

```c
uniform sampler2D positionTexture;
uniform sampler2D colorTexture;

// ...
```

The input textures needed are the vertex positions in view space and the scene's colors.
Refer back to [SSAO](ssao.md#vertex-positions) for acquiring the vertex positions.

### Matrices

```c
// ...

uniform mat4 previousViewWorldMat;
uniform mat4 worldViewMat;
uniform mat4 lensProjection;

// ...
```

The motion blur technique determines the blur direction by comparing
the previous frame's vertex positions with the current frame's vertex positions.
To do this, you'll need the previous frame's view-to-world matrix,
the current frame's world-to-view matrix,
and the camera lens' projection matrix.

### Parameters

```c
// ...

uniform vec2 parameters;

// ...

void main() {
  int   size       = int(parameters.x);
  float separation =     parameters.y;

// ...
```

The adjustable parameters are `size` and `separation`.
`size` controls how many samples are taken along the blur direction.
Increasing `size` increases the amount of blur at the cost of performance.
`separation` controls how spread out the samples are along the blur direction.
Increasing `separation` increases the amount of blur at the cost of accuracy.

### Blur Direction

```c
  // ...

  vec2 texSize  = textureSize(colorTexture, 0).xy;
  vec2 texCoord = gl_FragCoord.xy / texSize;

  vec4 position1 = texture(positionTexture, texCoord);
  vec4 position0 = worldViewMat * previousViewWorldMat * position1;

  // ...
```

To determine which way to blur this fragment,
you'll need to know where things were last frame and where things are this frame.
To figure out where things are now,
sample the current vertex position.
To figure out where things were last frame,
transform the current position from view space to world space,
using the previous frame's view-to-world matrix,
and then transform it back to view space from world space using this frame's world-to-view matrix.
This transformed position is this fragment's previous interpolated vertex position.

<p align="center">
<img src="https://i.imgur.com/oQqdxM9.gif" alt="Position Projection" title="Position Projection">
</p>

```c
  // ...

  position0      = lensProjection * position0;
  position0.xyz /= position0.w;
  position0.xy   = position0.xy * 0.5 + 0.5;

  position1      = lensProjection * position1;
  position1.xyz /= position1.w;
  position1.xy   = position1.xy * 0.5 + 0.5;

  // ...
```

Now that you have the current and previous positions,
transform them to screen space.
With the positions in screen space,
you can trace out the 2D direction you'll need to blur the onscreen image.

```c
  // ...

  //   position1.xy = position0.xy + direction;
  vec2 direction    = position1.xy - position0.xy;

  if (length(direction) <= 0.0) { return; }

  // ...
```

The blur direction goes from the previous position to the current position.

### Blurring

```c
  // ...

  fragColor = texture(colorTexture, texCoord);

  // ...
```

Sample the current fragment's color.
This will be the first of the colors blurred together.

```c
  // ...

  direction.xy *= separation;

  // ...
```

Multiply the direction vector by the separation.

```c
  // ...

  vec2  forward  = texCoord;
  vec2  backward = texCoord;

  // ...
```

For a more seamless blur,
sample in the direction of the blur and in the opposite direction of the blur.
For now, set the two vectors to the fragment's UV coordinate.

```c
  // ...

  float count = 1.0;

  // ...
```

`count` is used to average all of the samples taken.
It starts at one since you've already sampled the current fragment's color.

```c
  // ...

  for (int i = 0; i < size; ++i) {
    forward  += direction;
    backward -= direction;

    fragColor +=
      texture
        ( colorTexture
        , forward
        );
    fragColor +=
      texture
        ( colorTexture
        , backward
        );

    count += 2.0;
  }

  // ...
```

Sample the screen's colors both in the forward and backward direction of the blur.
Be sure to add these samples together as you travel along.

```c
  // ...

  fragColor /= count;
}
```

The final fragment color is the average color of the samples taken.

### Source

- [main.cxx](../demonstration/src/main.cxx)
- [basic.vert](../demonstration/shaders/vertex/basic.vert)
- [position.frag](../demonstration/shaders/fragment/position.frag)
- [motion-blur.frag](../demonstration/shaders/fragment/motion-blur.frag)

## Copyright

(C) 2020 David Lettier
<br>
[lettier.com](https://www.lettier.com)

[:arrow_backward:](ssao.md)
[:arrow_double_up:](../README.md)
[:arrow_up_small:](#)
[:arrow_down_small:](#copyright)
[:arrow_forward:](screen-space-reflection.md)
