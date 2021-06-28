#include "ci/StaticAnalyzer.hpp"

#include "ci/ciClassList.hpp"
#include "ci/ciObjectFactory.hpp"
#include "classfile/vmClassMacros.hpp"
#include "code/debugInfoRec.hpp"
#include "code/dependencies.hpp"
#include "code/exceptionHandlerTable.hpp"
#include "compiler/compilerThread.hpp"
#include "compiler/compileTask.hpp"
#include "compiler/compileBroker.hpp"
#include "oops/methodData.hpp"
#include "runtime/thread.hpp"
#include "runtime/interfaceSupport.inline.hpp"

StaticAnalyzer::StaticAnalyzer()
    : ci_env((CompileTask*)NULL) { }

StaticAnalyzer::StaticAnalyzer(ciEnv *env)
    : ci_env((CompileTask*)NULL)
    , env(env) { }

void StaticAnalyzer::analyze_method(Method *initialMethod) {
    Thread *thead = Thread::current();
    printf("StaticAnalyzer isCompilerThread=%d\n", Thread::current()->is_Compiler_thread() ? 1 : 0);
    
    ciMethod* target = ci_env.get_method(initialMethod); // get_method_from_handle
    {   // ciMethod::load_code does ThreadInVMFroMNative.
        // V  [libjvm.dylib+0x37341d]  ThreadInVMfromNative::ThreadInVMfromNative(JavaThread*)+0x1d
        // V  [libjvm.dylib+0x4b3273]  ciMethod::load_code()+0x33
        // V  [libjvm.dylib+0x2ed4cb]  ciMethod::code()+0x2b
        // V  [libjvm.dylib+0x2ed424]  ciBytecodeStream::reset_to_method(ciMethod*)+0x64
        // V  [libjvm.dylib+0x2ed3ab]  ciBytecodeStream::ciBytecodeStream(ciMethod*)+0x3b
        // V  [libjvm.dylib+0x2e9fed]  ciBytecodeStream::ciBytecodeStream(ciMethod*)+0x1d
        // V  [libjvm.dylib+0x4ba1da]  ciMethodBlocks::do_analysis()+0x2a
        // V  [libjvm.dylib+0x4babd1]  ciMethodBlocks::ciMethodBlocks(Arena*, ciMethod*)+0x341
        // V  [libjvm.dylib+0x4bad55]  ciMethodBlocks::ciMethodBlocks(Arena*, ciMethod*)+0x25
        // V  [libjvm.dylib+0x4b83bc]  ciMethod::get_method_blocks()+0x6c
        // V  [libjvm.dylib+0x4e9f9d]  ciTypeFlow::ciTypeFlow(ciEnv*, ciMethod*, int)+0xed
        // V  [libjvm.dylib+0x4ea05b]  ciTypeFlow::ciTypeFlow(ciEnv*, ciMethod*, int)+0x2b
        // V  [libjvm.dylib+0x4b4074]  ciMethod::get_flow_analysis()+0x74

        ThreadToNativeFromVM ttn(CompilerThread::current()); //Dan
        
        ciTypeFlow* type_flow = target->get_flow_analysis();

        printf("StaticAnalyzer::analyze_method generated flow\n");
    }
}

void StaticAnalyzer::thread_entry(JavaThread* thread, TRAPS) {
    printf("\n ===>StaticAnalyzer::thread_entry\n");
    CompileBroker::analyze_thread_loop(CompilerThread::cast(thread));
}