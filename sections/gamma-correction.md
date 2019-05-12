[:arrow_backward:](lookup-table.md)
[:arrow_double_up:](../README.md)
[:arrow_up_small:](#)
[:arrow_down_small:](#copyright)
[:arrow_forward:](setup.md)

# 3D Game Shaders For Beginners

## Gamma Correction

<p align="center">
<img src="https://i.imgur.com/IG7A6cj.gif" alt="Gamma Correction" title="Gamma Correction">
</p>

Correcting for gamma will make your color calculations look correct.
This isn't to say they'll look amazing but with gamma correction,
you'll find that the colors meld together better,
the shadows are more nuanced,
and the highlights are more subtle.
Without gamma correction,
the shadowed areas tend to get crushed while the highlighted areas tend to get blown-out and
over saturated making for a harsh contrast overall.

If you're aiming for realism,
gamma correction is especially important.
As you perform more and more calculations,
the tiny errors add up making it harder to achieve photorealism.
The equations will be correct but the inputs and outputs will be wrong leaving you frustrated.

It's easy to get twisted around when thinking about gamma correction
but essentially it boils down to knowing what color space a color is in and how to convert that color to the color space you need.
With those two pieces of the puzzle,
gamma correction becomes a tedious yet simple chore you'll have to perform from time to time.

### Color Spaces

<p align="center">
<img src="https://i.imgur.com/a1U5oBq.png" alt="sRGB vs RGB" title="sRGB vs RGB">
</p>

The two color spaces you'll need to be aware of are sRGB (standard Red Green Blue) and RGB or linear color space.

```bash
identify -format "%[colorspace]\n" house-diffuse-srgb.png
sRGB

identify -format "%[colorspace]\n" house-diffuse-rgb.png
RGB
```

Knowing what color space a color texture is in will determine how you handle it in your shaders.
To determine the color space of a texture, use ImageMagick's `identify`.
You'll find that most textures are in sRGB.

```bash
convert house-diffuse-srgb -colorspace rgb house-diffuse-rgb.png
```

To convert a texture to a particular color space, use ImageMagick's `convert` program.
Notice how a texture is darkened when transforming from sRGB to RGB.

### Decoding


The red, green, and blue values in a sRGB color texture are encoded and cannot be modified directly.
Modifying them directly would be like running spellcheck on an encrypted message.
Before you can run spellcheck,
you first have to decrypt the message.
Similarly,
to modify the values of an sRGB texture,
you first have to decode or transform them to RGB or linear color space.

```c
  // ...

  color     = texture(color_texture, uv);
  color.rgb = pow(color.rgb, 2.2);

  // ...
```

To decode a sRGB encoded color,
raise the `rgb` values to the power of `2.2`.
Once you have decoded the color,
you are now free to add, subtract, multiply, and divide it.

By raising the color values to the power of `2.2`,
you're converting them from sRGB to RGB or linear color space.
This conversion has the effect of darkening the colors.

<p align="center">
<img src="https://i.imgur.com/E5nkRfG.png" alt="Color Darkening" title="Color Darkening">
</p>

For example,
`vec3(0.9, 0.2, 0.3)` becomes `vec3(0.793, 0.028, 0.07)`.

<p align="center">
<img src="https://i.imgur.com/TOEb0EC.gif" alt="Gamma Curves" title="Gamma Curves">
</p>

The `2.2` value is known as gamma.
Loosely speaking, gamma can either be `1.0 / 2.2`, `2.2`, or `1.0`.
As you've seen, `2.2` is for decoding sRGB encoded color textures.
As you will see, `1.0 / 2.2` is for encoding linear or RGB color textures.
And `1.0` is RGB or linear color space since `y = 1 * x + 0` and
any base raised to the power of `1.0` is itself.

#### Non-color Data

<p align="center">
<img src="https://i.imgur.com/reA2qjs.png" alt="Non-color Data" title="Non-color Data">
</p>

One important exception to decoding is when the "colors" of a texture represent non-color data.
Some examples of non-color data would be the normals in a normal map,
the alpha channel,
the heights in a height map,
and the directions in a flow map.
Only decode color related data or data that represents color.
When dealing with non-color data,
treat the sRGB color values as RGB or linear and skip the decoding process.

### Encoding

<p align="center">
<img src="https://i.imgur.com/tRxkKNe.gif" alt="Perceptually versus Actually Linear" title="Perceptually versus Actually Linear">
</p>

The necessity for encoding and decoding stems from the fact that humans do not perceive lightness linearly and
most displays (like a monitor) lack the precision or number of bits to accurately show both lighter and darker tonal values or shades.
With only so many bits to go around,
colors are encoded in such a way that more bits are devoted to the darker shades than the lighter shades
since humans are more sensitive to darker tones than lighter tones.
Encoding it this way uses the limited number of bits more effectively for human perception.
Still, the only thing to remember is that your display is expecting sRGB encoded values.
Therefore, if you decoded a sRGB value, you have to encode it before it makes its way to your display.

```c
  // ...

  color     = texture(color_texture, uv);
  color.rgb = pow(color.rgb, 1.0 / 2.2);

  // ...
```

To encode a linear value or convert RGB to sRGB,
raise the `rgb` values to the power of `1.0 / 2.2`.
Notice how `1.1 / 2.2` is the reciprocal of `2.2` or `2.2 / 1.0`.
Here you see the symmetry in decoding and encoding.

<p align="center">
<img src="https://i.imgur.com/4km0pdv.gif" alt="Not Gamma Corrected versus Gamma Corrected" title="Not Gamma Corrected versus Gamma Corrected">
</p>

### Source

- [main.cxx](../demonstration/src/main.cxx)
- [basic.vert](../demonstration/shaders/vertex/basic.vert)
- [gamma-correction.frag](../demonstration/shaders/fragment/gamma-correction.frag)

## Copyright

(C) 2020 David Lettier
<br>
[lettier.com](https://www.lettier.com)

[:arrow_backward:](lookup-table.md)
[:arrow_double_up:](../README.md)
[:arrow_up_small:](#)
[:arrow_down_small:](#copyright)
[:arrow_forward:](setup.md)
