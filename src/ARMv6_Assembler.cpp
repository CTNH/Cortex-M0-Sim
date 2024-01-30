#include "ARMv6_Assembler.h"
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <ctype.h>			// For toupper() / tolower()
#include <bits/stdc++.h>	// For sort()
#include <iterator>
// C library
extern "C" {
	#include "basiclib_string.h"
}


ARMv6_Assembler::ARMv6_Assembler() {}

uint16_t ARMv6_Assembler::djb2Hash(string text) {
	uint16_t hash = 5381;
	// Go through each character in string
	for (int i=0; i<text.length(); i++) {
		hash = ((hash << 5) + hash) + toupper(text[i]);
	}
	return hash;
}

bool ARMv6_Assembler::hashUniqueCheck() {
	// Cortex-M0+ Devices Generic User Guide 3.1 
	string instructionSet[] = {
		"ADCS",
		"ADD",
		"ADDS",
		"ADR",
		"ANDS",
		"ASRS",
		"B",
		"BEQ",
		"BNE",
		"BCS",
		"BHS",
		"BCC",
		"BLO",
		"BMI",
		"BPL",
		"BVS",
		"BVC",
		"BHI",
		"BLS",
		"BGE",
		"BLT",
		"BGT",
		"BLE",
		"BAL",
		"BICS",
		"BKPT",
		"BL",
		"BLX",
		"BX",
		"CMN",
		"CMP",
		"CPSID",
		"CPSIE",
		"DMB",
		"DSB",
		"EORS",
		"ISB",
		"LDM",
		"LDR",
		"LDRB",
		"LDRH",
		"LDRSB",
		"LDRSH",
		"LSLS",
		"LSRS",
		"MOV",
		"MOVS",
		"MRS",
		"MSR",
		"MULS",
		"MVNS",
		"NOP",
		"ORRS",
		"POP",
		"PUSH",
		"REV",
		"REV16",
		"REVSH",
		"RORS",
		"RSBS",
		"SBCS",
		"SEV",
		"STM",
		"STR",
		"STRB",
		"STRH",
		"SUB",
		"SUBS",
		"SVC",
		"SXTB",
		"SXTH",
		"TST",
		"UXTB",
		"UXTH",
		"WFE",
		"WFI"
	};

	int instructionSetSize = sizeof(instructionSet)/sizeof(string);
	// Get hash for every instruction
	uint16_t hash[instructionSetSize];
	for (int i=0; i<instructionSetSize; i++) {
		hash[i] = djb2Hash(instructionSet[i]);
		printf("%04x\n",hash[i]);
	}
	// Sort hash values
	sort(hash, hash+instructionSetSize);
	// Count number of collisions
	int collision = 0;
	for (int i=1; i<instructionSetSize; i++) {
		if (hash[i] == hash[i-1]) {
			collision++;
		}
	}

	return ~collision;
};

int ARMv6_Assembler::getRegNum(char* reg) {
	// Assumes input is in format of "Rn" where n is 0-15
	int out = strtol(reg, NULL, 0);
	switch (out) {
		case 1 ... 15:
			return out;
		case 0:
			if (strcmp(reg, "R0") == 0)
				return 0;
			else if (strcmp(reg, "SP") == 0)
				return 13;
			else if (strcmp(reg, "LR") == 0)
				return 14;
			else if (strcmp(reg, "PC") == 0)
				return 15;
			// Invalid Register
			else
		default:
				return -1;
	}
}

void ARMv6_Assembler::log(string msg, int msgLvl) {
	if (msgLvl >= logLvl)
		cout << msg << endl;
}

void ARMv6_Assembler::log16bitOpcode(string instruction, uint16_t opcode) {
	// Create buffer of 1 character
	char* buf = (char*) malloc(sizeof(char));
	// Get buffer size
	int bufSz = snprintf(buf, 1, "[%04x]", opcode);
	// Reallocate buffer
	buf = (char*) malloc(bufSz * sizeof(char));
	// Store log message into buffer
	snprintf(buf, bufSz, "[%04x] generated from [%s]", opcode, instruction.c_str());

	log((string)buf, 3);
}

uint16_t ARMv6_Assembler::genOpcode(string instruction) {
	// Remove comments
	if (instruction.find(";") != -1) {
		instruction.erase(instruction.find(";"));
	}

	// Separate instruction into arguments list by ' ' or ','
	char** args = strtokSplit(&instruction[0], (char*)" ,");
	int argLen = strArrLen(args);

	OpcodeResult result = {0, 0};

	// Flag to track if result opcode is invalid
	bool invalidInstruction = 0;
	uint16_t opcode = 0;
	switch (djb2Hash(instruction)) {
		case 0xde60:		// ADCS
			{
				int Rd = getRegNum(args[1]);
				int Rn = getRegNum(args[argLen-2]);
				int Rm = getRegNum(args[argLen-1]);
				opcode |= 0b0100000101000000;
			}
			break;
		case 0xd06e:		// ADD
			{
				int Rd = getRegNum(args[1]);
				int Rn = getRegNum(args[argLen-2]);
				int Rm = getRegNum(args[argLen-1]);
				// SP Plus Immediate
				if (args[argLen-1][0] == '#') {
					int imm = strtol(args[argLen-1]+1, NULL, 0);
					if (imm %4 != 0) {
						log("Invalid immediate value: must be an integer multiple of four!", 1);
						invalidInstruction = 1;
						break;
					}

					// Encoding T1
					if (Rd >= 0 and Rd < 8) {
						if (Rn != 13 && Rn !=15) {
							log("Invalid register: Rn must be SP or PC when Rd is between R0 and R7!", 1);
							invalidInstruction = 1;
							break;
						}
						if (Rn != 13 && Rn != 15) {
							log("Invalid register: Rn must be SP or PC!", 1);
							invalidInstruction = 1;
							break;
						}
						if (imm < 0 or imm > 1020) {
							log("Invalid immediate value: must be between 0 and 1020!", 1);
							invalidInstruction = 1;
							break;
						}

						opcode |= imm / 4;
						opcode |= Rd << 8;
						opcode |= 0b10101 << 11;
						log16bitOpcode(instruction, opcode);
					}
					// Encoding T2
					else if (Rd == 13) {
						if (Rn != 13) {
							log("Invalid register: Rn must also be SP!", 1);
						}
						if (imm < 0 or imm > 508) {
							log("Invalid immediate value: must be between 0 and 508!", 1);
							invalidInstruction = 1;
							break;
						}
						
						opcode |= imm / 4;
						opcode |= 0b101100000 << 7;
						log16bitOpcode(instruction, opcode);
					}
					else {
						log("Invalid register: Rd must be between R0 and R7 or SP!", 1);
						invalidInstruction = 1;
						break;
					}
				}
				// Register
				else {
					// Encoding T2
					if (Rd != Rn) {
						log("Invalid registers: Rd and Rn must be equal!", 1);
						invalidInstruction = 1;
						break;
					}
					if (Rd < 0 or Rd > 15 or Rm < 0 or Rm > 15) {
						log("Invalid register: registers must be between R0 and R15!", 1);
						invalidInstruction = 1;
						break;
					}
					if (Rn == 15 and Rm == 15) {
						log("Invalid registers: Rn and Rm must not both specify PC!", 1);
						invalidInstruction = 1;
						break;
					}
					opcode |= Rd;
					opcode |= Rm << 3;
					opcode |= (Rd & 0x1) << 7;		// DN bit
					opcode |= 0b1000100 << 8;
				}
			}
			break;
		case 0xde81:		// ADDS
			{
				int Rd = getRegNum(args[1]);
				int Rn = getRegNum(args[argLen-2]);
				int Rm = getRegNum(args[argLen-1]);
				// Immediate
				if (args[argLen-1][0] == '#') {
					int imm = strtol(args[argLen-1]+1, NULL, 0);

					if (Rd < 0 or Rd > 7 or Rn < 0 or Rn > 7) {
						log("Invalid register: Rd and Rn must be between R0 and R7!", 1);
						invalidInstruction = 1;
						break;
					}

					// Encoding T2
					if (Rd == Rn) {
						if (imm < 0 or imm > 255) {
							log("Invalid immediate value: must be between 0 and 255!", 1);
							invalidInstruction = 1;
							break;
						}

						opcode |= imm;
						opcode |= getRegNum(args[1]) << 8;		// Rdn
						opcode |= 0b110 << 11;
					}
					// Encoding T1
					else {
						if (imm < 0 or imm > 7) {
							log("Invalid immediate value: must be between 0 and 7!", 1);
							invalidInstruction = 1;
							break;
						}

						opcode |= Rd;
						opcode |= Rn << 3;
						opcode |= imm << 6;
						opcode |= 0b1110 << 9;
					}
				}
				// Register
				else {
					// Encoding T1
					if (Rm < 0 or Rm > 7) {
						log("Invalid register: Rm must be between 0 and 7!", 1);
						invalidInstruction = 1;
						break;
					}

					opcode |= Rd;
					opcode |= Rn << 3;
					opcode |= Rm << 6;
					opcode |= 0b1100 << 9;
				}
			}
			break;
		case 0xd07c:		// ADR
			{
				// TODO
				opcode |= 0;
			}
			break;
		case 0x090b:		// ANDS
			genOpcode_bitwise(args, 0b0000);
			break;
		case 0x201e:		// ASRS
			genOpcode_bitShift(args, 0b10, 0b0100);
			break;
		case 0xb5e7:		// B
			{
			}
			break;
		case 0xd4dd:		// BEQ
		case 0xd5fa:		// BNE
		case 0xd49d:		// BCS
		case 0xd542:		// BHS
		case 0xd48d:		// BCC
		case 0xd5c2:		// BLO
		case 0xd5dd:		// BMI
		case 0xd643:		// BPL
		case 0xd710:		// BVS
		case 0xd700:		// BVC
		case 0xd538:		// BHI
		case 0xd5c6:		// BLS
		case 0xd513:		// BGE
		case 0xd5c7:		// BLT
		case 0xd522:		// BGT
		case 0xd5b8:		// BLE
		case 0xd454:		// BAL
		case 0x8006:		// BICS
			genOpcode_bitwise(args, 0b1110);
			break;
		case 0x8a36:		// BKPT
		case 0x7313:		// BL
		case 0xd5cb:		// BLX
		case 0x731f:		// BX
		case 0xda23:		// CMN
			{
				int Rn = getRegNum(args[1]);
				int Rm = getRegNum(args[2]);

				if (Rn < 0 or Rn > 7 or Rm < 0 or Rm > 7) {
					log("Invalid register: Rn and Rm must be between R0 and R7!", 1);
					invalidInstruction = 1;
					break;
				}

				opcode |= Rn;
				opcode |= Rm;
				opcode |= 0b100001011 << 6;
			}
			break;
		case 0xda25:		// CMP
			{
				int Rn = getRegNum(args[1]);
				if (Rn < 0 or Rn > 14) {
					log("Invalid register: Rn must be between R0 and R14!", 1);
					invalidInstruction = 1;
					break;
				}

				// Immediate
				if (args[2][0] == '#') {
					int imm = strtol(args[2]+1, NULL, 0);
					if (imm < 0 or imm > 255) {
						log("Invalid immediate value: must be between 0 and 255!", 1);
						invalidInstruction = 1;
						break;
					}

					opcode |= imm;
					opcode |= Rn << 8;
					opcode |= 0b101 >> 11;
				}
				// Register
				else {
					int Rm = getRegNum(args[2]);
					if (Rm < 0 or Rm > 14) {
						log("Invalid register: Rm must be between R0 and R14!", 1);
						invalidInstruction = 1;
						break;
					}

					// Encoding T1 - Rn and Rm both from R0-R7
					if (Rn < 8 and Rm < 8) {
						opcode |= Rn;
						opcode |= Rm << 3;
						opcode |= 0b100001010;
					}
					else {
						opcode |= Rn;
						opcode |= Rm << 3;
						opcode |= (Rn >> 3) << 7;	// N bit
						opcode |= 0b1000101 << 8;
					}
				}
			}
			break;
		case 0xb2f8:		// CPSID
			{
				opcode |= 0b1011011001110010;
			}
			break;
		case 0xb2f9:		// CPSIE
			{
				opcode |= 0b1011011001100010;
			}
			break;
		case 0xde58:		// DMB
			{
				// TODO: 32-bit instruction
			}
			break;
		case 0xdf1e:		// DSB
			{
				// TODO: 32-bit instruction
			}
			break;
		case 0x409e:		// EORS
			genOpcode_bitwise(args, 0b0001);
			break;
		case 0xf463:		// ISB
			{
				// TODO: 32-bit instruction
			}
			break;
		case 0xff42:		// LDM
			{
				// TODO: reglist
				int Rn = getRegNum(args[1]);
				if (Rn < 0 or Rn > 7) {
					log("Invalid register: must be between R0 and R7!", 1);
					invalidInstruction = 1;
					break;
				}
				// Writeback
				if (args[1][strlen(args[1])-1] == '!') {}
			}
			break;
		case 0xff47:		// LDR
			{
			}
			break;
		case 0xe869:		// LDRB
		case 0xe86f:		// LDRH
		case 0xf7fc:		// LDRSB
		case 0xf802:		// LDRSH
		case 0x2783:		// LSLS
			genOpcode_bitShift(args, 0b00, 0b0010);
			break;
		case 0x2849:		// LSRS
			genOpcode_bitShift(args, 0b01, 0b0011);
			break;
		case 0x04f7:		// MOV
			{
				// Register Only
				int Rd = getRegNum(args[1]);
				int Rm = getRegNum(args[2]);
				if (Rd < 0 or Rd > 15 or Rm < 0 or Rm > 15) {
					log("Invalid registers: no such register!", 1);
					invalidInstruction = 1;
					break;
				}

				opcode |= Rd;
				opcode |= Rm << 3;
				opcode |= (Rd >> 3) << 7;	// D bit
				opcode |= 0b1000110 << 8;
			}
		case 0xa42a:		// MOVS
			{
				int Rd = getRegNum(args[1]);
				if (Rd < 0 or Rd > 7) {
					log("Invalid register: Rd must be between R0 and R7!", 1);
					invalidInstruction = 1;
					break;
				}

				// Immediate
				if (args[2][0] == '#') {
					int imm = strtol(args[argLen-1]+1, NULL, 0);
					if (imm < 0 or imm > 255) {
						log("Invalid immediate value: must be between 0 and 255!", 1);
						invalidInstruction = 1;
						break;
					}

					opcode |= imm;
					opcode |= Rd << 8;
					opcode |= 0b100 << 11;
				}
				// Register
				else {
					int Rm = getRegNum(args[2]);
					if (Rm < 0 or Rm > 7) {
						log("Invalid register: Rm must be between R0 and R7!", 1);
						invalidInstruction = 1;
						break;
					}

					opcode |= Rd;
					opcode |= Rm << 3;
				}
			}
			break;
		case 0x0557:		// MRS
			{
				// TODO: 32-bit instruction
			}
			break;
		case 0x0577:		// MSR
			{
				// TODO: 32-bit instruction
			}
			break;
		case 0xbc66:		// MULS
			{
				// TODO: Rd can be omitted when d==n==m
				int Rd = getRegNum(args[1]);
				int Rn = getRegNum(args[2]);
				int Rm = getRegNum(args[3]);
				if (Rd != Rm) {
					log("Invalid registers: Rd and Rm must be the same!", 1);
					invalidInstruction = 1;
					break;
				}
				if (Rd < 0 or Rd > 7 or Rm < 0 or Rm > 7) {
					log("Invalid register: Rm must be between R0 and R7!", 1);
					invalidInstruction = 1;
					break;
				}

				opcode |= Rd;
				opcode |= Rn << 3;
				opcode |= 0b100001101 << 6;
			}
			break;
		case 0xc0e9:		// MVNS
			{
				int Rd = getRegNum(args[1]);
				int Rm = getRegNum(args[2]);
				if (Rd < 0 or Rd > 7 or Rm < 0 or Rm > 7) {
					log("Invalid register: Rd and Rm must be between R0 and R7!", 1);
					invalidInstruction = 1;
					break;
				}

				opcode |= Rd;
				opcode |= Rm << 3;
				opcode |= 0b100001111 << 6;
			}
		case 0x0932:		// NOP
			{
				opcode |= 0b10111111 << 8;
			}
			break;
		case 0xc92b:		// ORRS
			genOpcode_bitwise(args, 0b1100);
			break;
		case 0x11b4:		// POP
			{}
			break;
		case 0x6265:		// PUSH
			{}
			break;
		case 0x18f2:		// REV
			result = genOpcode_reverseBytes(args, 0b1011101000);
			break;
		case 0x23f9:		// REV16
			result = genOpcode_reverseBytes(args, 0b1011101001);
			break;
		case 0x286d:		// REVSH
			result = genOpcode_reverseBytes(args, 0b1011101011);
			break;
		case 0x618b:		// RORS
			{
				int Rd = getRegNum(args[1]);
				int Rn = getRegNum(args[argLen-2]);
				int Rm = getRegNum(args[argLen-1]);
				if (Rd != Rn) {
					log("Invalid registers: Rd and Rn must be the same!", 1);
					invalidInstruction = 1;
					break;
				}
				if (Rd < 0 or Rd > 7 or Rm < 0 or Rm > 7) {
					log("Invalid registers: all registers must be between R0 and R7!", 1);
					invalidInstruction = 1;
					break;
				}

				opcode |= Rd;
				opcode |= Rm << 3;
				opcode |= 0b100000111 << 6;
			}
			break;
		case 0x707f:		// RSBS
			{}
			break;
		case 0xb4b0:		// SBCS
			{}
			break;
		case 0x1d33:		// SEV
		case 0x1f19:		// STM
		case 0x1f1e:		// STR
		case 0x0320:		// STRB
		case 0x0326:		// STRH
		case 0x1f2f:		// SUB
		case 0x0562:		// SUBS
		case 0x1f51:		// SVC
		case 0x1466:		// SXTB
		case 0x146c:		// SXTH
		case 0x2340:		// TST
		case 0x2d28:		// UXTB
		case 0x2d2e:		// UXTH
		case 0x2e47:		// WFE
		case 0x2e4b:		// WFI
			break;

		invalid:
			invalidInstruction = 1;
			break;
	}
	return 0;
}


ARMv6_Assembler::OpcodeResult ARMv6_Assembler::genOpcode_bitShift(char** args, uint8_t opcodeImmPrefix, uint8_t opcodeRegPrefix) {
	OpcodeResult result = {0, 0};
	int argLen = strArrLen(args);

	int Rd = getRegNum(args[1]);

	// Immediate
	if (args[argLen-1][0] == '#') {
		int Rm = getRegNum(args[argLen-2]);
		if (Rd < 0 or Rd > 7 or Rm < 0 or Rm > 7) {
			log("Invalid registers: Rd and Rm must be between R0 and R7!", 1);
			result.invalid = 1;
			return result;
		}

		int imm = strtol(args[argLen-1]+1, NULL, 0);
		if (imm < 1 or imm > 32) {
			log("Invalid immediate value: must be between 1 and 32!", 1);
			result.invalid = 1;
			return result;
		}

		// ASRS  000 10 imm5 Rm Rd
		// LSLS  000 00 imm5 Rm Rd
		// LSRS  000 01 imm5 Rm Rd
		result.opcode |= Rd;
		result.opcode |= Rm << 3;
		result.opcode |= imm << 6;
		result.opcode |= opcodeImmPrefix << 11;
	}
	// Register
	else {
		int Rn = getRegNum(args[argLen-2]);
		if (Rd != Rn) {
			log("Invalid registers: Rd and Rn must be the same!", 1);
			result.invalid = 1;
			return result;
		}
		int Rm = getRegNum(args[argLen-1]);
		if (Rd < 0 or Rd > 7 or Rm < 0 or Rm > 7) {
			log("Invalid registers: all registers must be between R0 and R7!", 1);
			result.invalid = 1;
			return result;
		}

		// ASRS  010000 0100 Rm Rdn
		// LSLS  010000 0010 Rm Rdn
		// LSRS  010000 0011 Rm Rdn
		result.opcode |= Rd;
		result.opcode |= Rm << 3;
		result.opcode |= opcodeRegPrefix << 6;
		result.opcode |= 0b10000 << 10;
	}

	return result;
}


ARMv6_Assembler::OpcodeResult ARMv6_Assembler::genOpcode_bitwise(char** args, uint8_t opcodePrefix) {
	OpcodeResult result = {0, 0};
	int argLen = strArrLen(args);

	int Rd = getRegNum(args[1]);
	int Rn = getRegNum(args[argLen-2]);
	int Rm = getRegNum(args[argLen-1]);
	if (Rd != Rn) {
		log("Invalid registers: Rd and Rn must be the same!", 1);
		result.invalid = 1;
		return result;
	}
	if (Rd < 0 or Rd > 7 or Rm < 0 or Rm > 7) {
		log("Invalid registers: all registers must be between R0 and R7!", 1);
		result.invalid = 1;
		return result;
	}

	// ANDS 010000 0000 Rm Rdn
	// EORS 010000 0001 Rm Rdn
	// ORRS 010000 1100 Rm Rdn
	// BICS 010000 1110 Rm Rdn
	result.opcode |= Rd;
	result.opcode |= Rm << 3;
	result.opcode |= opcodePrefix << 6;
	result.opcode |= 0b10000 << 10;
	// log("EORS "s + args[1] + ","s + args[argLen-1], 3);

	return result;
}


ARMv6_Assembler::OpcodeResult ARMv6_Assembler::genOpcode_reverseBytes(char** args, uint8_t opcodePrefix) {
	OpcodeResult result = {0, 0};
	int Rd = getRegNum(args[1]);
	int Rm = getRegNum(args[2]);
	if (Rd < 0 or Rd > 7 or Rm < 0 or Rm > 7) {
		log("Invalid registers: Rd and Rm must be between R0 and R7!", 1);
		result.invalid = 1;
		return result;
	}

	result.opcode |= Rd;
	result.opcode |= Rm << 3;
	result.opcode |= opcodePrefix << 6;
	
	return result;
}

