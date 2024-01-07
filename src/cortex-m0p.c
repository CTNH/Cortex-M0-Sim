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
			}
			break;

		// ========
		// 3.5.3
		// ========
		// N & Z flags are updated, C is updated to last bit shifted out, V is unaffected
		case 0x369:     // ASRS		{Rd,} Rm, <Rs|#imm>
			{
				/*
				// Arithmetic Shift Right
				uint32_t *rd = getRegPtr(instArgs[1]);
				uint32_t *rm = getRegPtr(instArgs[iLen-2]);

				uint32_t *rs = getRegPtr(instArgs[iLen-1]);
				// Most significant bit
				int msb = (*rm >> 31) & 1;
				// Register holding shift length or immediate value
				if (*rs != NULL) {	// Register
					if ((*rd >> (*rs-1))
						updateFlag('C', 1);
					else
						updateFlag('C', 0);
					*rd = *rm >> *rs;
					if (msb)	// If most significant bit is 1
						*rd |= (uint32_t)(pow(2, (*rs)) - 1) << (32-*rs);
				}
				else {	// Immediate value
					int imm = strtoint(instArgs[iLen-1]+1);
					*rd = *rm >> imm;
					if (msb)	// If most significant bit is 1
						*rd |= (uint32_t)(pow(2, (imm)) - 1) << (32-imm);
				}

				// Update condition flags
				updateFlag('N', (int32_t)*rd<0);
				updateFlag('Z', *rd==0);
				// TODO: Update carry flag

				*/


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
			}
			break;
		case 0x969:     // LSRS
			{
				/*
				// Logical Shift Right
				uint32_t *rd = getRegPtr(instArgs[1]);
				uint32_t *rm;
				uint32_t *rs;
				if (iLen == 3) {	// Rd is omitted
					rm = getRegPtr(instArgs[1]);
					rs = getRegPtr(instArgs[2]);
				}
				else {
					rm = getRegPtr(instArgs[2]);
					rs = getRegPtr(instArgs[3]);
				}
				if (*rs == NULL) {		// Immediate Value
					instArgs[iLen-1] = instArgs[iLen-1]+1;	// Remove '#'
					uint32_t imm = strtoint(instArgs[iLen-1]);

					*rd = *rm >> imm;
				}
				else {
					*rd = *rm >> *rs;
				}

				updateFlag('N', (int32_t)*rd<0);
				updateFlag('Z', *rd==0);
				// TODO: Uodate C flag to last bit shifted out except when shift length is 0
				*/

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
			}
			break;





		case 0xbe6:     // ADR
			// Generates a PC relative address
			{
				uint32_t *rd = getRegPtr(instArgs[1]);
				char* label = instArgs[2];

				// TODO
			}
			break;
		case 0x062:     // B
			break;
		case 0x35a:     // BKPT
			{
				// Breakpoint
			}
			break;
		case 0x37e:     // BL
			{
				// Branch with link call to function
			}
			break;
		case 0xc6c:     // BLX
			break;
		case 0x38a:     // BX
			break;
		case 0xcaa:     // CMN
			{
				// Compare Negative
				uint32_t *rn = getRegPtr(instArgs[1]);
				uint32_t *rm = getRegPtr(instArgs[2]);

				// TODO
			}
			break;
		case 0xcac:     // CMP
			{
				// Compare
				uint32_t *rn = getRegPtr(instArgs[1]);
				uint32_t *rm = getRegPtr(instArgs[2]);

				// TODO
				// Immediate value
				if (*rm == NULL) {
				}
				else {}
			}
			break;
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
		case 0xcde:     // DMB
			{
				// Data Memory Barrier
			}
			break;
		case 0xd0e:     // DSB
			{
				// Data Synchronization Barrier
			}
			break;
		case 0xe4e:     // ISB
			{
				// Instruction Synchronization Barrier
			}
			break;
		case 0xea1:     // LDM
			{
				// Load Multiple Registers
			}
			break;
		case 0xea6:     // LDR
			break;
		case 0x598:     // LDRB
			break;
		case 0x59e:     // LDRH
			break;
		case 0xdb2:     // LDRSB
			break;
		case 0xdb8:     // LDRSH
			break;
		case 0xf42:     // MOV
			{
				// Move
				uint32_t *rd = getRegPtr(instArgs[1]);
				uint32_t *rm = getRegPtr(instArgs[2]);

				*rd = *rm;
			}
			break;
		case 0xa89:     // MOVS
			{
				// Same as move but update N,Z flags
			}
			break;
		case 0xf57:     // MRS
			break;
		case 0xf5e:     // MSR
			break;
		case 0xbb9:     // MULS
			break;
		case 0xc09:     // MVNS
			break;
		case 0xf7c:     // NOP
			break;
		case 0xffc:     // POP
			break;
		case 0x1e6:     // PUSH
			break;
		case 0x032:     // REV
			break;
		case 0xe76:     // REV16
			break;
		case 0x0b8:     // REVSH
			break;
		case 0x469:     // RORS
			break;
		case 0x072:     // SEV
			break;
		case 0x0e1:     // STM
			break;
		case 0x0e6:     // STR
			break;
		case 0x798:     // STRB
			break;
		case 0x79e:     // STRH
			break;
		case 0x0e7:     // SVC
			break;
		case 0x8a8:     // SXTB
			break;
		case 0x8ae:     // SXTH
			break;
		case 0x120:     // TST
			break;
		case 0xca8:     // UXTB
			break;
		case 0xcae:     // UXTH
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
