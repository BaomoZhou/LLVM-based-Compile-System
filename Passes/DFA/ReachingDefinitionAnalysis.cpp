#include "231DFA.h"
#include "llvm/Pass.h"
#include <string>
using namespace llvm;

namespace llvm{
    class ReachingInfo: public Info{
        private:
        std::set<unsigned> infoList; 

        public:
            ReachingInfo(){}
            ~ReachingInfo() {}

            std::set<unsigned>& getInfoList(){
                return this->infoList;
            }

            void setInforList(std::set<unsigned> input){
                this->infoList = input;
            }

            /*
            * Print out the information
            *
            * Direction:
            *   In your subclass you should implement this function according to the project specifications.
            */
            void print(){
                for(auto const I: infoList){
                    errs() << I << "|";
                }
                errs()<<'\n';
            }

            /*
            * Compare two pieces of information
            *
            * Direction:
            *   In your subclass you need to implement this function.
            */
            static bool equals(ReachingInfo * info1, ReachingInfo * info2){
                return info1->getInfoList() == info2->getInfoList();
            }
            /*
            * Join two pieces of information.
            * The third parameter points to the result.
            *
            * Direction:
            *   In your subclass you need to implement this function.
            */
            static ReachingInfo* join(ReachingInfo * info1, ReachingInfo * info2, ReachingInfo * result){
                std::set<unsigned> resList = info1->getInfoList();
                std::set<unsigned> info2List = info2->getInfoList();
                resList.insert(info2List.begin(), info2List.end());
                result->setInforList(resList);
                return nullptr;
            }

    };

    class ReachingDefinitionAnalysis: public DataFlowAnalysis<ReachingInfo, true>{
    private:
		typedef std::pair<unsigned, unsigned> Edge;
	public:
		ReachingDefinitionAnalysis(ReachingInfo & initialState, ReachingInfo & bottom) : 
			DataFlowAnalysis(initialState, bottom) {}
        /*
        * The flow function.
        *   Instruction I: the IR instruction to be processed.
        *   std::vector<unsigned> & IncomingEdges: the vector of the indices of the source instructions of the incoming edges.
        *   std::vector<unsigned> & IncomingEdges: the vector of indices of the source instructions of the outgoing edges.
        *   std::vector<Info *> & Infos: the vector of the newly computed information for each outgoing eages.
        *
        * Direction:
        * 	 Implement this function in subclasses.
        */
            void flowfunction(Instruction * I, std::vector<unsigned> & IncomingEdges, 
                                std::vector<unsigned> & OutgoingEdges, std::vector<ReachingInfo *> & Infos){
            if(I == nullptr){
                    return;
                }
            auto IndexToInstr = getIndexToInstr();
            auto InstrToIndex = getInstrToIndex();
            auto EdgeToInfo = getEdgeToInfo();
            unsigned Idx = InstrToIndex[I];
			ReachingInfo * resInfo = new ReachingInfo();
            for(auto incomingEdge: IncomingEdges){
                    Edge inEdge = std::make_pair(incomingEdge, Idx);
                    ReachingInfo *addInfo = EdgeToInfo[inEdge];
                    ReachingInfo::join(resInfo, addInfo, resInfo);
                }
            std::string opcodeName = I->getOpcodeName();
            std::map<std::string,int> opcodeNameToCategory = {{"alloca", 1}, {"load", 1}, {"icmp", 1}, {"fcmp", 1}, {"getelementptr", 1}, {"select", 1}, 
                                                            {"br", 2}, {"switch", 2}, {"store", 2},
                                                            {"phi", 3}};
            int cateNum;
            if(opcodeNameToCategory.find(opcodeName) != opcodeNameToCategory.end()){
                cateNum = opcodeNameToCategory[opcodeName];
            }
            else{
                cateNum = 2;
            }
            cateNum = I->isBinaryOp() ? 1 : cateNum;
            //Category 1
			if (cateNum == 1) {
                ReachingInfo* addInfo = new ReachingInfo();
                std::set<unsigned> tempset = {Idx};
                addInfo->setInforList(tempset);
                ReachingInfo::join(resInfo, addInfo, resInfo);
			}
            //Category 3
			else if (cateNum == 3) {
                Instruction *currInstr = I;
                std::set<unsigned> tempset;
                while(currInstr != nullptr && currInstr->getOpcode() == 53){
                    tempset.insert(InstrToIndex[currInstr]);
                    currInstr = currInstr->getNextNode();
                } 
                ReachingInfo* addInfo = new ReachingInfo();
                addInfo->setInforList(tempset);
                ReachingInfo::join(resInfo, addInfo, resInfo);
			}

            for(auto iter: OutgoingEdges){
                Infos.push_back(resInfo);
            }
        }

    };

    struct ReachingDefinitionAnalysisPass: public FunctionPass{
        static char ID;
        ReachingDefinitionAnalysisPass() : FunctionPass(ID) {}

        bool runOnFunction(Function &F) override {
            ReachingInfo initialState;
            ReachingInfo bottom;
            ReachingDefinitionAnalysis  ReDeAnalysis(initialState, bottom);
            ReDeAnalysis.runWorklistAlgorithm(&F);
            ReDeAnalysis.print();
            return false;
        }
    };
}
char ReachingDefinitionAnalysisPass::ID = 0;
static RegisterPass<ReachingDefinitionAnalysisPass> X("cse231-reaching", "CFG",
                             false /* Only looks at CFG */,
                             false /* Analysis Pass */);