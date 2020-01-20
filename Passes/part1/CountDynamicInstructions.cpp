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
struct CountDymInstruPass : public FunctionPass {
  static char ID;
  CountDymInstruPass() : FunctionPass(ID) {}

  bool runOnFunction(Function &F) override {
      Module *ModPar = F.getParent();
      LLVMContext &Context = ModPar->getContext();
      // reverse all blocks of a function
      Function::iterator Block;
      for(Block = F.begin(); Block != F.end(); Block++) {
            std::map<int,int> InstruCountMap;

            // reverse all instructions of a block and construct the map
            BasicBlock::iterator Instru;
            for(Instru = Block->begin(); Instru != Block->end(); Instru++){
                int opcode = Instru -> getOpcode();
                InstruCountMap[opcode]++;
            }

            int NumInstrus = InstruCountMap.size();
            vector<Value*> args;
            vector<Constant*> keys;
            vector<Constant*> vals;

            // reverse map and construct keys and vals
            map<int,int>::iterator iter;
            for(iter = InstruCountMap.begin(); iter != InstruCountMap.end(); iter++){
                keys.push_back(ConstantInt::get(Type::getInt32Ty(Context),iter->first));
                vals.push_back(ConstantInt::get(Type::getInt32Ty(Context),iter->second));
            }

            // initialize the Array with length of NumInstrus
            ArrayType* ArrTy = ArrayType::get(IntegerType::getInt32Ty(Context), NumInstrus);

            GlobalVariable *KeysGlobal = new GlobalVariable(
                *ModPar,
                ArrTy,
                true,
                GlobalVariable::InternalLinkage,
                ConstantArray::get(ArrTy,keys),
                "KeysGlobal");
            GlobalVariable *ValsGlobal = new GlobalVariable(
                *ModPar,
                ArrTy,
                true,
                GlobalVariable::InternalLinkage,
                ConstantArray::get(ArrTy,vals),
                "ValsGlobal");

            // insert updateInstrInfo at each end of each block
            IRBuilder<> Builder(&*Block);
            Builder.SetInsertPoint(Block->getTerminator());
            args.push_back((Value*)ConstantInt::get(Type::getInt32Ty(Context),NumInstrus));
            args.push_back(Builder.CreatePointerCast(KeysGlobal, Type::getInt32PtrTy(Context)));
            args.push_back(Builder.CreatePointerCast(ValsGlobal, Type::getInt32PtrTy(Context)));

            FunctionCallee UpdateFunc = ModPar->getOrInsertFunction(
                "updateInstrInfo", // function name
                Type::getVoidTy(Context), // return type
                Type::getInt32Ty(Context), // first parameter
                Type::getInt32PtrTy(Context), // second parameter
                Type::getInt32PtrTy(Context)  // third parameter
            );

            Builder.CreateCall(UpdateFunc, args);

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

char CountDymInstruPass::ID = 0;
static RegisterPass<CountDymInstruPass> X("cse231-cdi", "Dynamic Intructions Counting Pass",
                             false /* Only looks at CFG */,
                             false /* Analysis Pass */);