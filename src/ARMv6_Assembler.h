#ifndef ARMV6_ASSEMBLER_H
#define ARMV6_ASSEMBLER_H
#include <string>
#include <cstdint>
using namespace std;

class ARMv6_Assembler {
	public:
		// Class Constructor
		ARMv6_Assembler();
		// Generate an opcode given a string instruction
		uint16_t genOpcode(string instruction);		
};


#endif
