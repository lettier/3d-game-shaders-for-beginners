[:arrow_backward:](fog.md)
[:arrow_double_up:](../README.md)
[:arrow_up_small:](#)
[:arrow_down_small:](#copyright)
[:arrow_forward:](bloom.md)

# 3D Game Shaders For Beginners

## Blur

<p align="center">
<img src="https://i.imgur.com/b5vw2AJ.gif" alt="Kuwahara Filter" title="Kuwahara Filter">
</p>

The need to blur this or that can come up quite often as you try to obtain
a particular look or perform some technique like motion blur.
Below are just some of ways you can blur your game's imagery.

### Box Blur

<p align="center">
<img src="https://i.imgur.com/uaXC1JM.gif" alt="Box Blur" title="Box Blur">
</p>

The box blur or mean filter algorithm is a simple to implement blurring effect.
It's fast and gets the job done.
If you need more finesse, you can upgrade to a Gaussian blur.

```c
  // ...

  vec2 texSize  = textureSize(colorTexture, 0).xy;
  vec2 texCoord = gl_FragCoord.xy / texSize;

  int size  = int(parameters.x);
  if (size <= 0) { fragColor = texture(colorTexture, texCoord); return; }

  // ...
```

The `size` parameter controls how blurry the result is.
If the `size` is zero or less, return the fragment untouched.

```c
  // ...

  float separation = parameters.y;
        separation = max(separation, 1);

  // ...
```

The `separation` parameter spreads out the blur without having to sample additional fragments.
`separation` ranges from one to infinity.

<p align="center">
<img src="https://i.imgur.com/bRw0OkX.png" alt="Blur Kernel" title="Blur Kernel">
</p>

```c
  // ...

  for (int i = -size; i <= size; ++i) {
    for (int j = -size; j <= size; ++j) {
      // ...
    }
  }

  // ...
```

Like the [outlining](outlining.md) technique,
the box blur technique uses a kernel/matrix/window centered around the current fragment.
The size of the window is `size * 2 + 1` by `size * 2 + 1`.
So for example, with a `size` setting of two, the window uses `(2 * 2 + 1)^2 = 25` samples per fragment.

```c
      // ...

      fragColor +=
        texture
          ( colorTexture
          ,   ( gl_FragCoord.xy
              + (vec2(i, j) * separation)
              )
            / texSize
          );

      // ...
```

To compute the mean or average of the samples in the window,
start by loop through the window, adding up each color vector.


```c
  // ...

  fragColor /= pow(size * 2 + 1, 2);

  // ...
```

To finish computing the mean, divide the sum of the colors sampled by the number of samples taken.
The final fragment color is the mean or average of the fragments sampled inside the window.

### Median Filter

<p align="center">
<img src="https://i.imgur.com/T1nEEn3.gif" alt="Median Filter" title="Median Filter">
</p>

The box blur uses the mean color of the samples taken.
The median filter uses the median color of the samples taken.
By using the median instead of the mean,
the edges in the image are preserved—meaning the edges stay nice and crisp.
For example, look at the windows in the box blurred image versus the median filtered image.

Unfortunately, finding the median can be slower than finding the mean.
You could sort the values and choose the middle one but that would take at least quasilinear time.
There is a technique to find the median in linear time but it can be quite awkward inside a shader.
The numerical approach below approximates the median in linear time.
How well it approximates the median can be controlled.

<p align="center">
<img src="https://i.imgur.com/AnbzUmN.png" alt="Painterly" title="Painterly">
</p>

At lower quality approximations,
you end up with a nice [painterly](https://en.wikipedia.org/wiki/Painterliness) look.

```c
// ...

#define MAX_SIZE        4
#define MAX_KERNEL_SIZE ((MAX_SIZE * 2 + 1) * (MAX_SIZE * 2 + 1))
#define MAX_BINS_SIZE   100

// ...
```

These are the hard limits for the `size` parameter, window size, and `bins` array.


```c
  // ...

  vec2 texSize  = textureSize(colorTexture, 0).xy;
  vec2 texCoord = gl_FragCoord.xy / texSize;

  int size = int(parameters.x);
  if (size <= 0) { fragColor = texture(colorTexture, texCoord); return; }
  if (size > MAX_SIZE) { size = MAX_SIZE; }
  int kernelSize = int(pow(size * 2 + 1, 2));

  // ...
```

The `size` parameter controls how blurry or smeared the effect is.
If the size is at or below zero, return the current fragment untouched.
From the `size` parameter, calculate the total size of the kernel or window.
This is how many samples you'll be taking per fragment.

```c
  // ...

  int binsSize = int(parameters.y);
      binsSize = clamp(binsSize, 1, MAX_BINS_SIZE);

  // ...
```

Set up the `binsSize`, making sure to limit it by the `MAX_BINS_SIZE`.

```c
  // ...

  int i        = 0;
  int j        = 0;
  int count    = 0;
  int binIndex = 0;

  // ...
```

`i` and `j` are used to sample the given texture around the current fragment.
`i` is also used as a general for loop count.
`count` is used in the initialization of the `colors` array which you'll see later.
`binIndex` is used to approximate the median color.

```c
  // ...

  vec4  colors[MAX_KERNEL_SIZE];
  float bins[MAX_BINS_SIZE];
  int   binIndexes[colors.length()];

  // ...
```

The `colors` array holds the sampled colors taken from the input texture.
`bins` is used to approximate the median of the sampled colors.
Each bin holds a count of how many colors fall into its range when converting each color into a greyscale value (between zero and one).
As `binsSize` approaches 100, the algorithm finds the true median almost always.
`binIndexes` stores the `bins` index or which bin each sample falls into.

```c
  // ...

  float total = 0;
  float limit = floor(float(kernelSize) / 2) + 1;

  // ...
```

`total` keeps track of how many colors you've come across as you loop through `bins`.
When `total` reaches `limit`, you return whatever `bins` index you're at.
The `limit` is the median index.
For example, if the window size is 81, `limit` is 41 which is directly in the middle (40 samples below and 40 samples above).

```c
  // ...

  float value       = 0;
  vec3  valueRatios = vec3(0.3, 0.59, 0.11);

  // ...
```

These are used to covert and hold each color sample's greyscale value.
Instead of dividing red, green, and blue by one third,
it uses 30% of red, 59% of green, and 11% of blue for a total of 100%.

```c
  // ...

  for (i = -size; i <= size; ++i) {
    for (j = -size; j <= size; ++j) {
      colors[count] =
        texture
          ( colorTexture
          ,   ( gl_FragCoord.xy
              + vec2(i, j)
              )
            / texSize
          );
      count += 1;
    }
  }

  // ...
```

Loop through the window and collect the color samples into `colors`.

```c
  // ...

  for (i = 0; i < binsSize; ++i) {
    bins[i] = 0;
  }

  // ...
```

Initialize the `bins` array with zeros.

```c
  // ...

  for (i = 0; i < kernelSize; ++i) {
    value           = dot(colors[i].rgb, valueRatios);
    binIndex        = int(floor(value * binsSize));
    binIndex        = clamp(binIndex, 0, binsSize - 1);
    bins[binIndex] += 1;
    binIndexes[i]   = binIndex;
  }

  // ...
```

Loop through the colors and convert each one to a greyscale value.
`dot(colors[i].rgb, valueRatios)` is the weighted sum `colors.r * 0.3 + colors.g * 0.59 + colors.b * 0.11`.

Each value will fall into some bin.
Each bin covers some range of values.
For example, if the number of bins is 10, the first bin covers everything from zero up to but not including 0.1.
Increment the number of colors that fall into this bin and remember the color sample's bin index so you can look it up later.

```c
  // ...

  binIndex = 0;

  for (i = 0; i < binsSize; ++i) {
    total += bins[i];
    if (total >= limit) {
      binIndex = i;
      break;
    }
  }

  // ...
```

Loop through the bins, tallying up the number of colors seen so far.
When you reach the median index, exit the loop and remember the last `bins` index reached.

```c
  // ...

  fragColor = colors[0];

  for (i = 0; i < kernelSize; ++i) {
    if (binIndexes[i] == binIndex) {
      fragColor = colors[i];
      break;
    }
  }

  // ...
```

Now loop through the `binIndexes` and find the first color with the last `bins` indexed reached.
Its greyscale value is the approximated median which in many cases will be the true median value.
Set this color as the fragColor and exit the loop and shader.

### Kuwahara Filter

<p align="center">
<img src="https://i.imgur.com/b5vw2AJ.gif" alt="Kuwahara Filter" title="Kuwahara Filter">
</p>

Like the median filter, the kuwahara filter preserves the major edges found in the image.
You'll notice that it has a more block like or chunky pattern to it.
In practice,
the Kuwahara filter runs faster than the median filter, allowing for larger `size` values without a noticeable slowdown.

```c
// ...

#define MAX_SIZE        5
#define MAX_KERNEL_SIZE ((MAX_SIZE * 2 + 1) * (MAX_SIZE * 2 + 1))

// ...
```

Set a hard limit for the `size` parameter and the number of samples taken.

```c
// ...

int i     = 0;
int j     = 0;
int count = 0;

// ...
```

These are used to sample the input texture and set up the `values` array.

```c
// ...

vec3  valueRatios = vec3(0.3, 0.59, 0.11);

// ...
```

Like the median filter, you'll be converting the color samples into greyscale values.

```c
// ...

float values[MAX_KERNEL_SIZE];

// ...
```

Initialize the `values` array.
This will hold the greyscale values for the color samples.

```c
// ...

vec4  color       = vec4(0);
vec4  meanTemp    = vec4(0);
vec4  mean        = vec4(0);
float valueMean   = 0;
float variance    = 0;
float minVariance = -1;

// ...
```

The Kuwahara filter works by computing the variance of four subwindows and then using the mean of the subwindow with the smallest variance.

```c
// ...

void findMean(int i0, int i1, int j0, int j1) {

// ...
```

`findMean` is a function defined outside of `main`.
Each run of `findMean` will remember the mean of the given subwindow that has the lowest variance seen so far.

```c
  // ...

  meanTemp = vec4(0);
  count    = 0;

  // ...
```

Make sure to reset `count` and `meanTemp` before computing the mean of the given subwindow.

```c
  // ...

  for (i = i0; i <= i1; ++i) {
    for (j = j0; j <= j1; ++j) {
      color  =
        texture
          ( colorTexture
          ,   (gl_FragCoord.xy + vec2(i, j))
            / texSize
          );

      meanTemp += color;

      values[count] = dot(color.rgb, valueRatios);

      count += 1;
    }
  }

  // ...
```

Similar to the box blur, loop through the given subwindow and add up each color.
At the same time, make sure to store the greyscale value for this sample in `values`.

```c
  // ...

  meanTemp.rgb /= count;
  valueMean     = dot(meanTemp.rgb, valueRatios);

  // ...
```

To compute the mean, divide the samples sum by the number of samples taken.
Calculate the greyscale value for the mean.

```c
  // ...

  for (i = 0; i < count; ++i) {
    variance += pow(values[i] - valueMean, 2);
  }

  variance /= count;

  // ...
```

Now calculate the variance for this given subwindow.
The variance is the average squared difference between each sample's greyscale value the mean greyscale value.

```c
  // ...

  if (variance < minVariance || minVariance <= -1) {
    mean = meanTemp;
    minVariance = variance;
  }
}

// ...
```

If the variance is smaller than what you've seen before or this is the first variance you've seen,
set the mean of this subwindow as the final mean and update the minimum variance seen so far.

```c
// ...

void main() {
  int size = int(parameters.x);
  if (size <= 0) { fragColor = texture(colorTexture, texCoord); return; }

  // ...
```

Back in `main`, set the `size` parameter.
If the size is at or below zero, return the fragment unchanged.

<p align="center">
<img src="https://i.imgur.com/iuLbLKO.gif" alt="Kuwahara Kernal" title="Kuwahara Kernal">
</p>

```c
  // ...

  // Lower Left

  findMean(-size, 0, -size, 0);

  // Upper Right

  findMean(0, size, 0, size);

  // Upper Left

  findMean(-size, 0, 0, size);

  // Lower Right

  findMean(0, size, -size, 0);

  // ...
```

As stated above,
the Kuwahara filter works by computing the variance of four subwindows
and then using the mean of the subwindow with the lowest variance as the final fragment color.
Note that the four subwindows overlap each other.

```c
  // ...

  mean.a    = 1;
  fragColor = mean;

  // ...
```

After computing the variance and mean for each subwindow,
set the fragment color to the mean of the subwindow with the lowest variance.

### Source

- [main.cxx](../demonstration/src/main.cxx)
- [basic.vert](../demonstration/shaders/vertex/basic.vert)
- [position.frag](../demonstration/shaders/fragment/position.frag)
- [box-blur.frag](../demonstration/shaders/fragment/box-blur.frag)
- [median-filter.frag](../demonstration/shaders/fragment/median-filter.frag)
- [kuwahara-filter.frag](../demonstration/shaders/fragment/kuwahara-filter.frag)

## Copyright

(C) 2019 David Lettier
<br>
[lettier.com](https://www.lettier.com)

[:arrow_backward:](fog.md)
[:arrow_double_up:](../README.md)
[:arrow_up_small:](#)
[:arrow_down_small:](#copyright)
[:arrow_forward:](bloom.md)
