[:arrow_backward:](outlining.md)
[:arrow_double_up:](../README.md)
[:arrow_up_small:](#)
[:arrow_down_small:](#copyright)
[:arrow_forward:](posterization.md)

# 3D Game Shaders For Beginners

## Depth Of Field

<p align="center">
<img src="https://i.imgur.com/DEa77Bh.gif" alt="Depth Of Field" title="Depth Of Field">
</p>

Like [SSAO](ssao.md), depth of field is an effect you can't live without once you've used it.
Artistically, you can use it to draw your viewer's eye to a certain subject.
But in general, depth of field adds a lot of realism for a little bit of effort.

### In Focus

The first step is to render your scene completely in focus.
Be sure to render this into a framebuffer texture.
This will be one of the inputs to the depth of field shader.

### Out Of Focus

The second step is to blur the scene as if it was completely out of focus.
Like bloom and SSAO, you can use a [box blur](blur.md#box-blur).
Be sure to render this out-of-focus-scene into a framebuffer texture.
This will be one of the inputs to the depth of field shader.

#### Bokeh

<p align="center">
<img src="https://i.imgur.com/aQ9Ga8J.gif" alt="Bokeh" title="Bokeh">
</p>

For a great bokeh effect, dilate the out of focus texture and use that as the out of focus input.
See [dilation](dilation.md) for more details.

### Mixing

```c
  // ...

  float minDistance = 1.0;
  float maxDistance = 3.0;

  // ...
```

Feel free to tweak these two parameters.
All positions at or below `minDistance` will be completely in focus.
All positions at or beyond `maxDistance` will be completely out of focus.

```c
  // ...

  vec4 focusColor      = texture(focusTexture, texCoord);
  vec4 outOfFocusColor = texture(outOfFocusTexture, texCoord);

  // ...
```

You'll need the in focus and out of focus colors.

```c
  // ...

  vec4 position = texture(positionTexture, texCoord);

  // ...
```

You'll also need the vertex position in view space.
You can reuse the position framebuffer texture you used for [SSAO](ssao.md#vertex-positions).

```c
  // ...

  vec4 focusPoint = texture(positionTexture, mouseFocusPoint);

  // ...
```

The focus point is a position somewhere in your scene.
All of the points in your scene are measured from the focus point.

Choosing the focus point is up to you.
The demo uses the scene position directly under the mouse when clicking the middle mouse button.
However, it could be a constant distance from the camera or a static position.

<p align="center">
<img src="https://i.imgur.com/idDZr62.png" alt="smoothstep" title="smoothstep">
</p>

```c
  // ...

  float blur =
    smoothstep
      ( minDistance
      , maxDistance
      , abs(position.y - focusPoint.y)
      );

  // ...
```

`smoothstep` returns values from zero to one.
The `minDistance` is the left-most edge.
Any position less than the minimum distance, from the focus point, will be in focus or have a blur of zero.
The `maxDistance` is the right-most edge.
Any position greater than the maximum distance, from the focus point, will be out of focus or have a blur or one.
For distances between the edges,
blur will be between zero and one.
These values are interpolated along a s-shaped curve.

```c

  // ...

  fragColor = mix(focusColor, outOfFocusColor, blur);

  // ...
```

The `fragColor` is a mixture of the in focus and out of focus color.
The closer `blur` is to one, the more it will use the `outOfFocusColor`.
Zero `blur` means this fragment is entirely in focus.
At `blur >= 1`, this fragment is completely out of focus.

### Source

- [main.cxx](../demonstration/src/main.cxx)
- [basic.vert](../demonstration/shaders/vertex/basic.vert)
- [box-blur.frag](../demonstration/shaders/fragment/box-blur.frag)
- [depth-of-field.frag](../demonstration/shaders/fragment/depth-of-field.frag)

## Copyright

(C) 2019 David Lettier
<br>
[lettier.com](https://www.lettier.com)

[:arrow_backward:](outlining.md)
[:arrow_double_up:](../README.md)
[:arrow_up_small:](#)
[:arrow_down_small:](#copyright)
[:arrow_forward:](posterization.md)
