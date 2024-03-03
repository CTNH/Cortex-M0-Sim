#ifndef CORTEXM0P_CORE_H
#define CORTEXM0P_CORE_H

#include "cortex-m0p_memory.h"
#include "ARMv6_Assembler.h"
#include <cstdint>
#include <string>

using namespace std;

class CM0P_Core {
	private:
		// Processor Core Registers
		uint32_t 		R[16];		// General Purpose Registers R0-R15
		uint32_t		PSR;		// Program Status Register
		uint32_t		PRIMASK;
		uint32_t		CONTROL;
		// Declare pointers to the general purpose registers
		uint32_t*		SP = &R[13];
		uint32_t*		LR = &R[14];
		uint32_t*		PC = &R[15];

		uint32_t		stack[40];

		const uint32_t INST_BASEADDR = 0;	// Set base address of an instruction
		const uint32_t INST_MAINADDR = 0;	// Address of first instruction to run

		CM0P_Memory memory;

		uint32_t update_flag_addition(uint32_t a, uint32_t b);
		uint32_t update_flag_subtraction(uint32_t a, uint32_t b);
		void stackPush(uint32_t data);
	public:
		CM0P_Core(vector<ARMv6_Assembler::OpcodeResult>, uint32_t startAddr);	// Constructor
		bool get_flag(char flag);
		void update_flag(char flag, bool bit);
		void step_inst();		// Run instruction in memory
		void setPC(uint32_t addr);			// Setter for PC
		uint32_t* getCoreRegisters();		// Returns R

		CM0P_Memory* getMemPtr();
};

#endif
