#ifdef _WIN32
  #include <windows.h>
  #include "C:\\zuko\\ZObject.h"
  #define EXPORT __declspec(dllexport)
#else
  #include <unistd.h>
  #include <dlfcn.h>
  #include "/opt/zuko/ZObject.h"
  #define EXPORT
#endif

extern "C"
{
    EXPORT ZObject init();

    EXPORT ZObject LOAD_LIB(ZObject*,int32_t);

    EXPORT ZObject LIB_CALL(ZObject*,int32_t);
    
}