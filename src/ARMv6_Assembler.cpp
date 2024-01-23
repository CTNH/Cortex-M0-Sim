#include "ARMv6_Assembler.h"
#include <ctype.h>			// For toupper() / tolower()
#include <bits/stdc++.h>	// For sort()


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

uint16_t ARMv6_Assembler::genOpcode(string instruction) {
	switch (djb2Hash(instruction)) {
		case 0xde60:
			break;
		case 0xd06e:
			break;
		case 0xde81:
			break;
		case 0xd07c:
			break;
		case 0x090b:
			break;
		case 0x201e:
			break;
		case 0xb5e7:
			break;
		case 0xd4dd:
			break;
		case 0xd5fa:
			break;
		case 0xd49d:
			break;
		case 0xd542:
			break;
		case 0xd48d:
			break;
		case 0xd5c2:
			break;
		case 0xd5dd:
			break;
		case 0xd643:
			break;
		case 0xd710:
			break;
		case 0xd700:
			break;
		case 0xd538:
			break;
		case 0xd5c6:
			break;
		case 0xd513:
			break;
		case 0xd5c7:
			break;
		case 0xd522:
			break;
		case 0xd5b8:
			break;
		case 0xd454:
			break;
		case 0x8006:
			break;
		case 0x8a36:
			break;
		case 0x7313:
			break;
		case 0xd5cb:
			break;
		case 0x731f:
			break;
		case 0xda23:
			break;
		case 0xda25:
			break;
		case 0xb2f8:
			break;
		case 0xb2f9:
			break;
		case 0xde58:
			break;
		case 0xdf1e:
			break;
		case 0x409e:
			break;
		case 0xf463:
			break;
		case 0xff42:
			break;
		case 0xff47:
			break;
		case 0xe869:
			break;
		case 0xe86f:
			break;
		case 0xf7fc:
			break;
		case 0xf802:
			break;
		case 0x2783:
			break;
		case 0x2849:
			break;
		case 0x04f7:
			break;
		case 0xa42a:
			break;
		case 0x0557:
			break;
		case 0x0577:
			break;
		case 0xbc66:
			break;
		case 0xc0e9:
			break;
		case 0x0932:
			break;
		case 0xc92b:
			break;
		case 0x11b4:
			break;
		case 0x6265:
			break;
		case 0x18f2:
			break;
		case 0x23f9:
			break;
		case 0x286d:
			break;
		case 0x618b:
			break;
		case 0x707f:
			break;
		case 0xb4b0:
			break;
		case 0x1d33:
			break;
		case 0x1f19:
			break;
		case 0x1f1e:
			break;
		case 0x0320:
			break;
		case 0x0326:
			break;
		case 0x1f2f:
			break;
		case 0x0562:
			break;
		case 0x1f51:
			break;
		case 0x1466:
			break;
		case 0x146c:
			break;
		case 0x2340:
			break;
		case 0x2d28:
			break;
		case 0x2d2e:
			break;
		case 0x2e47:
			break;
		case 0x2e4b:
			break;
	}
	return 0;
}

