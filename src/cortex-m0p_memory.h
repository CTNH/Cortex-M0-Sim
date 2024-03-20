#ifndef CORTEXM0P_MEMORY_H
#define CORTEXM0P_MEMORY_H

#include <cstdint>
#include <string>
#include <exception>

using WORD = uint32_t;
using HALFWORD = uint16_t;
using BYTE = uint8_t;

class CM0P_Memory {
	private:
		uint8_t* memory;
		// Default memory provides up to 4GB (0x40000000) of addressable memory
		const static int size = 0x20000000;	// 512 MB
		bool endianness;
		// Check the endianness bit in the AIRCR register and update the endianness variable
		void check_endian();
		// Check address validity; Called by all read and write functions
		bool valid_address(uint32_t address);
	public:
		// Read data inside memory
		BYTE		read_byte(uint32_t address);
		HALFWORD	read_halfword(uint32_t address);
		WORD		read_word(uint32_t address);
		// Write data to memory
		void		write_byte(uint32_t address, BYTE data);
		void		write_halfword(uint32_t address, HALFWORD data);
		void		write_word(uint32_t address, WORD data);
		// Constructor
		CM0P_Memory();
		// Deconstructor
		~CM0P_Memory();

		int getSize();
};
#endif
