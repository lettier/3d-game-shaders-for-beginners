[:arrow_backward:](lighting.md)
[:arrow_double_up:](../README.md)
[:arrow_up_small:](#)
[:arrow_down_small:](#copyright)
[:arrow_forward:](deferred-rendering.md)

# 3D Game Shaders For Beginners

## Normal Mapping

![Normal Mapping](https://i.imgur.com/7O6SG2g.gif)

Normal mapping allows you to add surface details without adding any geometry.
Typically, in a modeling program like Blender, you create a high poly and a low poly version of your mesh.
You take the vertex normals from the high poly mesh and bake them into a texture.
This texture is the normal map.
Then inside the fragment shader, you replace the low poly mesh's vertex normals with the
high poly mesh's normals you baked into the normal map.
Now when you light your mesh, it will appear to have more polygons than it really has.
This will keep your FPS high while at the same time retain most of the details from the high poly version.

![From high to low poly with normal mapping.](https://i.imgur.com/nSY9AW4.gif)

Here you see the progression from the high poly model to the low poly model to the low poly model with the normal map applied.

![Normal Map Illusion](https://i.imgur.com/jvkRPE7.gif)

Keep in mind though, normal mapping is only an illusion.
After a certain angle, the surface will look flat again.

### Vertex

```c
// ...

uniform mat3 p3d_NormalMatrix;

// ...

in vec3 p3d_Normal;

// ...

in vec3 p3d_Binormal;
in vec3 p3d_Tangent;

  // ...

  vertexNormal = normalize(p3d_NormalMatrix * p3d_Normal);
  binormal     = normalize(p3d_NormalMatrix * p3d_Binormal);
  tangent      = normalize(p3d_NormalMatrix * p3d_Tangent);

  // ...
```

Starting in the vertex shader,
you'll need to output to the fragment shader the normal vector, binormal vector, and the tangent vector.
These vectors are used, in the fragment shader, to transform the normal map normal from tangent space to view space.

`p3d_NormalMatrix` transforms the vertex normal, binormal, and tangent vectors to view space.
Remember that in view space, all of the coordinates are relative to the camera's position.

<blockquote>
[p3d_NormalMatrix] is the upper 3x3 of the inverse transpose of the ModelViewMatrix.
It is used to transform the normal vector into view-space coordinates.
<br>
<br>
<footer>
<a href="http://www.panda3d.org/manual/?title=List_of_GLSL_Shader_Inputs">Source</a>
</footer>
</blockquote>

```c
// ...

in vec2 p3d_MultiTexCoord0;

// ...

out vec2 normalCoord;

  // ...

  normalCoord   = p3d_MultiTexCoord0;

  // ...
```

![Normal Maps](https://i.imgur.com/tLIA6Hu.gif)

You'll also need to output, to the fragment shader, the UV coordinates for the normal map.

### Fragment

Recall that the vertex normal was used to calculate the lighting.
However, the normal map provides us with different normals to use when calculating the lighting.
In the fragment shader, you need to swap out the vertex normals for the normals found in the normal map.

```c
// ...

uniform sampler2D p3d_Texture0;

// ...

in vec2 normalCoord;

  // ...

  /* Find */
  vec4 normalTex   = texture(p3d_Texture0, normalCoord);

  // ...
```

Using the normal map coordinates the vertex shader sent, pull out the normal from the normal map.

```c
  // ...

  vec3 normal;

    // ...

    /* Unpack */
    normal =
      normalize
        ( normalTex.rgb
        * 2.0
        - 1.0
        );

    // ...
```

Earlier I showed how the normals are mapped to colors to create the normal map.
Now this process needs to be reversed so you can get back the original normals that were baked into the map.

```c
(r, g, b) =
  ( r * 2 - 1
  , g * 2 - 1
  , b * 2 - 1
  ) =
    (x, y, z)
```

Here's the process for unpacking the normals from the normal map.

```c
    // ...

    /* Transform */
    normal =
      normalize
        ( mat3
            ( tangent
            , binormal
            , vertexNormal
            )
        * normal
        );

    // ...
```

The normals you get back from the normal map are typically in tangent space.
They could be in another space, however.
For example, Blender allows you to bake the normals in tangent, object, world, or camera space.

![Replacing the vertex normals with the normal map normals.](https://i.imgur.com/EzHJPd4.gif)

To take the normal map normal from tangent space to view pace,
construct a three by three matrix using the tangent, binormal, and vertex normal vectors.
Multiply the normal by this matrix and be sure to normalize it.

At this point, you're done.
The rest of the lighting calculations are the same.

### Source

- [main.cxx](../demo/src/main.cxx)
- [base.vert](../demo/shaders/vertex/base.vert)
- [base.frag](../demo/shaders/fragment/base.frag)

## Copyright

(C) 2019 David Lettier
<br>
[lettier.com](https://www.lettier.com)

[:arrow_backward:](lighting.md)
[:arrow_double_up:](../README.md)
[:arrow_up_small:](#)
[:arrow_down_small:](#copyright)
[:arrow_forward:](deferred-rendering.md)
