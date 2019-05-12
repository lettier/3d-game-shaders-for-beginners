[:arrow_backward:](flow-mapping.md)
[:arrow_double_up:](../README.md)
[:arrow_up_small:](#)
[:arrow_down_small:](#copyright)
[:arrow_forward:](posterization.md)

# 3D Game Shaders For Beginners

## Depth Of Field

![Depth Of Field](https://i.imgur.com/6Yiyy1M.gif)

Like [SSAO](ssao.md), depth of field is an effect you can't live without once you've used it.
Artistically, you can use it to draw your viewer's eye to a certain subject.
But in general, depth of field adds a lot of realism for a little bit of effort.

### In Focus

The first step is to render your scene completely in focus.
Be sure to render this into a framebuffer texture.
This will be one of the inputs to the depth of field shader.

### Out Of Focus

```c
  // ...

  vec4 result = vec4(0);

  for (int i = 0; i < size2; ++i) {
    x = size - xCount;
    y = yCount - size;

    result +=
      texture
        ( blurTexture
        ,   texCoord
          + vec2(x * separation, y * separation)
        );

    xCount -= 1;
    if (xCount < countMin) { xCount = countMax; yCount -= 1; }
  }

  result = result / size2;

  // ...
```

The second step is to blur the scene as if it was completely out of focus.
Like bloom and SSAO, you can use a box blur.
Be sure to render this out-of-focus-scene into a framebuffer texture.
This will be one of the inputs to the depth of field shader.

### Mixing

![Depth Of Field Graph](https://i.imgur.com/sP0JcK4.gif)

```c
  // ...

  float focalLengthSharpness = 100;
  float blurRate             = 6;

  // ...
```

Feel free to tweak these two parameters.
`focalLengthSharpness` affects how out of focus the scene is at the focal length.
The smaller `focalLengthSharpness` is, the more out of focus the scene is at the focal length.
`blurRate` affects how fast the scene blurs as it moves away from the focal length.
The smaller the `blurRate` is, the less blurry the scene is as you move away from the focal point.

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
You can reuse the position framebuffer texture you used for SSAO.

```c
  // ...

  float blur =
    clamp
      (   pow
            ( blurRate
            , abs(position.y - focalLength.x)
            )
        / focalLengthSharpness
      , 0
      , 1
      );

  // ...

    fragColor = mix(focusColor, outOfFocusColor, blur);

    // ...
```

Here's the actual mixing.
The closer `blur` is to one, the more it will use the `outOfFocusColor`.
Zero `blur` means this fragment is entirely in focus.
At `blur >= 1`, this fragment is completely out of focus.

### Source

- [main.cxx](../demo/src/main.cxx)
- [basic.vert](../demo/shaders/vertex/basic.vert)
- [blur.frag](../demo/shaders/fragment/depth-of-field.frag)
- [depth-of-field.frag](../demo/shaders/fragment/depth-of-field.frag)

## Copyright

(C) 2019 David Lettier
<br>
[lettier.com](https://www.lettier.com)

[:arrow_backward:](flow-mapping.md)
[:arrow_double_up:](../README.md)
[:arrow_up_small:](#)
[:arrow_down_small:](#copyright)
[:arrow_forward:](posterization.md)
