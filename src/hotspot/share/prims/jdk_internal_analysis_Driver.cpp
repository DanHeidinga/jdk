#include "precompiled.hpp"
#include "jni.h"
#include "jvm.h"
#include "classfile/vmSymbols.hpp"
#include "memory/allocation.inline.hpp"
#include "memory/resourceArea.hpp"
#include "oops/oop.inline.hpp"
#include "runtime/interfaceSupport.inline.hpp"
#include "ci/compilerInterface.hpp"
#include "ci/StaticAnalyzer.hpp"


#define ANALYSIS_DRIVER_ENTRY(result_type, header) \
  JVM_ENTRY(result_type, header)

#define ANALYSIS_DRIVER_END JVM_END

ANALYSIS_DRIVER_ENTRY(void, Driver_Analyze(JNIEnv *env, jobject unused, jobject method)) {
    jmethodID mid = 0;
    {
      ThreadToNativeFromVM ttn(thread);
      mid = env->FromReflectedMethod(method);
    }
    Method* initialMethod = Method::checked_resolve_jmethod_id(mid);
    {
    // NoHandleMark  nhm;
    
    // Must switch to native to allocate ci_env
    ThreadToNativeFromVM ttn(thread);
    StaticAnalyzer analyzer;
    analyzer.analyze_method(initialMethod);
    }

} ANALYSIS_DRIVER_END

/// JVM_RegisterDriverMethods

#define CC (char*)  /*cast a literal from (const char*)*/
#define FN_PTR(f) CAST_FROM_FN_PTR(void*, &f)
#define JLRM "Ljava/lang/reflect/Method;"
#define JLS "Ljava/lang/String;"
#define CL_ARGS     CC "(" JLS "IIJ)" BB
#define CBA_ARGS    CC "(" JLS "II[BI)" BB

static JNINativeMethod drivermethods[] = {
  {CC "analyze",              CC "(" JLRM ")V" , FN_PTR(Driver_Analyze)},
};

#undef CBA_ARGS
#undef CL_ARGS
#undef JLS
#undef JLRM
#undef FN_PTR
#undef CC

// This one function is exported, used by NativeLookup.
JVM_ENTRY(void, JVM_RegisterDriverMethods(JNIEnv *env, jclass driverclass))
  {
    ThreadToNativeFromVM ttnfv(thread);
    int ok = env->RegisterNatives(driverclass, drivermethods, sizeof(drivermethods)/sizeof(JNINativeMethod));
    guarantee(ok == 0, "register driver natives");
  }
JVM_END