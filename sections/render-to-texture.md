[:arrow_backward:](glsl.md)
[:arrow_double_up:](../README.md)
[:arrow_up_small:](#)
[:arrow_down_small:](#copyright)
[:arrow_forward:](texturing.md)

# 3D Game Shaders For Beginners

## Render To Texture

Instead of rendering/drawing/painting directly to the screen, the example code uses a technique called "render to texture".
In order to render to a texture, you'll need to set up a framebuffer and bind a texture to it.
Multiple textures can be bound to a single framebuffer.

The textures bound to the framebuffer hold the vector(s) returned by the fragment shader.
Typically these vectors are color vectors `(r, g, b, a)` but they could also be position or normal vectors `(x, y, z, w)`.
For each bound texture, the fragment shader can output a different vector.
For example you could output a vertex's position and normal in a single pass.

Most of the example code dealing with Panda3D involves setting up
[framebuffer textures](https://www.panda3d.org/manual/?title=Render-to-Texture_and_Image_Postprocessing).
To keep things straightforward, each fragment shader in the example code has only one output.
However, you'll want to output as much as you can each render pass to keep your frames per second (FPS) high.

There are two framebuffer texture setups found in the example code.

![The first framebuffer texture setup.](https://i.imgur.com/t3iLKhx.gif)

The first setup renders the mill scene into a framebuffer texture using a variety of vertex and fragment shaders.
This setup will go through each of the mill scene's vertexes and corresponding fragments.

In this setup, the example code performs the following.

- Stores geometry data (like vertex position or normal) for later use.
- Stores material data (like the diffuse color) for later use.
- UV maps the various textures (diffuse, normal, shadow, etc.).
- Calculates the ambient, diffuse, specular, and emission lighting.

![The second framebuffer texture setup.](https://i.imgur.com/o8H6cTy.png)

The second setup is an orthographic camera pointed at a screen-shaped rectangle.
This setup will go through just the four vertexes and their corresponding fragments.

In this second setup, the example code performs the following.

- Manipulates the output of another framebuffer texture.
- Combines various framebuffer textures into one.

I like to think of this second setup as using layers in GIMP, Krita, or Inkscape.

![Tabbing Through Framebuffer Textures](https://i.imgur.com/L6Hwuxa.gif)

In the example code, you can see the output of a particular framebuffer texture
by using the <kbd>Tab</kbd> key or the <kbd>Shift</kbd>+<kbd>Tab</kbd> keys.

## Copyright

(C) 2019 David Lettier
<br>
[lettier.com](https://www.lettier.com)

[:arrow_backward:](glsl.md)
[:arrow_double_up:](../README.md)
[:arrow_up_small:](#)
[:arrow_down_small:](#copyright)
[:arrow_forward:](texturing.md)
