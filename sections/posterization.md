[:arrow_backward:](depth-of-field.md)
[:arrow_double_up:](../README.md)
[:arrow_up_small:](#)
[:arrow_down_small:](#copyright)
[:arrow_forward:](pixelization.md)

# 3D Game Shaders For Beginners

## Posterization

<p align="center">
<img src="https://i.imgur.com/hpP5G9z.gif" alt="Posterization" title="Posterization">
</p>

Posterization or color quantization is the process of reducing the number of unique colors in an image.
You can use this shader to give your game a comic book or retro look.
Combine it with [outlining](outlining.md) for a full-on cartoon art style.

There are various different ways to implement posterization.
This method works directly with the greyscale values and indirectly with the RGB values of the image.
For each fragment, it maps the RGB color to a greyscale value.
This greyscale value is then mapped to both its lower and upper level value.
The closest level to the original greyscale value is then mapped back to an RGB value
This new RGB value becomes the fragment color.
I find this method produces nicer results than the more typical methods you'll find.

```c
  // ...

  float levels = 10;

  // ...
```

The `levels` parameter controls how many discrete bands or steps there are.
This will break up the continuous values from zero to one into chunks.
With four levels, `0.0` to `1.0` becomes `0.0`, `0.25`, `0.5`, `0.75`, and `1.0`.


```c
  // ...

  fragColor = texture(posterizeTexture, texCoord);

  // ...
```

Sample the current fragment's color.

```c
  // ...

  float greyscale = max(fragColor.r, max(fragColor.g, fragColor.b));

  // ...
```

Map the RGB values to a greyscale value.
In this instance, the greyscale value is the maximum value of the R, G, and B values.

```c
  // ...

  float lower     = floor(greyscale * levels) / levels;
  float lowerDiff = abs(greyscale - lower);

  // ...
```

Map the greyscale value to its lower level and
then calculate the difference between its lower level and itself.
For example,
if the greyscale value is `0.87` and there are four levels, its lower level is `0.75` and the difference is `0.12`.


```c
  // ...

  float upper     = ceil(greyscale * levels) / levels;
  float upperDiff = abs(upper - greyscale);

  // ...
```

Now calculate the upper level and the difference.
Keeping with the example up above, the upper level is `1.0` and the difference is `0.13`.

```c
  // ...

  float level      = lowerDiff <= upperDiff ? lower : upper;
  float adjustment = level / greyscale;

  // ...
```

The closest level is used to calculate the adjustment.
The adjustment is the ratio between the quantized and unquantized greyscale value.
This adjustment is used to map the quantized greyscale value back to an RGB value.


```c
  // ...

  fragColor.rgb * adjustment;

  // ...
```

After multiplying `rgb` by the adjustment, `max(r, max(g, b))` will now equal the quantized greyscale value.
This maps the quantized greyscale value back to a red, green, and blue vector.

### Source

- [main.cxx](../demonstration/src/main.cxx)
- [basic.vert](../demonstration/shaders/vertex/basic.vert)
- [posterize.frag](../demonstration/shaders/fragment/posterize.frag)

## Copyright

(C) 2019 David Lettier
<br>
[lettier.com](https://www.lettier.com)

[:arrow_backward:](depth-of-field.md)
[:arrow_double_up:](../README.md)
[:arrow_up_small:](#)
[:arrow_down_small:](#copyright)
[:arrow_forward:](pixelization.md)
