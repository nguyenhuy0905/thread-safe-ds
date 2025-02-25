# Building and installing

- Why would you even want to install this project though. Goodness.
- Requires a compiler with C++23 support, and module support. Doesn't need to
support import std.

## Release mode

- For Unix:

> [!NOTE]
> So far, only Linux and Mac have been tested.
> For Mac, only tested through GitHub Actions.

```sh
# if you do use conan
conan install . -s build_type=RelWithDebInfo -s compiler.cppstd=20 --build=missing
cmake --preset release-conan-unix
cmake --build build/RelWithDebInfo
```

```sh
# otherwise
cmake --preset release
cmake --build build/RelWithDebInfo
```

- For Windows:

```sh
# if you do use conan
conan install . -s build_type=RelWithDebInfo -s compiler.cppstd=20 --build=missing
cmake --preset release-conan-msvc
cmake --build build/RelWithDebInfo
```

```sh
# otherwise
cmake --preset release-msvc
cmake --build build/RelWithDebInfo
```

- To install, run:

```sh
cmake --build build/RelWithDebInfo --target install
```

## Developer mode

- For Unix:

> [!NOTE]
> So far, only Linux and Mac have been tested.
> For Mac, only tested through GitHub Actions.

```sh
# if you do use conan
conan install . -s build_type=Debug -s compiler.cppstd=20 --build=missing
# requires doxygen, gcovr and graphviz
cmake --preset dev-conan-unix
# if you do NOT use conan
cmake --build build/Debug
```

```sh
# otherwise
cmake --preset dev
cmake --build build/Debug
# default options:
# -D tsds_ASAN=ON
# -D tsds_MSAN=OFF
# -D tsds_TSAN=OFF
# -D tsds_UBSAN=ON
# -D tsds_DEV=ON
```

- Some extra developer options:

```sh
# only with dev-conan.
# coverage requires gcovr
# coverage only works with GCC/Clang
# docs requires doxygen and graphviz
cmake --build build/dev --target coverage
cmake --build build/dev --target docs
```

- For Windows

```sh
# if you do use conan
conan install . -s build_type=Debug -s compiler.cppstd=20 --build=missing
# requires doxygen and graphviz. Coverage not possible for Windows.
# assuming you use Visual Studio generator.
cmake --preset dev-conan-msvc
# otherwise
# cmake --preset dev
cmake --build build
```

```sh
# otherwise
cmake --preset dev
cmake --build build/Debug
# the default options don't do crap on Windows. Save for tsds_DEV.
```
