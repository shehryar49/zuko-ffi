/*
FFI Module for Plutonium
Uses libffi and currently supports primitive types only
Created by Shahryar Ahmad (MIT License)
*/

#include <cstring>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <ffi.h> //libffi
#include "ffi.h"
#include <vector>
#include <string>
#include <cassert>
using namespace  std;


ZObject nil;
enum Types
{
 C_INT=0,
 C_CHAR=1,
 C_LONG=2,
 C_LLONG=3,
 C_DOUBLE=4,
 C_FLOAT=5,
 C_SHORT=6,
 C_SIZET=7,
 C_STR=8,//this is a special type, libffi does not support it, i added it
 C_BOOL=9,
 C_PTR=10,
 C_VOID=11
};
ffi_type* ffi_types[] = 
{
    &ffi_type_sint,
    &ffi_type_schar,
    &ffi_type_slong,
    &ffi_type_sint64,
    &ffi_type_double,
    &ffi_type_float,
    &ffi_type_sshort,
    &ffi_type_sint64,
    &ffi_type_pointer,
    &ffi_type_uchar,
    &ffi_type_pointer,
    &ffi_type_void
};


Klass* libklass;
ZObject init()
{

    nil.type = Z_NIL;
    Module* ffiModule = vm_allocModule();
    ffiModule->name = "ffi";
    Module_addSigNativeFun(ffiModule, "loadLibrary", &LOAD_LIB,"s");
    
    Module_addMember(ffiModule,"C_INT",ZObjFromInt(C_INT));
    Module_addMember(ffiModule,"C_CHAR",ZObjFromInt(C_CHAR));
    Module_addMember(ffiModule,"C_LONG",ZObjFromInt(C_LONG));
    Module_addMember(ffiModule,"C_LLONG",ZObjFromInt(C_LLONG));
    Module_addMember(ffiModule,"C_DOUBLE",ZObjFromInt(C_DOUBLE));
    Module_addMember(ffiModule,"C_FLOAT",ZObjFromInt(C_FLOAT));
    Module_addMember(ffiModule,"C_SHORT",ZObjFromInt(C_SHORT));
    Module_addMember(ffiModule,"C_SIZET",ZObjFromInt(C_SIZET));
    Module_addMember(ffiModule,"C_STR",ZObjFromInt(C_STR));
    Module_addMember(ffiModule,"C_BOOL",ZObjFromInt(C_BOOL));
    Module_addMember(ffiModule,"C_PTR",ZObjFromInt(C_PTR));
    Module_addMember(ffiModule,"C_VOID",ZObjFromInt(C_VOID));

    
    //
    //Library class
    libklass = vm_allocKlass();
    Klass_addNativeMethod(libklass,"call",&LIB_CALL);
    vm_markImportant((void*)libklass);
    
    return ZObjFromModule(ffiModule);
}
ZObject LOAD_LIB(ZObject* args,int32_t n)
{
  if(n!=1)
    return Z_Err(ArgumentError,"1 argument required!");
  if(args[0].type != Z_STR)
    return Z_Err(TypeError,"String argument required!");
  const char* path = AS_STR(args[0])->val;
  #ifdef _WIN32
    HINSTANCE handle = LoadLibrary(path.c_str());
    if(!handle)
    {
      return Z_Err(Error,"Unable to load library: "+to_string(GetLastError()));
    }
  #else
    void* handle = dlopen(path,RTLD_LAZY);
    if(!handle)
      return Z_Err(Error,dlerror());
  #endif
  
  KlassObject* obj = vm_allocKlassObject(libklass);
  KlassObj_setMember(obj,".libhandle", ZObjFromPtr(handle));
  return ZObjFromKlassObj(obj);
}

//Libklass methods
ZObject LIB_CALL(ZObject* args,int32_t n)
{
    //Typechecking
    if(n < 3)
      return Z_Err(ArgumentError,"call(self,name,argTypes,...) takes atleast 3 arguments");
    if(args[0].type != Z_OBJ || ((KlassObject*)args[0].ptr) -> klass!=libklass)
      return Z_Err(TypeError,"First argument must be an object of library class.");
    if(args[1].type != Z_STR)
      return Z_Err(TypeError,"Second argument must be a string!");
    if(args[2].type != Z_LIST)
      return Z_Err(TypeError,"Third argument must be a list!");

    //Variables to store return value when function is called
    int iret;
    char cret;
    long lret;
    int64_t i64ret;
    double dret;
    float fret;
    short sret;
    size_t sztret;
    char* strret;
    char bret;//for boolean
    void* pret;
    //Addresses of return variables
    //this has 1 to 1 mapping with enum Types 
    void* retAddr[] = {
      &iret,
      &cret,
      &lret,
      &i64ret,
      &dret,
      &fret,
      &sret,
      &sztret,
      &strret,
      &bret,
      &pret,
      NULL
    };
    if(sizeof(long) == 8)
      retAddr[2] = &i64ret;
    
    const ZList& argTypes = *AS_LIST(args[2]);
    const char* name = AS_STR(args[1])->val;
    KlassObject& obj = *((KlassObject*)args[0].ptr);
    static string tmp;
    tmp = "."+(string)name;

    ZObject* p = StrMap_getRef(&(obj.members),tmp.c_str());
    
    void* thefunc = NULL;
    if(!p)
    {
        #ifdef _WIN32
          HINSTANCE handle = KlassObj_getMember(&obj,".libhandle").ptr;
          auto fn = GetProcAddress(name.c_str(),handle);
        #else
          void* handle = KlassObj_getMember(&obj,".libhandle").ptr;
          void* fn = dlsym(handle,name);
        #endif

        if(!fn)
          return Z_Err(NameError,"Unable to load function from library!");
        ZObject z = ZObjFromStr(tmp.c_str());
        KlassObj_setMember(&obj, AS_STR(z)->val, ZObjFromPtr(fn));
        vm_markImportant(z.ptr);
        thefunc = fn;
    }
    else
    {
      thefunc = p->ptr;
    }

    //libffi magic
    if(argTypes.size < 1)
      return Z_Err(ValueError,"List must be at least of size 1");
    ffi_type** ffiargs = new ffi_type*[argTypes.size-1];
    size_t l = argTypes.size-1;
    ffi_type* ret_type = ffi_types[argTypes.arr[0].i];
    void** values = NULL;
    if(l!=0)
      values = new void*[l];
    size_t j = 3;

    char buffer[512];
    size_t k = 0;
    uint8_t* buffer1 = new uint8_t[l*8];

    for(size_t i=0;i<l;i++)
    {
      ffiargs[i] = ffi_types[argTypes.arr[i+1].i];
      switch(argTypes.arr[i+1].i)
      {
        case C_INT:
        {
            if(args[j].type != Z_INT)
            {
              snprintf(buffer,512,"Argument %zu is not an integer!",j);
              return Z_Err(TypeError,buffer);
            }
            values[i] = (void*)&(args[j].i);
            break;
        }
        case C_SHORT:
        {
            if(args[j].type != Z_INT)
            {
              snprintf(buffer,512,"Argument %zu is not an integer!",j);
              return Z_Err(TypeError,buffer);
            }
            short tmp = (short)args[j].i;
            memcpy(buffer1+k,&tmp,sizeof(short));
            values[i] = (void*)(buffer+k);
            k += sizeof(short);
            break;
        }
        case C_LONG:
        {
            if(args[j].type != Z_INT)
            {
              snprintf(buffer,512,"Required int32 for C long. Argument %zu is not an integer!",j);
              return Z_Err(TypeError,buffer);
            }
            if(sizeof(long) == 8)
            {
              long tmp = (long)args[j].i;
              memcpy(buffer1+k,&tmp,sizeof(long));
              values[i] = (void*)(buffer1+k);
              k += sizeof(long);
            }
            else
              values[i] = (void*)&(args[j].i);
            break;
        }
        case C_LLONG:
        case C_SIZET:
        {
            if(args[j].type != Z_INT64)
            {
              snprintf(buffer,512,"Argument %zu is not an integer 64 bit!",j);
              return Z_Err(TypeError,buffer);
            }
            values[i] = (void*)&(args[j].l);
            break;
        }
        case C_DOUBLE:
        {
          if(args[j].type != Z_FLOAT)
          {
            snprintf(buffer,512,"Argument %zu is not a float!",j);
            return Z_Err(TypeError,buffer);
          }
          values[i] = (void*)&(args[j].f);
          break;
        }
        case C_FLOAT:
        {
          if(args[j].type != Z_FLOAT)
          {
            snprintf(buffer,512,"Argument %zu is not a float!",j);
            return Z_Err(TypeError,buffer);
          }
          float tmp = (float)args[j].f;
          memcpy(buffer1+k,&tmp,sizeof(tmp));
          values[i] = (void*)(buffer1+k);
          k += sizeof(tmp);
          break;
        }
        case C_CHAR:
        {
          if(args[j].type != Z_BYTE)
          {
            snprintf(buffer,512,"Argument %zu is not a byte!",j);
            return Z_Err(TypeError,buffer);
          }
          char tmp = (char)args[j].i;
          memcpy(buffer1+k,&tmp,sizeof(tmp));
          values[i] = (void*)(buffer1+k);
          k += sizeof(tmp);
          break;
        }
        case C_BOOL:
        {
          if(args[j].type != Z_BOOL)
          {
            snprintf(buffer,512,"Argument %zu is not a boolean!",j);
            return Z_Err(TypeError,buffer);
          }
          char tmp = (char)args[j].i;
          memcpy(buffer1+k,&tmp,sizeof(tmp));
          values[i] = (void*)(buffer1+k);
          k += sizeof(tmp);
          break;
        }
        case C_PTR:
        {
          if(args[j].type != Z_POINTER)
          {
            snprintf(buffer,512,"Argument %zu is not a pointer!",j);
            return Z_Err(TypeError,buffer);
          }
          values[i] = (void*)&(args[j].ptr);
          break;
        }
        case C_STR:
        {
          if(args[j].type != Z_STR)
          {
              snprintf(buffer,512,"Argument %zu is not a string!",j);
              return Z_Err(TypeError,buffer);            
          }
          values[i] = (void*)( & (AS_STR(args[j])->val) );
          break;
        }
        case C_VOID:
        {
            return Z_Err(Error,"C_VOID type must not be used as argument type!");
            break;
        }

        default:
        {
            return Z_Err(Error,"Unknown type specified in list!");
            break;
        }
      }
      j++;
    }
    //
    ffi_cif cif;
    ffi_status status = ffi_prep_cif(&cif,FFI_DEFAULT_ABI,l,ret_type,ffiargs);
    if(status != FFI_OK)
        return Z_Err(Error,"ffi_prep_cif() failed.");
    
    ffi_call(&cif,FFI_FN(thefunc),(void*)retAddr[argTypes.arr[0].i],values);
    delete[] values;
    delete[] ffiargs;
    delete[] buffer1;
    switch(argTypes.arr[0].i)
    {
      case C_CHAR:
        return ZObjFromByte(cret);
      case C_SHORT:
        return ZObjFromInt(sret);
      case C_BOOL:
        return ZObjFromBool(bret);
      case C_INT:
        return ZObjFromInt(iret);
      case C_LLONG:
        return ZObjFromInt64(i64ret);
      case C_SIZET:
        return ZObjFromInt64(sztret);
      case C_LONG:
      {
        if(sizeof(long) == 8)
          return ZObjFromInt64(i64ret);
        return ZObjFromInt(lret);
      }
      case C_FLOAT:
        return ZObjFromDouble(fret);
      case C_DOUBLE:
        return ZObjFromDouble(dret);
      case C_STR:
        return ZObjFromStr(strret);
      case C_PTR:
        return ZObjFromPtr(pret);   
    }
    return nil;
}