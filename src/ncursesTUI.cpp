#include "ncursesTUI.h"
#include <algorithm>	// For sort
#include <ncurses.h>
#include <string>

// void ApplicationTUI::resizeWin(int foo) {}

ApplicationTUI::ApplicationTUI(
		CM0P_Core* core,
		unordered_map<string, uint32_t> labels,
		vector<pair<string, ARMv6_Assembler::OpcodeResult>> asmResults
	) {
	this -> core = core;

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

	createMemoryWin();
	createLabelsWin(labels);
	createFlagsWin();
	createRegisterWin();
	createASMWin(asmResults);

	selectWin(memory);
}

void ApplicationTUI::clean() {
	endwin();
}

int ApplicationTUI::getWinCh(ApplicationTUI::winId id) {
	return wgetch(getWin(id));
}

void ApplicationTUI::updateRegisterWin() {
	uint32_t* coreRegs = core -> getCoreRegisters();
	for (int i=0; i<16; i++) {
		mvwprintw(registerWin, i+1, 17, "0x%08x", coreRegs[i]);
	}
	updateRegisterWinCursorVertical(0);
}

void ApplicationTUI::regWinChangeReg() {
	string ustr = "0x" + statusWinInputPrompt("New Register Value: 0x", 1);
	int regVal = (int)strtol(ustr.c_str(), NULL, 0);
	if (regWinCur == 16) { 		// PC
		// If address is over max of memory set to max
		if (regVal > core->getMemPtr()->getSize() or regVal%2) {
			createStatusWin("Invalid PC value! Must be within memory and is a multiple of 2.");
			wgetch(registerWin);
			createStatusWin(getWinStat(selectedWin));
			return;
		}
		core -> setPC(regVal);
		memWinGoto(regVal);
	}
	else {
		core->getCoreRegisters()[regWinCur-1] = regVal;
	}
	updateRegisterWin();
}

void ApplicationTUI::createStatusWin(string msg) {
	statusWin = newwin(1, winWidth, winHeight-1, 0);
	keypad(statusWin, TRUE);
	// string separator = "  |  ";
	// string msg = " q: quit";
	// // u8"\u25c0\u25bc\u25b2\u25b6" + 
	// msg += separator + "h,j,k,l/arrow keys: navigate";
	// msg += separator + "n: next instruction";
	// msg += separator + "p: previous instruction";
	// msg += separator + "/: goto address";
	// mvwprintw(statusWin, 0, 0, "q: quit");
	mvwprintw(statusWin, 0, 0, "%s", msg.c_str());
	wrefresh(statusWin);
}
void ApplicationTUI::updateStatusWin() {
	mvwprintw(statusWin, 0, winWidth-9, "%08x", (memWinPos+memWinCurY-1)*4*memWinWordPerLine + memWinCurX*2);
	wrefresh(statusWin);
}

void ApplicationTUI::createRegisterWin() {
	// Create window
	registerWin = newwin(18, 29, winHeight-19, winWidth/2-1);
	keypad(registerWin, TRUE);

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

	updateRegisterWin();

	// Draw window border
	wborder(registerWin, '|', '|', '-', '-', '+', '+', '+', '+');

	// Put on screen
	wrefresh(registerWin);
}
string ApplicationTUI::statusWinInputPrompt(string prompt, int inType=0) {
	wclear(statusWin);

	mvwprintw(statusWin, 0, 1, "%s", prompt.c_str());
	wrefresh(statusWin);
	int i = 0;
	string out = "";
	bool loop = 1;
	while (loop) {
		int c = wgetch(statusWin);
		switch (c) {
			case KEY_ENTER:
			case 10:
				loop = 0;
				break;
			case 8:		// ^h
			case KEY_BACKSPACE:
			case 127:
				if (i == 0)
					break;
				i--;
				out.erase(i,1);
				mvwprintw(statusWin, 0, prompt.size()+1, "%s ", out.c_str()); 
				break;
			case KEY_RIGHT:
			case 6:		// ^f
			case 2967:
				if (i < out.size()) {
					i++;
				}
				break;
			case KEY_LEFT:
			case 2:		// ^b
			case 2968:
				if (i > 0) {
					i--;
				}
				break;
			case KEY_HOME:
			case 1:		// ^a
				i=0;
				break;
			case KEY_END:
			case 5:		// ^e
				i = out.size();
				break;
			case 27:	// ESC
				createStatusWin(getWinStat(selectedWin));
				return " ";
			// Clear line
			case 21:	// ^U
				mvwprintw(statusWin, 0, prompt.size() + 1, "%s", string(out.size(), ' ').c_str());
				out.erase(0, i);
				mvwprintw(statusWin, 0, prompt.size()+1, "%s  ", out.c_str()); 
				i=0;
				break;
			case ' ':
			case '!' ... '/':
			case ':' ... '@':
			case 'G' ... '`':
			case 'g' ... '~':
				if (inType > 0)
					break;
			case 'A' ... 'F':
			case 'a' ... 'f':
				if (inType > 1)
					break;
			case '0' ... '9':
				out.insert(i, 1, c);
				mvwprintw(statusWin, 0, prompt.size()+1, "%s  ", out.c_str()); 
				i++;
				break;
			default:
				// mvwprintw(statusWin, 0, prompt.size()+1, "%d", c); 
				break;
		}
		mvwprintw(statusWin, 0, i+prompt.size() + 1, "");

	}
	out.push_back('\0');

	createStatusWin(getWinStat(selectedWin));
	return out;
}

void ApplicationTUI::memWinGoto() {
	string ustr = "0x" + statusWinInputPrompt("Goto address : 0x", 1);
	int address = (int)strtol(ustr.c_str(), NULL, 0);
	memWinGoto(address);
}
void ApplicationTUI::memWinGoto(uint32_t address) {
	// If address is over max of memory set to max
	if (address > core->getMemPtr()->getSize())
		address = core -> getMemPtr() -> getSize() - 1;
	removeMemoryWinCursor();
	int newMemWinPos = (address / (memWinWordPerLine*4));
	bool setBtm=0, scroll=1;
	// Check if new line is in current scroll view
	if (newMemWinPos - memWinPos > 0) {
		// new CurY at btm
		setBtm = 1;
		// Check if new line is in current scroll view
		scroll = (newMemWinPos - memWinPos) > (winHeight - 4);
	}
	// Scroll window
	if (scroll) {
		memWinPos = newMemWinPos;
		// Scroll up
		if (setBtm) {
			// Set position to be on top of scroll view
			memWinPos -= winHeight-4;
		}
		// Scroll down
		else if (memWinPos > memWinMaxPos) {
			memWinPos = memWinMaxPos;
		}
	}

	memWinCurX = (address % (memWinWordPerLine*4)) / 2;
	memWinCurY = newMemWinPos - memWinPos + 1;

	updateMemoryWin();
	drawMemoryWinCursor();
	updateStatusWin();
}

void ApplicationTUI::createMemoryWin() {
	memoryWin = newwin(winHeight-1, winWidth/2, 0, 0);
	keypad(memoryWin, TRUE);
	wborder(memoryWin, '|', '|', '-', '-', '+', '+', '+', '+');

	// Set max position for memory window
	memWinMaxPos = (2*memWinWordPerLine);
	memWinMaxPos = (core->getMemPtr()->getSize() + memWinMaxPos - 1) / memWinMaxPos;
	memWinMaxPos = memWinMaxPos - (winHeight-3);
	updateMemoryWin();
	drawMemoryWinCursor();
}

void ApplicationTUI::createFlagsWin() {
	flagsWin = newwin(6, 29, winHeight-24, winWidth/2-1);
	mvwprintw(flagsWin, 1, 2, "N");
	mvwprintw(flagsWin, 2, 2, "Z");
	mvwprintw(flagsWin, 3, 2, "C");
	mvwprintw(flagsWin, 4, 2, "V");
	updateFlagsWin();
	wborder(flagsWin, '|', '|', '-', '-', '+', '+', '+', '+');
	wrefresh(flagsWin);
}
void ApplicationTUI::createLabelsWin(unordered_map<string, uint32_t> labels) {
	labelsWin = newwin(winHeight-23, 29, 0, winWidth/2-1);
	mvwprintw(labelsWin, 1, 2, "-- Labels --");
	// Sort labels by their memory addresses
	vector<pair<string, uint32_t>> sortedLabels(labels.begin(), labels.end());
	sort(sortedLabels.begin(), sortedLabels.end(),
		[](pair<string, uint32_t>&a,pair<string, uint32_t>&b) {
			return a.second < b.second;
		}
	);
	int i=2;
	for (auto &it: sortedLabels) {
		mvwprintw(labelsWin, i, 2, "%08x   %s", it.second, it.first.c_str());
		i++;
	}
	wborder(labelsWin, '|', '|', '-', '-', '+', '+', '+', '+');
	wrefresh(labelsWin);
}

void ApplicationTUI::createASMWin(vector<pair<string, ARMv6_Assembler::OpcodeResult>> asmResults) {
	asmWin = newwin(winHeight-1, winWidth/2-28, 0, winWidth/2+27);
	int maxLine = winHeight - 0;
	if (maxLine > asmResults.size())
		maxLine = asmResults.size();
	int addr = core->getBaseAddr();
	for (int i=0; i<maxLine; i++) {
		ARMv6_Assembler::OpcodeResult ores = asmResults.at(i).second;
		// mvwprintw(asmWin, i, 2, "%08x %s", addr, asmResults.at(i-1).first.c_str());
		mvwprintw(asmWin, i+1, 2, "%08x %08x %s", addr, ores.opcode, asmResults.at(i).first.c_str());
		addr += (ores.i32+1) * 2;
	}
	wborder(asmWin, '|', '|', '-', '-', '+', '+', '+', '+');
	wrefresh(asmWin);
}

void ApplicationTUI::removeMemoryWinCursor() {
	HALFWORD hword = core->getMemPtr() -> read_halfword((memWinPos+memWinCurY-1)*4*memWinWordPerLine + memWinCurX*2);
	mvwprintw(memoryWin, memWinCurY, memWinCurX*5 + 14 + memWinCurX/2, "%04x", hword);
	wrefresh(memoryWin);
}
void ApplicationTUI::drawMemoryWinCursor() {
	wattron(memoryWin, A_BOLD);
	wattron(memoryWin, A_STANDOUT);
	HALFWORD hword = core->getMemPtr() -> read_halfword((memWinPos+memWinCurY-1)*4*memWinWordPerLine + memWinCurX*2);
	mvwprintw(memoryWin, memWinCurY, memWinCurX*5 + 14 + memWinCurX/2, "%04x", hword);
	wattroff(memoryWin, A_STANDOUT);
	wattroff(memoryWin, A_BOLD);
	// Remove extra highlighted character
	mvwprintw(memoryWin, memWinCurY, memWinCurX*5 + 14 + memWinCurX/2 + 3, "");
	wrefresh(memoryWin);
}

void ApplicationTUI::updateMemoryWinCursorVertical(int lines) {
	removeMemoryWinCursor();
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
	else if (lines != 0) {
		// Only moves cursor
		if (memWinCurY+lines > 0) {
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
	drawMemoryWinCursor();
	updateStatusWin();
}
void ApplicationTUI::updateMemoryWinCursorHorizontal(int cols) {
	removeMemoryWinCursor();
	if (cols > 0) {
		if (memWinCurX < memWinWordPerLine*2 -1) {
			memWinCurX += cols;
		}
		else if (memWinPos < memWinMaxPos or memWinCurY+1 < winHeight-2) {
			memWinCurX = 0;
			updateMemoryWinCursorVertical(1);
		}
	}
	else {
		if (memWinCurX > 0) {
			memWinCurX--;
		}
		else if (memWinPos > 0 or memWinCurY-1 > 0) {
			memWinCurX = memWinWordPerLine*2 - 1;
			updateMemoryWinCursorVertical(-1);
		}
	}
	drawMemoryWinCursor();
	updateStatusWin();
}

void ApplicationTUI::updateMemoryWin() {
	// Highlight selected
	for (int i=0; i<winHeight-3; i++) {
		// Address of line
		mvwprintw(memoryWin, i+1, 2, "%08x", (memWinPos+i)*4*memWinWordPerLine);
		// Value in each memory space
		for (int j=0; j<memWinWordPerLine; j++) {
			WORD word = core->getMemPtr()->read_word((memWinPos+i)*4*memWinWordPerLine + j*4);
			mvwprintw(memoryWin, i+1, j*11 + 14, "%04x %04x", word>>16, word&0xFFFF);
		}
	}
}

void ApplicationTUI::setPresetMemWinCur(int moveid) {
	removeMemoryWinCursor();
	switch (moveid) {
		// View top
		case 0:
			memWinCurY = 1;
			break;
		// View bottom
		case 1:
			memWinCurY = winHeight - 3;
			break;
		// First memory row
		case 2:
			memWinPos = 0;
			updateMemoryWin();
			memWinCurY = 1;
			break;
		// Last memory row
		case 3:
			memWinPos = memWinMaxPos;
			updateMemoryWin();
			memWinCurY = winHeight - 3;
		default:
			break;
	}
	drawMemoryWinCursor();
	updateStatusWin();
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
		case flags:
			return flagsWin;
		case labels:
			return labelsWin;
		case assembly:
			return asmWin;
		default:
			return nullptr; 
	}
}

string ApplicationTUI::getWinStat(winId id) {
	switch(id) {
		case memory:
			return " q: quit | h,j,k,l/arrow keys: navigate | n: next instruction | /: goto address | *: goto PC";
		case registers:
			return " q: quit | j:down | k:up | c: change register value";
		case help:
		case opcode:
		case status:
		case flags:
		case labels:
		case assembly:
		default:
			return "";
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
		createStatusWin(getWinStat(selectedWin));
	}
}

bool ApplicationTUI::confirmExit() {
	wclear(statusWin);
	mvwprintw(statusWin, 0, 0, "Exit? (y/N)");
	if (getWinCh(status) == 'y')
		return true;
	// CLeanup
	createStatusWin(getWinStat(selectedWin));
	return false;
}

void ApplicationTUI::updateFlagsWin() {
	mvwprintw(flagsWin, 1, 5, "%d", core -> get_flag('N'));
	mvwprintw(flagsWin, 2, 5, "%d", core -> get_flag('Z'));
	mvwprintw(flagsWin, 3, 5, "%d", core -> get_flag('C'));
	mvwprintw(flagsWin, 4, 5, "%d", core -> get_flag('V'));

	wrefresh(flagsWin);
}

void ApplicationTUI::updateRegisterWinCursorVertical(int lines) {
	mvwprintw(registerWin, regWinCur, 17, "0x%08x", core->getCoreRegisters()[regWinCur-1]);

	if (lines != 0) {
		if (lines > 0) {
			if (regWinCur+lines < 17) {
				regWinCur += lines;
			}
			else {
				regWinCur = 16;
			}
		}
		else {
			if (regWinCur+lines < 1) {
				regWinCur = 1;
			}
			else {
				regWinCur += lines;
			}
		}
	}

	wattron(registerWin, A_BOLD);
	mvwprintw(registerWin, regWinCur, 17, "0x%08x", core->getCoreRegisters()[regWinCur-1]);
	wattroff(registerWin, A_BOLD);
	wrefresh(registerWin);
}

