[:arrow_backward:](film-grain.md)
[:arrow_double_up:](../README.md)
[:arrow_up_small:](#)
[:arrow_down_small:](#copyright)
[:arrow_forward:](gamma-correction.md)

# 3D Game Shaders For Beginners

## Lookup Table (LUT)

<p align="center">
<img src="https://i.imgur.com/WrPzVlW.gif" alt="LUT" title="LUT">
</p>

The lookup table or LUT shader allows you to transform the colors of your game
using an image editor like the [GIMP](https://www.gimp.org/).
From color grading to turning day into night,
the LUT shader is a handy tool for tweaking the look of your game.

<p align="center">
<img src="https://i.imgur.com/NPdJNGj.png" alt="Neutral LUT" title="Neutral LUT">
</p>

Before you can get started,
you'll need to find a neutral LUT image.
Neutral meaning that it leaves the fragment colors unchanged.
The LUT needs to be 256 pixels wide by 16 pixels tall and contain 16 blocks
with each block being 16 by 16 pixels.

The LUT is mapped out into 16 blocks.
Each block has a different level of blue.
As you move across the blocks, from left to right, the amount of blue increases.
You can see the amount of blue in each block's upper-left corner.
Within each block,
the amount of red increases as you move from left to right and
the amount of green increases as you move from top to bottom.
The upper-left corner of the first block is black since every RGB channel is zero.
The lower-right corner of the last block is white since every RGB channel is one.

<p align="center">
<img src="https://i.imgur.com/KyxPm1r.png" alt="LUT And Screenshot" title="LUT And Screenshot">
</p>

With the neutral LUT in hand, take a screenshot of your game and open it in your image editor.
Add the neutral LUT as a new layer and merge it with the screenshot.
As you manipulate the colors of the screenshot, the LUT will be altered in the same way.
When you're done editing, select only the LUT and save it as a new image.
You now have your new lookup table and can begin writing your shader.

```c
  // ...

  vec2 texSize = textureSize(colorTexture, 0).xy;

  vec4 color = texture(colorTexture, gl_FragCoord.xy / texSize);

  // ...
```

The LUT shader is a screen space technique.
Therefore, sample the scene's color at the current fragment or screen position.

```c
  // ...

  float u  =  floor(color.b * 15.0) / 15.0 * 240.0;
        u  = (floor(color.r * 15.0) / 15.0 *  15.0) + u;
        u /= 255.0;

  float v  = ceil(color.g * 15.0);
        v /= 15.0;
        v  = 1.0 - v;

  // ...
```

In order to transform the current fragment's color,
using the LUT,
you'll need to map the color to two UV coordinates on the lookup table texture.
The first mapping (shown up above) is to the nearest left or lower bound block location and
the second mapping (shown below) is to the nearest right or upper bound block mapping.
At the end, you'll combine these two mappings to create the final color transformation.

<p align="center">
<img src="https://i.imgur.com/j2JmyQ2.png" alt="RGB Channel Mapping" title="RGB Channel Mapping">
</p>

Each of the red, green, and blue channels maps to one of 16 possibilities in the LUT.
The blue channel maps to one of the 16 upper-left block corners.
After the blue channel maps to a block,
the red channel maps to one of the 16 horizontal pixel positions within the block and
the green channel maps to one of the 16 vertical pixel positions within the block.
These three mappings will determine the UV coordinate you'll need to sample a color from the LUT.

```c
  // ...

        u /= 255.0;

        v /= 15.0;
        v  = 1.0 - v;

  // ...
```

To calculate the final U coordinate, divide it by 255 since the LUT is 256 pixels wide and U ranges from zero to one.
To calculate the final V coordinate, divide it by 15 since the LUT is 16 pixels tall and V ranges from zero to one.
You'll also need to subtract the normalized V coordinate from one since V ranges from zero at the bottom to one at the top while
the green channel ranges from zero at the top to 15 at the bottom.

```c
  // ...

  vec3 left = texture(lookupTableTexture, vec2(u, v)).rgb;

  // ...
```

Using the UV coordinates, sample a color from the lookup table.
This is the nearest left block color.

```c
  // ...

  u  =  ceil(color.b * 15.0) / 15.0 * 240.0;
  u  = (ceil(color.r * 15.0) / 15.0 *  15.0) + u;
  u /= 255.0;

  v  = 1.0 - (ceil(color.g * 15.0) / 15.0);

  vec3 right = texture(lookupTableTexture, vec2(u, v)).rgb;

  // ...
```

Now you'll need to calculate the UV coordinates for the nearest right block color.
Notice how `ceil` or ceiling is being used now instead of `floor`.

<p align="center">
<img src="https://i.imgur.com/uciq7Um.png" alt="Mixing" title="Mixing">
</p>

```c
  // ...

  color.r = mix(left.r, right.r, fract(color.r * 15.0));
  color.g = mix(left.g, right.g, fract(color.g * 15.0));
  color.b = mix(left.b, right.b, fract(color.b * 15.0));

  // ...
```

Not every channel will map perfectly to one of its 16 possibilities.
For example, `0.5` doesn't map perfectly.
At the lower bound (`floor`),
it maps to `0.4666666666666667` and at the upper bound (`ceil`),
it maps to `0.5333333333333333`.
Compare that with `0.4` which maps to `0.4` at the lower bound and `0.4` at the upper bound.
For those channels which do not map perfectly,
you'll need to mix the left and right sides based on where the channel falls between its lower and upper bound.
For `0.5`, it falls directly between them making the final color a mixture of half left and half right.
However,
for `0.132` the mixture will be 98% right and 2% left since the fractional part of `0.123` times `15.0` is `0.98`.

```c
  // ...

  fragColor = color;

  // ...
```

Set the fragment color to the final mix and you're done.

### Source

- [main.cxx](../demonstration/src/main.cxx)
- [basic.vert](../demonstration/shaders/vertex/basic.vert)
- [lookup-table.frag](../demonstration/shaders/fragment/lookup-table.frag)

## Copyright

(C) 2020 David Lettier
<br>
[lettier.com](https://www.lettier.com)

[:arrow_backward:](film-grain.md)
[:arrow_double_up:](../README.md)
[:arrow_up_small:](#)
[:arrow_down_small:](#copyright)
[:arrow_forward:](gamma-correction.md)
