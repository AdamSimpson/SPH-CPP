A demonstration of using Thrust and modern C++ to produce portable parallel applications.

# macOS dependencies
```
$ brew install gcc6 --without-multilib
$ brew install sdl2
$ brew install mpich
$ brew install freetype
$ brew install glew
$ brew install glm
$ brew install cmake
$ brew install boost
```
# macOS building
```
$ mkdir build
$ cd build
$ CC=gcc-6 CXX=g++-6 cmake -DCMAKE_BUILD_TYPE=Release -DOPENMP=1 -DCPP_PAR=false ..
```
