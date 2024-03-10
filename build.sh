#!/usr/bin/sh
g++ -shared ffi.cpp -o ffi.so -fPIC -g -lffi
sudo cp ffi.so /opt/zuko/modules
