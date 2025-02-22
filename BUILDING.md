# Building

- Requires a compiler with C++23 support, and module support. Doesn't need to
support import std.

## Release mode

```sh
# if you do use conan
conan install . -s build_type=RelWithDebInfo -s compiler.cppstd=23 --build=missing
cmake --preset release-conan
cmake --build build/rel
```

```sh
# otherwise
cmake --preset release
cmake --build build/rel
```

- To install, run:

```sh
cmake --build build/rel --target install
```

## Developer mode

```sh
# if you do use conan
conan install . -s build_type=Debug -s compiler.cppstd=23 --build=missing
cmake --preset dev-conan
cmake --build build/dev
```

```sh
# otherwise
cmake --preset dev-conan 
cmake --build build/dev
# default options:
# -D tsds_ASAN=ON
# -D tsds_MSAN=OFF
# -D tsds_TSAN=OFF
# -D tsds_UBSAN=ON
# -D tsds_DEV=ON
```

- Some extra developer options:

```sh
# some fancy HTML files
cmake --build build/dev --target coverage
cmake --build build/dev --target docs
```
