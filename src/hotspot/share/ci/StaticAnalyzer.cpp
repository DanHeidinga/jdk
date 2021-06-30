#include "ci/StaticAnalyzer.hpp"

#include "ci/ciClassList.hpp"
#include "ci/ciObjectFactory.hpp"
#include "ci/ciTypeFlow.hpp"
#include "ci/ciConstant.hpp"
#include "ci/ciField.hpp"
#include "ci/ciMethodBlocks.hpp"
#include "ci/ciStreams.hpp"
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

void recordReferncedType(ciType *type) {
    printf("ReferencedType: >");
    fflush(stdout);
    type->print_name();
    printf("<\n");
}

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
        type_flow->print_on(tty);

        printf("=== Processing the signature stream ===\n");
        for (ciSignatureStream str(target->signature());
            !str.at_return_type();
            str.next()
        ) {
            recordReferncedType(str.type());
        }
        printf("=== Processing the bytecodes per block stream ===\n");
        do_analysis(target);
    }
}

// based on bcEscapeAnalyzer
void StaticAnalyzer::do_analysis(ciMethod *target) {
  Arena* arena = CURRENT_ENV->arena();
  // identify basic blocks
  ciMethodBlocks *methodBlocks = target->get_method_blocks();

  iterate_blocks(arena, target, methodBlocks);
}

// based on bcEscapeAnalyzer
void StaticAnalyzer::iterate_blocks(Arena *arena, ciMethod *method, ciMethodBlocks *methodBlocks) {
  int numblocks = methodBlocks->num_blocks();
  int stkSize   = method->max_stack();
  int numLocals = method->max_locals();

  GrowableArray<ciBlock *> worklist(arena, numblocks / 4, 0, NULL);
  GrowableArray<ciBlock *> successors(arena, 4, 0, NULL);

  methodBlocks->clear_processed();

  // initialize block 0 state from method signature
  ciSignature* sig = method->signature();
  ciBlock* first_blk = methodBlocks->block_containing(0);
  int fb_i = first_blk->index();
  for (int i = 0; i < sig->count(); i++) {
    ciType* t = sig->type_at(i);
    if (!t->is_primitive_type()) {
      recordReferncedType(t);
    }
  }

  worklist.push(first_blk);
  while(worklist.length() > 0) {
    ciBlock *blk = worklist.pop();
    if (blk->is_handler() || blk->is_ret_target()) {
      // for an exception handler or a target of a ret instruction, we assume the worst case,
      // that any variable could contain any argument
      
    } else {
      
    }
    iterate_one_block(blk, /* state, */ successors);
    // if this block has any exception handlers, push them
    // onto successor list
    if (blk->has_handler()) {
      DEBUG_ONLY(int handler_count = 0;)
      int blk_start = blk->start_bci();
      int blk_end = blk->limit_bci();
      for (int i = 0; i < numblocks; i++) {
        ciBlock *b = methodBlocks->block(i);
        if (b->is_handler()) {
          int ex_start = b->ex_start_bci();
          int ex_end = b->ex_limit_bci();
          if ((ex_start >= blk_start && ex_start < blk_end) ||
              (ex_end > blk_start && ex_end <= blk_end)) {
            successors.push(b);
          }
          DEBUG_ONLY(handler_count++;)
        }
      }
      assert(handler_count > 0, "must find at least one handler");
    }
    // merge computed variable state with successors
    // while(successors.length() > 0) {
    //   ciBlock *succ = successors.pop();
    //   merge_block_states(blockstates, succ, &state);
    //   if (!succ->processed())
    //     worklist.push(succ);
    // }
  }
}

// based on BCEscapeAnalyzer
void StaticAnalyzer::iterate_one_block(ciBlock *blk, GrowableArray<ciBlock *> &successors) {
}

void StaticAnalyzer::thread_entry(JavaThread* thread, TRAPS) {
    printf("\n ===>StaticAnalyzer::thread_entry\n");
    CompileBroker::analyze_thread_loop(CompilerThread::cast(thread));
}