A demonstration of using Thrust and modern C++ to produce portable parallel applications.

# macOS dependencies
```
$ brew install gcc6 --without-multilib # OpenMP backend
$ brew install gcc   # Serial backend
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

# Serial
$ CC=gcc-6 CXX=g++-6 cmake -DCMAKE_BUILD_TYPE=Release -DCPP_PAR=true ..

# Parallel OpenMP
$ CC=gcc-6 CXX=g++-6 cmake -DCMAKE_BUILD_TYPE=Release -DOPENMP=true -DCPP_PAR=false ..

$ make
$ make install
```

# macOS running
```
cd ../Runtime
mpirun -n 1 ./sph-renderer : -n 1 ./sph
```
