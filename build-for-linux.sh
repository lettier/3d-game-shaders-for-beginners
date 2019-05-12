g++ \
  -Wfatal-errors \
  -c main.cxx \
  -o 3d-game-shaders-for-beginners.o \
  -std=gnu++11 \
  -O3 \
  -I/usr/include/python2.7/ \
  -I/usr/include/panda3d/

g++ \
  3d-game-shaders-for-beginners.o \
  -o 3d-game-shaders-for-beginners \
  -L/usr/lib/panda3d \
  -lp3framework \
  -lpanda \
  -lpandafx \
  -lpandaexpress \
  -lp3dtoolconfig \
  -lp3dtool \
  -lp3pystub \
  -lp3direct \
  -lpthread
