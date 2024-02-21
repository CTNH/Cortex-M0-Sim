#include <iostream>
#include "ARMv6_Assembler.h"
#include "basiclib_string.h"

int main (int argc, char *argv[]) {
	ARMv6_Assembler assembler("main.c.s");
	// assembler.hashUniqueCheck();
	cout << endl << endl;
	assembler.getFinalResult();

	/*
	string inst = "ADDS 5,1, #7";
	char** instArgs = strtokSplit(&inst[0], (char*)" ,");

	for (int i=0; i<strArrLen(instArgs); i++) {
		cout << instArgs[i] << "-";
	}
	cout << endl;

	ARMv6_Assembler::OpcodeResult opcode = assembler.genOpcode(instArgs);
	printf("%016b\n", opcode.opcode);
	
	return 0;
	*/
}
