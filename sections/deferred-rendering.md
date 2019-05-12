[:arrow_backward:](normal-mapping.md)
[:arrow_double_up:](../README.md)
[:arrow_up_small:](#)
[:arrow_down_small:](#copyright)
[:arrow_forward:](outlining.md)

# 3D Game Shaders For Beginners

## Deferred Rendering

![Deferred vs Forward](https://i.imgur.com/rxTYLGv.png)

Deferred rendering (deferred shading) is a screen space lighting technique.
Instead of calculating the lighting for a scene while you traverse its
geometry—you defer or wait to perform the lighting calculations until
after the scene's geometry fragments have been culled or discarded.
This can give you a performance boost depending on the complexity of your scene.

### Phases

![G-buffer](https://i.imgur.com/uTimQJj.gif)

Deferred rendering is performed in two phases.
The first phase involves going through the scene's geometry and rendering its
positions or depths,
normals,
and materials
into a framebuffer known as the geometry buffer or G-buffer.
With the exception of some transformations,
this is mostly a read-only phase so its performance cost is minimal.
After this phase, you're only dealing with 2D textures in the shape of the screen.

![Lighting Phase](https://i.imgur.com/frYp6to.gif)

The second and last phase is where you perform your lighting calculations using the output of the first phase.
This is when you calculate the ambient, diffuse, and specular colors.
Shadow and normal mapping are performed in this phase as well.

### Advantages

The reason for using deferred rendering is to reduce the number of lighting calculations made.
With forward rendering, the number of lighting calculations scales with the number of fragments and lights.
However, with deferred shading, the number of lighting calculations scales with the number of pixels and lights.
Recall that for a single pixel, there can be multiple fragments produced.
As you add geometry,
the number of lighting calculations per pixel increases when using forward but not when using deferred.

For simple scenes,
deferred rendering doesn't provide much of a performance boost and may even hurt performance.
However, for complex scenes with lots of lighting, it becomes the better option.
Deferred becomes faster than forward because you're only calculating the lighting per light, per pixel.
In forward rendering,
you're calculating the lighting per light per fragment which can be multiple times per pixel.

### Disadvantages

Deferred rendering allows you render complex scenes using many lights but it does come with its own set of tradeoffs.
Transparency becomes an issue because the geometry data you could see through a semitransparent object is discarded in the first phase.
Other tradesoffs include increased memory consumption due to the G-buffer and the extra workarounds needed to deal with aliasing.

## Copyright

(C) 2019 David Lettier
<br>
[lettier.com](https://www.lettier.com)

[:arrow_backward:](normal-mapping.md)
[:arrow_double_up:](../README.md)
[:arrow_up_small:](#)
[:arrow_down_small:](#copyright)
[:arrow_forward:](outlining.md)
