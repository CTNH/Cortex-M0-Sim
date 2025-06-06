#ifndef APP_TUI_H
#define APP_TUI_H

#include "cortex-m0p_memory.h"
#include "ncurses.h"
#include "cortex-m0p_core.h"
#include <string>
// #include "form.h"
using namespace std;

class ApplicationTUI {
	public:
		enum winId {
			help,
			memory,
			opcode,
			status,
			registers,
			flags,
			labels,
			assembly
		};

	private:
		// Min size of the application
		const int minWidth = 40;
		const int minHeight = 18;

		int memWinWordPerLine;

		CM0P_Core* core;

		// Avaliable max size of the application
		int winWidth, winHeight;

		int memWinPos = 0;
		int memWinMaxPos = 0;	// Calculated in createMemoryWin
		int memWinCurX = 0, memWinCurY = 1;
		int regWinCur = 16;

		WINDOW *helpWin;		// Help menu
		WINDOW *memoryWin;
		WINDOW *opcodeWin;
		WINDOW *statusWin;		// Keybinds / current window
		WINDOW *registerWin;	// Register values
		WINDOW *flagsWin;
		WINDOW *labelsWin;
		WINDOW *asmWin;

		// Array of text in each left and right window
		string *leftWinTxt, *rightWinText;

		vector<string> asmSrc;

		void resizeWin(int foo);
		void createStatusWin(string msg);
		void createRegisterWin();
		void createMemoryWin();
		void createFlagsWin();
		void createLabelsWin(unordered_map<string, uint32_t> labels);
		void createASMWin(vector<pair<string, ARMv6_Assembler::OpcodeResult>> asmResults);
		void updateStatusWin();

		winId selectedWin;
		WINDOW* getWin(winId id);
		// Get status bar message for given window
		string getWinStat(winId id);

		// Input type: 0 for all, 1 for hex only, 2 for num only
		string statusWinInputPrompt(string prompt, int inType);

		void removeMemoryWinCursor();
		void drawMemoryWinCursor();

	public:
		// Constructor
		ApplicationTUI(CM0P_Core* core, unordered_map<string, uint32_t> labels, vector<pair<string, ARMv6_Assembler::OpcodeResult>> asmResults);

		void updateRegisterWin();
		void updateMemoryWin();
		void updateFlagsWin();

		// Update cursor of memory window
		void updateMemoryWinCursorVertical(int lines);
		void updateMemoryWinCursorHorizontal(int cols);
		// Move cursor of memory window to presets
		void setPresetMemWinCur(int moveid);
		// Set cursor to address or from prompt
		void memWinGoto();
		void memWinGoto(uint32_t address);

		// Update cursor of register window
		void updateRegisterWinCursorVertical(int lines);

		// Get key press given window
		int getWinCh(winId id);
		// Update selected window and highlight
		void selectWin(winId id);
		// Change PC with user input
		void regWinChangeReg();

		// Cleanup; only called before delete
		void clean();

		// Creates prompt to check for exit
		bool confirmExit();
};

#endif
