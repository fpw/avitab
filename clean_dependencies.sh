#!/bin/bash

OUTDIR=`pwd`/build-third/
rm -rf "$OUTDIR"

cd lib

echo "Clearning detex..."
cd detex
git clean -d -x -f
cd ..

echo "Cleaning mupdf..."
cd mupdf
git clean -d -x -f
cd ..

echo "Cleaning mbedtls..."
cd mbedtls
git clean -d -x -f
cd ..

echo "Cleaning curl..."
cd curl
git clean -d -x -f
cd ..

echo "Cleaning libtiff..."
cd libtiff
git clean -d -x -f
cd ..

echo "Cleaning libproj..."
cd proj
git clean -d -x -f
cd ..

echo "Cleaning libgeotiff..."
cd libgeotiff
git clean -d -x -f
cd ..
