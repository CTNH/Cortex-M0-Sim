#ifndef ARMV6_ASSEMBLER_H
#define ARMV6_ASSEMBLER_H
#include <string>		// For type string
#include <cstdint>		// For type uint16_t
using namespace std;

extern "C" {
	#include "basiclib_string.h"
}

class ARMv6_Assembler {
	private:
		int logLvl = 1;
		// Creates hash from text using the djb2 algorithm
		uint16_t djb2Hash(string text);
		int getRegNum(char* reg);
		// Log function to process logs
		void log(string msg, int msgLvl);
	public:
		// Class Constructor
		ARMv6_Assembler();
		// Checks if all hashes are unique
		bool hashUniqueCheck();
		// Generate an opcode given a string instruction
		uint16_t genOpcode(string instruction);		
};

#endif
