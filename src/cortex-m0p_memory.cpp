#include "cortex-m0p_memory.h"
#include <cstdlib>

BYTE CM0P_Memory:: read_byte(uint32_t address) {
	if (address >= size)
		return 0;
	return memory[address];
}

HALFWORD CM0P_Memory:: read_halfword(uint32_t address) {
	return read_byte(address) << 8 | read_byte(address+1);
}

WORD CM0P_Memory:: read_word(uint32_t address) {
	return read_halfword(address) << 16 | read_halfword(address+2);
}


void CM0P_Memory:: write_byte(uint32_t address, BYTE data) {
	if (address < size)
		memory[address] = data;
}

void CM0P_Memory:: write_halfword(uint32_t address, HALFWORD data) {
	write_byte(address, data >> 8);
	write_byte(address+1, data & 0xFF);
}

void CM0P_Memory:: write_word(uint32_t address, WORD data) {
	write_halfword(address, data >> 16);
	write_halfword(address+2, data & 0xFFFF);
}

CM0P_Memory::CM0P_Memory() {
	// memory = (uint32_t*)calloc(size, sizeof(uint32_t));
	memory = new uint8_t[size]();		// Zero init memory
}

CM0P_Memory::~CM0P_Memory() {
	free(memory);
}

int CM0P_Memory::getSize() {
	return size;
}

