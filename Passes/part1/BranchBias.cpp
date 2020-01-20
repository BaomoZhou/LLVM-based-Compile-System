#include "llvm/Pass.h"
#include "llvm/IR/Function.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/IR/Type.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/Value.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Instruction.h"
#include "llvm/IR/BasicBlock.h"
#include "llvm/IR/DerivedTypes.h"

#include <string>
#include <map>
#include <vector>

using namespace std;
using namespace llvm;

namespace {
struct CountBranchBias: public FunctionPass {
  static char ID;
  CountBranchBias() : FunctionPass(ID) {}

  bool runOnFunction(Function &F) override {
      Module *ModPar = F.getParent();
      LLVMContext &Context = ModPar->getContext();
      // reverse all blocks of a function
      Function::iterator Block;
      for(Block = F.begin(); Block != F.end(); Block++) {

            vector<Value*> args;

            // insert updateInstrInfo at each end of each block
            IRBuilder<> Builder(&*Block);
            Builder.SetInsertPoint(Block->getTerminator());
            FunctionCallee UpdateFunc = ModPar->getOrInsertFunction(
                "updateInstrInfo", // function name
                Type::getVoidTy(Context), // return type
                Type::getInt32Ty(Context), // first parameter
                Type::getInt32PtrTy(Context), // second parameter
                Type::getInt32PtrTy(Context)  // third parameter
            );

            // branch should be at each end of each block
            BranchInst *InstBranch = dyn_cast<BranchInst>(Block->getTerminator());
            
            if (InstBranch != NULL && InstBranch->isConditional()){
                args.push_back(InstBranch->getCondition());
                Builder.CreateCall(UpdateFunc, args);
            }

            BasicBlock::iterator BlkPrint;
            for(BlkPrint = Block->begin(); BlkPrint != Block->end(); BlkPrint++){
                if ((string)BlkPrint->getOpcodeName() == "ret"){
                    // insert printOutInstrInfo at here
                    Builder.SetInsertPoint(&*BlkPrint);

                    FunctionCallee PrintFunc = ModPar->getOrInsertFunction(
                        "printOutInstrInfo", // function name
                        Type::getVoidTy(Context) // return type
                    );

                    Builder.CreateCall(PrintFunc);
                }
            }
      }
      return false;
  }
}; // end of struct CountDymInstruPass
}  // end of anonymous namespace

char CountBranchBias::ID = 0;
static RegisterPass<CountBranchBias> X("cse231-bb", "Dynamic BrachBias Counting Pass",
                             false /* Only looks at CFG */,
                             false /* Analysis Pass */);