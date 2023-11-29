#include <stdio.h>
#include "basiclib_string.h"
#include "cortex-m0p_instructions.h"

int main() {
	printf("Hello world\n");

	char tmp[76][8] = {
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
	// char bCond[17][4] = {
	// 	"beq",
	// 	"bne"
	// };

	int hashes[76];
	for (int i = 0; i < 76; i++) {
		hashes[i] = instructionHash(tmp[i]);
		//printf("%2d %03x\n", i+1, hashes[i]);
		// printf("%03x\n", hashes[i]);
		for (int j=0; j<i; j++) {
			if (hashes[i] == hashes[j]) {
				// printf("!: %d %x\n", j, hashes[j]);
				break;
			}
		}
	}

	printf("\n");
	printf("%x\n", instructionHash("sub"));
	printf("ASDHAIDH\n");

	char** s = strSplit("Test sdfdhs", " o");
	// char** s = strtokSplit("Hello  World !", " o");
	int i = 0;
	while (s[i][0] != -1) {
		printf("%s\n", s[i]);
		i++;
	}

	return 0;
}

