#ifndef CORTEXM0P_MEMORY_H
#define CORTEXM0P_MEMORY_H

#include <cstdint>

class CM0P_Memory {
	public:
		// Read data inside memory
		uint8_t 	read_byte(uint8_t address);
		uint16_t	read_halfword(uint8_t address);
		uint32_t	read_word(uint8_t address);
};

#endif
