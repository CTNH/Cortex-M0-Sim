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

char ApplicationTUI::getWinCh(int winId) {
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
	msg += separator + "j: down";
	msg += separator + "k: up";
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

void ApplicationTUI::updateMemoryWinCursor() {
	wattron(memoryWin, A_BOLD);
	wattron(memoryWin, A_STANDOUT);
	mvwprintw(memoryWin, memWinCurY, memWinCurX, "");
	wattroff(memoryWin, A_STANDOUT);
	wattroff(memoryWin, A_BOLD);
	wrefresh(memoryWin);
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

