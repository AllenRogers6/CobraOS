#!/bin/bash

echo "Compiling lib"
make
#mv build/* arch/x86/build
echo "Compiling x86"
cd arch/x86 || exit 1
make
