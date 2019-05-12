[:arrow_backward:](sharpen.md)
[:arrow_double_up:](../README.md)
[:arrow_up_small:](#)
[:arrow_down_small:](#copyright)
[:arrow_forward:](film-grain.md)

# 3D Game Shaders For Beginners

## Dilation

<p align="center">
<img src="https://i.imgur.com/z751O74.gif" alt="Dilation" title="Dilation">
</p>

Dilation dilates or enlarges the brighter areas of an image while at the same time,
contracts or shrinks the darker areas of an image.
This tends to create a pillowy look.
You can use dilation for a glow/bloom effect or to add bokeh to your [depth of field](depth-of-field.md).

```c
  // ...

  int   size         = int(parameters.x);
  float separation   =     parameters.y;
  float minThreshold = 0.1;
  float maxThreshold = 0.3;

  // ...
```

The `size` and `separation` parameters control how dilated the image becomes.
A larger `size` will increase the dilation at the cost of performance.
A larger `separation` will increase the dilation at the cost of quality.
The `minThreshold` and `maxThreshold` parameters control which parts of the image become dilated.

```c
  // ...

  vec2 texSize   = textureSize(colorTexture, 0).xy;
  vec2 fragCoord = gl_FragCoord.xy;

  fragColor = texture(colorTexture, fragCoord / texSize);

  // ...
```

Sample the color at the current fragment's position.

```c
  // ...

  float  mx = 0.0;
  vec4  cmx = fragColor;

  for (int i = -size; i <= size; ++i) {
    for (int j = -size; j <= size; ++j) {
      // ...
    }
  }

  // ...
```

Loop through a `size` by `size` window, centered at the current fragment position.
As you loop, find the brightest color based on the surrounding greyscale values.

<p align="center">
<img src="https://i.imgur.com/X3uIyIL.png" alt="Dilation Window" title="Dilation Window">
</p>

```c
      // ...

      // For a rectangular shape.
      //if (false);

      // For a diamond shape;
      //if (!(abs(i) <= size - abs(j))) { continue; }

      // For a circular shape.
      if (!(distance(vec2(i, j), vec2(0, 0)) <= size)) { continue; }

      // ...
```

The window shape will determine the shape of the dilated parts of the image.
For a rectangular shape, you can use every fragment covered by the window.
For any other shape, skip the fragments that fall outside the desired shape.

```c
      // ...

      vec4 c =
        texture
          ( colorTexture
          ,   ( gl_FragCoord.xy
              + (vec2(i, j) * separation)
              )
            / texSize
          );

      // ...
```

Sample a fragment color from the surrounding window.

```c
      // ...

      float mxt = dot(c.rgb, vec3(0.21, 0.72, 0.07));

      // ...
```

Convert the sampled color to a greyscale value.

```c
      // ...

      if (mxt > mx) {
        mx  = mxt;
        cmx = c;
      }

      // ...
```

If the sampled greyscale value is larger than the current maximum greyscale value,
update the maximum greyscale value and its corresponding color.

```c
  // ...

  fragColor.rgb =
    mix
      ( fragColor.rgb
      , cmx.rgb
      , smoothstep(minThreshold, maxThreshold, mx)
      );

  // ...
```

The new fragment color is a mixture between the existing fragment color and
the brightest color found.
If the maximum greyscale value found is less than `minThreshold`,
the fragment color is unchanged.
If the maximum greyscale value is greater than `maxThreshold`,
the fragment color is replaced with the brightest color found.
For any other case,
the fragment color is a mix between the current fragment color and the brightest color.

### Source

- [main.cxx](../demonstration/src/main.cxx)
- [basic.vert](../demonstration/shaders/vertex/basic.vert)
- [dilation.frag](../demonstration/shaders/fragment/dilation.frag)

## Copyright

(C) 2020 David Lettier
<br>
[lettier.com](https://www.lettier.com)

[:arrow_backward:](sharpen.md)
[:arrow_double_up:](../README.md)
[:arrow_up_small:](#)
[:arrow_down_small:](#copyright)
[:arrow_forward:](film-grain.md)
