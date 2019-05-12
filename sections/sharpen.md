[:arrow_backward:](pixelization.md)
[:arrow_double_up:](../README.md)
[:arrow_up_small:](#)
[:arrow_down_small:](#copyright)
[:arrow_forward:](film-grain.md)

# 3D Game Shaders For Beginners

## Sharpen

![Sharpen](https://i.imgur.com/rOfy9Sv.gif)

The sharpen effect increases the contrast at the edges of the image.
This comes in handy when your graphics are bit too soft.

```c
  // ...

  float amount = 0.8;

  // ...
```

You can control how sharp the result is by adjusting the amount.
An amount of zero leaves the image untouched.
Try negative values for an odd look.

```c
  // ...

  float neighbor = amount * -1;
  float center   = amount * 4 + 1;

  // ...
```

Neighboring fragments are multiplied by `amount * -1`.
The current fragment is multiplied by `amount * 4 + 1`.

```c
  // ...

  vec3 color =
        texture(sharpenTexture, vec2(gl_FragCoord.x + 0, gl_FragCoord.y + 1) / texSize).rgb
      * neighbor

      + texture(sharpenTexture, vec2(gl_FragCoord.x - 1, gl_FragCoord.y + 0) / texSize).rgb
      * neighbor
      + texture(sharpenTexture, vec2(gl_FragCoord.x + 0, gl_FragCoord.y + 0) / texSize).rgb
      * center
      + texture(sharpenTexture, vec2(gl_FragCoord.x + 1, gl_FragCoord.y + 0) / texSize).rgb
      * neighbor

      + texture(sharpenTexture, vec2(gl_FragCoord.x + 0, gl_FragCoord.y - 1) / texSize).rgb
      * neighbor
      ;

  // ...
```

The neighboring fragments are up, down, left, and right.
After multiplying both the neighbors and the current fragment by their particular values, sum the result.

```c
    // ...

    fragColor = vec4(color, texture(sharpenTexture, texCoord).a);

    // ...
```

This sum is the final fragment color.

### Source

- [main.cxx](../demo/src/main.cxx)
- [basic.vert](../demo/shaders/vertex/basic.vert)
- [sharpen.frag](../demo/shaders/fragment/sharpen.frag)

## Copyright

(C) 2019 David Lettier
<br>
[lettier.com](https://www.lettier.com)

[:arrow_backward:](pixelization.md)
[:arrow_double_up:](../README.md)
[:arrow_up_small:](#)
[:arrow_down_small:](#copyright)
[:arrow_forward:](film-grain.md)
