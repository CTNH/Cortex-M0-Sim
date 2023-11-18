#include "cortex-m0p_instructions.h"
#include <stdio.h>
#include <string.h>

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


int execInstruction(char* inst) {
	int instHash = instructionHash(inst);
	switch (instHash) {
		case 0xf31:		// ADCS
			break;
		case 0xbd8:     // ADD
			break;
		case 0xf39:     // ADDS
			break;
		case 0xbe6:     // ADR
			break;
		case 0x1b9:     // ANDS
			break;
		case 0x369:     // ASRS
			break;
		case 0x062:     // B
			break;
		case 0x271:     // BICS
			break;
		case 0x35a:     // BKPT
			break;
		case 0x37e:     // BL
			break;
		case 0xc6c:     // BLX
			break;
		case 0x38a:     // BX
			break;
		case 0xcaa:     // CMN
			break;
		case 0xcac:     // CMP
			break;
		case 0x5a4:     // CPSID
			break;
		case 0x5a5:     // CPSIE
			break;
		case 0xcde:     // DMB
			break;
		case 0xd0e:     // DSB
			break;
		case 0xa69:     // EORS
			break;
		case 0xe4e:     // ISB
			break;
		case 0xea1:     // LDM
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
		case 0x939:     // LSLS
			break;
		case 0x969:     // LSRS
			break;
		case 0xf42:     // MOV
			break;
		case 0xa89:     // MOVS
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
		case 0xf29:     // ORRS
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
		case 0x4e9:     // RSBS
			break;
		case 0x2b1:     // SBCS
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
		case 0x0de:     // SUB
			break;
		case 0x769:     // SUBS
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
