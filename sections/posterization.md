[:arrow_backward:](depth-of-field.md)
[:arrow_double_up:](../README.md)
[:arrow_up_small:](#)
[:arrow_down_small:](#copyright)
[:arrow_forward:](pixelization.md)

# 3D Game Shaders For Beginners

## Posterization

![Posterization](https://i.imgur.com/KHQr4XG.gif)

Posterization or color quantization is the process of reducing the number of unique colors in an image.
You can use this shader to give your game a comic book or retro look.
Combine it with [outlining](outlining.md) for a full-on cartoon art style.

```c
  // ...

  float levels = 10;

  // ...
```

The `levels` parameter controls how many discrete bands or steps there are.
This will break up the continuous values from zero to one into chunks.
With two levels, zero to one becomes zero, 0.5, and one.


```c
  // ...

  fragColor = texture(posterizeTexture, texCoord);

  // ...
```

Sample the current fragment's color.

```c
  // ...

  float unquantized = dot(fragColor.rgb, vec3(0.3, 0.59, 0.11));
  float quantized   = floor(unquantized * levels) / levels;
  float adjustment  = quantized / max(unquantized, 0.0001);

  fragColor.rgb *= adjustment;

  // ...
```

There are various different ways to implement posterization.
I find this method produces nicer results than the more typical methods you'll find.

To reduce the color palette, first convert the color to a greyscale value.
Quantize the color by mapping it to one of the levels.
Calculate the ratio between the quantized greyscale value and the non-quantized greyscale value.
Multiply the fragment's color by this ratio.
This adjusts the fragment's color to have the same greyscale value as the quantized greyscale value.
Or in other words, this maps the quantized greyscale value back to a red, green, and blue vector.

### Pre-blurring

![Pre-blurring Posterization](https://i.imgur.com/tXVNGxY.gif)

While not a requirement, blurring the input texture first can produce a nicer result.
Refer to the [blur](blur.md) section for more details.

### Cel Shading

![Cel Shaded](https://i.imgur.com/PXWFrEZ.gif)

Posterization can produce the cel shaded look as cel shading or toon shading is the process of quantizing
the diffuse and specular colors into discrete shades.
For the best results,
use only solid diffuse colors,
no to subtle normal mapping,
and a small value for `levels`.

### Source

- [main.cxx](../demo/src/main.cxx)
- [basic.vert](../demo/shaders/vertex/basic.vert)
- [posterize.frag](../demo/shaders/fragment/posterize.frag)

## Copyright

(C) 2019 David Lettier
<br>
[lettier.com](https://www.lettier.com)

[:arrow_backward:](depth-of-field.md)
[:arrow_double_up:](../README.md)
[:arrow_up_small:](#)
[:arrow_down_small:](#copyright)
[:arrow_forward:](pixelization.md)
