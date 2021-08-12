#ifndef SHARE_CI_STATICANALYZER_HPP
#define SHARE_CI_STATICANALYZER_HPP

#include "ci/ciClassList.hpp"
#include "ci/ciEnv.hpp"
#include "ci/ciObjectFactory.hpp"
#include "ci/ciMethodBlocks.hpp"
#include "classfile/vmClassMacros.hpp"
#include "code/debugInfoRec.hpp"
#include "code/dependencies.hpp"
#include "code/exceptionHandlerTable.hpp"
#include "compiler/compilerThread.hpp"
#include "oops/methodData.hpp"
#include "runtime/thread.hpp"

class StaticAnalyzer {
private:
    ciEnv ci_env;
	ciEnv *env;

	ciMethod *_method;
	ciMethodBlocks *_methodBlocks;
	//TODO: GrowableArray of ciKlasses to load
	GrowableArray<ciType *> _discovered_klasses;

public:
	StaticAnalyzer();
	StaticAnalyzer(ciEnv *env);
	
	void analyze_method(Method *method);
	void do_analysis(ciMethod *target);
	void iterate_blocks(Arena *arena, ciMethod *method, ciMethodBlocks *methodBlocks);
	void iterate_one_block(ciBlock *blk, ciMethod *method, ciMethodBlocks *methodBlocks, GrowableArray<ciBlock *> &successors);

	static void thread_entry(JavaThread* thread, TRAPS);

	void enqueueMethod(ciMethod *method);
	void recordReferencedType(ciType *type);
};



#endif /* SHARE_CI_STATICANALYZER_HPP */