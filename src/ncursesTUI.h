#ifndef APP_TUI_H
#define APP_TUI_H

#include "ncurses.h"
#include <string>
// #include "form.h"
using namespace std;

class ApplicationTUI {
	private:
		// Avaliable max size of the application
		int winWidth, winHeight;
		// Min size of the application
		int minWidth = 40;
		int minHeight = 18;

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
		// Constructor
		ApplicationTUI();

		void updateReg(int reg, uint32_t value);
};

#endif
