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

	ApplicationTUI appTui(core.getMemPtr());

	ApplicationTUI::winId currWin = appTui.memory;
	appTui.selectWin(currWin);
	bool loop = true;
	bool winLoop = true;
	
	while(loop) {
		switch(currWin) {
			case ApplicationTUI::help:
				break;
			case ApplicationTUI::memory:
				winLoop = true;
				while(winLoop) {
					switch(appTui.getWinCh(currWin)) {
						case 'q':
							if (appTui.confirmExit()) {
								loop = false;
								winLoop = false;
							}
							break;
						// MemWin_lft
						case KEY_LEFT:
						case 'h':
							if (appTui.memWinCurX > 0) {
								appTui.memWinCurX -= 1;
								appTui.refreshMemoryWinCursor();
							}
							break;
						// MemWin_rgt
						case KEY_RIGHT:
						case 'l':
							appTui.memWinCurX += 1;
							appTui.refreshMemoryWinCursor();
							break;
						// MemWin_dn
						case KEY_DOWN:
						case 'j':
							appTui.updateMemoryWinCursor(1);
							break;
						// MemWin_up
						case KEY_UP:
						case 'k':
							appTui.updateMemoryWinCursor(-1);
							break;
						// MemWin_btm
						case 'L':
							appTui.setMemWinCurY("viewbtm");
							break;
						// MemWin_top
						case 'H':
							appTui.setMemWinCurY("viewtop");
							break;
						// MemWin_PgDn
						case KEY_NPAGE:
						case 'D':
							appTui.updateMemoryWinCursor(20);
							break;
						// MemWin_PgUp
						case KEY_PPAGE:
						case 'U':
							appTui.updateMemoryWinCursor(-20);
							break;
						case 'g':
							appTui.setMemWinCurY("top");
							break;
						case 'G':
							appTui.setMemWinCurY("btm");
							break;
						case '\t':
							currWin = ApplicationTUI::registers;
							appTui.selectWin(ApplicationTUI::registers);
							winLoop = false;
							break;
						default:
							break;
					}
				}
				break;
			case ApplicationTUI::opcode:
				break;
			case ApplicationTUI::status:
				break;
			case ApplicationTUI::registers:
				winLoop = true;
				while (winLoop) {
					switch(appTui.getWinCh(currWin)) {
						case 'q':
							if (appTui.confirmExit()) {
								loop = false;
								winLoop = false;
							}
							break;
						case '\t':
							currWin = ApplicationTUI::memory;
							appTui.selectWin(ApplicationTUI::memory);
							winLoop = false;
							break;
						default:
							break;
					}
				}
				break;
		}
	}

	appTui.clean();
	return 0;
}
