#include "ncursesTUI.h"
#include <ncurses.h>
#include <string>

// void ApplicationTUI::resizeWin(int foo) {}

ApplicationTUI::ApplicationTUI(CM0P_Memory* memPtr) {
	this->coreMem = memPtr;
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
	createFlagsWin();

	selectWin(memory);
}

void ApplicationTUI::clean() {
	endwin();
}

int ApplicationTUI::getWinCh(ApplicationTUI::winId id) {
	return wgetch(getWin(id));
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
	wborder(registerWin, '|', '|', '-', '-', '+', '+', '+', '+');

	// Put on screen
	wrefresh(registerWin);
}

void ApplicationTUI::createMemoryWin() {
	memoryWin = newwin(winHeight-1, winWidth/2, 0, 0);
	keypad(memoryWin, TRUE);
	wborder(memoryWin, '|', '|', '-', '-', '+', '+', '+', '+');

	// Set max position for memory window
	memWinMaxPos = (2*memWinWordPerLine);
	memWinMaxPos = (coreMem->getSize() + memWinMaxPos - 1) / memWinMaxPos;
	memWinMaxPos = memWinMaxPos - (winHeight-3);
	updateMemoryWin();
	refreshMemoryWinCursor();
}

void ApplicationTUI::createFlagsWin() {
	flagsWin = newwin(6, 29, winHeight-24, winWidth/2-1);
	mvwprintw(flagsWin, 1, 2, "N");
	mvwprintw(flagsWin, 2, 2, "Z");
	mvwprintw(flagsWin, 3, 2, "C");
	mvwprintw(flagsWin, 4, 2, "V");
	wborder(flagsWin, '|', '|', '-', '-', '+', '+', '+', '+');
	wrefresh(flagsWin);
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
			if (memWinPos+lines < memWinMaxPos) {
				memWinPos += lines;
			}
			else {
				memWinCurY = winHeight - 3;
				memWinPos = memWinMaxPos;
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
		mvwprintw(memoryWin, i+1, 2, "%08x", (memWinPos+i)*4*memWinWordPerLine);
		// Value in each memory space
		for (int j=0; j<memWinWordPerLine; j++) {
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
		memWinPos = memWinMaxPos;
		updateMemoryWin();
		memWinCurY = winHeight - 3;
	}
	refreshMemoryWinCursor();
}

WINDOW* ApplicationTUI::getWin(ApplicationTUI::winId id) {
	switch(id) {
		case memory:
			return memoryWin;
		case help:
			return helpWin;
		case registers:
			return registerWin;
		case opcode:
			return opcodeWin;
		case status:
			return statusWin;
		default:
			return (WINDOW*)NULL;
	}
}
void ApplicationTUI::selectWin(ApplicationTUI::winId id) {
	if (id != selectedWin) {
		// 'Unselect' previous selected window
		wborder(getWin(selectedWin), '|', '|', '-', '-', '+', '+', '+', '+');
		wrefresh(getWin(selectedWin));
		// 'Select' new window
		wattron(getWin(id), A_BOLD);
		wattron(getWin(id), A_STANDOUT);
		wborder(getWin(id), '|', '|', '-', '-', '+', '+', '+', '+');
		wattroff(getWin(id), A_STANDOUT);
		wattroff(getWin(id), A_BOLD);
		wrefresh(getWin(id));

		selectedWin = id;
	}
}

bool ApplicationTUI::confirmExit() {
	wclear(statusWin);
	mvwprintw(statusWin, 0, 0, "Exit? (y/N)");
	if (getWinCh(status) == 'y')
		return true;
	// CLeanup
	createStatusWin();
	return false;
}

void ApplicationTUI::updateFlagsWin(char flag, bool value) {
	switch (flag) {
		case 'N':
			mvwprintw(flagsWin, 1, 5, "%d", value);
			break;
		case 'Z':
			mvwprintw(flagsWin, 2, 5, "%d", value);
			break;
		case 'C':
			mvwprintw(flagsWin, 3, 5, "%d", value);
			break;
		case 'V':
			mvwprintw(flagsWin, 4, 5, "%d", value);
			break;
	}
	wrefresh(flagsWin);
}

