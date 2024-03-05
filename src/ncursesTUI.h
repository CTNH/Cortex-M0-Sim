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
			flags
		};

	private:
		// Min size of the application
		const int minWidth = 40;
		const int minHeight = 18;

		const int memWinWordPerLine = 6;

		CM0P_Core* core;

		// Avaliable max size of the application
		int winWidth, winHeight;

		int memWinPos = 0;
		int memWinMaxPos = 0;	// Calculated in createMemoryWin
		int memWinCurX = 0, memWinCurY = 1;

		WINDOW *helpWin;		// Help menu
		WINDOW *memoryWin;
		WINDOW *opcodeWin;
		WINDOW *statusWin;		// Keybinds / current window
		WINDOW *registerWin;	// Register values
		WINDOW *flagsWin;
		WINDOW *labelsWin;

		// Array of text in each left and right window
		string *leftWinTxt, *rightWinText;

		void resizeWin(int foo);
		void createStatusWin();
		void createRegisterWin();
		void createMemoryWin();
		void createFlagsWin();
		void createLabelsWin(unordered_map<string, uint32_t> labels);
		void updateStatusWin();

		winId selectedWin;
		WINDOW* getWin(winId id);

	public:
		// Constructor
		ApplicationTUI(CM0P_Core* core, unordered_map<string, uint32_t> labels);

		void updateRegisterWin();
		void updateMemoryWin();
		void updateFlagsWin();

		// Redraw cursor on screen
		void removeMemoryWinCursor();
		void drawMemoryWinCursor();
		// Move the cursor up or down
		void updateMemoryWinCursorVertical(int lines);
		void updateMemoryWinCursorHorizontal(int cols);

		// Get key press given window
		int getWinCh(winId id);

		// Move cursor position
		void setMemWinCurY(string position);

		void selectWin(winId id);

		// Cleanup; only called before delete
		void clean();

		// Creates prompt to check for exit
		bool confirmExit();
};

#endif
