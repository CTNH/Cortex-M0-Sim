#include "cortex-m0p_memory.h"

void CM0P_Memory:: check_endian() {
	// endianness = CORE.AIRCR.endianness;
}
uint8_t CM0P_Memory:: read_byte(uint8_t address) {
	check_endian();
	return 0;
}

uint16_t CM0P_Memory:: read_halfword(uint8_t address) {
	check_endian();
	return 0;
}

uint32_t CM0P_Memory:: read_word(uint8_t address) {
	check_endian();
	return 0;
}


void CM0P_Memory:: write_byte(uint8_t address, BYTE data) {
}

void CM0P_Memory:: write_halfword(uint8_t address, HALFWORD data) {
}

void CM0P_Memory:: write_word(uint8_t address, WORD data) {
}

