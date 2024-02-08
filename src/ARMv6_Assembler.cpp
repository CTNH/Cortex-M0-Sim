#include "ARMv6_Assembler.h"
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <ctype.h>			// For toupper() / tolower()
#include <bits/stdc++.h>	// For sort()
#include <iterator>
#include <string>
#include <vector>
// C library
extern "C" {
	#include "basiclib_string.h"
}


ARMv6_Assembler::ARMv6_Assembler() {
	vector<char**> instArgs = cleanInstructions(readASMFile("test.m"));
	for (int i=0; i<instArgs.size(); i++) {
		genOpcode(instArgs.at(i));
	}
}

vector<string> ARMv6_Assembler::readASMFile(string fpath) {
	vector<string> out;

	string line;
	// OPen file using given file path
	ifstream file(fpath);
	if (file.is_open()) {
		while (getline(file, line)) {
			// Only add line to vector if line is not empty
			if (line != "")
				out.push_back(line);
		}
		// Clean up
		file.close();
	}
	else {
		log("Unable to open file "s + fpath, 1);
	}

	return out;
}

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

uint8_t ARMv6_Assembler::getSYSm(char* spReg) {
	map<string, int> sysmValues = {
		{"APSR",    0},
		{"IAPSR",   1},
		{"EAPSR",   2},
		{"XPSR",    3},
		{"IPSR",    5},
		{"EPSR",    6},
		{"IEPSR",   7},
		{"MSP",     8},
		{"PSP",     9},
		{"PRIMASK", 16},
		{"CONTROL", 20}
	};

	// No special register found
	if(sysmValues.count(spReg))
		return 0b11111111;
	else
		return sysmValues.at(spReg);
}


void ARMv6_Assembler::addLabel(string label) {
	// TODO: duplicate labels
	
	// Add label to list
	labels[label] = PC;
	log("Added '"s + label + "' to labels", 2);
}

pair<bool, int> ARMv6_Assembler::labelOffsetLookup(string label) {
	// Boolean value -> if output is valid(1) or invalid(0)
	// Integer -> immediate offset of label
	pair<bool, uint32_t> out(1, 0);

	// Invalid
	if (labels.find(label) == labels.end()) {
	// if (~out.first) {
		log("Invalid label: no label with name "s + label + "found!", 1);
		out.first = false;
		return out;
	}

	out.second = labels.find(label) -> second;

	return out;
}

vector<char**> ARMv6_Assembler::cleanInstructions(vector<string> lines) {
	vector<char**> instructions;
	for (int i=0; i<lines.size(); i++) {
		// Remove starting spaces and tabs
		if (lines.at(i).find_first_not_of(" \t") != string::npos)
			lines.at(i).erase(0, lines.at(i).find_first_not_of(" \t"));

		// Remove comments
		if (lines.at(i).find(";") != -1) {
			lines.at(i).erase(lines.at(i).find(";"));
		}

		// Separate instruction into arguments list by ' ' or ','
		char** args = strtokSplit(&lines.at(i)[0], (char*)" ,");
		// If no elements in array skip
		if (strArrLen(args) == 0)
			continue;

		instructions.push_back(args);
	}
	return instructions;
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

ARMv6_Assembler::OpcodeResult ARMv6_Assembler::genOpcode(char** args) {
//uint16_t ARMv6_Assembler::genOpcode(string instruction) {
	int argLen = strArrLen(args);
	OpcodeResult result = {};
	
	// TODO: Need to separate into function; labels declared later can be called
	// Label; if first argument ends with ':'
	if (args[0][strlen(args[0])-1] == ':') {
		addLabel(args[0]);
		args++;		// Remove first argument
		if (argLen - 1 == 0)
			// TODO: return label flag
			return result;
		
		argLen--;
	}
	// If first argument starts with '.'
	else if (args[0][0] == '.') {}

	switch (djb2Hash(args[0])) {
		case 0xde60:		// ADCS
			{
				int Rd = getRegNum(args[1]);
				int Rn = getRegNum(args[argLen-2]);
				int Rm = getRegNum(args[argLen-1]);
				result.opcode = 0b0100000101000000;
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
						result.invalid = 1;
						break;
					}

					// Encoding T1
					if (Rd >= 0 and Rd < 8) {
						if (Rn != 13 && Rn !=15) {
							log("Invalid register: Rn must be SP or PC when Rd is between R0 and R7!", 1);
							result.invalid = 1;
							break;
						}
						if (Rn != 13 && Rn != 15) {
							log("Invalid register: Rn must be SP or PC!", 1);
							result.invalid = 1;
							break;
						}
						if (imm < 0 or imm > 1020) {
							log("Invalid immediate value: must be between 0 and 1020!", 1);
							result.invalid = 1;
							break;
						}

						result.opcode = imm / 4;
						result.opcode |= Rd << 8;
						result.opcode |= 0b10101 << 11;
						// log16bitOpcode(instruction, opcode);
					}
					// Encoding T2
					else if (Rd == 13) {
						if (Rn != 13) {
							log("Invalid register: Rn must also be SP!", 1);
						}
						if (imm < 0 or imm > 508) {
							log("Invalid immediate value: must be between 0 and 508!", 1);
							result.invalid = 1;
							break;
						}
						
						result.opcode = imm / 4;
						result.opcode |= 0b101100000 << 7;
						// log16bitOpcode(instruction, opcode);
					}
					else {
						log("Invalid register: Rd must be between R0 and R7 or SP!", 1);
						result.invalid = 1;
						break;
					}
				}
				// Register
				else {
					// Encoding T2
					if (Rd != Rn) {
						log("Invalid registers: Rd and Rn must be equal!", 1);
						result.invalid = 1;
						break;
					}
					if (Rd < 0 or Rd > 15 or Rm < 0 or Rm > 15) {
						log("Invalid register: registers must be between R0 and R15!", 1);
						result.invalid = 1;
						break;
					}
					if (Rn == 15 and Rm == 15) {
						log("Invalid registers: Rn and Rm must not both specify PC!", 1);
						result.invalid = 1;
						break;
					}
					result.opcode = Rd;
					result.opcode |= Rm << 3;
					result.opcode |= (Rd & 0x1) << 7;		// DN bit
					result.opcode |= 0b1000100 << 8;
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
						result.invalid = 1;
						break;
					}

					// Encoding T2
					if (Rd == Rn) {
						if (imm < 0 or imm > 255) {
							log("Invalid immediate value: must be between 0 and 255!", 1);
							result.invalid = 1;
							break;
						}

						result.opcode = imm;
						result.opcode |= getRegNum(args[1]) << 8;		// Rdn
						result.opcode |= 0b110 << 11;
					}
					// Encoding T1
					else {
						if (imm < 0 or imm > 7) {
							log("Invalid immediate value: must be between 0 and 7!", 1);
							result.invalid = 1;
							break;
						}

						result.opcode = Rd;
						result.opcode |= Rn << 3;
						result.opcode |= imm << 6;
						result.opcode |= 0b1110 << 9;
					}
				}
				// Register
				else {
					// Encoding T1
					if (Rm < 0 or Rm > 7) {
						log("Invalid register: Rm must be between 0 and 7!", 1);
						result.invalid = 1;
						break;
					}

					result.opcode |= Rd;
					result.opcode |= Rn << 3;
					result.opcode |= Rm << 6;
					result.opcode |= 0b1100 << 9;
				}
			}
			break;
		case 0xd07c:		// ADR
			{
				// TODO
				result.opcode = 0;
			}
			break;
		case 0x090b:		// ANDS
			result = genOpcode_bitwise(args, 0b0000);
			break;
		case 0x201e:		// ASRS
			result = genOpcode_bitShift(args, 0b10, 0b0100);
			break;
		case 0xb5e7:		// B
			result = genOpcode_branch(args, 0, 1);
			break;
		case 0xd4dd:		// BEQ
			result = genOpcode_branch(args, 0b0000);
			break;
		case 0xd5fa:		// BNE
			result = genOpcode_branch(args, 0b0001);
			break;
		case 0xd49d:		// BCS
		case 0xd542:		// BHS
			result = genOpcode_branch(args, 0b0010);
			break;
		case 0xd48d:		// BCC
		case 0xd5c2:		// BLO
			result = genOpcode_branch(args, 0b0011);
			break;
		case 0xd5dd:		// BMI
			result = genOpcode_branch(args, 0b0100);
			break;
		case 0xd643:		// BPL
			result = genOpcode_branch(args, 0b0101);
			break;
		case 0xd710:		// BVS
			result = genOpcode_branch(args, 0b0110);
			break;
		case 0xd700:		// BVC
			result = genOpcode_branch(args, 0b0111);
			break;
		case 0xd538:		// BHI
			result = genOpcode_branch(args, 0b1000);
			break;
		case 0xd5c6:		// BLS
			result = genOpcode_branch(args, 0b1001);
			break;
		case 0xd513:		// BGE
			result = genOpcode_branch(args, 0b1010);
			break;
		case 0xd5c7:		// BLT
			result = genOpcode_branch(args, 0b1011);
			break;
		case 0xd522:		// BGT
			result = genOpcode_branch(args, 0b1100);
			break;
		case 0xd5b8:		// BLE
			result = genOpcode_branch(args, 0b1101);
			break;
		case 0xd454:		// BAL
			result = genOpcode_branch(args, 0b1110);
			break;
		case 0x8006:		// BICS
			result = genOpcode_bitwise(args, 0b1110);
			break;
		case 0x8a36:		// BKPT
			{
				int imm = strtol(args[1]+1, NULL, 0);
				if (imm < 0 or imm > 255) {
					log("Invalid immediate value: must be between 0 and 255!", 1);
					result.invalid = 1;
					break;
				}

				result.opcode = imm;
				result.opcode |= 0b10111110 << 8;
			}
			break;
		case 0x7313:		// BL
		case 0xd5cb:		// BLX
			result = genOpcode_branchExchange(args, 0b10001111);
			break;
		case 0x731f:		// BX
			result = genOpcode_branchExchange(args, 0b10001110);
			break;
		case 0xda23:		// CMN
			{
				int Rn = getRegNum(args[1]);
				int Rm = getRegNum(args[2]);

				if (Rn < 0 or Rn > 7 or Rm < 0 or Rm > 7) {
					log("Invalid register: Rn and Rm must be between R0 and R7!", 1);
					result.invalid = 1;
					break;
				}

				result.opcode = Rn;
				result.opcode |= Rm;
				result.opcode |= 0b100001011 << 6;
			}
			break;
		case 0xda25:		// CMP
			{
				int Rn = getRegNum(args[1]);
				if (Rn < 0 or Rn > 14) {
					log("Invalid register: Rn must be between R0 and R14!", 1);
					result.invalid = 1;
					break;
				}

				// Immediate
				if (args[2][0] == '#') {
					int imm = strtol(args[2]+1, NULL, 0);
					if (imm < 0 or imm > 255) {
						log("Invalid immediate value: must be between 0 and 255!", 1);
						result.invalid = 1;
						break;
					}

					result.opcode = imm;
					result.opcode |= Rn << 8;
					result.opcode |= 0b101 >> 11;
				}
				// Register
				else {
					int Rm = getRegNum(args[2]);
					if (Rm < 0 or Rm > 14) {
						log("Invalid register: Rm must be between R0 and R14!", 1);
						result.invalid = 1;
						break;
					}

					// Encoding T1 - Rn and Rm both from R0-R7
					if (Rn < 8 and Rm < 8) {
						result.opcode = Rn;
						result.opcode |= Rm << 3;
						result.opcode |= 0b100001010;
					}
					else {
						result.opcode = Rn;
						result.opcode |= Rm << 3;
						result.opcode |= (Rn >> 3) << 7;	// N bit
						result.opcode |= 0b1000101 << 8;
					}
				}
			}
			break;
		case 0xb2f8:		// CPSID
			result.opcode = 0b1011011001110010;
			break;
		case 0xb2f9:		// CPSIE
			result.opcode = 0b1011011001100010;
			break;
		case 0xde58:		// DMB
			result = genOpcode_barrier(args, 0b0101);
			break;
		case 0xdf1e:		// DSB
			result = genOpcode_barrier(args, 0b0100);
			break;
		case 0x409e:		// EORS
			result = genOpcode_bitwise(args, 0b0001);
			break;
		case 0xf463:		// ISB
			result = genOpcode_barrier(args, 0b0110);
			break;
		case 0xff42:		// LDM
			{
				// TODO: reglist
				int Rn = getRegNum(args[1]);
				if (Rn < 0 or Rn > 7) {
					log("Invalid register: must be between R0 and R7!", 1);
					result.invalid = 1;
					break;
				}
				// Writeback
				if (args[1][strlen(args[1])-1] == '!') {}
			}
			break;
		case 0xff47:		// LDR
			{
				// LDR Rt, [<Rn | SP> {, #imm}]		// Immediate
				// 		LDR R0, [R1
				// 		LDR R0, [R1,#100]
				// 		LDR R0, [SP,#100]
				// LDR Rt, [Rn , Rm]				// Register
				// 		LDR R0, [R1, R2]
				// LDR Rt, <label | [PC, #imm]>		// Literal
				// 		LDR R0, LabelX
				// 		LDR R0, [PC,#100]
				int Rt = getRegNum(args[1]);
				if (Rt < 0 or Rt > 7) {
					log("Invalid register: Rt must be between R0 and R7!", 1);
					result.invalid = 1;
					break;
				}

				enum instType {
					imm,	// Immediate Offset
					reg,	// Register Offset
					ltr		// Literal
				};
				instType itype;
				// Determines if instruction is using immediate offset, register offset or literal
				if (argLen < 4) {
					if (args[2][0] == '[')
						itype = imm;
					else
						itype = ltr;
				}
				else {
					if (args[3][0] == '#') {
						// Rn is PC
						if (getRegNum(args[2]+1) == 15)
							itype = ltr;
						else
							itype = imm;
					}
					else
						itype = reg;
				}

				// TODO: remove ']' from last arg if exist
				switch (itype) {
					case imm:
						{
							/*
							int Rn = getRegNum(args[2]+1);
							if ((Rn < 0 or Rn > 7) and Rn != 13) {
								log("Invalid register: Rn must be between R0 and R7 or SP!", 1);
								invalidInstruction = 1;
								break;
							}

							int imm = 0;
							// Only change immediate value if argument exists
							if (argLen > 3) {
								imm = strtol(args[3]+1, NULL, 0);
								// Assume R0-R7 as base; use 124 as upper limit
								int uplim = 124;
								// SP as base
								if (Rn == 13)
									uplim = 1020;

								if (imm < 0 or imm > uplim) {
									log("Invalid immediate value: must be between 0 and "s + to_string(uplim) + "!", 1);
									invalidInstruction = 1;
									break;
								}
							}
							else {
								log("Immediate offset not provided, setting to 0.", 3);
							}

							// Encoding T1
							if (Rn != 13) {
								// LDR  011 0 1 imm5 Rn Rt
								opcode |= Rt;
								opcode |= Rn << 3;
								opcode |= imm << 6;
								opcode |= 0b01101 << 11;
							}
							// Encoding T2
							else {
								// LDR  1001 1 Rt imm8
								opcode |= imm;
								opcode |= Rt << 8;
								opcode |= 0b10011 << 11;
							}
						*/
							result = genOpcode_loadStoreImm(args, 0b01101);
						}
						break;
					case reg:
						/*
						{
							int Rn = getRegNum(args[2]+1);
							int Rm = getRegNum(args[3]);
							if (Rn < 0 or Rn > 7 or Rm < 0 or Rm > 7) {
								log("Invalid register: all registers must be between R0 and R7!", 1);
								invalidInstruction = 1;
								break;
							}

							// LDR  0101 100 Rm Rn Rt
							opcode |= Rt;
							opcode |= Rn << 3;
							opcode |= Rm << 6;
							opcode |= 0b0101100;
						}
						*/
						result = genOpcode_loadStoreReg(args, 0b100);
						break;
					case ltr:
						{
							// TODO: labels
						}
						break;
				}
			}
			break;
		case 0xe869:		// LDRB
			{
				// Immediate
				if (argLen < 4 or args[3][0] == '#')
					result = genOpcode_loadStoreImm(args, 0b01111);
				// Register
				else
					result = genOpcode_loadStoreReg(args, 0b110);
			}
			break;
		case 0xe86f:		// LDRH
			{
				// Immediate
				if (argLen < 4 or args[3][0] == '#')
					result = genOpcode_loadStoreImm(args, 0b10001);
				// Register
				else
					result = genOpcode_loadStoreReg(args, 0b101);
			}
			break;
		case 0xf7fc:		// LDRSB
			result = genOpcode_loadStoreReg(args, 0b011);
			break;
		case 0xf802:		// LDRSH
			result = genOpcode_loadStoreReg(args, 0b111);
			break;
		case 0x2783:		// LSLS
			result = genOpcode_bitShift(args, 0b00, 0b0010);
			break;
		case 0x2849:		// LSRS
			result = genOpcode_bitShift(args, 0b01, 0b0011);
			break;
		case 0x04f7:		// MOV
			{
				// Register Only
				int Rd = getRegNum(args[1]);
				int Rm = getRegNum(args[2]);
				if (Rd < 0 or Rd > 15 or Rm < 0 or Rm > 15) {
					log("Invalid registers: no such register!", 1);
					result.invalid = 1;
					break;
				}

				result.opcode = Rd;
				result.opcode |= Rm << 3;
				result.opcode |= (Rd >> 3) << 7;	// D bit
				result.opcode |= 0b1000110 << 8;
			}
		case 0xa42a:		// MOVS
			{
				int Rd = getRegNum(args[1]);
				if (Rd < 0 or Rd > 7) {
					log("Invalid register: Rd must be between R0 and R7!", 1);
					result.invalid = 1;
					break;
				}

				// Immediate
				if (args[2][0] == '#') {
					int imm = strtol(args[argLen-1]+1, NULL, 0);
					if (imm < 0 or imm > 255) {
						log("Invalid immediate value: must be between 0 and 255!", 1);
						result.invalid = 1;
						break;
					}

					result.opcode = imm;
					result.opcode |= Rd << 8;
					result.opcode |= 0b100 << 11;
				}
				// Register
				else {
					int Rm = getRegNum(args[2]);
					if (Rm < 0 or Rm > 7) {
						log("Invalid register: Rm must be between R0 and R7!", 1);
						result.invalid = 1;
						break;
					}

					result.opcode = Rd;
					result.opcode |= Rm << 3;
				}
			}
			break;
		case 0x0557:		// MRS
			{
				result.i32 = 1;
				int Rd = getRegNum(args[1]);
				if (Rd < 0 or Rd == 13 or Rd == 15) {
					log("Invalid register: Rd must not be SP or PC!", 1);
					result.invalid = 1;
					break;
				}
				uint8_t SYSm = getSYSm(args[2]);
				if (SYSm == 0b11111111) {
					log("Invalid special register: cannot find such register!", 1);
					result.invalid = 1;
					break;
				}

				result.opcode = SYSm;
				result.opcode |= Rd << 8;
				result.opcode |= 0b11110011111011111000 << 12;
			}
			break;
		case 0x0577:		// MSR
			{
				result.i32 = 1;
				uint8_t SYSm = getSYSm(args[1]);
				if (SYSm == 0b11111111) {
					log("Invalid special register: cannot find such register!", 1);
					result.invalid = 1;
					break;
				}
				int Rd = getRegNum(args[2]);
				if (Rd < 0 or Rd == 13 or Rd == 15) {
					log("Invalid register: Rd must not be SP or PC!", 1);
					result.invalid = 1;
					break;
				}

				result.opcode = SYSm;
				result.opcode |= 0b10001000 << 8;
				result.opcode |= Rd << 16;
				result.opcode |= 0b111100111000 << 20;
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
					result.invalid = 1;
					break;
				}
				if (Rd < 0 or Rd > 7 or Rm < 0 or Rm > 7) {
					log("Invalid register: Rm must be between R0 and R7!", 1);
					result.invalid = 1;
					break;
				}

				result.opcode = Rd;
				result.opcode |= Rn << 3;
				result.opcode |= 0b100001101 << 6;
			}
			break;
		case 0xc0e9:		// MVNS
			{
				int Rd = getRegNum(args[1]);
				int Rm = getRegNum(args[2]);
				if (Rd < 0 or Rd > 7 or Rm < 0 or Rm > 7) {
					log("Invalid register: Rd and Rm must be between R0 and R7!", 1);
					result.invalid = 1;
					break;
				}

				result.opcode = Rd;
				result.opcode |= Rm << 3;
				result.opcode |= 0b100001111 << 6;
			}
		case 0x0932:		// NOP
			result.opcode = 0b10111111 << 8;
			break;
		case 0xc92b:		// ORRS
			result = genOpcode_bitwise(args, 0b1100);
			break;
		case 0x11b4:		// POP
			{}
			break;
		case 0x6265:		// PUSH
			{}
			break;
		case 0x18f2:		// REV
			result = genOpcode_reverseBytes(args, 0b00);
			break;
		case 0x23f9:		// REV16
			result = genOpcode_reverseBytes(args, 0b01);
			break;
		case 0x286d:		// REVSH
			result = genOpcode_reverseBytes(args, 0b11);
			break;
		case 0x618b:		// RORS
			{
				int Rd = getRegNum(args[1]);
				int Rn = getRegNum(args[argLen-2]);
				int Rm = getRegNum(args[argLen-1]);
				if (Rd != Rn) {
					log("Invalid registers: Rd and Rn must be the same!", 1);
					result.invalid = 1;
					break;
				}
				if (Rd < 0 or Rd > 7 or Rm < 0 or Rm > 7) {
					log("Invalid registers: all registers must be between R0 and R7!", 1);
					result.invalid = 1;
					break;
				}

				result.opcode = Rd;
				result.opcode |= Rm << 3;
				result.opcode |= 0b100000111 << 6;
			}
			break;
		case 0x707f:		// RSBS
			{}
			break;
		case 0xb4b0:		// SBCS
			{}
			break;
		case 0x1d33:		// SEV
			result.opcode = 0b1011111101000000;
			break;
		case 0x1f19:		// STM
		case 0x1f1e:		// STR
			{
				// Immediate
				if (argLen < 4 or args[3][0] == '#')
					result = genOpcode_loadStoreImm(args, 0b01100);
				// Register
				else
					result = genOpcode_loadStoreReg(args, 0b000);
			}
			break;
		case 0x0320:		// STRB
			{
				// Immediate
				if (argLen < 4 or args[3][0] == '#')
					result = genOpcode_loadStoreImm(args, 0b01110);
				// Register
				else
					result = genOpcode_loadStoreReg(args, 0b010);
			}
			break;
		case 0x0326:		// STRH
			{
				// Immediate
				if (argLen < 4 or args[3][0] == '#')
					result = genOpcode_loadStoreImm(args, 0b10000);
				// Register
				else
					result = genOpcode_loadStoreReg(args, 0b001);
			}
			break;
		case 0x1f2f:		// SUB
		case 0x0562:		// SUBS
		case 0x1f51:		// SVC
			{
				int imm = strtol(args[1]+1, NULL, 0);
				if (imm < 0 or imm > 255) {
					log("Invalid immediate value: must be between 0 and 255", 1);
					break;
				}

				result.opcode = imm;
				result.opcode |= 0b11011111 << 8;
			}
			break;
		case 0x1466:		// SXTB
			result = genOpcode_extendRegister(args, 0b01);
			break;
		case 0x146c:		// SXTH
			result = genOpcode_extendRegister(args, 0b00);
			break;
		case 0x2340:		// TST
			{
				int Rn = getRegNum(args[1]);
				int Rm = getRegNum(args[1]);
				if (Rn < 0 or Rn > 7 or Rm < 0 or Rm > 7) {
					log("Invalid registers: Rd and Rm must be between R0 and R7!", 1);
					result.invalid = 1;
					break;
				}

				result.opcode = Rn;
				result.opcode |= Rm << 3;
				result.opcode |= 0b0100001000 << 6;
			}
			break;
		case 0x2d28:		// UXTB
			result = genOpcode_extendRegister(args, 0b11);
			break;
		case 0x2d2e:		// UXTH
			result = genOpcode_extendRegister(args, 0b10);
			break;
		case 0x2e47:		// WFE
			result.opcode = 0b1011111100100000;
			break;
		case 0x2e4b:		// WFI
			result.opcode = 0b1011111100110000;
			break;

		default:
			log("Invalid instruction: instruction not found!", 1);
			result.invalid = 1;
			break;
	}

	if (result.invalid)
		return result;


	// Increment PC on valid instruction
	// TODO: increment PC by 8 on 32-bits instructions
	PC += 4;

	return result;
}


ARMv6_Assembler::OpcodeResult ARMv6_Assembler::genOpcode_bitShift(char** args, uint8_t opcodeImmPrefix, uint8_t opcodeRegPrefix) {
	OpcodeResult result = {};
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
	OpcodeResult result = {};
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


ARMv6_Assembler::OpcodeResult ARMv6_Assembler::genOpcode_branch(char** args, uint8_t opcodePrefix, bool t2) {
	OpcodeResult result = {};

	// Encoding T1
	pair<bool, int> labelOffset = labelOffsetLookup(args[1]);
	if (!labelOffset.first) {
		result.invalid = 1;
		return result;
	}
	int immOffset = labelOffset.second - PC;

	// Encoding T2
	if (t2 and (immOffset < -2048 or immOffset > 2046 or immOffset % 2 != 0)) {
		log("Invalid immediate offset: must be even number in between -2048 and 2046!", 1);
		result.invalid = 1;
		return result;
	}
	// Encoding T1
	else if (immOffset < -256 or immOffset > 254 or immOffset % 2 != 0) {
		log("Invalid immediate offset: must be even number in between -256 and 254!", 1);
		result.invalid = 1;
		return result;
	}

	// EQ 0b0000
	// NE 0b0001
	// CS 0b0010
	// CC 0b0011
	// MI 0b0100
	// PL 0b0101
	// VS 0b0110
	// VC 0b0111
	// HI 0b1000
	// LS 0b1001
	// GE 0b1010
	// LT 0b1011
	// GT 0b1100
	// LE 0b1101
	// AL 0b1110
	result.opcode |= immOffset;
	// Encoding T2
	if (t2)
		result.opcode |= 0b11100 << 11;
	// Encoding T1
	else {
		result.opcode |= opcodePrefix << 8;
		result.opcode |= 0b1101 << 12;
	}

	return result;
}


ARMv6_Assembler::OpcodeResult ARMv6_Assembler::genOpcode_branchExchange(char** args, uint8_t opcodePrefix) {
	OpcodeResult result = {};
	int Rm = getRegNum(args[1]);
	if (Rm < 0 or Rm == 13 or Rm == 15) {
		log("Invalid register: SP or PC can not be used!", 1);
		result.invalid = 1;
		return result;
	}
	result.opcode = Rm << 3;
	result.opcode |= opcodePrefix << 7;
	return result;
}


ARMv6_Assembler::OpcodeResult ARMv6_Assembler::genOpcode_reverseBytes(char** args, uint8_t opcodePrefix) {
	OpcodeResult result = {};
	int Rd = getRegNum(args[1]);
	int Rm = getRegNum(args[2]);
	if (Rd < 0 or Rd > 7 or Rm < 0 or Rm > 7) {
		log("Invalid registers: Rd and Rm must be between R0 and R7!", 1);
		result.invalid = 1;
		return result;
	}

	// REV    1011 1010 00 Rm Rd
	// REV16  1011 1010 01 Rm Rd
	// REVSH  1011 1010 11 Rm Rd
	result.opcode |= Rd;
	result.opcode |= Rm << 3;
	result.opcode |= opcodePrefix << 6;
	result.opcode |= 0b10111010 << 8;
	
	return result;
}


ARMv6_Assembler::OpcodeResult ARMv6_Assembler::genOpcode_loadStoreImm(char** args, uint8_t opcodePrefix) {
	OpcodeResult result = {};

	int Rt = getRegNum(args[1]);
	int Rn = getRegNum(args[2]+1);
	// Instructin is either LDR or STR
	bool possibleSP = strcmp(args[0], "LDR") == 0 or strcmp(args[0], "STR") == 0;
	if (
			// Either Rt or Rn is not in 0-7
			Rt < 0 or Rt > 7 or Rn < 0 or Rn > 7 or
			// Rn is SP and instruction is not LDR or STR
			(Rn == 13 and possibleSP == 0)
		) {
		if (possibleSP)
			log("Invalid register: Rt and Rn must be between R0 and R7 or SP!", 1);
		else
			log("Invalid register: Rt and Rn must be between R0 and R7!", 1);
		result.invalid = 1;
		return result;
	}

	// Calculate immediate value
	int imm = 0;
	// Only change immediate value if argument exists
	if (strArrLen(args) > 3) {
		imm = strtol(args[3]+1, NULL, 0);
		int uplim = 0;
		int mul = 0;	// Immediate must be multiples of n
		switch (djb2Hash(args[0])) {
			case 0xff47:	// LDR
			case 0x1f1e:	// STR
				{
					// Assume R0-R7 as base; use 124 as upper limit
					uplim = 124;
					// SP as base
					if (Rn == 13)
						uplim = 1020;
					mul = 4;
				}
				break;
			case 0xe869:	// LDRB
			case 0x0320:	// STRB
				uplim = 31;
				break;
			case 0xe86f:	// LDRH
			case 0x0326:	// STRH
				uplim = 62;
				mul = 2;
				break;
		}

		if (imm < 0 or imm > uplim or imm % mul != 0) {
			log("Invalid immediate value: must be between 0 and "s + to_string(uplim) + ", also must be a multiple of "s + to_string(mul) + "!", 1);
			result.invalid = 1;
			return result;
		}
	}
	else {
		log("Immediate offset not provided, setting to 0.", 3);
	}

	// LDR   011 0 1 imm5 Rn Rt
	// LDRB  011 1 1 imm5 Rn Rt
	// STR   011 0 0 imm5 Rn Rt
	// STRB  011 1 0 imm5 Rn Rt
	// LDRH  1000 1 imm5 Rn Rt
	// STRH  1000 0 imm5 Rn Rt

	// Encoding T1
	if (Rn != 13) {
		// LDR  011 0 1 imm5 Rn Rt
		result.opcode |= Rt;
		result.opcode |= Rn << 3;
		result.opcode |= imm << 6;
		result.opcode |= opcodePrefix << 11;
	}
	// Encoding T2 (SP)
	else {
		// LDR  1001 1 Rt imm8
		// STR	1001 0 Rt imm8
		result.opcode |= imm;
		result.opcode |= Rt << 8;
		result.opcode |= (opcodePrefix & 1) << 11;
		result.opcode |= 0b1001 << 12;
	}

	return result;
}


ARMv6_Assembler::OpcodeResult ARMv6_Assembler::genOpcode_loadStoreReg(char** args, uint8_t opcodePrefix) {
	OpcodeResult result = {};
	
	int Rt = getRegNum(args[1]);
	int Rn = getRegNum(args[2]+1);
	int Rm = getRegNum(args[3]);
	if (Rt < 0 or Rt > 7 or Rn < 0 or Rn > 7 or Rm < 0 or Rm > 7) {
		log("Invalid register: all registers must be between R0 and R7!", 1);
		result.invalid = 1;
		return result;
	}

	// LDR     0101 100 Rm Rn Rt
	// LDRB    0101 110 Rm Rn Rt
	// LDRH    0101 101 Rm Rn Rt
	// LDRSB   0101 011 Rm Rn Rt
	// LDRSH   0101 111 Rm Rn Rt
	// STR     0101 000 Rm Rn Rt
	// STRB    0101 010 Rm Rn Rt
	// STRH    0101 001 Rm Rn Rt
	result.opcode |= Rt;
	result.opcode |= Rn << 3;
	result.opcode |= Rm << 6;
	result.opcode |= opcodePrefix << 9;
	result.opcode |= 0b0101 << 12;

	return result;
}


ARMv6_Assembler::OpcodeResult ARMv6_Assembler::genOpcode_extendRegister(char** args, uint8_t opcodePrefix) {
	OpcodeResult result = {};

	int Rd = getRegNum(args[1]);
	int Rm = getRegNum(args[2]);
	if (Rd < 0 or Rd > 7 or Rm < 0 or Rm > 7) {
		log("Invalid registers: Rd and Rm must be between R0 and R7!", 1);
		result.invalid = 1;
		return result;
	}

	// SXTB  1011 0010 01 Rm Rd
	// SXTH  1011 0010 00 Rm Rd
	// UXTB  1011 0010 11 Rm Rd
	// UXTH  1011 0010 10 Rm Rd
	result.opcode |= Rd;
	result.opcode |= Rm << 3;
	result.opcode |= opcodePrefix << 6;
	result.opcode |= 0b10110010 << 8;

	return result;
}


ARMv6_Assembler::OpcodeResult ARMv6_Assembler::genOpcode_barrier(char** args, uint8_t opcodePrefix) {
	OpcodeResult result = {};

	result.i32 = 1;
	if (strArrLen(args) > 1) {
		if ((string)args[1] == "SY")
			result.opcode |= 0b1111;
		else {
			log("Invalid option: must either be SY or omitted!", 1);
			result.invalid = 1;
			return result;
		}
	}
	// DMB  11110 0 111 01 1 1111 10 0 0 1111 0101 option
	// DSB  11110 0 111 01 1 1111 10 0 0 1111 0100 option
	// ISB  11110 0 111 01 1 1111 10 0 0 1111 0110 option
	result.opcode |= opcodePrefix << 4;
	result.opcode |= 0b111100111011111110001111 << 8;

	return result;
}


