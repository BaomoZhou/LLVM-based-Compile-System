
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
        Module* ModPar = F.getParent();
        LLVMContext &Context = ModPar->getContext();

        FunctionCallee UpdateFunc = ModPar->getOrInsertFunction(
            "updateBranchInfo",         // name of function
            Type::getVoidTy(Context),   // return type
            Type::getInt1Ty(Context)    // first parameter type
        );
        FunctionCallee PrintFunc = ModPar->getOrInsertFunction(
            "printOutBranchInfo",        // name of function
            Type::getVoidTy(Context)     // return type                   
        );

        Function::iterator Block;
        for (Block = F.begin(); Block != F.end(); Block++) {
            BasicBlock::iterator BlkPrint;
            for (BlkPrint = Block->begin(); BlkPrint != Block->end(); BlkPrint++) {
                if (BlkPrint->getOpcode() == 2) {
                    BranchInst* InstBranch = dyn_cast<BranchInst>(BlkPrint);
                    if (InstBranch->isConditional() ) {
                        IRBuilder<> Builder(InstBranch);
                        vector<Value *> args;
                        args.push_back(InstBranch->getCondition());
                        Builder.CreateCall(UpdateFunc, args);
                    }
                } 

            }
        }

        IRBuilder<> Builder(&(F.back().back()));
        Builder.CreateCall(PrintFunc);
        return false;
    }
}; // end of struct CountDymInstruPass
}  // end of anonymous namespace

char CountBranchBias::ID = 0;
static RegisterPass<CountBranchBias> X("cse231-bb", "Dynamic BrachBias Counting Pass",
                             false /* Only looks at CFG */,
                             false /* Analysis Pass */);
                             
