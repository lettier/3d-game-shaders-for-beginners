[:arrow_backward:](fresnel-factor.md)
[:arrow_double_up:](../README.md)
[:arrow_up_small:](#)
[:arrow_down_small:](#copyright)
[:arrow_forward:](cel-shading.md)

# 3D Game Shaders For Beginners

## Rim Lighting

<p align="center">
<img src="https://i.imgur.com/tfgKgyn.gif" alt="Rim Lighting" title="Rim Lighting">
</p>

Taking inspiration from the [fresnel factor](fresnel-factor.md),
rim lighting targets the rim or silhouette of an object.
When combined with [cel shading](cel-shading.md) and [outlining](outlining.md),
it can really complete that cartoon look.
You can also use it to highlight objects in the game,
making it easier for players to navigate and accomplish tasks.

```c
  // ...

  vec3 eye = normalize(-vertexPosition.xyz);

  // ...
```

As it was for the fresnel factor,
you'll need the eye vector.
If your vertex positions are in view space,
the eye vector is the negation of the vertex position.

<p align="center">
<img src="https://i.imgur.com/mMsQFbE.gif" alt="Rim Light" title="Rim Light">
</p>

```c
  // ...

  float rimLightIntensity = dot(eye, normal);
        rimLightIntensity = 1.0 - rimLightIntensity;
        rimLightIntensity = max(0.0, rimLightIntensity);

  // ...
```

The Intensity of the rim light ranges from zero to one.
When the eye and normal vector point in the same direction,
the rim light intensity is zero.
As the two vectors start to point in different directions,
the rim light intensity increases
until it eventually reaches one when the eye and normal become orthogonal or perpendicular to one another.

<p align="center">
<img src="https://i.imgur.com/AAFI8p1.gif" alt="Rim Light Power" title="Rim Light Power">
</p>

```c
  // ...

  rimLightIntensity = pow(rimLightIntensity, rimLightPower);

  // ...
```

You can control the falloff of the rim light using the power function.

```c
  // ...

  rimLightIntensity = smoothstep(0.3, 0.4, rimLightIntensity)

  // ...
```

`step` or `smoothstep` can also be used to control the falloff.
This tends to look better when using [cel shading](cel-shading.md).
You'll learn more about these functions in later sections.

```c
  // ...

  vec4 rimLight   = rimLightIntensity * diffuse;
       rimLight.a = diffuse.a;

  // ...
```

What color you use for the rim light is up to you.
The demo code multiplies the diffuse light by the `rimLightIntensity`.
This will highlight the silhouette without overexposing it
and without lighting any shadowed fragments.

```c
  // ...

  vec4 outputColor     = vec4(0.0);
       outputColor.a   = diffuseColor.a;
       outputColor.rgb =
           ambient.rgb
        +  diffuse.rgb
        + specular.rgb
        + rimLight.rgb
        + emission.rgb;

  // ...
```

After you've calculated the rim light,
add it to the ambient, diffuse, specular, and emission lights.

### Source

- [main.cxx](../demonstration/src/main.cxx)
- [basic.vert](../demonstration/shaders/vertex/basic.vert)
- [base.frag](../demonstration/shaders/fragment/base.frag)

## Copyright

(C) 2020 David Lettier
<br>
[lettier.com](https://www.lettier.com)

[:arrow_backward:](fresnel-factor.md)
[:arrow_double_up:](../README.md)
[:arrow_up_small:](#)
[:arrow_down_small:](#copyright)
[:arrow_forward:](cel-shading.md)
