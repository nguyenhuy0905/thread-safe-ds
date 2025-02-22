#!/bin/bash

buildtyp="$(if [[ -z "$1" ]]; then
  echo "Debug RelWithDebInfo"
else
  echo "$1"
fi)"

conan profile detect -f
conan profile update settings.compiler="$CC" default
conan profile update settings.compiler.cppstd=20 default
conan profile update conf.tools.cmake.cmaketoolchain:generator=Ninja default
IFS=' ' read -r -a buildtyp <<<"${buildtyp}"

if [ -f conan_cache_save.tgz ]; then
  conan cache restore conan_cache_save.tgz
fi
conan remove \* --lru=1M -c
for bt in "${buildtyp[@]}"; do
  echo "${bt}"
  conan install . -s compiler="$CC" -s build_type="${bt}" -b missing
done
conan cache save '*/*:*' --file=conan_cache_save.tgz
