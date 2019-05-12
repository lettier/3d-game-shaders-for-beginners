[:arrow_backward:](gamma-correction.md)
[:arrow_double_up:](../README.md)
[:arrow_up_small:](#)
[:arrow_down_small:](#copyright)
[:arrow_forward:](building-the-demo.md)

# 3D Game Shaders For Beginners

## Setup

<p align="center">
<img src="https://i.imgur.com/fYpIWNk.gif" alt="Setup" title="Setup">
</p>

Below is the setup used to develop and test the example code.

### Environment

The example code was developed and tested using the following environment.

- Linux manjaro 4.9.135-1-MANJARO
- OpenGL renderer string: GeForce GTX 970/PCIe/SSE2
- OpenGL version string: 4.6.0 NVIDIA 410.73
- g++ (GCC) 8.2.1 20180831
- Panda3D 1.10.1-1

### Materials

Each [Blender](https://blender.org) material used to build `mill-scene.egg` has five textures
in the following order.

- Diffuse
- Normal
- Specular
- Reflection
- Refraction

By having the same maps in the same positions for all models,
the shaders can be generalized, reducing the need to duplicate code.

If an object uses its vertex normals, a "flat blue" normal map is used.

<p align="center">
<img src="https://i.imgur.com/tFmKgoH.png" alt="A flat normal map." title="A flat normal map.">
</p>

Here is an example of a flat normal map.
The only color it contains is flat blue `(red = 128, green = 128, blue = 255)`.
This color represents a unit (length one) normal pointing in the positive z-axis `(0, 0, 1)`.

```c
(0, 0, 1) =
  ( round((0 * 0.5 + 0.5) * 255)
  , round((0 * 0.5 + 0.5) * 255)
  , round((1 * 0.5 + 0.5) * 255)
  ) =
    (128, 128, 255) =
      ( round(128 / 255 * 2 - 1)
      , round(128 / 255 * 2 - 1)
      , round(255 / 255 * 2 - 1)
      ) =
        (0, 0, 1)
```

Here you see the unit normal `(0, 0, 1)`
converted to flat blue `(128, 128, 255)`
and flat blue converted to the unit normal.
You'll learn more about this in the [normal mapping](normal-mapping.md) technique.

<p align="center">
<img src="https://i.imgur.com/R9FgZKx.png" alt="Specular Map" title="Specular Map">
</p>

Up above is one of the specular maps used.
The red and blue channel work to control the amount of specular reflection seen based on the camera angle.
The green channel controls the shininess factor.
You'll learn more about this in the [lighting](lighting.md) and [fresnel factor](fresnel-factor.md) sections.

The reflection and refraction textures mask off the objects that are either reflective, refractive, or both.
For the reflection texture, the red channel controls the amount of reflection and the green channel controls how
clear or blurry the reflection is.

### Panda3D

The example code uses
[Panda3D](https://www.panda3d.org/)
as the glue between the shaders.
This has no real influence over the techniques below,
meaning you'll be able to take what you learn here and apply it to your stack or game engine of choice.
Panda3D does provide some conveniences.
I have pointed these out so you can either find an equivalent convenience provided by your stack or
replicate it yourself, if your stack doesn't provide something equivalent.

Three Panda3D configurations were changed for the purposes of the demo program.
You can find these in [config.prc](../demonstration/config.prc).
The configurations changed were
`gl-coordinate-system default`,
`textures-power-2 down`, and
`textures-auto-power-2 1`.
Refer to the
[Panda3D configuration](http://www.panda3d.org/manual/?title=Configuring_Panda3D)
page in the manual for more details.

Panda3D defaults to a z-up, right-handed coordinate system while OpenGL uses a y-up, right-handed system.
`gl-coordinate-system default` keeps you from having to translate between the two inside your shaders.
`textures-auto-power-2 1` allows us to use texture sizes that are not a power of two if the system supports it.
This comes in handy when doing SSAO and other screen/window sized related techniques since the screen/window size
is usually not a power of two.
`textures-power-2 down` downsizes our textures to a power of two if the system only supports texture sizes being a power of two.

## Copyright

(C) 2019 David Lettier
<br>
[lettier.com](https://www.lettier.com)

[:arrow_backward:](gamma-correction.md)
[:arrow_double_up:](../README.md)
[:arrow_up_small:](#)
[:arrow_down_small:](#copyright)
[:arrow_forward:](building-the-demo.md)
