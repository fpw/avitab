#!/bin/bash

echo "Buildint detex..."
if [ ! -f detex/libdetex.a ]; then
    cd detex
    OPTCFLAGS=-fPIC make library
    cd ..
fi

echo "Building mupdf..."
if [ ! -f mupdf/build/release/libmupdf.a ] || [ ! -f mupdf/build/release/libmupdf-third.a ]; then
    patch -p1 -s -N < mupdf.diff
    cd mupdf
    XCFLAGS=-fPIC HAVE_X11=no HAVE_GLUT=no make -j10
    cd thirdparty/libjpeg
    ./configure
    cd ../../../ 
fi

echo "Building mbedtls..."
if [ ! -f mbedtls/build/library/libmbedtls.a ]; then
    cd mbedtls
    mkdir build
    cd build
    cmake -G "Unix Makefiles" -DCMAKE_C_FLAGS=-fPIC -DCMAKE_HOST_UNIX=ON ..
    make -j10 mbedtls
    mkdir lib
    cp library/*.a lib/
    cd ../..
fi

echo "Building curl..."
if [ ! -f curl/lib/.libs/libcurl.a ]; then
    cd curl
    ./buildconf
    ./configure --without-ssl --with-mbedtls="`pwd`/../mbedtls/build" --with-zlib="`pwd`/../mupdf/thirdparty/zlib/" --without-libpsl --disable-ldap --without-brotli --without-winidn --without-libidn2 --disable-shared --disable-crypto-auth --disable-ftp --disable-telnet --disable-gopher --disable-dict --disable-imap --disable-pop3 --disable-rtsp --disable-smtp --disable-tftp
    CFLAGS=-fPIC make -j10
    cd ..
fi

echo "Building libtiff..."
if [ ! -f libtiff/libtiff/.libs/libtiff.a ]; then
    cd libtiff
    ./autogen.sh
    CFLAGS=-fPIC ./configure --build=x86_64 --disable-jbig --disable-lzma --without-x --without-jpeg
    make -j10
    cd ..
fi

echo "Building libproj..."
if [ ! -f proj/build/lib/libproj.a ]; then
    cd proj
    mkdir build
    cd build
    cmake -G "Unix Makefiles" -DBUILD_LIBPROJ_SHARED=OFF -DCMAKE_C_FLAGS=-fPIC ..
    make -j10 proj
    cp lib/libproj_5_2.a lib/libproj.a
    cp ../src/proj_api.h lib
    cd ../..
fi

echo "Building libgeotiff..."
if [ ! -f libgeotiff/build/lib/libgeotiff.a ]; then
cd libgeotiff
    mkdir build
    cd build
    cmake -G "Unix Makefiles" -DCMAKE_C_FLAGS=-fPIC -DWITH_PROJ4=1 -DPROJ4_FOUND=1 -DPROJ4_INCLUDE_DIR="`pwd`/../../proj/build/lib"  ..
    make -j10 geotiff_archive || make -j10 geotiff_library
    make -j10 xtiff
    cd ../..
fi
