#include <iostream>
#include "cortex-m0p_core.h"
#include "ARMv6_Assembler.h"
#include "ncursesTUI.h"
using namespace std;

int main (int argc, char *argv[]) {

	ARMv6_Assembler assembler("main.c.s");
	// assembler.hashUniqueCheck();
	cout << endl << endl;
	vector<pair<string, ARMv6_Assembler::OpcodeResult>> asmResults = assembler.getFinalResult();
	vector<ARMv6_Assembler::OpcodeResult> opcodes;
	for (auto &it: asmResults) {
		opcodes.push_back(it.second);
	}

	CM0P_Core core(opcodes, assembler.getStartAddr());
	for (auto &it: asmResults) {
		core.step_inst();
		// Use padding for 16 and 32 bit instructions
		if (it.second.i32)
			printf("%s\t0x%08x\n", it.first.c_str(), it.second.opcode);
		else
			printf("%s\t0x%04x\n", it.first.c_str(), it.second.opcode);
		uint32_t* coreRegs = core.getCoreRegisters();
		cout << coreRegs[4] << endl;
		// cout << "Press enter to continue.";
		// cin.ignore();
	}

	ApplicationTUI appTui;

	return 0;
}
