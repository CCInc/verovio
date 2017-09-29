#!/bin/bash

#initialize emscripten
alias nodejs="'D:/New Programs/nodejs/node.exe'"
cd D:/Data/emLinux/emsdk-portable
./emsdk activate latest > /dev/null
source ./emsdk_env.sh > /dev/null

# run the build process
cd D:/Projects/music/Verovio/verovio/emscripten
./buildToolkit -DHPX