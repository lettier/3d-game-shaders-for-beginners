[:arrow_backward:](screen-space-refraction.md)
[:arrow_double_up:](../README.md)
[:arrow_up_small:](#)
[:arrow_down_small:](#copyright)
[:arrow_forward:](flow-mapping.md)

# 3D Game Shaders For Beginners

## Foam

<p align="center">
<img src="https://i.imgur.com/SVLPYKn.gif" alt="Foam" title="Foam">
</p>

Foam is typically used when simulating some body of water.
Anywhere the water's flow is disrupted, you add some foam.
The foam isn't much by itself but it can really connect the water with the rest of the scene.

<p align="center">
<img src="https://i.imgur.com/HCqvd8c.gif" alt="Lava River" title="Lava River">
</p>

But don't stop at just water.
You can use the same technique to make a river of lava for example.

### Vertex Positions

Like
[screen space refraction](screen-space-refraction.md),
you'll need both the foreground and background vertex positions.
The foreground being the scene with the foamy surface
and the background being the scene without the foamy surface.
Referrer back to [SSAO](ssao.md#vertex-positions) for the details
on how to acquire the vertex positions in view space.

### Mask

<p align="center">
<img src="https://i.imgur.com/N6TWBw8.gif" alt="Foam Mask" title="Foam Mask">
</p>

You'll need to texture your scene with a foam mask.
The demo masks everything off except the water.
For the water, it textures it with a foam pattern.

```c
// ...

uniform sampler2D foamPatternTexture;

in vec2 diffuseCoord;

out vec4 fragColor;

void main() {
  vec4 foamPattern = texture(foamPatternTexture, diffuseCoord);

  fragColor = vec4(vec3(dot(foamPattern.rgb, vec3(1)) / 3), 1);
}
```

Here you see the fragment shader that generates the foam mask.
It takes a foam pattern texture and UV maps it to the scene's geometry using the diffuse UV coordinates.
For every model, except the water, the shader is given a solid black texture as the `foamPatternTexture`.

```c
  // ...

  fragColor = vec4(vec3(dot(foamPattern.rgb, vec3(1)) / 3), 1);

  // ...
```

The fragment color is converted to greyscale,
as a precaution,
since the foam shader expects the foam mask to be greyscale.

### Uniforms

```c
// ...

uniform sampler2D maskTexture;
uniform sampler2D positionFromTexture;
uniform sampler2D positionToTexture;

// ...
```

The foam shader accepts a mask texture,
the foreground vertex positions (`positionFromTexture`),
and the background vertex positions (`positionToTexture`).

### Parameters

```c
  // ...

  float foamDepth = 4;
  vec4  foamColor = vec4(0.8, 0.85, 0.92, 1);

  // ...
```

The adjustable parameters for the foam shader are the foam depth and color.
The foam depth controls how much foam is shown.
As the foam depth increases, the amount of foam shown increases.

### Distance

```c
  // ...

  vec4 positionFrom = texture(positionFromTexture, texCoord);
  vec4 positionTo   = texture(positionToTexture,   texCoord);

  float depth  = (positionTo.xyz - positionFrom.xyz).y;

  // ...
```

Compute the distance from the foreground position to the background position.
Since the positions are in view (camera) space, we only need the y value since it goes into the screen.

### Amount

```c
  // ...

  float amount  = clamp(depth / foamDepth.x, 0, 1);
        amount  = 1 - amount;
        amount *= mask.r;
        amount  = amount * amount / (2 * (amount * amount - amount) + 1);

  // ...
```

The amount of foam is based on the depth, the foam depth parameter, and the mask value.

<p align="center">
<img src="https://i.imgur.com/CDIPmin.png" alt="Easing equation." title="Easing equation.">
</p>

```c
        // ...

        amount  = amount * amount / (2 * (amount * amount - amount) + 1);

        // ...
```

Reshape the amount using the ease in and out easing function.
This will give a lot of foam near depth zero and little to no foam near `foamDepth`.

### Fragment Color

```c
  // ...

  fragColor = mix(vec4(0), foamColor, amount);

  // ...
```

The fragment color is a mix between transparent black and the foam color based on the amount.

### Source

- [main.cxx](../demonstration/src/main.cxx)
- [base.vert](../demonstration/shaders/vertex/base.vert)
- [basic.vert](../demonstration/shaders/vertex/basic.vert)
- [position.frag](../demonstration/shaders/fragment/position.frag)
- [foam-mask.frag](../demonstration/shaders/fragment/foam-mask.frag)
- [foam.frag](../demonstration/shaders/fragment/foam.frag)
- [base-combine.frag](../demonstration/shaders/fragment/base-combine.frag)

## Copyright

(C) 2019 David Lettier
<br>
[lettier.com](https://www.lettier.com)

[:arrow_backward:](screen-space-refraction.md)
[:arrow_double_up:](../README.md)
[:arrow_up_small:](#)
[:arrow_down_small:](#copyright)
[:arrow_forward:](flow-mapping.md)
