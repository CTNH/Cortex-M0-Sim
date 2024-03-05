#ifndef APP_TUI_H
#define APP_TUI_H

#include "cortex-m0p_memory.h"
#include "ncurses.h"
#include "cortex-m0p_core.h"
#include <string>
// #include "form.h"
using namespace std;

class ApplicationTUI {
	private:
		CM0P_Memory* coreMem;

		// Avaliable max size of the application
		int winWidth, winHeight;
		// Min size of the application
		const static int minWidth = 40;
		const static int minHeight = 18;

		int memWinPos = 0;
		int memWinMaxPos = 0;
		int memWinWordPerLine = 6;
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

	public:
		enum winId {
			help,
			memory,
			opcode,
			status,
			registers,
			flags
		};

		// Constructor
		ApplicationTUI(CM0P_Memory* memPtr, unordered_map<string, uint32_t> labels);

		void updateRegisterWin(int reg, uint32_t value);
		void updateMemoryWin();
		void updateFlagsWin(char flag, bool value);

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
	private:
		winId selectedWin;
		WINDOW* getWin(winId id);
};

#endif
