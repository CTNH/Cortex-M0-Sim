#ifndef CORTEXM0P_MEMORY_H
#define CORTEXM0P_MEMORY_H

#include <cstdint>

using WORD = uint32_t;
using HALFWORD = uint16_t;
using BYTE = uint8_t;

class CM0P_Memory {
	private:
		bool endianness;
		// Check the endianness bit in the AIRCR register and update the endianness variable
		void check_endian();
		// Check address validity; Called by all read and write functions
		bool valid_address(uint8_t address);
	public:
		// Read data inside memory
		BYTE		read_byte(uint8_t address);
		HALFWORD	read_halfword(uint8_t address);
		WORD		read_word(uint8_t address);
		// Write data to memory
		void		write_byte(uint8_t address, BYTE data);
		void		write_halfword(uint8_t address, HALFWORD data);
		void		write_word(uint8_t address, WORD data);
};

#endif
