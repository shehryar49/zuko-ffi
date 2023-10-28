import ffi

var lib = ffi.loadLibrary("/home/shahryar/ffi/demo.so")

var k = lib.call("sum",[ffi.C_INT,ffi.C_INT,ffi.C_INT],8,5)
println(k)
k = lib.call("sum",[ffi.C_INT,ffi.C_INT,ffi.C_INT],10,20)
println(k)

lib.call("sayhello",[ffi.C_VOID])
lib.call("MYPUTS",[ffi.C_VOID,ffi.C_STR],"please print this")

k = lib.call("doubleSum",[ffi.C_DOUBLE,ffi.C_DOUBLE,ffi.C_DOUBLE],1.2,3.3)
println(k)