#include <iostream>
#include <fstream>
#include <assert.h>
#include <stdarg.h>
#include <stdlib.h>
#include "pin.H"

using namespace std;

ofstream OutFile;

#define truncate(val, bits) ((val) & ((1UL << (bits)) - 1))   // 截取低bits位

static UINT64 takenCorrect = 0;
static UINT64 takenIncorrect = 0;
static UINT64 notTakenCorrect = 0;
static UINT64 notTakenIncorrect = 0;

template <size_t N, UINT64 init = (1 << N)/2 - 1>   // N < 64
// template <size_t N, UINT64 init = (1 << N) - 1>   // N < 64
class SaturatingCnt
{
    UINT64 val;
    public:
        SaturatingCnt() { reset(); }

        void increase() { if (val < (1 << N) - 1) val++; }
        void decrease() { if (val > 0) val--; }

        void leftRoll(int n) { val = truncate(val << n, N) ;}
        void rightRoll(int n) { val = truncate(val >> n, N) ;}

        void reset() { val = init; }
        UINT64 getVal() { return val; }

        BOOL isTaken() { return (val > (1 << N)/2 - 1); }
};


template<size_t N>      // N < 64
class ShiftReg
{
    UINT64 val;
    public:
        ShiftReg() { val = 0; }

        bool shiftIn(bool b)
        {
            bool ret = !!(val&(1<<(N-1)));
            val <<= 1;
            val |= b;
            val &= (1<<N)-1;
            return ret;
        }

        UINT64 getVal() { return val; }
};

class BranchPredictor
{
    public:
        BranchPredictor() { }
        virtual BOOL predict(ADDRINT addr) { return FALSE; };
        virtual void update(BOOL takenActually, BOOL takenPredicted, ADDRINT addr) {};
};

BranchPredictor* BP;
// BranchPredictor BP;



/* ===================================================================== */
/* 实现下列3种动态预测方法                                                 */
/* ===================================================================== */
// 1. BHT-based branch predictor
template<size_t L>
class BHTPredictor: public BranchPredictor
{
    SaturatingCnt<2> counter[1 << L];
    UINT64 tag[1<<L];
    public:
        BHTPredictor() { }

        BOOL predict(ADDRINT addr)
        {
            return counter[truncate(addr, L)].isTaken();                         // 根据val倒数第二位获取值
        }

        void update(BOOL takenActually, BOOL takenPredicted, ADDRINT addr)
        {
            // 预测跳转
            if(takenPredicted)
            {
                // 预测跳转，实际跳转 10->11 11->11
                if(takenActually)
                    counter[truncate(addr, L)].increase();
                // 预测跳转，实际不跳转 10->00 11->10
                else
                {
                    UINT64 val = counter[truncate(addr, L)].getVal();
                    counter[truncate(addr, L)].decrease();
                    if(val==2)
                        counter[truncate(addr, L)].decrease();
                    
                }
            }
            // 预测不跳转
            else
            {
                // 01->11 00->01
                if(takenActually)
                {   
                    UINT64 val = counter[truncate(addr, L)].getVal();
                    counter[truncate(addr, L)].increase();
                    if(val==1)
                        counter[truncate(addr, L)].increase();
                }
                // 01->00 00->00
                else
                    counter[truncate(addr, L)].decrease();
            }
        }

        void outPut()
        {
            cout<<"counter:"<<endl;
            for(int i = 0; i < (1<<L)-1; i++)
                cout<<i<<": "<<counter[i].getVal()<<endl;
        }
};

// 2. Global-history-based branch predictor
template<size_t L, size_t H, UINT64 BITS = 2>
class GlobalHistoryPredictor: public BranchPredictor
{
    SaturatingCnt<BITS> bhist[1 << L];  // PHT中的分支历史字段
    ShiftReg<H> GHR;
public:  
    BOOL predict(ADDRINT addr)
    {
        return bhist[truncate(addr ^ GHR.getVal(), L)].isTaken();     // 根据val倒数第二位获取值
    }

    void update(BOOL takenActually, BOOL takenPredicted, ADDRINT addr)
    {
        UINT64 idx = truncate(addr ^ GHR.getVal(), L);
        // 预测跳转
        if(takenPredicted)
        {
            // 预测跳转，实际跳转 10->11 11->11
            if(takenActually)
                bhist[idx].increase();
            // 预测跳转，实际不跳转 10->00 11->10
            else
            {
                UINT64 val = bhist[idx].getVal();
                bhist[idx].decrease();
                if(val==2)
                    bhist[idx].decrease();
            }
        }
        // 预测不跳转
        else
        {
            // 01->11 00->01
            if(takenActually)
            {   
                UINT64 val = bhist[idx].getVal();
                bhist[idx].increase();
                if(val==1)
                    bhist[idx].increase();
            }
            // 01->00 00->00
            else
                bhist[idx].decrease();
            // GHR.shiftIn(0);
        }
        // 更新GHR
        if(takenActually)
            GHR.shiftIn(1);
        else
            GHR.shiftIn(0);
    }

};

// 3. Local-history-based branch predictor
template<size_t L, size_t H, size_t HL = 6, UINT64 BITS = 2>
class LocalHistoryPredictor: public BranchPredictor
{
    SaturatingCnt<BITS> bhist[1 << L];  // PHT中的分支历史字段
    ShiftReg<H> LHT[1 << HL];

public:  
    BOOL predict(ADDRINT addr)
    {
        UINT64 val = bhist[truncate(addr ^ LHT[truncate(addr, L)].getVal(), L)].getVal();   // 取addr低L位查表
        return (BOOL)truncate(val>>1, 1);                         // 根据val倒数第二位获取值
    }

    void update(BOOL takenActually, BOOL takenPredicted, ADDRINT addr)
    {
        UINT64 idx = truncate(addr ^ LHT[truncate(addr, L)].getVal(), L);
        // 预测跳转
        if(takenPredicted)
        {
            // 预测跳转，实际跳转 10->11 11->11
            if(takenActually)
                bhist[idx].increase();
            // 预测跳转，实际不跳转 10->00 11->10
            else
            {
                bhist[idx].leftRoll(1);
            }
        }
        // 预测不跳转
        else
        {
            // 01->11 00->01
            if(takenActually)
            {   
                bhist[idx].leftRoll(1);
                bhist[idx].increase();
            }
            // 01->00 00->00
            else
                bhist[idx].decrease();
        }
        if(takenActually)
            LHT[truncate(addr, L)].shiftIn(1);
        else
            LHT[truncate(addr, L)].shiftIn(0);
    }
};

/* ===================================================================== */
/* 锦标赛预测器的选择机制可用全局法或局部法实现，二选一即可                   */
/* ===================================================================== */
// 1. Tournament predictor: Select output by global selection history
template<UINT64 BITS = 2>
class TournamentPredictor_GSH: public BranchPredictor
{
    SaturatingCnt<BITS> GSHR;
    BranchPredictor* BPs[2];
    BOOL res0;
    BOOL res1;

    public:
        TournamentPredictor_GSH(BranchPredictor* BP0, BranchPredictor* BP1)
        {
            BPs[0] = BP0;
            BPs[1] = BP1;
        }

        BOOL predict(ADDRINT addr)
        {
            res0 = BPs[0]->predict(addr);
            res1 = BPs[1]->predict(addr);
            if(truncate(GSHR.getVal()>>1, 1))
                return res1;
            return res0;
        }

        void update(BOOL takenActually, BOOL takenPredicted, ADDRINT addr)
        {
            BPs[0]->update(takenActually, takenPredicted, addr);
            BPs[1]->update(takenActually, takenPredicted, addr);
            if(res0!=res1)
            {
                if(res0 == takenActually)
                    GSHR.decrease();
                else if(res1==takenActually)
                    GSHR.increase();
            }
        }
};

// 2. Tournament predictor: Select output by local selection history
template<size_t L, UINT64 BITS = 2>
class TournamentPredictor_LSH: public BranchPredictor
{
    SaturatingCnt<BITS> LSHT[1 << L];
    BranchPredictor* BPs[2];

    public:
        TournamentPredictor_LSH(BranchPredictor* BP0, BranchPredictor* BP1)
        {
            BPs[0] = BP0;
            BPs[1] = BP1;
        }

        // TODO:
};

// This function is called every time a control-flow instruction is encountered
void predictBranch(ADDRINT pc, BOOL direction)
{
    BOOL prediction = BP->predict(pc);
    BP->update(direction, prediction, pc);
    if (prediction)
    {
        if (direction)
            takenCorrect++;
        else
            takenIncorrect++;
    }
    else
    {
        if (direction)
            notTakenIncorrect++;
        else
            notTakenCorrect++;
    }
}

// Pin calls this function every time a new instruction is encountered
void Instruction(INS ins, void * v)
{
    if (INS_IsControlFlow(ins) && INS_HasFallThrough(ins))
    {
        INS_InsertCall(ins, IPOINT_TAKEN_BRANCH, (AFUNPTR)predictBranch,
                        IARG_INST_PTR, IARG_BOOL, TRUE, IARG_END);

        INS_InsertCall(ins, IPOINT_AFTER, (AFUNPTR)predictBranch,
                        IARG_INST_PTR, IARG_BOOL, FALSE, IARG_END);
    }
}

// This knob sets the output file name
KNOB<string> KnobOutputFile(KNOB_MODE_WRITEONCE, "pintool", "o", "brchPredict.txt", "specify the output file name");

// This function is called when the application exits
VOID Fini(int, VOID * v)
{
	double precision = 100 * double(takenCorrect + notTakenCorrect) / (takenCorrect + notTakenCorrect + takenIncorrect + notTakenIncorrect);
    
    cout << "takenCorrect: " << takenCorrect << endl
    	<< "takenIncorrect: " << takenIncorrect << endl
    	<< "notTakenCorrect: " << notTakenCorrect << endl
    	<< "nnotTakenIncorrect: " << notTakenIncorrect << endl
    	<< "Precision: " << precision << endl;
    
    OutFile.setf(ios::showbase);
    OutFile << "takenCorrect: " << takenCorrect << endl
    	<< "takenIncorrect: " << takenIncorrect << endl
    	<< "notTakenCorrect: " << notTakenCorrect << endl
    	<< "nnotTakenIncorrect: " << notTakenIncorrect << endl
    	<< "Precision: " << precision << endl;
    
    OutFile.close();
}

/* ===================================================================== */
/* Print Help Message                                                    */
/* ===================================================================== */

INT32 Usage()
{
    cerr << "This tool counts the number of dynamic instructions executed" << endl;
    cerr << endl << KNOB_BASE::StringKnobSummary() << endl;
    return -1;
}

/* ===================================================================== */
/* Main                                                                  */
/* ===================================================================== */
/*   argc, argv are the entire command line: pin -t <toolname> -- ...    */
/* ===================================================================== */

int main(int argc, char * argv[])
{
    // TODO: New your Predictor below.
    BranchPredictor *BP0 = new BHTPredictor<20>;
    // BranchPredictor *BP1 = new GlobalHistoryPredictor<20, 20>;
    // BranchPredictor *BP2 = new LocalHistoryPredictor<20, 20>;
    BP = BP0;
    // BP = new TournamentPredictor_GSH<> (BP1, BP2);

    // Initialize pin
    if (PIN_Init(argc, argv)) return Usage();
    
    OutFile.open(KnobOutputFile.Value().c_str());

    // Register Instruction to be called to instrument instructions
    INS_AddInstrumentFunction(Instruction, 0);

    // Register Fini to be called when the application exits
    PIN_AddFiniFunction(Fini, 0);

    // Start the program, never returns
    PIN_StartProgram();

    return 0;
}
