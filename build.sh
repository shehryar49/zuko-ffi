#!/usr/bin/sh
g++ -shared ffi.cpp -o ffi.so -fPIC -g -lffi -lzapi -L /opt/zuko/lib
sudo cp ffi.so /opt/zuko/modules
