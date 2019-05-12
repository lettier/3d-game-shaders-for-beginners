[:arrow_backward:](setup.md)
[:arrow_double_up:](../README.md)
[:arrow_up_small:](#)
[:arrow_down_small:](#copyright)
[:arrow_forward:](running-the-demo.md)

# 3D Game Shaders For Beginners

## Building The Demo

<p align="center">
<img src="https://i.imgur.com/PQcDnIu.gif" alt="Building The Demo" title="Building The Demo">
</p>

Before you can try out the demo program, you'll have to build the example code first.

### Dependencies

Before you can compile the example code, you'll need to install
[Panda3D](https://www.panda3d.org/)
for your platform.
Panda3D is available for Linux, Mac, and Windows.

### Linux

Start by [installing](https://www.panda3d.org/manual/?title=Installing_Panda3D_in_Linux) the
[Panda3D SDK](https://www.panda3d.org/download/sdk-1-10-1/) for your distribution.

Make sure to locate where the Panda3D headers and libraries are.
The headers and libraries are most likely in `/usr/include/panda3d/` and `/usr/lib/panda3d/` respectively.

Next clone this repository and change directory into it.

```bash
git clone https://github.com/lettier/3d-game-shaders-for-beginners.git
cd 3d-game-shaders-for-beginners/demonstration
```

Now compile the source code into an object file.

```bash
g++ \
  -c src/main.cxx \
  -o 3d-game-shaders-for-beginners.o \
  -std=gnu++11 \
  -O2 \
  -I/usr/include/python2.7/ \
  -I/usr/include/panda3d/
```

With the object file created, create the executable by linking the object file to its dependencies.

```bash
g++ \
  3d-game-shaders-for-beginners.o \
  -o 3d-game-shaders-for-beginners \
  -L/usr/lib/panda3d \
  -lp3framework \
  -lpanda \
  -lpandafx \
  -lpandaexpress \
  -lpandaphysics \
  -lp3dtoolconfig \
  -lp3dtool \
  -lp3pystub \
  -lp3direct \
  -lpthread
```

For more help, see the [Panda3D manual](https://www.panda3d.org/manual/?title=How_to_compile_a_C++_Panda3D_program_on_Linux).

### Mac

Start by installing the [Panda3D SDK](https://www.panda3d.org/download/sdk-1-10-1/) for Mac.

Make sure to locate where the Panda3D headers and libraries are.

Next clone this repository and change directory into it.

```bash
git clone https://github.com/lettier/3d-game-shaders-for-beginners.git
cd 3d-game-shaders-for-beginners
```

Now compile the source code into an object file.
You'll have to find where the Python 2.7 and Panda3D include directories are.

```bash
clang++ \
  -c main.cxx \
  -o 3d-game-shaders-for-beginners.o \
  -std=gnu++11 \
  -g \
  -O2 \
  -I/usr/include/python2.7/ \
  -I/Developer/Panda3D/include/
```

With the object file created, create the executable by linking the object file to its dependencies.
You'll need to track down where the Panda3D libraries are located.

```bash
clang++ \
  3d-game-shaders-for-beginners.o \
  -o 3d-game-shaders-for-beginners \
  -L/Developer/Panda3D/lib \
  -lp3framework \
  -lpanda \
  -lpandafx \
  -lpandaexpress \
  -lpandaphysics \
  -lp3dtoolconfig \
  -lp3dtool \
  -lp3pystub \
  -lp3direct \
  -lpthread
```

For more help, see the [Panda3D manual](https://www.panda3d.org/manual/?title=How_to_compile_a_C++_Panda3D_program_on_macOS).

### Windows

Start by [installing](https://www.panda3d.org/manual/?title=Installing_Panda3D_in_Windows) the
[Panda3D SDK](https://www.panda3d.org/download/sdk-1-10-1/) for Windows.

Make sure to locate where the Panda3D headers and libraries are.

Next clone this repository and change directory into it.

```bash
git clone https://github.com/lettier/3d-game-shaders-for-beginners.git
cd 3d-game-shaders-for-beginners
```

For more help, see the [Panda3D manual](https://www.panda3d.org/manual/?title=Running_your_Program&language=cxx).

## Copyright

(C) 2019 David Lettier
<br>
[lettier.com](https://www.lettier.com)

[:arrow_backward:](setup.md)
[:arrow_double_up:](../README.md)
[:arrow_up_small:](#)
[:arrow_down_small:](#copyright)
[:arrow_forward:](running-the-demo.md)
