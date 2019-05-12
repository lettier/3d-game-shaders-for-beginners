[:arrow_backward:](rim-lighting.md)
[:arrow_double_up:](../README.md)
[:arrow_up_small:](#)
[:arrow_down_small:](#copyright)
[:arrow_forward:](normal-mapping.md)

# 3D Game Shaders For Beginners

## Cel Shading

<p align="center">
<img src="https://i.imgur.com/W80Ke1y.gif" alt="Cel Shaded" title="Cel Shaded">
</p>

Cel shading is a technique to make 3D objects look 2D or flat.
In 2D,
you can make an object look 3D by applying a smooth gradient.
However, with cel shading, you're breaking up the gradients into abrupt transitions.
Typically there is only one transition where the shading goes from fully lit to fully shadowed.
When combined with [outlining](outlining.md), cel shading can really sell the 2D cartoon look.

## Diffuse

```c
    // ...

    float diffuseIntensity = max(dot(normal, unitLightDirection), 0.0);
          diffuseIntensity = step(0.1, diffuseIntensity);

    // ...
```

Revisiting the [lighting](lighting.md#diffuse) model,
modify the `diffuseIntensity` such that it is either zero or one.

<p align="center">
<img src="https://i.imgur.com/lyLweFc.png" alt="Step Function" title="Step Function">
</p>

The `step` function returns zero if the input is less than the edge and one otherwise.

<p align="center">
<img src="https://i.imgur.com/EI6QJ60.png" alt="Steps Function" title="Steps Function">
</p>

```c
  // ...

  if      (diffuseIntensity >= 0.8) { diffuseIntensity = 1.0; }
  else if (diffuseIntensity >= 0.6) { diffuseIntensity = 0.6; }
  else if (diffuseIntensity >= 0.3) { diffuseIntensity = 0.3; }
  else                              { diffuseIntensity = 0.0; }

  // ...
```

If you would like to have a few steps or transitions,
you can perform something like the above.

<p align="center">
<img src="https://i.imgur.com/7KK65mi.png" alt="Step Texture" title="Step Texture">
</p>

```c
  // ...

  diffuseIntensity = texture(steps, vec2(diffuseIntensity, 0.0)).r;

  // ...
```

Another approach is to put your step values into a texture with the transitions going from darker to lighter.
Using the `diffuseIntensity` as a U coordinate, it will automatically transform itself.

## Specular

```c

      // ...

      float specularIntensity = clamp(dot(normal, halfwayDirection), 0.0, 1.0);
            specularIntensity = step(0.98, specularIntensity);

      // ...
```

Using the `step` function again, set the `specularIntensity` to be either zero or one.
You can also use one of the other approaches described up above for the specular highlight as well.
After you've altered the `specularIntensity`, the rest of the lighting calculations are the same.

### Source

- [main.cxx](../demonstration/src/main.cxx)
- [base.vert](../demonstration/shaders/vertex/base.vert)
- [base.frag](../demonstration/shaders/fragment/base.frag)

## Copyright

(C) 2020 David Lettier
<br>
[lettier.com](https://www.lettier.com)

[:arrow_backward:](rim-lighting.md)
[:arrow_double_up:](../README.md)
[:arrow_up_small:](#)
[:arrow_down_small:](#copyright)
[:arrow_forward:](normal-mapping.md)
