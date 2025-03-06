#!/bin/bash

# Define these variables before starting the script!
# export LIBLCF_PATH=
# export IOS_TOOLCHAIN_FILE=
# export TOOLCHAIN_PATH=

cd ..
echo "[*] Deleting old build files"
rm -rf build CMakeFiles CMakeCache.txt
echo "[*] Creating build files"
cmake . -DBUILD_SHARED_LIBS=ON -DPLAYER_TARGET_PLATFORM=libretro -DPLAYER_BUILD_LIBLCF=ON -DCMAKE_IGNORE_PATH=/usr/include -DCMAKE_BUILD_TYPE=Release -DCMAKE_OSX_ARCHITECTURES=arm64 -DCMAKE_OSX_DEPLOYMENT_TARGET=11.0 -DCMAKE_SYSTEM_NAME=iOS "-DCMAKE_PREFIX_PATH=$TOOLCHAIN_PATH;$LIBLCF_PATH" -Bbuild "-DCMAKE_TOOLCHAIN_FILE=$IOS_TOOLCHAIN_FILE" -DCMAKE_C_FLAGS="-isysroot /Applications/Xcode.app/Contents/Developer/Platforms/iPhoneOS.platform/Developer/SDKs/iPhoneOS14.4.sdk" -DCMAKE_CXX_FLAGS="-isysroot /Applications/Xcode.app/Contents/Developer/Platforms/iPhoneOS.platform/Developer/SDKs/iPhoneOS14.4.sdk" -DPLATFORM=OS64 -DCMAKE_OSX_SYSROOT="/Applications/Xcode.app/Contents/Developer/Platforms/iPhoneOS.platform/Developer/SDKs/iPhoneOS14.4.sdk" -DLIBLCF_ENABLE_TOOLS=false '-DCMAKE_EXE_LINKER_FLAGS=-dynamiclib -miphoneos-version-min=11.0 -isysroot /Applications/Xcode.app/Contents/Developer/Platforms/iPhoneOS.platform/Developer/SDKs/iPhoneOS14.4.sdk -framework Foundation -framework UIKit -framework OpenGLES -framework QuartzCore -framework CoreAudio -framework AudioToolbox -framework CoreGraphics -framework CoreMotion -framework GameController -framework AudioUnit -framework AVFoundation' '-DCMAKE_EXE_LINKER_FLAGS_INIT=-dynamiclib -miphoneos-version-min=11.0 -isysroot /Applications/Xcode.app/Contents/Developer/Platforms/iPhoneOS.platform/Developer/SDKs/iPhoneOS14.4.sdk -framework Foundation -framework UIKit -framework OpenGLES -framework QuartzCore -framework CoreAudio -framework AudioToolbox -framework CoreGraphics -framework CoreMotion -framework GameController -framework AudioUnit -framework AVFoundation'
echo "[*] Building"
cmake --build build --target easyrpg_libretro --config Release