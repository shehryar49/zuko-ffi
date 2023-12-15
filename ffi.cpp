/*
FFI Module for Plutonium
Uses libffi and currently supports primitive types only
Created by Shahryar Ahmad (MIT License)
*/

#include <stdio.h>
#include <stdlib.h>
#include <ffi.h> //libffi
#include "ffi.h"

using namespace  std;

ZObject nil;
enum ctypes
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

Klass* libklass;
ZObject init()
{
    nil.type = Z_NIL;
    Module* ffiModule = vm_allocModule();
    ffiModule->name = "ffi";
    ffiModule->addNativeFunction("loadLibrary",&LOAD_LIB);
    ffiModule->members.emplace("C_INT",ZObjFromInt(C_INT));
    ffiModule->members.emplace("C_CHAR",ZObjFromInt(C_CHAR));
    ffiModule->members.emplace("C_LONG",ZObjFromInt(C_LONG));
    ffiModule->members.emplace("C_LLONG",ZObjFromInt(C_LLONG));
    ffiModule->members.emplace("C_DOUBLE",ZObjFromInt(C_DOUBLE));
    ffiModule->members.emplace("C_FLOAT",ZObjFromInt(C_FLOAT));
    ffiModule->members.emplace("C_SHORT",ZObjFromInt(C_SHORT));
    ffiModule->members.emplace("C_SIZET",ZObjFromInt(C_SIZET));
    ffiModule->members.emplace("C_STR",ZObjFromInt(C_STR));
    ffiModule->members.emplace("C_BOOL",ZObjFromInt(C_BOOL));
    ffiModule->members.emplace("C_PTR",ZObjFromInt(C_PTR));
    ffiModule->members.emplace("C_VOID",ZObjFromInt(C_VOID));
    
    //
    //Library class
    libklass = vm_allocKlass();
    libklass->members.emplace("call",ZObjFromMethod("call",&LIB_CALL,libklass));
    vm_markImportant((void*)libklass);
    return ZObjFromModule(ffiModule);
}
ZObject LOAD_LIB(ZObject* args,int32_t n)
{
  if(n!=1)
    return Z_Err(ArgumentError,"1 argument required!");
  if(args[0].type != Z_STR)
    return Z_Err(TypeError,"String argument required!");
  const string& path = AS_STR(args[0]);
  #ifdef _WIN32
    HINSTANCE handle = LoadLibrary(path.c_str());
    if(!handle)
    {
      return Z_Err(Error,"Unable to load library: "+to_string(GetLastError()));
    }
  #else
    void* handle = dlopen(path.c_str(),RTLD_LAZY);
    if(!handle)
    {
      return Z_Err(Error,"Unable to load library: "+(string)dlerror());
    }
  #endif
  
  KlassObject* obj = vm_allocKlassObject();
  obj->klass = libklass;
  obj->members = libklass->members;
  obj->members.emplace(".libhandle",ZObjFromPtr(handle));
  return ZObjFromKlassObj(obj);
}

//Libklass methods
ZObject LIB_CALL(ZObject* args,int32_t n)
{
    if(n < 3)
      return Z_Err(ArgumentError,"call(self,name,argTypes,...) takes atleast 3 arguments");
    if(args[0].type != Z_OBJ || ((KlassObject*)args[0].ptr) -> klass!=libklass)
      return Z_Err(TypeError,"First argument must be an object of library class.");
    if(args[1].type != Z_STR)
      return Z_Err(TypeError,"Second argument must be a string!");
    if(args[2].type != Z_LIST)
      return Z_Err(TypeError,"Third argument must be a list!");
    const ZList& argTypes = *(ZList*)args[2].ptr;
    const string& name = AS_STR(args[1]);
    KlassObject& obj = *((KlassObject*)args[0].ptr);
    static string tmp;
    tmp = "."+name;
    std::unordered_map<string,ZObject>::iterator it;
    void* thefunc = NULL;
    if((it = obj.members.find(tmp)) == obj.members.end())
    {
        #ifdef _WIN32
          HINSTANCE handle = obj.members[".libhandle"].ptr;
          auto fn = GetProcAddress(name.c_str(),handle);
        #else
          void* handle = obj.members[".libhandle"].ptr;
          void* fn = dlsym(handle,name.c_str());
        #endif

        if(!fn)
          return Z_Err(NameError,"Unable to load function from library!");
        obj.members.emplace(tmp,ZObjFromPtr(fn));
        thefunc = fn;
    }
    else
    {
      thefunc = obj.members[tmp].ptr;
    }

    //libffi magic
    if(argTypes.size() < 1)
      return Z_Err(ValueError,"List must be at least of size 1");
    ffi_type** ffiargs = new ffi_type*[argTypes.size()-1];
    size_t l = argTypes.size()-1;

    ffi_type* ret_type = ffi_types[argTypes[0].i];
    void** values = NULL;
    if(l!=0)
      values = new void*[l];
    size_t j = 3;
    vector<void*> ptrs;
    for(size_t i=0;i<l;i++)
    {
      ffiargs[i] = ffi_types[argTypes[i+1].i];
      switch(argTypes[i+1].i)
      {
        case C_INT:
        {
            if(args[j].type != Z_INT)
              return Z_Err(TypeError,"Argument "+to_string(j)+" is not an integer!");
            values[i] = (void*)&(args[j].i);
            break;
        }
        case C_LLONG:
        {
            if(args[j].type != Z_INT64)
              return Z_Err(TypeError,"Argument "+to_string(j)+" is not an integer 64 bit!");
            values[i] = (void*)&(args[j].i);
            break;
        }
        case C_DOUBLE:
        {
          if(args[j].type != Z_FLOAT)
            return Z_Err(TypeError,"Argument "+to_string(j)+" is not plutonium float!");
          values[i] = (void*)&(args[j].f);
          break;
        }
        case C_STR:
        {
          if(args[j].type != Z_STR)
            return Z_Err(TypeError,"Argument "+to_string(j)+" is not a string!");
          ptrs.push_back((void*)((string*)args[j].ptr)->c_str());
          values[i] = (void*)&ptrs.back();
          char* str = (char*)(*((void**)values[i]));
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
    
    ffi_call(&cif,FFI_FN(thefunc),(void*)retAddr[argTypes[0].i],values);
    delete[] values;
    delete[] ffiargs;
    switch(argTypes[0].i)
    {
      case C_CHAR:
        return ZObjFromInt(iret);
      case C_SHORT:
        return ZObjFromInt(iret);
      case C_BOOL:
        return ZObjFromInt(iret);
      case C_INT:
        return ZObjFromInt(iret);
      case C_LLONG:
        return ZObjFromInt64(i64ret);
      case C_SIZET:
        return ZObjFromInt64(sztret);
      case C_LONG:
        return ZObjFromInt64(i64ret);
      case C_FLOAT:
        return ZObjFromDouble(fret);
      case C_DOUBLE:
        return ZObjFromDouble(dret);
      case C_STR:
        return ZObjFromStr((string)strret);
      case C_PTR:
        return ZObjFromPtr(pret);   
    }
    return nil;
}