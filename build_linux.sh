#!/bin/bash

# Define these variables before starting the script!
# export LIBLCF_PATH=
# export TOOLCHAIN_PATH=
# export PREFIX=

export 'PKG_CONFIG=pkg-config --static'
export STATICLIBSPATH=$TOOLCHAIN_PATH
export PKG_CONFIG_PATH=$TOOLCHAIN_PATH/lib/pkgconfig:$LIBLCF_PATH/lib/pkgconfig
export PKG_CONFIG_LIBDIR=
export 'CC=ccache gcc -static-libgcc -static-libstdc++'
export 'CXX=ccache g++ -static-libgcc -static-libstdc++'
export 'CXXFLAGS=-Wall -Wextra -O0 -g3 -pipe -flto -ffunction-sections -fdata-sections'
export LDFLAGS=-Wl,-O1,--sort-common,--as-needed,--gc-sections,-flto,-z,relro,-z,now
autoreconf -fi
./configure --prefix=$PREFIX --enable-fmmidi "--enable-append-version=(CI, $(date +%Y-%m-%d))" bashcompinstdir=etc/completion.d
make clean
make check -j12
make install
make dist
cd ~/EasyRPG/player-linux
cp -up bin/easyrpg-player easyrpg-player
objcopy --only-keep-debug easyrpg-player easyrpg-player.debug
strip --strip-debug --strip-unneeded easyrpg-player
objcopy --add-gnu-debuglink=easyrpg-player.debug easyrpg-player
tar czf easyrpg-player-linux-static.tar.gz easyrpg-player
tar czf easyrpg-player-linux-static_dbgsym.tar.gz easyrpg-player.debug
