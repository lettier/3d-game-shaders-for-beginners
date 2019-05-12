[:arrow_backward:](render-to-texture.md)
[:arrow_double_up:](../README.md)
[:arrow_up_small:](#)
[:arrow_down_small:](#copyright)
[:arrow_forward:](lighting.md)

# 3D Game Shaders For Beginners

## Texturing

<p align="center">
<img src="https://i.imgur.com/cqbgT8b.gif" alt="Diffuse Texture Only" title="Diffuse Texture Only">
</p>

Texturing involves mapping some color or some other kind of vector to a fragment using UV coordinates.
Both U and V range from zero to one.
Each vertex gets a UV coordinate and this is outputted in the vertex shader.

<p align="center">
<img src="https://i.imgur.com/JjAdNfk.png" alt="UV Interpolation" title="UV Interpolation">
</p>

The fragment shader receives the UV coordinate interpolated.
Interpolated meaning the UV coordinate for the fragment is somewhere between the UV coordinates
for the vertexes that make up the triangle face.

### Vertex

```c
#version 150

uniform mat4 p3d_ModelViewProjectionMatrix;

in vec2 p3d_MultiTexCoord0;

in vec4 p3d_Vertex;

out vec2 texCoord;

void main()
{
  texCoord = p3d_MultiTexCoord0;

  gl_Position = p3d_ModelViewProjectionMatrix * p3d_Vertex;
}
```

Here you see the vertex shader outputting the texture coordinate to the fragment shader.
Notice how it's a two dimensional vector.
One dimension for U and one for V.

### Fragment

```c
#version 150

uniform sampler2D p3d_Texture0;

in vec2 texCoord;

out vec2 fragColor;

void main()
{
  texColor = texture(p3d_Texture0, texCoord);

  fragColor = texColor;
}
```

Here you see the fragment shader looking up the color at its UV coordinate and outputting that as the fragment color.

#### Screen Filled Texture

```c
#version 150

uniform sampler2D screenSizedTexture;

out vec2 fragColor;

void main()
{
  vec2 texSize  = textureSize(texture, 0).xy;
  vec2 texCoord = gl_FragCoord.xy / texSize;

  texColor = texture(screenSizedTexture, texCoord);

  fragColor = texColor;
}
```

When performing render to texture, the mesh is a flat rectangle with the same aspect ratio as the screen.
Because of this, you can calculate the UV coordinates knowing only
A) the width and height of the screen sized texture being UV mapped to the rectangle and
B) the fragment's x and y coordinate.
To map x to U, divide x by the width of the input texture.
Similarly, to map y to V, divide y by the height of the input texture.
You'll see this technique used in the example code.

## Copyright

(C) 2019 David Lettier
<br>
[lettier.com](https://www.lettier.com)

[:arrow_backward:](render-to-texture.md)
[:arrow_double_up:](../README.md)
[:arrow_up_small:](#)
[:arrow_down_small:](#copyright)
[:arrow_forward:](lighting.md)
