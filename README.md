# radius-cacher
A thin radius server for caching entries

## Building
### Dependencies
* C++ compiler
* CMake 3.2+

### Installing dependencies
To install CMake
```bash
# Mac
$ brew install cmake
```
```bash
# Linux
$ apt-get install cmake
```
```bash
# Windows
$ scoop install cmake
```

### Dependency notes
To have a faster build time, it is suggested to install boost and libmemcached and avoid downloading and build them at compile-time

### Compiling
```bash
$ cd <repository_folder>
$ git submodule update --init --recursive
$ mkdir build
$ cd build
$ cmake -DCMAKE_BUILD_TYPE=Release [-DRC_VERBOSE_LEVEL=<LEVEL>] [-DRC_BUFFER_SIZE=<BUFFER_SIZE>] [-DRC_CALLBACK_COUNT=<CALLBACK_COUNT>] ..
$ make
```

## Running
```bash
$ ./radius-cacher [-s SERVER_CONFIG_FILE] [-c CACHE_CONFIG_FILE]
```
