// Do NOT change. Changes will be lost next time file is generated

#define R__DICTIONARY_FILENAME CMMonitorDict
#define R__NO_DEPRECATION

/*******************************************************************/
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#define G__DICTIONARY
#include "RConfig.h"
#include "TClass.h"
#include "TDictAttributeMap.h"
#include "TInterpreter.h"
#include "TROOT.h"
#include "TBuffer.h"
#include "TMemberInspector.h"
#include "TInterpreter.h"
#include "TVirtualMutex.h"
#include "TError.h"

#ifndef G__ROOT
#define G__ROOT
#endif

#include "RtypesImp.h"
#include "TIsAProxy.h"
#include "TFileMergeInfo.h"
#include <algorithm>
#include "TCollectionProxyInfo.h"
/*******************************************************************/

#include "TDataMember.h"

// The generated code does not explicitly qualifies STL entities
namespace std {} using namespace std;

// Header files passed as explicit arguments
#include "/home/mgericke/MOLLERTestDAQ/CurrentMode/DataMonitor/include/CMMonitor.h"

// Header files passed via #pragma extra_include

namespace ROOT {
   static void delete_CMMonitor(void *p);
   static void deleteArray_CMMonitor(void *p);
   static void destruct_CMMonitor(void *p);

   // Function generating the singleton type initializer
   static TGenericClassInfo *GenerateInitInstanceLocal(const ::CMMonitor*)
   {
      ::CMMonitor *ptr = 0;
      static ::TVirtualIsAProxy* isa_proxy = new ::TInstrumentedIsAProxy< ::CMMonitor >(0);
      static ::ROOT::TGenericClassInfo 
         instance("CMMonitor", ::CMMonitor::Class_Version(), "CMMonitor.h", 65,
                  typeid(::CMMonitor), ::ROOT::Internal::DefineBehavior(ptr, ptr),
                  &::CMMonitor::Dictionary, isa_proxy, 4,
                  sizeof(::CMMonitor) );
      instance.SetDelete(&delete_CMMonitor);
      instance.SetDeleteArray(&deleteArray_CMMonitor);
      instance.SetDestructor(&destruct_CMMonitor);
      return &instance;
   }
   TGenericClassInfo *GenerateInitInstance(const ::CMMonitor*)
   {
      return GenerateInitInstanceLocal((::CMMonitor*)0);
   }
   // Static variable to force the class initialization
   static ::ROOT::TGenericClassInfo *_R__UNIQUE_DICT_(Init) = GenerateInitInstanceLocal((const ::CMMonitor*)0x0); R__UseDummy(_R__UNIQUE_DICT_(Init));
} // end of namespace ROOT

//______________________________________________________________________________
atomic_TClass_ptr CMMonitor::fgIsA(0);  // static to hold class pointer

//______________________________________________________________________________
const char *CMMonitor::Class_Name()
{
   return "CMMonitor";
}

//______________________________________________________________________________
const char *CMMonitor::ImplFileName()
{
   return ::ROOT::GenerateInitInstanceLocal((const ::CMMonitor*)0x0)->GetImplFileName();
}

//______________________________________________________________________________
int CMMonitor::ImplFileLine()
{
   return ::ROOT::GenerateInitInstanceLocal((const ::CMMonitor*)0x0)->GetImplFileLine();
}

//______________________________________________________________________________
TClass *CMMonitor::Dictionary()
{
   fgIsA = ::ROOT::GenerateInitInstanceLocal((const ::CMMonitor*)0x0)->GetClass();
   return fgIsA;
}

//______________________________________________________________________________
TClass *CMMonitor::Class()
{
   if (!fgIsA.load()) { R__LOCKGUARD(gInterpreterMutex); fgIsA = ::ROOT::GenerateInitInstanceLocal((const ::CMMonitor*)0x0)->GetClass(); }
   return fgIsA;
}

//______________________________________________________________________________
void CMMonitor::Streamer(TBuffer &R__b)
{
   // Stream an object of class CMMonitor.

   if (R__b.IsReading()) {
      R__b.ReadClassBuffer(CMMonitor::Class(),this);
   } else {
      R__b.WriteClassBuffer(CMMonitor::Class(),this);
   }
}

namespace ROOT {
   // Wrapper around operator delete
   static void delete_CMMonitor(void *p) {
      delete ((::CMMonitor*)p);
   }
   static void deleteArray_CMMonitor(void *p) {
      delete [] ((::CMMonitor*)p);
   }
   static void destruct_CMMonitor(void *p) {
      typedef ::CMMonitor current_t;
      ((current_t*)p)->~current_t();
   }
} // end of namespace ROOT for class ::CMMonitor

namespace {
  void TriggerDictionaryInitialization_libCMMonitorDict_Impl() {
    static const char* headers[] = {
"/home/mgericke/MOLLERTestDAQ/CurrentMode/DataMonitor/include/CMMonitor.h",
0
    };
    static const char* includePaths[] = {
"/usr/local/root/include",
"/home/mgericke/MOLLERTestDAQ/CurrentMode/DataMonitor/include",
"/home/mgericke/MOLLERTestDAQ/CurrentMode/DataMonitor/lib/include",
"/home/mgericke/MOLLERTestDAQ/CurrentMode/DataMonitor",
"/usr/local/root/include/",
"/home/mgericke/MOLLERTestDAQ/CurrentMode/DataMonitor/build/",
0
    };
    static const char* fwdDeclCode = R"DICTFWDDCLS(
#line 1 "libCMMonitorDict dictionary forward declarations' payload"
#pragma clang diagnostic ignored "-Wkeyword-compat"
#pragma clang diagnostic ignored "-Wignored-attributes"
#pragma clang diagnostic ignored "-Wreturn-type-c-linkage"
extern int __Cling_AutoLoading_Map;
class __attribute__((annotate("$clingAutoload$/home/mgericke/MOLLERTestDAQ/CurrentMode/DataMonitor/include/CMMonitor.h")))  CMMonitor;
)DICTFWDDCLS";
    static const char* payloadCode = R"DICTPAYLOAD(
#line 1 "libCMMonitorDict dictionary payload"


#define _BACKWARD_BACKWARD_WARNING_H
// Inline headers
#include "/home/mgericke/MOLLERTestDAQ/CurrentMode/DataMonitor/include/CMMonitor.h"

#undef  _BACKWARD_BACKWARD_WARNING_H
)DICTPAYLOAD";
    static const char* classesHeaders[] = {
"CMMonitor", payloadCode, "@",
nullptr
};
    static bool isInitialized = false;
    if (!isInitialized) {
      TROOT::RegisterModule("libCMMonitorDict",
        headers, includePaths, payloadCode, fwdDeclCode,
        TriggerDictionaryInitialization_libCMMonitorDict_Impl, {}, classesHeaders, /*hasCxxModule*/false);
      isInitialized = true;
    }
  }
  static struct DictInit {
    DictInit() {
      TriggerDictionaryInitialization_libCMMonitorDict_Impl();
    }
  } __TheDictionaryInitializer;
}
void TriggerDictionaryInitialization_libCMMonitorDict() {
  TriggerDictionaryInitialization_libCMMonitorDict_Impl();
}
