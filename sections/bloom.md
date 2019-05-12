[:arrow_backward:](blur.md)
[:arrow_double_up:](../README.md)
[:arrow_up_small:](#)
[:arrow_down_small:](#copyright)
[:arrow_forward:](ssao.md)

# 3D Game Shaders For Beginners

## Bloom

<p align="center">
<img src="https://i.imgur.com/UxKRz2r.gif" alt="Bloom" title="Bloom">
</p>

Adding bloom to a scene can really sell the illusion of the lighting model.
Light emitting objects are more believable and specular highlights get an extra dose of shimmer.

```c
  //...

  int   size       = 5;
  float separation = 3;
  float threshold  = 0.4;
  float amount     = 1;

  // ...
```

These parameters control the look and feel.
`size` determines how blurred the effect is.
`separation` spreads out the blur.
`threshold` controls which fragments are illuminated.
And the last parameter, `amount`, controls how much bloom is outputted.

```c
  // ...

  vec2 texSize = textureSize(colorTexture, 0).xy;

  float value = 0.0;
  float count = 0.0;

  vec4 result = vec4(0);
  vec4 color  = vec4(0);

  for (int i = -size; i <= size; ++i) {
    for (int j = -size; j <= size; ++j) {
      // ...
    }
  }

  // ...
```

The technique starts by looping through a kernel/matrix/window centered over the current fragment.
This is similar to the window used for [outlining](outlining.md).
The size of the window is `size * 2 + 1` by `size * 2 + 1`.
So for example, with a `size` setting of two, the window uses `(2 * 2 + 1)^2 = 25` samples per fragment.

```c
      // ...

      color =
        texture
          ( colorTexture
          ,   ( gl_FragCoord.xy
              + (vec2(i, j) * separation)
              )
            / texSize
          );

      value = max(color.r, max(color.g, color.b));
      if (value < threshold) { color = vec4(0); }

      result += color;
      count  += 1.0;

      // ...
```

For each iteration,
it retrieves the color from the input texture and turns the red, green, and blue values into a greyscale value.
If this greyscale value is less than the threshold, it discards this color by making it solid black.
After evaluating the sample's greyscale value, it adds its RGB values to `result`.

```c
  // ...

  result /= count;

  fragColor = mix(vec4(0), result, amount);

  // ...
```

After it's done summing up the samples, it divides the sum of the color samples by the number of samples taken.
The result is the average color of itself and its neighbors.
By doing this for every fragment, you end up with a blurred image.
This form of blurring is known as a [box blur](blur.md#box-blur).

<p align="center">
<img src="https://i.imgur.com/m4yedrM.gif" alt="Bloom progresssion." title="Bloom progresssion.">
</p>

Here you see the progression of the bloom algorithm.

### Source

- [main.cxx](../demonstration/src/main.cxx)
- [basic.vert](../demonstration/shaders/vertex/basic.vert)
- [bloom.frag](../demonstration/shaders/fragment/outline.frag)

## Copyright

(C) 2019 David Lettier
<br>
[lettier.com](https://www.lettier.com)

[:arrow_backward:](blur.md)
[:arrow_double_up:](../README.md)
[:arrow_up_small:](#)
[:arrow_down_small:](#copyright)
[:arrow_forward:](ssao.md)
