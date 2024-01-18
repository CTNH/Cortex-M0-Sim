#ifndef CORTEXM0P_CORE_H
#define CORTEXM0P_CORE_H

#include "cortex-m0p_memory.h"
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

		CM0P_Memory memory;

		uint32_t update_flag_addition(uint32_t a, uint32_t b);
		uint32_t update_flag_subtraction(uint32_t a, uint32_t b);
	public:
		CM0P_Core();			// Constructor
		bool get_flag(char flag);
		void update_flag(char flag, bool bit);
		void step_inst();		// Run instruction in memory
};

#endif
