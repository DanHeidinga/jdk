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
#include "compiler/compileBroker.hpp"
#include "runtime/jniHandles.hpp"


#define ANALYSIS_DRIVER_ENTRY(result_type, header) \
  JVM_ENTRY(result_type, header)

#define ANALYSIS_DRIVER_END JVM_END

ANALYSIS_DRIVER_ENTRY(void, Driver_Analyze(JNIEnv *env, jobject unused, jobject method, jobject jthread)) {
    jmethodID mid = 0;
    {
      ThreadToNativeFromVM ttn(thread);
      mid = env->FromReflectedMethod(method);
    }
    Method* initialMethod = Method::checked_resolve_jmethod_id(mid);
printf("\n\n==> Driver_Analyze: initialMethod=%p\n", initialMethod);
    ThreadsListHandle tlh(thread);
    JavaThread* analysis_thread = NULL;
    bool is_alive = tlh.cv_internal_thread_to_JavaThread(jthread, &analysis_thread, NULL);
    printf("\n\n==> Driver_Analyze: thread_alive=%s\n", is_alive ? "alive" : "dead");
    {
    // NoHandleMark  nhm;
    
    // Must switch to native to allocate ci_env
    // ThreadToNativeFromVM ttn(thread);
    // StaticAnalyzer analyzer;
    // analyzer.analyze_method(initialMethod);

    // add CompileTask for this
    //TODO start here - analysis_thread->queue->Add(CompileTask::allocate(initialMethod))
    methodHandle mh(thread, initialMethod);
    CompileBroker::analyze_method_base(mh,
                    CompLevel::CompLevel_full_optimization,
                    CompileTask::CompileReason::Reason_MustBeCompiled,
                    true,
                    thread); //analysis_thread);
    printf("\n\n==> Driver_Analyze: submitted method\n");
    }

} ANALYSIS_DRIVER_END

ANALYSIS_DRIVER_ENTRY(jobject, Driver_Get_Compiler_Thread(JNIEnv *env, jobject unused)) {
    JavaThread *jt = CompileBroker::create_analysis_compiler_thread(thread);
    return JNIHandles::make_local(THREAD, jt->threadObj());
} ANALYSIS_DRIVER_END

/// JVM_RegisterDriverMethods

#define CC (char*)  /*cast a literal from (const char*)*/
#define FN_PTR(f) CAST_FROM_FN_PTR(void*, &f)
#define JLRM "Ljava/lang/reflect/Method;"
#define JLS "Ljava/lang/String;"
#define JLT "Ljava/lang/Thread;"
#define CL_ARGS     CC "(" JLS "IIJ)" BB
#define CBA_ARGS    CC "(" JLS "II[BI)" BB

static JNINativeMethod drivermethods[] = {
  {CC "analyze",              CC "(" JLRM JLT ")V" , FN_PTR(Driver_Analyze)},
  {CC "get_compiler_thread",  CC "()" JLT, FN_PTR(Driver_Get_Compiler_Thread)},
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