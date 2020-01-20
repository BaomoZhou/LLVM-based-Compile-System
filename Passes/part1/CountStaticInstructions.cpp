#include "llvm/Pass.h"
#include "llvm/IR/Function.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/IR/InstIterator.h"
#include <map> 


using namespace llvm;

namespace {
struct CountInstruPass : public FunctionPass {
  static char ID;
  CountInstruPass() : FunctionPass(ID) {}

  bool runOnFunction(Function &F) override {
      std::map<const char*,int> InstruCountMap;
      
      for (inst_iterator I = inst_begin(F), E = inst_end(F); I != E; ++I) {
          InstruCountMap[&*I -> getOpcodeName()] ++;
      }
      std::map<const char*, int>::iterator iter;
      for (iter = InstruCountMap.begin(); iter != InstruCountMap.end(); iter++) {
          errs() << iter->first << "\t" << iter->second << "\n";
      }
      return false;
  }
}; // end of struct CountInstruPass
}  // end of anonymous namespace

char CountInstruPass::ID = 0;
static RegisterPass<CountInstruPass> X("cse231-csi", "Static Intructions Counting Pass",
                             false /* Only looks at CFG */,
                             false /* Analysis Pass */);