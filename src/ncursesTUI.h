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

		WINDOW *helpWin;		// Help menu
		WINDOW *memoryWin;
		WINDOW *opcodeWin;
		WINDOW *statusWin;		// Keybinds / current window
		WINDOW *registerWin;	// Register values


		// Array of text in each left and right window
		string *leftWinTxt, *rightWinText;

		void resizeWin(int foo);
		void createStatusWin();
		void createRegisterWin();
		void createMemoryWin();

	public:
		enum winId {
			help,
			memory,
			opcode,
			status,
			registers
		};
		int memWinCurX = 14, memWinCurY = 1;

		// Constructor
		ApplicationTUI(CM0P_Memory* memPtr);

		void updateRegisterWin(int reg, uint32_t value);
		void updateMemoryWin();

		// Redraw cursor on screen
		void refreshMemoryWinCursor();
		// Move the cursor up or down
		void updateMemoryWinCursor(int lines);

		// Get key press given window
		int getWinCh(winId id);

		// Setter
		void setCoreMem(CM0P_Memory* coreMem);
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
