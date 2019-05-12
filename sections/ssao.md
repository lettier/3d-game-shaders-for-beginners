[:arrow_backward:](bloom.md)
[:arrow_double_up:](../README.md)
[:arrow_up_small:](#)
[:arrow_down_small:](#copyright)
[:arrow_forward:](motion-blur.md)

# 3D Game Shaders For Beginners

## Screen Space Ambient Occlusion (SSAO)

<p align="center">
<img src="https://i.imgur.com/o7lCukD.gif" alt="SSAO" title="SSAO">
</p>

SSAO is one of those effects you never knew you needed and can't live without once you have it.
It can take a scene from mediocre to wow!
For fairly static scenes, you can bake ambient occlusion into a texture but for more dynamic scenes, you'll need a shader.
SSAO is one of the more fairly involved shading techniques, but once you pull it off, you'll feel like a shader master.

By using only a handful of textures, SSAO can approximate the
[ambient occlusion](https://en.wikipedia.org/wiki/Ambient_occlusion)
of a scene.
This is faster than trying to compute the ambient occlusion by going through all of the scene's geometry.
These handful of textures all originate in screen space giving screen space ambient occlusion its name.

### Inputs

The SSAO shader will need the following inputs.

- Vertex position vectors in view space.
- Vertex normal vectors in view space.
- Sample vectors in tangent space.
- Noise vectors in tangent space.
- The camera lens' projection matrix.

### Vertex Positions

<p align="center">
<img src="https://i.imgur.com/gr7IxKv.png" alt="Panda3D Vertex Positions" title="Panda3D Vertex Positions">
</p>

Storing the vertex positions into a framebuffer texture is not a necessity.
You can recreate them from the [camera's depth buffer](http://theorangeduck.com/page/pure-depth-ssao).
This being a beginners guide, I'll avoid this optimization and keep it straight forward.
Feel free to use the depth buffer, however, for your implementation.

```cpp
PT(Texture) depthTexture =
  new Texture("depthTexture");
depthTexture->set_format
  ( Texture::Format::F_depth_component32
  );

PT(GraphicsOutput) depthBuffer =
  graphicsOutput->make_texture_buffer
    ( "depthBuffer"
    , 0
    , 0
    , depthTexture
    );
depthBuffer->set_clear_color
  ( LVecBase4f(0, 0, 0, 0)
  );

NodePath depthCameraNP =
  window->make_camera();
DCAST(Camera, depthCameraNP.node())->set_lens
  ( window->get_camera(0)->get_lens()
  );
PT(DisplayRegion) depthBufferRegion =
  depthBuffer->make_display_region
    ( 0
    , 1
    , 0
    , 1
    );
depthBufferRegion->set_camera(depthCameraNP);
```

If you do decide to use the depth buffer, here's how you can set it up using Panda3D.

```c
in vec4 vertexPosition;

out vec4 fragColor;

void main() {
  fragColor = vertexPosition;
}
```

Here's the simple shader used to render out the view space vertex positions into a framebuffer texture.
The more involved work is setting up the framebuffer texture such that the fragment vector components it receives
are not clamped to `[0, 1]` and that each one has a high enough precision (a high enough number of bits).
For example, if a particular interpolated vertex position is `<-139.444444566, 0.00000034343, 2.5>`,
you don't want it stored into the texture as `<0.0, 0.0, 1.0>`.

```c
  // ...

  FrameBufferProperties fbp = FrameBufferProperties::get_default();

  // ...

  fbp.set_rgba_bits(32, 32, 32, 32);
  fbp.set_rgb_color(true);
  fbp.set_float_color(true);

  // ...
```

Here's how the example code sets up the framebuffer texture to store the vertex positions.
It wants 32 bits per red, green, blue, and alpha components and disables clamping the values to `[0, 1]`
The `set_rgba_bits(32, 32, 32, 32)` call sets the bits and also disables the clamping.

```c
  glTexImage2D
    ( GL_TEXTURE_2D
    , 0
    , GL_RGB32F
    , 1200
    , 900
    , 0
    , GL_RGB
    , GL_FLOAT
    , nullptr
    );
```

Here's the equivalent OpenGL call.
`GL_RGB32F` sets the bits and also disables the clamping.

<blockquote>
If the color buffer is fixed-point, the components of the source and destination
values and blend factors are each clamped to [0, 1] or [−1, 1] respectively for
an unsigned normalized or signed normalized color buffer prior to evaluating the blend
equation.
If the color buffer is floating-point, no clamping occurs.
<br>
<br>
<footer>
<a href="https://www.khronos.org/registry/OpenGL/specs/gl/glspec44.core.pdf">Source</a>
</footer>
</blockquote>

<p align="center">
<img src="https://i.imgur.com/V4nETME.png" alt="OpenGL Vertex Positions" title="OpenGL Vertex Positions">
</p>

Here you see the vertex positions with y being the up vector.

Recall that Panda3D sets z as the up vector but OpenGL uses y as the up vector.
The position shader outputs the vertex positions with z being up since Panda3D
was configured with `gl-coordinate-system default`.

### Vertex Normals

<p align="center">
<img src="https://i.imgur.com/ilnbkzq.gif" alt="Panda3d Vertex Normals" title="Panda3d Vertex Normals">
</p>

You'll need the vertex normals to correctly orient the samples you'll take in the SSAO shader.
The example code generates multiple sample vectors distributed in a hemisphere
but you could use a sphere and do away with the need for normals all together.

```c
in vec3 vertexNormal;

out vec4 fragColor;

void main() {
  vec3 normal = normalize(vertexNormal);

  fragColor = vec4(normal, 1);
}
```

Like the position shader, the normal shader is simple as well.
Be sure to normalize the vertex normal and remember that they are in view space.

<p align="center">
<img src="https://i.imgur.com/ucdx9Kp.gif" alt="OpenGL Vertex Normals" title="OpenGL Vertex Normals">
</p>

Here you see the vertex normals with y being the up vector.

Recall that Panda3D sets z as the up vector but OpenGL uses y as the up vector.
The normal shader outputs the vertex positions with z being up since Panda3D
was configured with `gl-coordinate-system default`.

<p align="center">
<img src="https://i.imgur.com/fiHXBex.gif" alt="SSAO using the normal maps." title="SSAO using the normal maps.">
</p>

Here you see SSAO being used with the normal maps instead of the vertex normals.
This adds an extra level of detail and will pair nicely with the normal mapped lighting.

```c
    // ...

    normal =
      normalize
        ( normalTex.rgb
        * 2.0
        - 1.0
        );
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

To use the normal maps instead,
you'll need to transform the normal mapped normals from tangent space to view space
just like you did in the lighting calculations.

### Samples

To determine the amount of ambient occlusion for any particular fragment,
you'll need to sample the surrounding area.
The more samples you use, the better the approximation at the cost of performance.

```cpp
  // ...

  for (int i = 0; i < numberOfSamples; ++i) {
    LVecBase3f sample =
      LVecBase3f
        ( randomFloats(generator) * 2.0 - 1.0
        , randomFloats(generator) * 2.0 - 1.0
        , randomFloats(generator)
        ).normalized();

    float rand = randomFloats(generator);
    sample[0] *= rand;
    sample[1] *= rand;
    sample[2] *= rand;

    float scale = (float) i / (float) numberOfSamples;
    scale = lerp(0.1, 1.0, scale * scale);
    sample[0] *= scale;
    sample[1] *= scale;
    sample[2] *= scale;

    ssaoSamples.push_back(sample);
  }

  // ...
```

The example code generates a number of random samples distributed in a hemisphere.
These `ssaoSamples` will be sent to the SSAO shader.

```cpp
    LVecBase3f sample =
      LVecBase3f
        ( randomFloats(generator) * 2.0 - 1.0
        , randomFloats(generator) * 2.0 - 1.0
        , randomFloats(generator) * 2.0 - 1.0
        ).normalized();
```

If you'd like to distribute your samples in a sphere instead,
change the random `z` component to range from negative one to one.

### Noise

```c
  // ...

  for (int i = 0; i < numberOfNoise; ++i) {
    LVecBase3f noise =
      LVecBase3f
        ( randomFloats(generator) * 2.0 - 1.0
        , randomFloats(generator) * 2.0 - 1.0
        , 0.0
        );

    ssaoNoise.push_back(noise);
  }

  // ...
```

To get a good sweep of the sampled area, you'll need to generate some noise vectors.
These noise vectors will randomly tilt the hemisphere around the current fragment.

### Ambient Occlusion

<p align="center">
<img src="https://i.imgur.com/KKt74VE.gif" alt="SSAO Texture" title="SSAO Texture">
</p>

SSAO works by sampling the view space around a fragment.
The more samples that are below a surface, the darker the fragment color.
These samples are positioned at the fragment and pointed in the general direction of the vertex normal.
Each sample is used to look up a position in the position framebuffer texture.
The position returned is compared to the sample.
If the sample is farther away from the camera than the position, the sample counts towards the fragment being occluded.

<p align="center">
<img src="https://i.imgur.com/Nm4CJDN.gif" alt="SSAO Sampling" title="SSAO Sampling">
</p>

Here you see the space above the surface being sampled for occlusion.

```c
  // ...

  float radius    = 1;
  float bias      = 0.01;
  float magnitude = 1.5;
  float contrast  = 1.5;

  // ...
```

Like some of the other techniques,
the SSAO shader has a few control knobs you can tweak to get the exact look you're going for.
The `bias` adds to the sample's distance from the camera.
You can use the bias to combat "acne".
The `radius` increases or decreases the coverage area of the sample space.
The `magnitude` either lightens or darkens the occlusion map.
The `contrast` either washes out or increases the starkness of the occlusion map.

```c
  // ...

  vec4 position =           texture(positionTexture, texCoord);
  vec3 normal   = normalize(texture(normalTexture,   texCoord).xyz);

  int  noiseX = int(gl_FragCoord.x - 0.5) % 4;
  int  noiseY = int(gl_FragCoord.y - 0.5) % 4;
  vec3 random = noise[noiseX + (noiseY * 4)];

  // ...
```

Retrieve the position, normal, and random vector for later use.
Recall that the example code created a set number of random vectors.
The random vector is chosen based on the current fragment's screen position.

```c
  // ...

  vec3 tangent  = normalize(random - normal * dot(random, normal));
  vec3 binormal = cross(normal, tangent);
  mat3 tbn      = mat3(tangent, binormal, normal);

  // ...
```

Using the random and normal vectors, assemble the tangent, binormal, and normal matrix.
You'll need this matrix to transform the sample vectors from tangent space to view space.

```c
  // ...

  float occlusion = NUM_SAMPLES;

  for (int i = 0; i < NUM_SAMPLES; ++i) {
    // ...
  }

  // ...
```

With the matrix in hand, the shader can now loop through the samples, subtracting how many are not occluded.

```c
    // ...

    vec3 samplePosition = tbn * samples[i];
         samplePosition = position.xyz + samplePosition * radius;

    // ...
```

Using the matrix, position the sample near the vertex/fragment position and scale it by the radius.

```c
    // ...

    vec4 offsetUV      = vec4(samplePosition, 1.0);
         offsetUV      = lensProjection * offsetUV;
         offsetUV.xyz /= offsetUV.w;
         offsetUV.xy   = offsetUV.xy * 0.5 + 0.5;

    // ...
```

Using the sample's position in view space, transform it from view space to clip space to UV space.

```c
-1 * 0.5 + 0.5 = 0
 1 * 0.5 + 0.5 = 1
```

Recall that clip space components range from negative one to one and that UV coordinates range from zero to one.
To transform clip space coordinates to UV coordinates, multiply by one half and add one half.

```c
    // ...

    vec4 offsetPosition = texture(positionTexture, offsetUV.xy);

    float occluded = 0;
    if (samplePosition.y + bias <= offsetPosition.y) { occluded = 0; } else { occluded = 1; }

    // ...
```

Using the offset UV coordinates,
created by projecting the 3D sample onto the 2D position texture,
find the corresponding position vector.
This takes you from view space to clip space to UV space back to view space.
The shader takes this round trip to find out if some geometry is behind, at, or in front of this sample.
If the sample is in front of or at some geometry, this sample doesn't count towards the fragment being occluded.
If the sample is behind some geometry, this sample counts towards the fragment being occluded.

```c
    // ...

    float intensity =
      smoothstep
        ( 0.0
        , 1.0
        ,   radius
          / abs(position.y - offsetPosition.y)
        );
    occluded *= intensity;

    occlusion -= occluded;

    // ...
```

Now weight this sampled position by how far it is inside or outside the radius.
Finally, subtract this sample from the occlusion factor since it assumes all of the samples are occluded before the loop.

```c
    // ...

    occlusion /= NUM_SAMPLES;

    // ...

    fragColor = vec4(vec3(occlusion), position.a);

    // ...
```

Divide the occluded count by the number of samples to scale the occlusion factor from `[0, NUM_SAMPLES]` to `[0, 1]`.
Zero means full occlusion and one means no occlusion.
Now assign the occlusion factor to the fragment's color and you're done.

```c
    // ...

    fragColor = vec4(vec3(occlusion), position.a);

    // ...
```

For the demo's purposes,
the example code sets the alpha channel to alpha channel of the position framebuffer texture to avoid covering up the background.

### Blurring

<p align="center">
<img src="https://i.imgur.com/QsqOhFR.gif" alt="SSAO Blur Texture" title="SSAO Blur Texture">
</p>

The SSAO framebuffer texture is noisy as is.
You'll want to blur it to remove the noise.
Refer back to the section on [blurring](blur.md).
For the best results, use a median or Kuwahara filter to preserve the sharp edges.

### Ambient Color

```c
  // ...

  vec2 ssaoBlurTexSize  = textureSize(ssaoBlurTexture, 0).xy;
  vec2 ssaoBlurTexCoord = gl_FragCoord.xy / ssaoBlurTexSize;
  float ssao            = texture(ssaoBlurTexture, ssaoBlurTexCoord).r;

  vec4 ambient = p3d_Material.ambient * p3d_LightModel.ambient * diffuseTex * ssao;

  // ...
```

The final stop for SSAO is back in the lighting calculation.
Here you see the occlusion factor being looked up in the
SSAO framebuffer texture and then included in the ambient light calculation.

### Source

- [main.cxx](../demonstration/src/main.cxx)
- [basic.vert](../demonstration/shaders/vertex/basic.vert)
- [base.vert](../demonstration/shaders/vertex/base.vert)
- [base.frag](../demonstration/shaders/fragment/base.frag)
- [position.frag](../demonstration/shaders/fragment/position.frag)
- [normal.frag](../demonstration/shaders/fragment/normal.frag)
- [ssao.frag](../demonstration/shaders/fragment/ssao.frag)
- [median-filter.frag](../demonstration/shaders/fragment/median-filter.frag)
- [kuwahara-filter.frag](../demonstration/shaders/fragment/kuwahara-filter.frag)

## Copyright

(C) 2019 David Lettier
<br>
[lettier.com](https://www.lettier.com)

[:arrow_backward:](bloom.md)
[:arrow_double_up:](../README.md)
[:arrow_up_small:](#)
[:arrow_down_small:](#copyright)
[:arrow_forward:](motion-blur.md)
