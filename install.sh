#!/bin/bash

read -p "Arch type: " arch
echo "Chosen arch: $arch"

if [ $arch == "x86" ]; then
  echo "Compiling lib"
  make
  mv build/* arch/x86/build
  echo "Compiling x86"
  cd arch/x86 || exit 1
  make
else
  echo "Did you pick the correct type?"
fi
