[:arrow_backward:](reference-frames.md)
[:arrow_double_up:](../README.md)
[:arrow_up_small:](#)
[:arrow_down_small:](#copyright)
[:arrow_forward:](render-to-texture.md)

# 3D Game Shaders For Beginners

## GLSL

<p align="center">
<img src="https://i.imgur.com/7b5MCBG.gif" alt="" title="">
</p>

Instead of using the
[fixed-function](https://en.wikipedia.org/wiki/Fixed-function)
pipeline,
you'll be using the programmable GPU rendering pipeline.
Since it is programmable, it is up to you to supply the programming in the form of shaders.
A shader is a (typically small) program you write using a syntax reminiscent of C.
The programmable GPU rendering pipeline has various different stages that you can program with shaders.
The different types of shaders include vertex, tessellation, geometry, fragment, and compute.
You'll only need to focus on the vertex and fragment stages for the techniques below.

```c
#version 150

void main() {}
```

Here is a bare-bones GLSL shader consisting of the GLSL version number and the main function.

```c
#version 150

uniform mat4 p3d_ModelViewProjectionMatrix;

in vec4 p3d_Vertex;

void main()
{
  gl_Position = p3d_ModelViewProjectionMatrix * p3d_Vertex;
}
```

Here is a stripped down GLSL vertex shader that transforms an incoming vertex to clip space
and outputs this new position as the vertex's homogeneous position.
The `main` procedure doesn't return anything since it is `void` and the `gl_Position` variable is a built-in output.

Take note of the keywords `uniform` and `in`.
The `uniform` keyword means this global variable is the same for all vertexes.
Panda3D sets the `p3d_ModelViewProjectionMatrix` for you and it is the same matrix for each vertex.
The `in` keyword means this global variable is being given to the shader.
The vertex shader receives each vertex that makes up the geometry the vertex shader is attached to.

```c
#version 150

out vec4 fragColor;

void main() {
  fragColor = vec4(0, 1, 0, 1);
}
```

Here is a stripped down GLSL fragment shader that outputs the fragment color as solid green.
Keep in mind that a fragment affects at most one screen pixel but a single pixel can be affected by many fragments.

Take note of the `out` keyword.
The `out` keyword means this global variable is being set by the shader.
The name `fragColor` is arbitrary so feel free to choose a different one.

<p align="center">
<img src="https://i.imgur.com/V25UzMa.gif" alt="Output of the stripped down shaders." title="Output of the stripped down shaders.">
</p>

This is the output of the two shaders shown above.

## Copyright

(C) 2019 David Lettier
<br>
[lettier.com](https://www.lettier.com)

[:arrow_backward:](reference-frames.md)
[:arrow_double_up:](../README.md)
[:arrow_up_small:](#)
[:arrow_down_small:](#copyright)
[:arrow_forward:](render-to-texture.md)
