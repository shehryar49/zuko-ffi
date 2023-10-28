#ifdef _WIN32
  #include <windows.h>
  #include "C:\\plutonium\\PltObject.h"
  #define EXPORT __declspec(dllexport)
#else
  #include <unistd.h>
  #include <dlfcn.h>
  #include "/opt/plutonium/PltObject.h"
  #define EXPORT
#endif

extern "C"
{
    EXPORT PltObject init();

    EXPORT PltObject LOAD_LIB(PltObject*,int32_t);

    EXPORT PltObject LIB_CALL(PltObject*,int32_t);
    
}