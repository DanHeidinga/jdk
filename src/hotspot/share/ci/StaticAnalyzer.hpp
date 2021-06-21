#ifndef SHARE_CI_STATICANALYZER_HPP
#define SHARE_CI_STATICANALYZER_HPP

#include "ci/ciClassList.hpp"
#include "ci/ciEnv.hpp"
#include "ci/ciObjectFactory.hpp"
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

public:
	StaticAnalyzer();
	
	void analyze_method(Method *method);

};

#endif /* SHARE_CI_STATICANALYZER_HPP */