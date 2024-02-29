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


	bool loop = true;
	while(loop) {
		switch(wgetch(memoryWin)) {
			case 'q':
				loop = false;
				break;
			default:
				break;
		}
	}


	endwin();
}

void ApplicationTUI::updateReg(int reg, uint32_t value) {
	mvwprintw(registerWin, reg+1, 17, "0x%08x", value);
	wrefresh(registerWin);
}

void ApplicationTUI::createStatusWin() {
	statusWin = newwin(1, winWidth, winHeight-1, 0);
	string separator = "    ";
	string msg = "q: quit";
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
	registerWin = newwin(18, 29, winHeight/2, winWidth/2-1);

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
		updateReg(i, 0); 	// TODO: Take pre-existing values
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

	wattron(memoryWin, A_BOLD);
	wattron(memoryWin, A_STANDOUT);
	mvwprintw(memoryWin, 1, 1, "Hello World");
	wattroff(memoryWin, A_STANDOUT);
	wattroff(memoryWin, A_BOLD);
	wrefresh(memoryWin);

	for (int i=2; i<40; i++) {
		std::string tmp = "";
		for (int j=0; j<88; j++) {
			tmp += "-";
		}
		mvwprintw(memoryWin, i, 1, "%s", tmp.c_str());
	}
	box(memoryWin, 0, 0);
}

