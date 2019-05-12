[:arrow_backward:](posterization.md)
[:arrow_double_up:](../README.md)
[:arrow_up_small:](#)
[:arrow_down_small:](#copyright)
[:arrow_forward:](sharpen.md)

# 3D Game Shaders For Beginners

## Pixelization

<p align="center">
<img src="https://i.imgur.com/IbnyYZN.gif" alt="Pixelization" title="Pixelization">
</p>

Pixelizing your 3D game can give it a interesting look and
possibly save you time by not having to create all of the pixel art by hand.
Combine it with the posterization for a true retro look.

```c
  // ...

  int pixelSize = 5;

  // ...
```

Feel free to adjust the pixel size.
The bigger the pixel size, the blockier the image will be.

<p align="center">
<img src="https://i.imgur.com/WF5MmM0.gif" alt="Pixelization Process" title="Pixelization Process">
</p>

```c
  // ...

  float x = int(gl_FragCoord.x) % pixelSize;
  float y = int(gl_FragCoord.y) % pixelSize;

  x = floor(pixelSize / 2.0) - x;
  y = floor(pixelSize / 2.0) - y;

  x = gl_FragCoord.x + x;
  y = gl_FragCoord.y + y;

  // ...
```

The technique works by mapping each fragment to the center of its closest, non-overlapping
pixel-sized window.
These windows are laid out in a grid over the input texture.
The center-of-the-window fragments determine the color for the other fragments in their window.

```c
    // ...

    fragColor = texture(colorTexture, vec2(x, y) / texSize);

    // ...
```

Once you have determined the correct fragment coordinate to use,
pull its color from the input texture and assign that to the fragment color.

### Source

- [main.cxx](../demonstration/src/main.cxx)
- [basic.vert](../demonstration/shaders/vertex/basic.vert)
- [pixelize.frag](../demonstration/shaders/fragment/pixelize.frag)

## Copyright

(C) 2019 David Lettier
<br>
[lettier.com](https://www.lettier.com)

[:arrow_backward:](posterization.md)
[:arrow_double_up:](../README.md)
[:arrow_up_small:](#)
[:arrow_down_small:](#copyright)
[:arrow_forward:](sharpen.md)
