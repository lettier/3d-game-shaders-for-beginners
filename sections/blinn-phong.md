[:arrow_backward:](lighting.md)
[:arrow_double_up:](../README.md)
[:arrow_up_small:](#)
[:arrow_down_small:](#copyright)
[:arrow_forward:](fresnel-factor.md)

# 3D Game Shaders For Beginners

## Blinn-Phong

<p align="center">
<img src="https://i.imgur.com/CFWEeGK.gif" alt="Blinn-Phong" title="Blinn-Phong">
</p>

Blinn-Phong is a slight adjustment of the Phong model you saw in the [lighting](lighting.md) section.
It provides more plausible or realistic specular reflections.
You'll notice that Blinn-Phong produces elliptical or elongated specular reflections
versus the spherical specular reflections produced by the Phong model.
In certain cases, Blinn-Phong can be more efficient to calculate than Phong.

```c
  // ...

  vec3 light   = normal(lightPosition.xyz - vertexPosition.xyz);
  vec3 eye     = normalize(-vertexPosition.xyz);
  vec3 halfway = normalize(light + eye);

  // ...
```

Instead of computing the reflection vector,
compute the halfway or half angle vector.
This vector is between the view/camera/eye and light direction vector.

<p align="center">
<img src="https://i.imgur.com/vtqd1Ox.gif" alt="Blinn-Phong vs Phong" title="Blinn-Phong vs Phong">
</p>

```c
    // ...

    float specularIntensity = dot(normal, halfway);

    // ...
```

The specular intensity is now the dot product of the normal and halfway vector.
In the Phong model, it is the dot product of the reflection and view vector.

<p align="center">
<img src="https://i.imgur.com/WZQqxEH.png" alt="Full specular intensity." title="Full specular intensity.">
</p>

The half angle vector (magenta arrow) will point in the same direction as the normal (green arrow) when the
view vector (orange arrow) points in the same direction as the reflection vector (magenta arrow).
In this case, both the Blinn-Phong and Phong specular intensity will be one.

<p align="center">
<img src="https://i.imgur.com/kiSdJzt.png" alt="Blinn-Phong vs Phong" title="Blinn-Phong vs Phong">
</p>

In other cases, the specular intensity for Blinn-Phong will be greater than zero
while the specular intensity for Phong will be zero.

### Source

- [main.cxx](../demonstration/src/main.cxx)
- [base.vert](../demonstration/shaders/vertex/base.vert)
- [base.frag](../demonstration/shaders/fragment/base.frag)

## Copyright

(C) 2020 David Lettier
<br>
[lettier.com](https://www.lettier.com)

[:arrow_backward:](lighting.md)
[:arrow_double_up:](../README.md)
[:arrow_up_small:](#)
[:arrow_down_small:](#copyright)
[:arrow_forward:](fresnel-factor.md)
