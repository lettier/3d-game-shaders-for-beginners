[:arrow_backward:](running-the-demo.md)
[:arrow_double_up:](../README.md)
[:arrow_up_small:](#)
[:arrow_down_small:](#copyright)
[:arrow_forward:](glsl.md)

# 3D Game Shaders For Beginners

## Reference Frames

Before you write any shaders, you should be familiar with the following frames of reference or coordinate systems.
All of them boil down to what origin `(0, 0, 0)` are these coordinates currently relative to?
Once you know that, you can then transform them, via some matrix, to some other vector space if need be.
Typically, when the output of some shader looks wrong, it's because of some coordinate system mix up.

### Model

<p align="center">
<img src="https://i.imgur.com/8xptajU.gif" alt="Model Space" title="Model Space">
</p>

The model or object coordinate system is relative to the origin of the model.
This is typically set to the center of the model's geometry in a modeling program like Blender.

### World

<p align="center">
<img src="https://i.imgur.com/fHl4ohX.gif" alt="World Space" title="World Space">
</p>

The world space is relative to the origin of the scene/level/universe that you've created.

### View

<p align="center">
<img src="https://i.imgur.com/3b4SGGH.gif" alt="View Space" title="View Space">
</p>

The view or eye coordinate space is relative to the position of the active camera.

### Clip

<p align="center">
<img src="https://i.imgur.com/iSEWS9Y.png" alt="Clip Space" title="Clip Space">
</p>

The clip space is relative to the center of the camera's film.
All coordinates are now homogeneous, ranging from negative one to one `(-1, 1)`.
X and y are parallel with the camera's film and the z coordinate is the depth.

<p align="center">
<img src="https://i.imgur.com/MhgmOLv.gif" alt="Frustum" title="Frustum">
</p>

Any vertex not within the bounds of the camera's frustum or view volume is clipped or discarded.
You can see this happening with the cube towards the back, clipped by the camera's far plane, and the cube off to the side.

### Screen

<p align="center">
<img src="https://i.imgur.com/bHHrjOl.png" alt="Screen Space" title="Screen Space">
</p>

The screen space is (typically) relative to the lower left corner of the screen.
X goes from zero to the screen width.
Y goes from zero to the screen height.

## Copyright

(C) 2019 David Lettier
<br>
[lettier.com](https://www.lettier.com)

[:arrow_backward:](running-the-demo.md)
[:arrow_double_up:](../README.md)
[:arrow_up_small:](#)
[:arrow_down_small:](#copyright)
[:arrow_forward:](glsl.md)
