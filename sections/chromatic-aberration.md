[:arrow_backward:](motion-blur.md)
[:arrow_double_up:](../README.md)
[:arrow_up_small:](#)
[:arrow_down_small:](#copyright)
[:arrow_forward:](screen-space-reflection.md)

# 3D Game Shaders For Beginners

## Chromatic Aberration

<p align="center">
<img src="https://i.imgur.com/bawgERm.gif" alt="Chromatic Aberration" title="Chromatic Aberration">
</p>

Chromatic aberration is a screen space technique that simulates lens distortion.
Use it to give your scene a cinematic, lo-fi analog feel or to emphasize a chaotic event.

### Texture

```c
uniform sampler2D colorTexture;

// ...
```

The input texture needed is the scene's colors captured into a framebuffer texture.

### Parameters

<p align="center">
<img src="https://i.imgur.com/fNpMaPL.gif" alt="Chromatic Aberration" title="Chromatic Aberration">
</p>

```c
  // ...

  float redOffset   =  0.009;
  float greenOffset =  0.006;
  float blueOffset  = -0.006;

  // ...
```

The adjustable parameters for this technique are the red, green, and blue offsets.
Feel free to play around with these to get the particular color fringe you're looking for.
These particular offsets produce a yellowish orange and blue fringe.

### Direction

```c
// ...

uniform vec2 mouseFocusPoint;

  // ...

  vec2 texSize  = textureSize(colorTexture, 0).xy;
  vec2 texCoord = gl_FragCoord.xy / texSize;

  vec2 direction = texCoord - mouseFocusPoint;

  // ...
```

The offsets can occur horizontally, vertically, or radially.
One approach is to radiate out from the [depth of field](depth-of-field.md) focal point.
As the scene gets more out of focus, the chromatic aberration increases.

### Samples

```c
// ...

out vec4 fragColor;

  // ...

  fragColor.r  = texture(colorTexture, texCoord + (direction * vec2(redOffset  ))).r;
  fragColor.g  = texture(colorTexture, texCoord + (direction * vec2(greenOffset))).g;
  fragColor.ba = texture(colorTexture, texCoord + (direction * vec2(blueOffset ))).ba;
}
```

With the direction and offsets,
make three samples of the scene's colors—one for the red, green, and blue channels.
These will be the final fragment color.

### Source

- [main.cxx](../demonstration/src/main.cxx)
- [basic.vert](../demonstration/shaders/vertex/basic.vert)
- [chromatic-aberration.frag](../demonstration/shaders/fragment/chromatic-aberration.frag)

## Copyright

(C) 2021 David Lettier
<br>
[lettier.com](https://www.lettier.com)

[:arrow_backward:](motion-blur.md)
[:arrow_double_up:](../README.md)
[:arrow_up_small:](#)
[:arrow_down_small:](#copyright)
[:arrow_forward:](screen-space-reflection.md)
