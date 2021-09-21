#ifndef SHARE_CI_BCREACHABILITYANALYZER_HPP
#define SHARE_CI_BCREACHABILITYANALYZER_HPP

#include "ci/ciMethod.hpp"
#include "ci/ciMethodBlocks.hpp"
#include "ci/ciType.hpp"
#include "utilities/growableArray.hpp"

class BCReachabilityAnalyzer : public ResourceObj {

private:
  Arena *_arena;        // ciEnv arena
  ciMethod *_method;
  ciMethodBlocks *_methodBlocks;

  BCReachabilityAnalyzer *_parent;

  GrowableArray<ciType *> _discovered_klasses;

private:
  void iterate_blocks();
  void iterate_one_block(ciBlock *blk, GrowableArray<ciBlock *> &successors);

  void recordReferencedType(ciType *type);
  void enqueueMethod(ciMethod *method);


 public:
  BCReachabilityAnalyzer(ciMethod* method, BCReachabilityAnalyzer* parent = NULL);

  BCReachabilityAnalyzer* parent() { return _parent; }
  ciMethod* method() const { return _method; }
};

#endif //SHARE_CI_BCREACHABILITYANALYZER_HPP