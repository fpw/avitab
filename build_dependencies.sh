#!/bin/bash

OUTDIR=`pwd`/build-third/
mkdir -p $OUTDIR
mkdir -p $OUTDIR/lib
mkdir -p $OUTDIR/include

cd lib

echo "Building json..."
if [ ! -f $OUTDIR/include/nlohmann/json.hpp ]; then
    cp -a json/include/* $OUTDIR/include/
fi

if [ ! -f $OUTDIR/include/nlohmann/json.hpp ]; then echo "Failed"; exit; fi

echo "Building gulrak/filesystem..."
if [ ! -f $OUTDIR/include/ghc/filesystem.hpp ]; then
    mkdir -p $OUTDIR/include/ghc
    cp filesystem/include/ghc/*.hpp $OUTDIR/include/ghc/
fi
if [ ! -f $OUTDIR/include/ghc/filesystem.hpp ]; then echo "Failed"; exit; fi

echo "Building stb..."
if [ ! -f $OUTDIR/include/stb/stb.h ]; then
    mkdir -p $OUTDIR/include/stb
    cp stb/*.h $OUTDIR/include/stb/
fi
if [ ! -f $OUTDIR/include/stb/stb.h ]; then echo "Failed"; exit; fi

echo "Building XPLM..."
if [ ! -f $OUTDIR/include/XPLM/XPLMPlugin.h ]; then
    cp -a XSDK/CHeaders/XPLM $OUTDIR/include
    cp XSDK/Libraries/Win/* $OUTDIR/lib
    cp -a XSDK/Libraries/Mac/* $OUTDIR/lib
fi
if [ ! -f $OUTDIR/include/XPLM/XPLMPlugin.h ]; then echo "Failed"; exit; fi

echo "Building detex..."
if [ ! -f $OUTDIR/lib/libdetex.a ]; then
    cd detex
    OPTCFLAGS=-fPIC make library
    make HEADER_FILE_INSTALL_DIR=$OUTDIR/include STATIC_LIB_DIR=$OUTDIR/lib install
    cd ..
fi
if [ ! -f $OUTDIR/lib/libdetex.a ]; then echo "Failed"; exit; fi

echo "Building mupdf..."
if [ ! -f $OUTDIR/lib/libmupdf-third.a ]; then
    cd mupdf
    patch -p1 -s -N < ../mupdf.diff
    XCFLAGS=-fPIC make HAVE_X11=no HAVE_GLUT=no prefix=$OUTDIR -j10 install
    cd thirdparty/libjpeg
    ./configure
    cd ../../../
fi
if [ ! -f $OUTDIR/lib/libmupdf-third.a ]; then echo "Failed"; exit; fi

echo "Building mbedtls..."
if [ ! -f $OUTDIR/lib/libmbedtls.a ]; then
    cd mbedtls
    mkdir -p build
    cd build
    cmake -G "Unix Makefiles" -DCMAKE_BUILD_TYPE=Release -DCMAKE_C_FLAGS=-fPIC -DCMAKE_INSTALL_PREFIX="" -DCMAKE_HOST_UNIX=ON ..
    make -j10 DESTDIR=$OUTDIR install
    cd ../../
fi
if [ ! -f $OUTDIR/lib/libmbedtls.a ]; then echo "Failed"; exit; fi

echo "Building curl..."
if [ ! -f $OUTDIR/lib/libcurl.a ]; then
    cd curl
    ./buildconf
    ./configure --prefix=$OUTDIR --without-ssl --without-nghttp2 --with-mbedtls=$OUTDIR --with-zlib="`pwd`/../mupdf/thirdparty/zlib/" --without-libpsl --disable-ldap --without-brotli --without-winidn --without-libidn2 --disable-shared --disable-crypto-auth --disable-ftp --disable-telnet --disable-gopher --disable-dict --disable-imap --disable-pop3 --disable-rtsp --disable-smtp --disable-tftp
    CFLAGS=-fPIC make -j10 install
    cd ..
fi
if [ ! -f $OUTDIR/lib/libcurl.a ]; then echo "Failed"; exit; fi

echo "Building libtiff..."
if [ ! -f $OUTDIR/lib/libtiff.a ]; then
    cd libtiff
    ./autogen.sh
    CFLAGS=-fPIC ./configure --prefix=$OUTDIR --build=x86_64 --disable-jbig --disable-lzma --without-x
    make -j10 install
    cd ..
fi
if [ ! -f $OUTDIR/lib/libtiff.a ]; then echo "Failed"; exit; fi

echo "Building libproj..."
if [ ! -f $OUTDIR/lib/libproj_5_2.a ] || [ ! -f $OUTDIR/lib/libproj.a ]; then
    cd proj
    mkdir -p build
    cd build
    cmake -G "Unix Makefiles" -DCMAKE_BUILD_TYPE=Release -DPROJ_TESTS=OFF -DBUILD_LIBPROJ_SHARED=OFF -DCMAKE_C_FLAGS=-fPIC -DCMAKE_INSTALL_PREFIX=$OUTDIR -DLIBDIR=$OUTDIR/lib -DINCLUDEDIR=$OUTDIR/include -DBINDIR=$OUTDIR/bin ..
    cd src
    make -j10 install
    cp $OUTDIR/lib/libproj.a $OUTDIR/lib/libproj_5_2.a
    cp $OUTDIR/lib/libproj_5_2.a $OUTDIR/lib/libproj.a
    cd ../../..
fi
if [ ! -f $OUTDIR/lib/libproj_5_2.a ]; then echo "Failed"; exit; fi

echo "Building libgeotiff..."
if [ ! -f $OUTDIR/lib/libgeotiff.a ]; then
    cd libgeotiff/libgeotiff
    mkdir -p build
    cd build
    cmake -G "Unix Makefiles" -DCMAKE_INSTALL_PREFIX=$OUTDIR -DCMAKE_BUILD_TYPE=Release -DCMAKE_C_FLAGS=-fPIC -DCMAKE_PREFIX_PATH="`pwd`/../../../" ..
    make -j10 install
    cp lib/libxtiff.a $OUTDIR/lib
    cd ../../..
fi
if [ ! -f $OUTDIR/lib/libgeotiff.a ]; then echo "Failed"; exit; fi

echo "Building QuickJS..."
if [ ! -f $OUTDIR/lib/libquickjs.a ]; then
    cd QuickJS
    make CC="gcc -fPIC" -j10 libquickjs.a
    cp libquickjs.a $OUTDIR/lib
    cd ..
fi
if [ ! -f $OUTDIR/lib/libquickjs.a ]; then echo "Failed"; exit; fi

cd ..
