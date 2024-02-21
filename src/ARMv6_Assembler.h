#ifndef ARMV6_ASSEMBLER_H
#define ARMV6_ASSEMBLER_H
#include <string>		// For type string
#include <cstdint>		// For type uint16_t
#include <unordered_map>
#include <vector>
using namespace std;

extern "C" {
	#include "basiclib_string.h"
}

class ARMv6_Assembler {
	public:
		struct OpcodeResult {
			bool invalid;		// True to invalidate all other struct members
			uint32_t opcode;
			bool i32;			// True for 32-bits instruction, False for 16-bits
			bool label;			// True if line includes a label
			bool unsupported;	// True if instruction is not implemented here
		};

	private:
		const uint32_t INST_BASEADDR = 0;	// Set base address of an instruction

		// Log level; smaller the number the less to print
		int logLvl = 1;

		uint16_t currAddress;
		// Program Counter
		uint32_t PC;
		// vector<string> labels;
		unordered_map<string, uint32_t> labels;

		// Vector to store final output from input opcodes
		vector<pair<string, OpcodeResult>> finalOpcodes;

		// Reads all lines from an assembly file
		vector<string> readASMFile(string fpath);

		// Creates hash from text using the djb2 algorithm
		uint16_t djb2Hash(string text);
		// Gets an integer given a string presentation of a register
		int getRegNum(char* reg);
		// Get register list given arguments and starting register
		uint16_t getRegList(char** args, int startArg);
		// Gets SYSm value given a string representation of a special register
		uint8_t getSYSm(char* spReg);

		// Refer to A4.2.1 in ARMv6-M Architecture Reference Manual for calculating PC value
			// Adds a label to the list; True for success, False for not
		bool addLabel(string label);
		// Get an immediate offset given a string label; boolean is used to track if offset is valid
		pair<bool, int> labelOffsetLookup(string label);
		// Clean up instructions to aid processing
		vector<char**> cleanInstructions(vector<string> lines);

		// Log function to process logs
		void log(string msg, int msgLvl);
		void logInvalidValue(string type, int min, int max);
		// Log of 16-bits opcodes generated
		void log16bitOpcode(string instruction, uint16_t opcode);

		// ==== Functions for genOpcode() to avoid code duplication
		// For ASRS, LSLS, LSRS
		OpcodeResult genOpcode_bitShift(char** args, uint8_t opcodeImmPrefix, uint8_t opcodeRegPrefix);
		// For ANDS, ORRS, EORS, BICS
		OpcodeResult genOpcode_bitwise(char** args, uint8_t opcodePrefix);
		// For B{cond}
		OpcodeResult genOpcode_branch(char** args, uint8_t opcodePrefix, bool t2=0);
		// For BLX, BX
		OpcodeResult genOpcode_branchExchange(char** args, uint8_t opcodePrefix);
		// For REV, REV16, REVSH
		OpcodeResult genOpcode_reverseBytes(char** args, uint8_t opcodePrefix);
		// For LDR{B|H}, STR{B|H}
		OpcodeResult genOpcode_loadStoreImm(char** args, uint8_t opcodePrefix);
		// For LDR{B|H|SB|SH}, STR{B|H}
		OpcodeResult genOpcode_loadStoreReg(char** args, uint8_t opcodePrefix);
		// For LDM, STM
		OpcodeResult genOpcode_loadStoreMulReg(char** args, uint8_t opcodePrefix);
		// For SXTB, SXTH, UXTB, UXTH
		OpcodeResult genOpcode_extendRegister(char** args, uint8_t opcodePrefix);
		// For DMB, DSB, ISB
		OpcodeResult genOpcode_barrier(char** args, uint8_t opcodePrefix);
		// For POP, PUSG
		OpcodeResult genOpcode_popPush(char** args, uint8_t opcodePrefix, char* extraReg);

		// ====
	public:
		// Class Constructor
		ARMv6_Assembler(string asmFilePath);
		// Checks if all hashes are unique
		bool hashUniqueCheck();
		// Generate an opcode given a string instruction
		OpcodeResult genOpcode(char** args, bool labelOnly);
		//uint16_t genOpcode(string instruction);		

		vector<pair<string, OpcodeResult>> getFinalResult();
};

#endif
