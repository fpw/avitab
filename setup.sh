#!/bin/bash

set -euo pipefail

# Constants
BUILD_FOLDER="$(pwd)"/build
CI_MODE="${AVITAB_CI:-0}"
BUILD_TARGET="${AVITAB_BUILD_TARGET:-}"
SKIP_PKG_INSTALL="${AVITAB_SKIP_PACKAGE_INSTALL:-0}"

aptInstall() {
  sudo apt install -y "$1"
}

pacmanInstall() {
  pacman -S "$1"
}

brewInstall() {
  brew install "$1"
}

copyResources() {
 # Move files from res/ to build/
  echo "Moving files..."
  if [ ! -f "$BUILD_FOLDER"/config.json ]; then
    cp -a "$(pwd)"/res/* "$BUILD_FOLDER"
  fi
  if [ ! -f "$BUILD_FOLDER"/config.json ]; then
    echo "Failed"
    exit
  fi

  cd "$BUILD_FOLDER" || exit
}

standalone() {
  echo "Making AviTab-standalone..."
  copyResources
  make AviTab-standalone
}

plugin() {
  echo "Making avitab_plugin..."
  copyResources
  make avitab_plugin
}

# OS Detection
case "$OSTYPE" in
linux*)
  echo "Linux detected..."
  if [ "$SKIP_PKG_INSTALL" != "1" ]; then
    aptInstall cmake
    aptInstall make
    aptInstall g++
    aptInstall autoconf
    aptInstall automake
    aptInstall libtool
    aptInstall patch
    aptInstall libglfw3
    aptInstall libglfw3-dev
    aptInstall uuid-dev
  fi
  ;;
msys*)
  echo "Windows detected..."
  echo "Have you installed MSYS2?"
  printf "1. Yes\n2. No\n"

  read -r answer

  if [ "$answer" == "1" ]; then
    pacmanInstall mingw-w64-x86_64-toolchain
    pacmanInstall mingw64/mingw-w64-x86_64-cmake
    pacmanInstall msys/git
    pacmanInstall msys/patch
    pacmanInstall msys/make
    pacmanInstall msys/autoconf
    pacmanInstall msys/automake
    pacmanInstall msys/libtool
    pacmanInstall mingw64/mingw-w64-x86_64-glfw
  elif [ "$answer" == "2" ]; then
    echo "Please download it from: https://www.msys2.org/ and install it, then run this script again."
    exit
  else
    echo "Exiting..."
    exit
  fi
  ;;
darwin*)
  echo "macOS detected..."
  echo "Have you installed brew?"
  printf "1. Yes\n2. No\n"

  read -r answer

  if [ "$answer" == "1" ]; then
    brewInstall cmake
    brewInstall make
    brewInstall autoconf
    brewInstall automake
    brewInstall libtool
    brewInstall glfw
    brewInstall pkgconfig
  elif [ "$answer" == "2" ]; then
    echo "Please download it from: https://brew.sh/ and install it, then run this script again."
    exit
  else
    echo "Exiting..."
    exit
  fi
  ;;
*)
  echo "Unkown system, exiting..."
  exit
  ;;
esac

# Build third party dependencies in the build-third directory
echo "Running build_dependencies..."
./build_dependencies.sh

# Setup build folder properly
echo "Running CMake..."
cmake -G 'Unix Makefiles' -B "$BUILD_FOLDER"

if [ "$CI_MODE" = "1" ]; then
  case "$BUILD_TARGET" in
  standalone)
    standalone
    ;;
  plugin)
    plugin
    ;;
  all)
    standalone
    plugin
    ;;
  *)
    echo "Invalid AVITAB_BUILD_TARGET='$BUILD_TARGET' for CI mode. Use standalone, plugin, or all."
    exit 1
    ;;
  esac
else
  echo "Select next step..."
  printf "1. Make AviTab-Standalone\n2. Make avitab-plugin\n3. Nothing\n"

  read -r selection

  case "$selection" in
  1)
    standalone
    ;;
  2)
    plugin
    ;;
  *)
    echo "Exiting..."
    ;;
  esac
fi
