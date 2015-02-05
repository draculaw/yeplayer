#!/bin/bash
NDK=/opt/android-ndk-r10d
SYSROOT=$NDK/platforms/android-19/arch-arm/
TOOLCHAIN=$NDK/toolchains/arm-linux-androideabi-4.9/prebuilt/linux-x86_64


#--disable-decoders \
#--enable-decoder=mpegvideo \
function build_one
{
./configure \
--prefix=$PREFIX \
--disable-encoders \
--disable-decoders \
--enable-decoder=mpegvideo \
--disable-yasm \
--disable-shared \
--enable-static \
--disable-doc \
--disable-symver \
--cross-prefix=$TOOLCHAIN/bin/arm-linux-androideabi- \
--target-os=linux \
--arch=arm \
--enable-cross-compile \
--sysroot=$SYSROOT \
--extra-cflags="-Os -fpic $ADDI_CFLAGS" \
--extra-ldflags="$ADDI_LDFLAGS" \
$ADDITIONAL_CONFIGURE_FLAG
make clean
make
make install
}
CPU=arm
PREFIX=$(pwd)/android/$CPU
ADDI_CFLAGS="-marm"
build_one
