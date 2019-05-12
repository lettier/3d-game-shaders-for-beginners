[:arrow_backward:](blinn-phong.md)
[:arrow_double_up:](../README.md)
[:arrow_up_small:](#)
[:arrow_down_small:](#copyright)
[:arrow_forward:](rim-lighting.md)

# 3D Game Shaders For Beginners

## Fresnel Factor

<p align="center">
<img src="https://i.imgur.com/3lQL51m.gif" alt="Fresnel" title="Fresnel">
</p>

The fresnel factor alters the reflectiveness of a surface based on the camera or viewing angle.
As a surface points away from the camera, its reflectiveness goes up.
Similarly, as a surface points towards the camera, its reflectiveness goes down.

<p align="center">
<img src="https://i.imgur.com/WolRRhX.png" alt="" title="">
</p>

In other words, as a surface becomes perpendicular with the camera, it becomes more mirror like.
Utilizing this property, you can vary the opacity of reflections
(such as [specular](lighting.md#specular) and [screen space reflections](screen-space-reflection.md))
and/or vary a surface's alpha values for a more plausible or realistic look.

### Specular Reflection

<p align="center">
<img src="https://i.imgur.com/FnOhXxv.gif" alt="Specular Intensity" title="Specular Intensity">
</p>

```c
  vec4 specular =   materialSpecularColor
                  * lightSpecularColor
                  * pow(max(dot(eye, reflection), 0.0), shininess);
```

In the [lighting](lighting.md#specular) section,
the specular component was a combination of the
material's specular color,
the light's specular color,
and by how much the camera pointed into the light's reflection direction.
Incorporating the fresnel factor,
you'll now vary the material specular color based on the angle between the camera and the surface it's pointed at.

```c
  // ...

  vec3 eye = normalize(-vertexPosition.xyz);

  // ...
```

The first vector you'll need is the eye/view/camera vector.
Recall that the eye vector points from the vertex position to the camera's position.
If the vertex position is in view or camera space,
the eye vector is the vertex position pointed in the opposite direction.

```c
  // ...

  vec3 light   = normal(lightPosition.xyz - vertexPosition.xyz);
  vec3 halfway = normalize(light + eye);

  // ...
```

The fresnel factor is calculated using two vectors.
The simplest two vectors to use are the eye and normal vector.
However, if you're using the halfway vector (from the [Blinn-Phong](blinn-phong.md) section),
you can instead calculate the fresnel factor using the halfway and eye vector.

```c

  // ...

  float fresnelFactor = dot(halfway, eye); // Or dot(normal, eye).
        fresnelFactor = max(fresnelFactor, 0.0);
        fresnelFactor = 1.0 - fresnelFactor;
        fresnelFactor = pow(fresnelFactor, fresnelPower);

  // ...
```

With the needed vectors in hand,
you can now compute the fresnel factor.
The fresnel factor ranges from zero to one.
When the dot product is one,
the fresnel factor is zero.
When the dot product is less than or equal to zero,
the fresnel factor is one.
This equation comes from
[Schlick's approximation](https://en.wikipedia.org/wiki/Schlick%27s_approximation).

<p align="center">
<img src="https://i.imgur.com/AAFI8p1.gif" alt="Fresnel Power" title="Fresnel Power">
</p>

In Schlick's approximation,
the `fresnelPower` is five but you can alter this to your liking.
The demo code varies it using the blue channel of the specular map with a maximum value of five.

```c
  // ...

  materialSpecularColor.rgb = mix(materialSpecularColor.rgb, vec3(1.0), fresnelFactor);

  // ...
```

Once the fresnel factor is determined,
use it to modulate the material's specular color.
As the fresnel factor approaches one,
the material becomes more like a mirror or fully reflective.

```c
  // ...

  vec4 specular      = vec4(vec3(0.0), 1.0);
       specular.rgb  =   materialSpecularColor.rgb
                       * lightSpecularColor.rgb
                       * pow
                          ( max(dot(normal, halfway), 0.0) // Or max(dot(reflection, eye), 0.0).
                          , shininess
                          );

  // ...
```

As before,
the specular component is a combination of the
material's specular color,
the light's specular color,
and by how much the camera points into the direction of the light's reflection.
However,
using the fresnel factor,
the material's specular color various depending on the orientation of the camera and the surface it's looking at.

### Source

- [main.cxx](../demonstration/src/main.cxx)
- [base.vert](../demonstration/shaders/vertex/base.vert)
- [base.frag](../demonstration/shaders/fragment/base.frag)

## Copyright

(C) 2020 David Lettier
<br>
[lettier.com](https://www.lettier.com)

[:arrow_backward:](blinn-phong.md)
[:arrow_double_up:](../README.md)
[:arrow_up_small:](#)
[:arrow_down_small:](#copyright)
[:arrow_forward:](rim-lighting.md)
