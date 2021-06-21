#include "ci/StaticAnalyzer.hpp"

#include "ci/ciClassList.hpp"
#include "ci/ciObjectFactory.hpp"
#include "classfile/vmClassMacros.hpp"
#include "code/debugInfoRec.hpp"
#include "code/dependencies.hpp"
#include "code/exceptionHandlerTable.hpp"
#include "compiler/compilerThread.hpp"
#include "oops/methodData.hpp"
#include "runtime/thread.hpp"

StaticAnalyzer::StaticAnalyzer()
	: ci_env((CompileTask*)NULL) { }

void StaticAnalyzer::analyze_method(Method *initialMethod) {
	Thread *thead = Thread::current();
	printf("StaticAnalyzer isCompilerThread=%d\n", Thread::current()->is_Compiler_thread() ? 1 : 0);
    
    ciMethod* target = ci_env.get_method(initialMethod); // get_method_from_handle
    
    ciTypeFlow* type_flow = target->get_flow_analysis();

	printf("StaticAnalyzer::analyze_method generated flow\n");
}