#include "cortex-m0p.h"
#include "cortex-m0p_registers.h"
#include "basiclib_string.h"
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <math.h>

// Define Registers
ProcessorCore_Register pcReg;
SystemControl_Register scReg;
NVIC_Register nvicReg;
MPU_Register mpuReg;

uint8_t memory[0xFFFFFFFF];

// Cycle counts for each instruction listed in RP2040 Datasheet page 69
int cycle = 0;

int instructionHash(char* inst) {
	int hash = 0;
	for (int i = 0; i < strlen(inst); i++) {
		hash = (hash << 3) + inst[i] + i*2;
	}
	return hash & 0xfff;	// Only keep last 3 bytes
}

// Returns 1 if all hashes are unique, 0 if not
int hashUniqueCheck() {
	char instructions[76][8] = {
		"adcs",
		"add",
		"adr",
		"ands",
		"asrs",
		"b",
		"bics",
		"bkpt",
		"bl",
		"blx",
		"bx",
		"cmn",
		"cmp",
		"cpsid",
		"cpsie",
		"dmb",
		"dsb",
		"eors",
		"isb",
		"ldm",
		"ldr",
		"ldrb",
		"ldrh",
		"ldrsb",
		"ldrsh",
		"lsls",
		"lsrs",
		"mov",
		"mrs",
		"msr",
		"muls",
		"mvns",
		"nop",
		"orrs",
		"pop",
		"push",
		"rev",
		"rev16",
		"revsh",
		"rors",
		"rsbs",
		"sbcs",
		"sev",
		"stm",
		"str",
		"strb",
		"strh",
		"sub",
		"svc",
		"sxtb",
		"sxth",
		"tst",
		"uxtb",
		"uxth",
		"wfe",
		"wfi",
		// s
		"adds",
		"subs",
		"movs",
		// branch conditional
		"beq",
		"bne",
		"bcs",
		"bhs",
		"bcc",
		"blo",
		"bmi",
		"bpl",
		"bvs",
		"bvc",
		"bhi",
		"bls",
		"bge",
		"blt",
		"bgt",
		"ble",
		"bal"
	};
	int hashes[76];
	for (int i = 0; i < 76; i++) {
		hashes[i] = instructionHash(instructions[i]);
		// Iterate through previous hashes
		for (int j=0; j<i; j++) {
			// Hash collision
			if (hashes[i] == hashes[j]) {
				return 0;
			}
		}
	}
	// When no hash collides
	return 1;
}

// Returns the number of the processor core register given its name
int pcRegNum(char* name) {
	if (strcmp(name, "R0") == 0)
		return 0;
	if (strcmp(name, "R1") == 0)
		return 1;
	if (strcmp(name, "R2") == 0)
		return 2;
	if (strcmp(name, "R3") == 0)
		return 3;
	if (strcmp(name, "R4") == 0)
		return 4;
	if (strcmp(name, "R5") == 0)
		return 5;
	if (strcmp(name, "R6") == 0)
		return 6;
	if (strcmp(name, "R7") == 0)
		return 7;
	if (strcmp(name, "R8") == 0)
		return 8;
	if (strcmp(name, "R9") == 0)
		return 9;
	if (strcmp(name, "R10") == 0)
		return 10;
	if (strcmp(name, "R11") == 0)
		return 11;
	if (strcmp(name, "R12") == 0)
		return 12;
	if (strcmp(name, "R13") == 0 | strcmp(name, "MSP") == 0 | strcmp(name, "PSP") == 0)
		return 13;
	if (strcmp(name, "LR") == 0)
		return 14;
	if (strcmp(name, "PC") == 0)
		return 15;
	return -1;
}

uint32_t* getRegPtr(char* reg) {
	switch (reg[0]) {
		case 'R':		// R0-R13
			switch (reg[1]) {
				case '0':
					return &pcReg.R0;
				case '1':
					if (strlen(reg) > 2)
						// R10-R13
						switch (reg[2]) {
							case '0':
								return &pcReg.R10;
							case '1':
								return &pcReg.R11;
							case '2':
								return &pcReg.R12;
							// No R13
						}
					else	// Single 'R1'
						return &pcReg.R1;
				case '2':
					return &pcReg.R2;
				case '3':
					return &pcReg.R3;
				case '4':
					return &pcReg.R4;
				case '5':
					return &pcReg.R5;
				case '6':
					return &pcReg.R6;
				case '7':
					return &pcReg.R7;
				case '8':
					return &pcReg.R8;
				case '9':
					return &pcReg.R9;
				default:
					break;
			}
			break;
		case 'L':
			return &pcReg.LR;
		case 'M':
			return &pcReg.MSP;
		case 'P':
			if (reg[1] == 'C')
				return &pcReg.PC;
			else
				// TODO: Confirm only PSP is used
				return &pcReg.PSP;
			break;
	}
	return NULL;
}

// Get the nth(bit) bit from value
int getBit(int value, int bit) {
	if (value & 1>>bit)
		return 1;
	else
		return 0;
}

void updateFlag(char flag, int value) {
	switch(flag) {
		case 'N':
			break;
		case 'Z':
			break;
		case 'C':
			break;
		case 'V':
			break;
	}
}
int getFlag(char flag) {
	return 0;
}

int execInstruction(char* inst) {
	
	char** instArgs = strSplit(inst, " ");
	// char** instArgs = strtokSplit(inst, " ");	// TODO: replace above
	int iLen=0;		// Length of arguments
	while (instArgs[iLen][0] != -1) {
		// Remove ',' from last character
		if (instArgs[iLen][strlen(instArgs[iLen])-1] == ',') {
			instArgs[iLen][strlen(instArgs[iLen])-1] = '\0';
		}
		printf("%s\n", instArgs[iLen]);
		iLen++;
	}
	if (iLen == 0) {
		return -1;
	}


	int instHash = instructionHash(instArgs[0]);
	switch (instHash) {
		// case 0xf31:		// ADCS
		// 	{
		// 		if (iLen != 4)
		// 			return -1;

		// 		uint32_t* regs[3];	// Rd, Rn, Rm
		// 		uint32_t *rd = getRegPtr(instArgs[1]);
		// 		uint32_t *rn = getRegPtr(instArgs[2]);
		// 		uint32_t *rm = getRegPtr(instArgs[3]);

		// 		// Rd = Rn + Rm + C (Conditional Carry flag in APSR)
		// 		*rd = *rn + *rm + ((pcReg.PSR.APSR >> 29) & 1);
		// 		if (pcReg.PSR.APSR) {				// If carry flag set
		// 			pcReg.PSR.APSR &= ~(1 << 29);	// Reset carry flag
		// 		}

		// 		// Cast as long to obtain actual value
		// 		long lrd = ((long) *rn) + ((long) *rm);
		// 		if (lrd >= (1L << 32)) {				// If rd is bigger
		// 			pcReg.PSR.APSR |= (1 << 29);	// Set the carry flag
		// 		}

		// 		// TODO: update N,Z,C,V flags
		// 	}
		// 	break;


		// 3.4 		Memory Access Instructions

		// ========
		// 3.4.1
		// ========
		case 0xbe6:     // ADR
			{
				// Generates a PC relative address
				uint32_t *rd = getRegPtr(instArgs[1]);
				char* label = instArgs[2];

				// TODO

				cycle++;
			}
			break;


		// ========
		// 3.4.2-4?
		// ========
		case 0xea6:     // LDR
			{
				// Load; immediate offset, register offset, PC-relative address
				uint32_t *rt = getRegPtr(instArgs[1]);
				uint32_t *rn = getRegPtr(instArgs[2]+1);	// +1 to remove '['
				uint32_t *rm = getRegPtr(instArgs[3]);
			}
			break;

		case 0x598:     // LDRB
			break;

		case 0x59e:     // LDRH
			break;

		case 0xdb2:     // LDRSB
			break;

		case 0xdb8:     // LDRSH
			break;

		case 0x0e6:     // STR
			break;
		case 0x798:     // STRB
			break;
		case 0x79e:     // STRH
			break;


		// ========
		// 3.4.5
		// ========
		case 0xea1:     // LDM
			{
				// Load Multiple Registers
			}
			break;
		case 0x0e1:     // STM
			break;


		// ========
		// 3.4.6
		// ========
		case 0xffc:     // POP
			break;
		case 0x1e6:     // PUSH
			break;


		// ========
		// 	3.5.1
		// ========
		case 0xf31:		// ADCS		{Rd,} Rn, Rm
			{
				// Add with carry
				uint32_t *rd = getRegPtr(instArgs[1]);
				uint32_t *rn = getRegPtr(instArgs[iLen-2]);
				uint32_t *rm = getRegPtr(instArgs[iLen-1]);

				// Add another one if flag is set
				int carrySet = getFlag('C');
				*rd = *rn + *rm + carrySet;

				// Update condition flags
				updateFlag('N', (int32_t)*rd<0);
				updateFlag('Z', *rd==0);
				updateFlag('C', *rn + *rm + carrySet > 0xFFFFFFFF);
				updateFlag(
					'V',
					(
						// Adding 2 positives results in negative value
						((int32_t)*rd < 0 && (int32_t)*rn > 0 && (int32_t)*rm > 0) ||
						// Adding 2 negatives results in positive value
						((int32_t)*rd > 0 && (int32_t)*rn < 0 && (int32_t)*rm < 0)
					)
				);
				// if (
				// 	((int32_t)*rd < 0 && (int32_t)*rn > 0 && (int32_t)*rm > 0) ||
				// 	((int32_t)*rd > 0 && (int32_t)*rn < 0 && (int32_t)*rm < 0)
				// 	)
				// 	updateFlag('V', 1);
				// else
				// 	updateFlag('V', 0);
				/*	For subtract
				if (
					((int32_t)*rd > 0 && (int32_t)*rn < 0 && (int32_t)*rm > 0) ||
					((int32_t)*rd < 0 && (int32_t)*rn > 0 && (int32_t)*rm < 0)
					)
					updateFlag('V', 1);
				else
					updateFlag('V', 0);
				*/

				cycle++;
			}
			break;
		case 0xbd8:     // ADD		{Rd,} Rn, <Rm|#imm>
		case 0xf39:     // ADDS		Same as ADD but updates N,Z,C,V flags
			{
				uint32_t *rd = getRegPtr(instArgs[1]);
				uint32_t *rn = getRegPtr(instArgs[iLen-2]);
				uint32_t *rm = getRegPtr(instArgs[iLen-1]);

				if (*rm == NULL) {		// Immediate value
					*rd = *rn + strtoint(instArgs[iLen-1]+1);	// +1 to remove '#'
				}
				else {
					*rd = *rn + *rm;
				}
				// Only update condition flags for ADDS
				if (instHash == 0xf39) {
					updateFlag('N', (int32_t)*rd<0);
					updateFlag('Z', *rd==0);
					updateFlag('C', *rn + *rm > 0xFFFFFFFF);
					updateFlag(
						'V',
						(
							// Adding 2 positives results in negative value
							((int32_t)*rd < 0 && (int32_t)*rn > 0 && (int32_t)*rm > 0) ||
							// Adding 2 negatives results in positive value
							((int32_t)*rd > 0 && (int32_t)*rn < 0 && (int32_t)*rm < 0)
						)
					);
				}

				cycle++;
				// TODO: If Rd is PC move cycle is 2
			}
			break;
		case 0x4e9:     // RSBS		{Rd,} Rn, Rm, #0
			{
				// TODO Uncertain what RSBS does
				// Reverse Subtract
				uint32_t *rd = getRegPtr(instArgs[1]);
				uint32_t *rn = getRegPtr(instArgs[iLen-3]);
				uint32_t *rm = getRegPtr(instArgs[iLen-2]);

				*rd = *rm - *rn;

				// Update condition flags
				updateFlag('N', (int32_t)*rd<0);
				updateFlag('Z', *rd==0);
				updateFlag('C', *rn + *rm > 0xFFFFFFFF);
				updateFlag(
					'V',
					(
						// Adding 2 positives results in negative value
						((int32_t)*rd < 0 && (int32_t)*rn > 0 && (int32_t)*rm > 0) ||
						// Adding 2 negatives results in positive value
						((int32_t)*rd > 0 && (int32_t)*rn < 0 && (int32_t)*rm < 0)
					)
				);

				cycle++;
			}
			break;
		case 0x2b1:     // SBCS		{Rd,} Rn, Rm
			{
				// Subtract with carry
				uint32_t *rd = getRegPtr(instArgs[1]);
				uint32_t *rn = getRegPtr(instArgs[iLen-2]);
				uint32_t *rm = getRegPtr(instArgs[iLen-1]);

				// Add another one if flag is set
				int carrySet = getFlag('C');
				*rd = *rn + *rm - carrySet;

				// Update condition flags
				updateFlag('N', (int32_t)*rd<0);
				updateFlag('Z', *rd==0);
				updateFlag('C', *rn < *rm);
				updateFlag(
					'V',
					(
						// Subtracting negative from positive results in negative value
						((int32_t)*rd < 0 && (int32_t)*rn > 0 && (int32_t)*rm < 0) ||
						// Subtracting positive from negative results in positive value
						((int32_t)*rd > 0 && (int32_t)*rn < 0 && (int32_t)*rm > 0)
					)
				);
				cycle++;
			}
			break;
		case 0x0de:     // SUB		{Rd,} Rn, <Rm|#imm>
		case 0x769:     // SUBS		Same as SUB but updates N,Z,C,V flags
			{
				// Subtract
				uint32_t *rd = getRegPtr(instArgs[1]);
				uint32_t *rn = getRegPtr(instArgs[iLen-2]);
				uint32_t *rm = getRegPtr(instArgs[iLen-1]);
				int imm;

				if (*rm == NULL) {		// Immediate value
					imm = strtoint(instArgs[iLen-1]+1);		// +1 to remove '#'
				}
				else {
					imm = *rm;
				}
				*rd = *rn - imm;

				// Only update condition flags for SUBS
				if (instHash == 0x769) {
					// Update condition flags
					updateFlag('N', (int32_t)*rd<0);
					updateFlag('Z', *rd==0);
					updateFlag('C', *rn < imm);
					updateFlag(
						'V',
						(
							// Subtracting negative from positive results in negative value
							((int32_t)*rd < 0 && (int32_t)*rn > 0 && (int32_t)*rm < 0) ||
							// Subtracting positive from negative results in positive value
							((int32_t)*rd > 0 && (int32_t)*rn < 0 && (int32_t)*rm > 0)
						)
					);
				}
				cycle++;
			}
			break;

		// ========
		// 3.5.2
		// ========
		// N & Z flags are updated, C & V are unaffected
		case 0x1b9:     // ANDS		{Rd,} Rn, Rm
			{
				// Bitwise AND
				uint32_t *rd = getRegPtr(instArgs[1]);
				uint32_t *rn = getRegPtr(instArgs[iLen-2]);
				uint32_t *rm = getRegPtr(instArgs[iLen-1]);

				*rd = *rm & *rn;

				updateFlag('N', (int32_t)*rd<0);
				updateFlag('Z', *rd==0);

				cycle++;
			}
			break;
		case 0xf29:     // ORRS		{Rd,} Rn, Rm
			{
				// Inclusive OR
				uint32_t *rd = getRegPtr(instArgs[1]);
				uint32_t *rn = getRegPtr(instArgs[iLen-2]);
				uint32_t *rm = getRegPtr(instArgs[iLen-1]);

				*rd = *rn | *rm;

				updateFlag('N', (int32_t)*rd<0);
				updateFlag('Z', *rd==0);

				cycle++;
			}
			break;
		case 0xa69:     // EORS		{Rd,} Rn, Rm
			{
				// Exclusive OR
				uint32_t *rd = getRegPtr(instArgs[1]);
				uint32_t *rn = getRegPtr(instArgs[iLen-2]);
				uint32_t *rm = getRegPtr(instArgs[iLen-1]);

				*rd = *rn ^ *rm;

				updateFlag('N', (int32_t)*rd<0);
				updateFlag('Z', *rd==0);

				cycle++;
			}
			break;
		case 0x271:     // BICS		{Rd,} Rn, Rm
			{
				// Bit clear
				uint32_t *rd = getRegPtr(instArgs[1]);
				uint32_t *rn = getRegPtr(instArgs[iLen-2]);
				uint32_t *rm = getRegPtr(instArgs[iLen-1]);

				*rd = *rn & (~*rm);

				updateFlag('N', (int32_t)*rd<0);
				updateFlag('Z', *rd==0);

				cycle++;
			}
			break;

		// ========
		// 3.5.3
		// ========
		// N & Z flags are updated, C is updated to last bit shifted out, V is unaffected
		case 0x369:     // ASRS		{Rd,} Rm, <Rs|#imm>
			{
				// Arithmetic Shift Right
				uint32_t *rd = getRegPtr(instArgs[1]);
				uint32_t *rm = getRegPtr(instArgs[iLen-2]);
				// Rs / Immediate
				uint32_t shiftLength = *getRegPtr(instArgs[iLen-1]);
				if (shiftLength == NULL) {
					shiftLength = strtoint(instArgs[iLen-1]+1);
				}

				int msb = getBit(*rm, 31);				// Most Significant Bit
				updateFlag('C', getBit(*rm, shiftLength-1));	// Last Shifted Out Bit

				*rd = *rm >> shiftLength;
				if (msb)	// If most significant bit is set
					*rd |= (uint32_t)(pow(2, (shiftLength)) - 1) << (32-shiftLength);


				// Update condition flags
				updateFlag('N', (int32_t)*rd<0);
				updateFlag('Z', *rd==0);

				cycle++;
			}
			break;
		case 0x939:     // LSLS
			{
				// Logical Shift Left
				uint32_t *rd = getRegPtr(instArgs[1]);
				uint32_t *rm = getRegPtr(instArgs[iLen-2]);
				// Rs / Immediate
				uint32_t shiftLength = *getRegPtr(instArgs[iLen-1]);
				if (shiftLength == NULL) {
					shiftLength = strtoint(instArgs[iLen-1]+1);
				}

				updateFlag('C', getBit(*rm, 32-shiftLength));	// Last Shifted Out Bit

				*rd = *rm << shiftLength;

				// Update condition flags
				updateFlag('N', (int32_t)*rd<0);
				updateFlag('Z', *rd==0);

				cycle++;
			}
			break;
		case 0x969:     // LSRS
			{
				// Logical Shift Right
				uint32_t *rd = getRegPtr(instArgs[1]);
				uint32_t *rm = getRegPtr(instArgs[iLen-2]);
				// Rs / Immediate
				uint32_t shiftLength = *getRegPtr(instArgs[iLen-1]);
				if (shiftLength == NULL) {
					shiftLength = strtoint(instArgs[iLen-1]+1);
				}

				updateFlag('C', getBit(*rm, shiftLength-1));	// Last Shifted Out Bit

				*rd = *rm >> shiftLength;

				// Update condition flags
				updateFlag('N', (int32_t)*rd<0);
				updateFlag('Z', *rd==0);

				cycle++;
			}
			break;
		case 0x469:     // RORS
			{
				// Right Rotation
				uint32_t *rd = getRegPtr(instArgs[1]);
				uint32_t *rm = getRegPtr(instArgs[iLen-2]);
				// Rs / Immediate
				uint32_t shiftLength = *getRegPtr(instArgs[iLen-1]);
				if (shiftLength == NULL) {
					shiftLength = strtoint(instArgs[iLen-1]+1);
				}

				if (shiftLength != 0 && shiftLength % 32 == 0)
					shiftLength = 32;
				else if (shiftLength > 32) {
					shiftLength %= 32;
				}

				updateFlag('C', getBit(*rm, shiftLength-1));	// Last Shifted Out Bit

				*rd = (*rm >> shiftLength) | (*rm << (32-shiftLength));

				// Update condition flags
				updateFlag('N', (int32_t)*rd<0);
				updateFlag('Z', *rd==0);

				cycle++;
			}
			break;

		// ========
		// 3.5.4
		// ========
		case 0xcac:     // CMP		Rn, <Rm|#imm>
			{
				// Compare
				// Same as SUBS but discard values
				uint32_t *rn = getRegPtr(instArgs[1]);
				uint32_t *rm = getRegPtr(instArgs[2]);

				uint32_t result = *rn + *rm;

				int imm;
				if (*rm == NULL) {		// Immediate value
					imm = strtoint(instArgs[iLen-1]+1);		// +1 to remove '#'
				}
				else {
					imm = *rm;
				}
				result = *rn - imm;

				// Update condition flags
				updateFlag('N', (int32_t)result<0);
				updateFlag('Z', result==0);
				updateFlag('C', *rn < imm);
				updateFlag(
					'V',
					(
						// Subtracting negative from positive results in negative value
						((int32_t)result < 0 && (int32_t)*rn > 0 && (int32_t)*rm < 0) ||
						// Subtracting positive from negative results in positive value
						((int32_t)result > 0 && (int32_t)*rn < 0 && (int32_t)*rm > 0)
					)
				);

				cycle++;
			}
			break;
		case 0xcaa:     // CMN		Rn, Rm
			{
				// Compare Negative
				// Same as ADDS but discard values
				uint32_t *rn = getRegPtr(instArgs[1]);
				uint32_t *rm = getRegPtr(instArgs[2]);

				uint32_t result = *rn + *rm;

				updateFlag('N', (int32_t)result<0);
				updateFlag('Z', result==0);
				updateFlag('C', *rn + *rm > 0xFFFFFFFF);
				updateFlag(
					'V',
					(
						// Adding 2 positives results in negative value
						((int32_t)result < 0 && (int32_t)*rn > 0 && (int32_t)*rm > 0) ||
						// Adding 2 negatives results in positive value
						((int32_t)result > 0 && (int32_t)*rn < 0 && (int32_t)*rm < 0)
					)
				);

				cycle++;
			}
			break;

		// ========
		// 3.5.5
		// ========
		// TODO: Handle when Rd is PC in a MOV instruction
		case 0xf42:     // MOV		Rd, Rm
			{
				// Move
				uint32_t *rd = getRegPtr(instArgs[1]);
				uint32_t *rm = getRegPtr(instArgs[2]);

				*rd = *rm;


				cycle++;
				// TODO: If Rd is PC move cycle is 2
			}
			break;
		case 0xa89:     // MOVS		Rd, <Rm|#imm>
			{
				// Same as move but Rm maybe immediate and update N,Z flags
				uint32_t *rd = getRegPtr(instArgs[1]);
				uint32_t *rm = getRegPtr(instArgs[2]);

				if (*rm == NULL) {		// Immediate
					*rd = strtoint(instArgs[2]+1);		// +1 to remove '#'
				}
				else {
					*rd = *rm;
				}

				// Only update condition flags for MOVS
				if (instHash == 0xa89) {
					updateFlag('N', (int32_t)*rd<0);
					updateFlag('Z', *rd==0);
				}

				cycle++;
			}
			break;
		case 0xc09:     // MVNS
			{
				// Move NOT
				uint32_t *rd = getRegPtr(instArgs[1]);
				uint32_t *rm = getRegPtr(instArgs[2]);

				*rd = ~*rm;

				// Updates N,Z flags only
				updateFlag('N', (int32_t)*rd<0);
				updateFlag('Z', *rd==0);

				cycle++;
			}
			break;

		// ========
		// 3.5.6
		// ========
		case 0xbb9:     // MULS		Rd, Rn, Rm
			{
				uint32_t *rd = getRegPtr(instArgs[1]);
				uint32_t *rn = getRegPtr(instArgs[2]);
				uint32_t *rm = getRegPtr(instArgs[3]);

				*rd = *rn * *rm;

				// Updates N,Z flags only
				updateFlag('N', (int32_t)*rd<0);
				updateFlag('Z', *rd==0);
			}
			break;

		// ========
		// 3.5.7		Reverse Bytes
		// ========
		case 0x032:     // REV		Rd, Rn
			{
				// 0x12345678 -> 0x78563412
				uint32_t *rd = getRegPtr(instArgs[1]);
				uint32_t *rn = getRegPtr(instArgs[2]);

				*rd = 0;
				for (int i=0; i<4; i++)
					*rd |= ((*rn >> i*8) & 0xFF) << (3-i)*8;

				cycle++;
			}
			break;
		case 0xe76:     // REV16	Rd, Rn
			{
				// 0x12345678 -> 0x34127856
				uint32_t *rd = getRegPtr(instArgs[1]);
				uint32_t *rn = getRegPtr(instArgs[2]);

				*rd = 0;
				*rd |= (*rn >> 24) << 16;
				*rd |= ((*rn >> 16) & 0xFF) << 24;
				*rd |= (*rn >> 8) & 0xFF;
				*rd |= (*rn & 0xFF) << 8;

				cycle++;
			}
			break;
		case 0x0b8:     // REVSH	Rd, Rn
			{
				// 0x12345678 -> 0x00007856
				uint32_t *rd = getRegPtr(instArgs[1]);
				uint32_t *rn = getRegPtr(instArgs[2]);

				*rd = 0;
				*rd |= (*rn >> 8) & 0xFF;
				*rd |= (*rn & 0xFF) << 8;

				// TODO: signed bit

				cycle++;
			}
			break;

		// ========
		// 3.5.8
		// ========
		case 0x8a8:     // SXTB		Rd, Rm
			{
				// Signed Extend Byte
				uint32_t *rd = getRegPtr(instArgs[1]);
				uint32_t *rm = getRegPtr(instArgs[2]);

				*rd = *rm & 0xFF;
				if ((int8_t)*rd < 0)
					*rd |= 0xFFFFFF00;

				cycle++;
			}
			break;
		case 0x8ae:     // SXTH		Rd, Rm
			{
				// Signed Extend Halfword
				uint32_t *rd = getRegPtr(instArgs[1]);
				uint32_t *rm = getRegPtr(instArgs[2]);

				*rd = *rm & 0xFFFF;
				if ((int16_t)*rd < 0)
					*rd |= 0xFFFF0000;

				cycle++;
			}
			break;
		case 0xca8:     // UXTB		Rd, Rm
			{
				// Unsigned Extend Byte
				uint32_t *rd = getRegPtr(instArgs[1]);
				uint32_t *rm = getRegPtr(instArgs[2]);

				*rd = *rm & 0xFF;

				cycle++;
			}
			break;
		case 0xcae:     // UXTH		Rd, Rm
			{
				// Unsigned Extend Halfword
				uint32_t *rd = getRegPtr(instArgs[1]);
				uint32_t *rm = getRegPtr(instArgs[2]);

				*rd = *rm & 0xFFFF;

				cycle++;
			}
			break;
		
		// ========
		// 3.5.9
		// ========
		case 0x120:     // TST		Rn, Rm
			{
				// Same as ANDS but discards result
				uint32_t *rn = getRegPtr(instArgs[iLen-2]);
				uint32_t *rm = getRegPtr(instArgs[iLen-1]);

				uint32_t result = *rm & *rn;

				updateFlag('N', (int32_t)result<0);
				updateFlag('Z', result==0);

				cycle++;
			}
			break;


		// ========
		// 3.6.1
		// ========
		case 0x062:     // B
			break;
		case 0x37e:     // BL
			{
				// Branch with link call to function
			}
			break;
		case 0x38a:     // BX
			break;
		case 0xc6c:     // BLX
			break;

		// ========
		// 3.7.1
		// ========
		case 0x35a:     // BKPT
			{
				// Breakpoint
			}
			break;

		// ========
		// 3.7.2		Change Processor State
		// ========
		case 0x5a4:     // CPSID
			{
				// TODO check if current execution mode is privileged
				// Change Processor State Interrupt Disable
				pcReg.PRIMASK |= 1;
			}
			break;
		case 0x5a5:     // CPSIE
			{
				// TODO check if current execution mode is privileged
				// Change Processor State Interrupt Enable
				pcReg.PRIMASK &= ~1;
			}
			break;

		// ========
		// 3.7.3
		// ========
		case 0xcde:     // DMB
			{
				// Data Memory Barrier
			}
			break;

		// ========
		// 3.7.4
		// ========
		case 0xd0e:     // DSB
			{
				// Data Synchronization Barrier
			}
			break;

		// ========
		// 3.7.5
		// ========
		case 0xe4e:     // ISB
			{
				// Instruction Synchronization Barrier
			}
			break;

		// ========
		// 3.7.6
		// ========
		case 0xf57:     // MRS
			{
				uint32_t *rd = getRegPtr(instArgs[1]);
				uint32_t *spec_reg = getRegPtr(instArgs[2]);

				// TODO: Check if current execution mode is privieged; if not all values other then ASPR is 0
			}
			break;

		// ========
		// 3.7.7
		// ========
		case 0xf5e:     // MSR
			break;

		// ========
		// 3.7.8
		// ========
		case 0xf7c:     // NOP
			// Does nothing
			break;

		// ========
		// 3.7.9
		// ========
		case 0x072:     // SEV
			{
				// Send Event
			}
			break;

		// ========
		// 3.7.9
		// ========
		case 0x0e7:     // SVC
			{
				// Supervisor Call
			}
			break;









		case 0x169:     // WFE
			break;
		case 0x16d:     // WFI
			break;
		case 0xc2d:     // BEQ
			break;
		case 0xc69:     // BNE
			break;
		case 0xc1f:     // BCS
			break;
		case 0xc47:     // BHS
			break;
		case 0xc0f:     // BCC
			break;
		case 0xc63:     // BLO
			break;
		case 0xc65:     // BMI
			break;
		case 0xc80:     // BPL
			break;
		case 0xcb7:     // BVS
			break;
		case 0xca7:     // BVC
			break;
		case 0xc3d:     // BHI
			break;
		case 0xc67:     // BLS
			break;
		case 0xc31:     // BGE
			break;
		case 0xc68:     // BLT
			break;
		case 0xc40:     // BGT
			break;
		case 0xc59:     // BLE
			break;
		case 0xc08:     // BAL
			break;
	}

	return 0;
}
