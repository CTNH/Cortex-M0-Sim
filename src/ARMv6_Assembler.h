#ifndef ARMV6_ASSEMBLER_H
#define ARMV6_ASSEMBLER_H
#include <string>		// For type string
#include <cstdint>		// For type uint16_t
using namespace std;

class ARMv6_Assembler {
	private:
		// Creates hash from text using the djb2 algorithm
		uint16_t djb2Hash(string text);
	public:
		// Class Constructor
		ARMv6_Assembler();
		// Checks if all hashes are unique
		bool hashUniqueCheck();
		// Generate an opcode given a string instruction
		uint16_t genOpcode(string instruction);		
};

#endif
