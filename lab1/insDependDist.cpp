/*
 * Copyright 2002-2020 Intel Corporation.
 * 
 * This software is provided to you as Sample Source Code as defined in the accompanying
 * End User License Agreement for the Intel(R) Software Development Products ("Agreement")
 * section 1.L.
 * 
 * This software and the related documents are provided as is, with no express or implied
 * warranties, other than those that are expressly stated in the License.
 */

#include <iostream>
#include <fstream>
#include <vector>
#include "pin.H"
using std::cerr;
using std::ofstream;
using std::ios;
using std::string;
using std::endl;
using std::vector;

ofstream OutFile;

// Convenience data structure
typedef uint32_t reg_t;
struct Registers
{
	vector<reg_t> read;
	vector<reg_t> write;
};

// Global variables
// The array storing the distance frequency between two dependant instructions
UINT64 *insDependDistance;
INT32 maxSize;
INT32 insPointer = 0;
INT32 lastInsPointer[1024] = { 0 };

// This function is called before every instruction is executed. 
// You have to edit this function to determine the dependency distance
// and populate the insDependDistance data structure.
VOID updateInsDependDistance(VOID *v)
{
	// Update the instruction pointer
	++insPointer;

	// regs contains the registers read and written by this instruction.
	// regs->read contains the registers read.
	// regs->write contains the registers written.
	Registers *regs = (Registers*)v;
	
	/* TODO:
		本函数需完成以下2个任务：	
			A. 遍历regs->read向量, 利用lastInsPointer数组计算当前指令的被读寄存器的依赖距离，
				即对任意的r属于regs->read, 其依赖距离 = 当前PC值 - lastInsPointer[r].
			B. 遍历regs->write向量, 利用lastInsPointer数组记录当前指令的被写寄存器所对应的PC值，
				即对任意的r属于regs->write, 将当前PC值赋值给lastInsPointer[r].
			思考: A和B哪个应该先执行? 请通过举例来说明理由.
	*/
	// 是否需要把下面两个调换？
	// Update the lastInstructionCount for the written registers
	// for (vector<reg_t>::iterator it = regs->write.begin(); it != regs->write.end(); it++)
	// 	lastInsPointer[*it] = insPointer; // TODO
		
	for (vector<reg_t>::iterator it = regs->read.begin(); it != regs->read.end(); it++)
	{
		reg_t reg = *it;

		if (lastInsPointer[reg] > 0)
		{
			// Compute the dependency distance
			INT32 distance = insPointer - lastInsPointer[reg];// TODO

			// Populate the insDependDistance array
			if (distance <= maxSize)
				insDependDistance[distance-1]++;// TODO
		}
	}

	for (vector<reg_t>::iterator it = regs->write.begin(); it != regs->write.end(); it++)
		lastInsPointer[*it] = insPointer; // TODO
}

// Pin calls this function every time a new instruction is encountered
VOID Instruction(INS ins, VOID *v)
{
	// regs stores the registers read, written by this instruction
	Registers* regs = new Registers();

	// Find all the register written
	for (uint32_t iw = 0; iw < INS_MaxNumWRegs(ins); iw++)
	{
		// 获取当前指令中被写的寄存器(即目的寄存器)
		REG wr = INS_RegW(ins, iw);
		// 获取寄存器名
		wr = REG_FullRegName(wr);
		if (!REG_valid(wr))
			continue;
    
    	// 将被写寄存器保存到regs向量当中
		if (std::find(regs->write.begin(), regs->write.end(), wr) == regs->write.end())
			regs->write.push_back(wr);
	}

	// Find all the registers read
	for (uint32_t ir = 0; ir < INS_MaxNumRRegs(ins); ir++)
	{
		REG rr = INS_RegR(ins, ir);/* TODO */
		rr = REG_FullRegName(rr);
		if (!REG_valid(rr))
			continue;
		
		if (std::find(regs->read.begin(), regs->read.end(), rr) == regs->read.end())
			regs->read.push_back(rr);
		/*
			TODO
		*/
	}

	// Insert a call to the analysis function -- updateInsDependDistance -- before every instruction.
	// Pass the regs structure to the analysis function.
	INS_InsertCall(ins, IPOINT_BEFORE, (AFUNPTR)updateInsDependDistance, IARG_PTR, (void*)regs, IARG_END);
}

// This knob sets the output file name
KNOB<string> KnobOutputFile(KNOB_MODE_WRITEONCE, "pintool", "o", "insDependDist.csv", "specify the output file name");

// This knob will set the maximum distance between two dependant instructions in the program
KNOB<string> KnobMaxDistance(KNOB_MODE_WRITEONCE, "pintool", "s", "100", "specify the maximum distance between two dependant instructions in the program");

// This function is called when the application exits
VOID Fini(INT32 code, VOID *v)
{
	// Write to a file since cout and cerr maybe closed by the application
    OutFile.setf(ios::showbase);
    for (INT32 i = 0; i < maxSize; i++)
	    OutFile << insDependDistance[i] << ",";
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
    // Initialize pin
    if (PIN_Init(argc, argv)) return Usage();
    
    OutFile.open(KnobOutputFile.Value().c_str());
    maxSize = atoi(KnobMaxDistance.Value().c_str());

    // Initializing depdendancy Distance
    insDependDistance = new UINT64[maxSize];
    memset((void*)insDependDistance, 0, sizeof(UINT64) * maxSize);

    // Register Instruction to be called to instrument instructions
    INS_AddInstrumentFunction(Instruction, 0);

    // Register Fini to be called when the application exits
    PIN_AddFiniFunction(Fini, 0);
    
    // Start the program, never returns
    PIN_StartProgram();
    
    return 0;
}

