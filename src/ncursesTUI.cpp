#include "ncursesTUI.h"
#include <ncurses.h>
#include <string>

// void ApplicationTUI::resizeWin(int foo) {}

ApplicationTUI::ApplicationTUI() {

	initscr();
	cbreak();
	noecho();
	keypad(stdscr, TRUE);

	// Get max window sizes of the application
	getmaxyx(stdscr, winHeight, winWidth);

	// Exit application if window size is too small
	if (winHeight < minHeight or winWidth < minWidth) {
		endwin();
		printf("Window size must be at least %dx%d!\n", minWidth, minHeight);
		return;
		// signal(SIGWINCH, resizeWin);
	}

	createStatusWin();
	createMemoryWin();
	createRegisterWin();
}

void ApplicationTUI::clean() {
	endwin();
}

int ApplicationTUI::getWinCh(int winId) {
	WINDOW* win;
	switch(winId) {
		case 1:
			win = memoryWin;
			break;
		default:
			return (char)NULL;
	}
	return wgetch(win);
}

void ApplicationTUI::updateRegisterWin(int reg, uint32_t value) {
	mvwprintw(registerWin, reg+1, 17, "0x%08x", value);
	wrefresh(registerWin);
}

void ApplicationTUI::createStatusWin() {
	statusWin = newwin(1, winWidth, winHeight-1, 0);
	string separator = "  |  ";
	string msg = " q: quit";
	// u8"\u25c0\u25bc\u25b2\u25b6" + 
	msg += separator + "h,j,k,l/arrow keys: navigate";
	msg += separator + "n: next instruction";
	msg += separator + "p: previous instruction";
	msg += separator + "g: goto address";
	// mvwprintw(statusWin, 0, 0, "q: quit");
	mvwprintw(statusWin, 0, 0, "%s", msg.c_str());
	wrefresh(statusWin);
}

void ApplicationTUI::createRegisterWin() {
	// Create window
	// registerWin = newwin(18, 29, winHeight/2, winWidth/2-1);
	registerWin = newwin(18, 29, winHeight-19, winWidth/2-1);

	wattron(registerWin, A_BOLD);
	wattron(registerWin, A_UNDERLINE);
	for (int i=0; i<16; i++) {
		mvwprintw(registerWin, i+1, 2, "Register %-2d   ", i);
	}
	mvwprintw(registerWin, 14, 11, "SP");
	mvwprintw(registerWin, 15, 11, "LR");
	mvwprintw(registerWin, 16, 11, "PC");
	wattroff(registerWin, A_UNDERLINE);
	wattroff(registerWin, A_BOLD);

	for (int i=0; i<16; i++) {
		updateRegisterWin(i, 0); 	// TODO: Take pre-existing values
	}

	// Draw window border
	box(registerWin, 0, 0);

	// Put on screen
	wrefresh(registerWin);
}

void ApplicationTUI::createMemoryWin() {
	memoryWin = newwin(winHeight-1, winWidth/2, 0, 0);
	box(memoryWin, 0, 0);
	keypad(memoryWin, TRUE);

	box(memoryWin, 0, 0);
}

void ApplicationTUI::setCoreMem(CM0P_Memory* coreMem) {
	this->coreMem = coreMem;
}

void ApplicationTUI::refreshMemoryWinCursor() {
	wattron(memoryWin, A_BOLD);
	wattron(memoryWin, A_STANDOUT);
	mvwprintw(memoryWin, memWinCurY, memWinCurX, "");
	wattroff(memoryWin, A_STANDOUT);
	wattroff(memoryWin, A_BOLD);
	wrefresh(memoryWin);
}

void ApplicationTUI::updateMemoryWinCursor(int lines) {
	// Move down
	if (lines > 0) {
		if (memWinCurY+lines < winHeight - 2) {
			memWinCurY += lines;
		}
		// Move memory window down
		else {
			// Do not allow scrolling past last address
			if (memWinPos+lines < (coreMem->getSize()/8 - (winHeight-3))) {
				memWinPos += lines;
			}
			else {
				memWinCurY = winHeight - 3;
				memWinPos = coreMem->getSize() / 8 - (winHeight-3);
			}
			updateMemoryWin();
		}
	}
	// Move Up
	else {
		// Only moves cursor
		if (memWinCurY+lines > 1) {
			memWinCurY += lines;
		}
		// Only move window but not cursor
		else {
			// Do not allow scrolling past first address
			if (memWinPos+lines > 0) {
				memWinPos += lines;
			}
			else {
				memWinCurY = 1;
				memWinPos = 0;
			}
			updateMemoryWin();
		}
	}
	refreshMemoryWinCursor();
}

void ApplicationTUI::updateMemoryWin() {
	// Highlight selected
	for (int i=0; i<winHeight-3; i++) {
		// Address of line
		mvwprintw(memoryWin, i+1, 2, "%08x", (memWinPos+i)*16);
		// Value in each memory space
		for (int j=0; j<4; j++) {
			WORD word = coreMem->read_word((memWinPos+i)*8 + j*4);
			mvwprintw(memoryWin, i+1, j*11 + 14, "%04x %04x", word>>16, word&0xFFFF);
		}
	}
}

void ApplicationTUI::setMemWinCurY(string position) {
	if (position == "viewtop") {
		memWinCurY = 1;
	}
	else if (position == "viewbtm")
		memWinCurY = winHeight - 3;
	else if (position == "top") {
		memWinPos = 0;
		updateMemoryWin();
		memWinCurY = 1;
	}
	else if (position == "btm") {
		memWinPos = coreMem->getSize() / 8 - (winHeight-3);
		updateMemoryWin();
		memWinCurY = winHeight - 3;
	}
	refreshMemoryWinCursor();
}

