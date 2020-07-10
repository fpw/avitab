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

echo "Building litehtml..."
if [ ! -f $OUTDIR/lib/liblitehtml.a ]; then
    cd litehtml
    mkdir -p build
    cd build
    cmake -G "Unix Makefiles" -DLITEHTML_UTF8=ON -DCMAKE_INSTALL_PREFIX=$OUTDIR -DCMAKE_BUILD_TYPE=Release -DCMAKE_C_FLAGS=-fPIC -DCMAKE_CXX_FLAGS=-fPIC ..
    cat ../include/master.css | xxd -i  > ../src/master.css.inc
     make -j10 install
    cd ..
fi
if [ ! -f $OUTDIR/lib/liblitehtml.a ]; then echo "Failed"; exit; fi

echo "Building pixman..."
if [ ! -f $OUTDIR/lib/libpixman-1.a ]; then
    cd pixman
    ./autogen.sh --prefix=$OUTDIR --enable-static --disable-shared --disable-libpng --disable-gtk --disable-static-testprogs --disable-openmp
    make -j10
    make install
    cd ..
fi
if [ ! -f $OUTDIR/lib/libpixman-1.a ]; then echo "Failed"; exit; fi

echo "Building cairo..."
if [ ! -f $OUTDIR/lib/libcairo.a ]; then
    cd cairo
    export pixman_CFLAGS=-I$OUTDIR/include/pixman-1
    export pixman_LIBS=$OUTDIR/lib/libpixman-1.a
    export FREETYPE_CFLAGS=-I$OUTDIR/../lib/mupdf/thirdparty/freetype/include
    export FREETYPE_LIBS=$OUTDIR/lib/libmupdf-third.a
    ./autogen.sh --prefix=$OUTDIR --enable-static --disable-egl --disable-glesv2 --disable-glesv3 --disable-glx --disable-gl --disable-valgrind --disable-xlib --enable-ft --disable-shared --disable-xlib-xrender --disable-xcb --disable-svg --disable-full-testing --disable-interpreter --disable-fc --disable-ps --disable-pdf --disable-glesv2 --disable-win32 --disable-win32-font --disable-drm --disable-png --disable-script --disable-quartz --disable-wgl --disable-gobject --disable-trace --disable-symbol-lookup
    sed -i src/cairo-misc.c -e "s/^#ifdef _WIN32 \/\* also defined on x86_64 \*\//#if 0/gi"
    sed -i build/configure.ac.warnings -e "^s/MAYBE_WARN=\"\$MAYBE_WARN -Wp,-D_FORTIFY_SOURCE=2\"//gi"
    make -j10
    make install
    cd ..
fi
if [ ! -f $OUTDIR/lib/libcairo.a ]; then echo "Failed"; exit; fi

cd ..
