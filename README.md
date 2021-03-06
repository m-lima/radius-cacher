# radius-cacher
A thin radius server for caching entries

## Building
### Dependencies
* C++17 compiler
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
To have a faster build time, it is suggested to install boost and libmemcached and avoid downloading and building them during compilation

### Compiling
```bash
$ cd <repository_folder>
$ mkdir build
$ cd build
$ cmake -DCMAKE_BUILD_TYPE=Release [-DRC_TEST=<ON|OFF>] ..
$ make
```

## Running
```bash
$ ./radius-cacher [-s SERVER_CONFIG_FILE] [-c CACHE_CONFIG_FILE] [-v VERBOSE_LEVEL]
```
