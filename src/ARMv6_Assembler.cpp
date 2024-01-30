#include "ARMv6_Assembler.h"
#include <cstdlib>
#include <cstring>
#include <ctype.h>			// For toupper() / tolower()
#include <bits/stdc++.h>	// For sort()
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

uint16_t ARMv6_Assembler::genOpcode(string instruction) {
	// Separate instruction into arguments list by ' ' or ','
	char** args = strtokSplit(&instruction[0], (char*)" ,");
	int argLen = strArrLen(args);

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
						return 0;
					}

					// Encoding T1
					if (Rd >= 0 and Rd < 8) {
						if (Rn != 13 && Rn !=15) {
							log("Invalid register: Rn must be SP or PC when Rd is between R0 and R7!", 1);
							return 0;
						}
						if (Rn != 13 && Rn != 15) {
							log("Invalid register: Rn must be SP or PC!", 1);
							return 0;
						}
						if (imm < 0 or imm > 1020) {
							log("Invalid immediate value: must be between 0 and 1020!", 1);
							return 0;
						}

						opcode |= imm / 4;
						opcode |= Rd << 8;
						opcode |= 0b10101 << 11;
					}
					// Encoding T2
					else if (Rd == 13) {
						if (Rn != 13) {
							log("Invalid register: Rn must also be SP!", 1);
						}
						if (imm < 0 or imm > 508) {
							log("Invalid immediate value: must be between 0 and 508!", 1);
							return 0;
						}
						
						opcode |= imm / 4;
						opcode |= 0b101100000 << 7;
					}
					else {
						log("Invalid register: Rd must be between R0 and R7 or SP!", 1);
						return 0;
					}
				}
				// Register
				else {
					// Encoding T2
					if (Rd != Rn) {
						log("Invalid registers: Rd and Rn must be equal!", 1);
						return 0;
					}
					if (Rd < 0 or Rd > 15 or Rm < 0 or Rm > 15) {
						log("Invalid register: registers must be between R0 and R15!", 1);
						return 0;
					}
					if (Rn == 15 and Rm == 15) {
						log("Invalid registers: Rn and Rm must not both specify PC!", 1);
						return 0;
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
						return 0;
					}

					// Encoding T2
					if (Rd == Rn) {
						if (imm < 0 or imm > 255) {
							log("Invalid immediate value: must be between 0 and 255!", 1);
							return 0;
						}

						opcode |= imm;
						opcode |= getRegNum(args[1]) << 8;		// Rdn
						opcode |= 0b110 << 11;
					}
					// Encoding T1
					else {
						if (imm < 0 or imm > 7) {
							log("Invalid immediate value: must be between 0 and 7!", 1);
							return 0;
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
						return 0;
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
			{
				int Rd = getRegNum(args[1]);
				int Rn = getRegNum(args[argLen-2]);
				int Rm = getRegNum(args[argLen-1]);
				if (Rd != Rn) {
					log("Invalid registers: Rd and Rn must be the same!", 1);
					return 0;
				}
				if (Rd < 0 or Rd > 7 or Rm < 0 or Rm > 7) {
					log("Invalid registers: all registers must be between R0 and R7!", 1);
					return 0;
				}

				opcode |= Rd;
				opcode |= Rm << 3;
				opcode |= 1 << 14;
			}
			break;
		case 0x201e:		// ASRS
			{
				int Rd = getRegNum(args[1]);
				int Rn = getRegNum(args[argLen-2]);
				if (Rd < 0 or Rd > 7 or Rn < 0 or Rn > 7) {
					log("Invalid registers: all registers must be between R0 and R7!", 1);
					return 0;
				}

				// Immediate
				if (args[argLen-1][0] == '#') {
					int imm = strtol(args[argLen-1]+1, NULL, 0);
					if (imm < 1 or imm > 32) {
						log("Invalid immediate value: must be between 1 and 32!", 1);
						return 0;
					}

					opcode |= Rd;
					opcode |= Rn << 3;
					opcode |= imm << 6;
					opcode |= 1 << 12;
				}
				// Register
				else {
					if (Rd != Rn) {
						log("Invalid register: Rd must be same as Rn!", 1);
						return 0;
					}
					int Rm = getRegNum(args[argLen-1]);
					if (Rm < 0 or Rm > 7) {
						log("Invalid registers: all registers must be between R0 and R7!", 1);
						return 0;
					}

					opcode |= Rd;
					opcode |= Rm << 3;
					opcode |= 0b100000100 << 6;
				}
			}
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
		case 0x8a36:		// BKPT
		case 0x7313:		// BL
		case 0xd5cb:		// BLX
		case 0x731f:		// BX
		case 0xda23:		// CMN
		case 0xda25:		// CMP
		case 0xb2f8:		// CPSID
		case 0xb2f9:		// CPSIE
		case 0xde58:		// DMB
		case 0xdf1e:		// DSB
		case 0x409e:		// EORS
		case 0xf463:		// ISB
		case 0xff42:		// LDM
		case 0xff47:		// LDR
		case 0xe869:		// LDRB
		case 0xe86f:		// LDRH
		case 0xf7fc:		// LDRSB
		case 0xf802:		// LDRSH
		case 0x2783:		// LSLS
		case 0x2849:		// LSRS
		case 0x04f7:		// MOV
		case 0xa42a:		// MOVS
		case 0x0557:		// MRS
		case 0x0577:		// MSR
		case 0xbc66:		// MULS
		case 0xc0e9:		// MVNS
		case 0x0932:		// NOP
		case 0xc92b:		// ORRS
		case 0x11b4:		// POP
		case 0x6265:		// PUSH
		case 0x18f2:		// REV
		case 0x23f9:		// REV16
		case 0x286d:		// REVSH
		case 0x618b:		// RORS
		case 0x707f:		// RSBS
		case 0xb4b0:		// SBCS
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
	}
	return 0;
}

