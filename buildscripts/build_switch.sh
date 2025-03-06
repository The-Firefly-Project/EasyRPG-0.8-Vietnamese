#!/bin/bash

# Define these variables before starting the script!
# export LIBLCF_PATH=
# export TOOLCHAIN_PATH=

cd ..
rm -rf build-switch CMakeFiles CMakeCache.txt
cmake -B build-switch . -G Ninja -DCMAKE_TOOLCHAIN_FILE=/opt/devkitpro/cmake/Switch.cmake -DCMAKE_CXX_COMPILER_LAUNCHER=ccache -DCMAKE_BUILD_TYPE=RelWithDebInfo -DPLAYER_TARGET_PLATFORM=switch "-DCMAKE_PREFIX_PATH=$TOOLCHAIN_PATH;$LIBLCF_PATH" -DPLAYER_WITH_FREETYPE=NO -DPLAYER_VERSIONED_PACKAGES=OFF "-DPLAYER_VERSION_APPEND=(CI, $(date +%Y-%m-%d))"
cmake --build build-switch
cmake --build build-switch --target package