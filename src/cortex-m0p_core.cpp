#include "cortex-m0p_core.h"

CM0P_Core::CM0P_Core(vector<ARMv6_Assembler::OpcodeResult> opcodes, uint32_t startAddr) {
	// Initialize registers
	for (int i=0; i<16; i++) {
		R[i] = 0;
	}

	// Write opcodes into memory
	int i=0;
	for (auto &it: opcodes) {
		if (it.i32) {
			memory.write_word(INST_BASEADDR+i, it.opcode);
			i+=2;
		}
		else {
			memory.write_halfword(INST_BASEADDR+i, it.opcode);
		}
		i+=2;
	}
	setPC(startAddr);
}

uint32_t CM0P_Core::update_flag_addition(uint32_t a, uint32_t b) {
	uint32_t result = a + b;
	update_flag('N', result >> 31);	// Value of MSb
	update_flag('Z', result == 0);
	update_flag('C', a + b > 0xFFFFFFFF);
	update_flag(
		'V',
		((result >> 31) && ~(a >> 31) && ~(b >> 31)) ||
		(~(result >> 31) && (a >> 31) && (b >> 31))
	);
	return result;
}
uint32_t CM0P_Core::update_flag_subtraction(uint32_t a, uint32_t b) {
	uint32_t result = a - b;
	update_flag('N', result >> 31);	// Value of MSb
	update_flag('Z', result == 0);
	update_flag('C', a >= b);	// Result of subtraction >= 0
	update_flag(
		'V',
		((result >> 31) && ~(a >> 31) && (b >> 31)) ||
		(~(result >> 31) && (a >> 31) && ~(b >> 31))
	);
	return result;
}

bool CM0P_Core:: get_flag(char flag) {
	switch (flag) {
		case 'N':
			return (condFlags >> 3) & 1;
		case 'Z':
			return (condFlags >> 2) & 1;
		case 'C':
			return (condFlags >> 1) & 1;
		case 'V':
			return (condFlags >> 0) & 1;
	}
	return 0;
}

void CM0P_Core::update_flag(char flag, bool bit) {
	int n=7;
	switch(flag) {
		// Negative Flag
		case 'N':
			n = 3;
			break;
		// Zero Flag
		case 'Z':
			n = 2;
			break;
		// Carry Flag
		case 'C':
			n = 1;
			break;
		// Overflow Flag
		case 'V':
			n = 0;
			break;
	}
	if (bit) {
		condFlags |= (uint)1 << n;
	}
	else {
		condFlags &= ~((uint)1 << n);
	}
}

void CM0P_Core::step_inst() {
	uint16_t opcode = memory.read_halfword(R[15]);
	// Indicate whether PC should be incremented at the end
	bool incrementPC = 1;

	// From ARMv6-M Architecture Reference Manual A5.2
	switch (opcode >> 10) {
		case 0b000000 ... 0b001111:
			switch(opcode >> 11) {
				// LSLS Logical Shift Left Immediate
				case 0b000:
					{
						uint8_t imm5 = (opcode >> 6) & 0b11111;
						uint8_t Rm = (opcode >> 3) & 0b111;
						uint8_t Rd = opcode & 0b111;
						R[Rd] = R[Rm] << imm5;
						update_flag('C', 0);	// TODO
						update_flag('N', R[Rd] >> 31);	// Value of MSb
						update_flag('Z', R[Rd] == 0);
					}
					break;

				// LSRS Logical Shift Right Immediate
				case 0b001:
					{
						uint8_t imm5 = (opcode >> 6) & 0b11111;
						uint8_t Rm = (opcode >> 3) & 0b111;
						uint8_t Rd = opcode & 0b111;
						R[Rd] = R[Rm] >> imm5;
						update_flag('C', 0);	// TODO
						update_flag('N', R[Rd] >> 31);	// Value of MSb
						update_flag('Z', R[Rd] == 0);
					}
					break;

				// ASRS Arithmetic Shift Right Immediate
				case 0b010:
					{
						uint8_t imm5 = (opcode >> 6) & 0b11111;
						uint8_t Rm = (opcode >> 3) & 0b111;
						uint8_t Rd = opcode & 0b111;
						bool msb = R[Rd] >> 31;
						R[Rd] = R[Rm] >> imm5;
						if (msb)
							R[Rd] |= ((1<<imm5) - 1) << (32-imm5);
						update_flag('C', 0);	// TODO
						update_flag('N', R[Rd] >> 31);	// Value of MSb
						update_flag('Z', R[Rd] == 0);
					}
					break;

				// MOVS Move Immediate
				case 0b100:
					{
						uint8_t imm8 = opcode & 0xFF;
						uint8_t Rd = (opcode >> 8) & 0b111;
						R[Rd] = imm8;
						update_flag('N', R[Rd] >> 31);	// Value of MSb
						update_flag('Z', R[Rd] == 0);
					}
					break;

				// CMP Compare Immediate
				case 0b101:
					{
						uint8_t Rn = (opcode >> 8) & 0b111;
						uint8_t imm8 = opcode & 0xFF;
						update_flag_subtraction(R[Rn], imm8);
					}
					break;

				// ADDS Add 8-bit Immediate
				case 0b110:
					{
						uint8_t Rdn = (opcode >> 8) & 0b111;
						uint8_t imm8 = opcode & 0xFF;
						R[Rdn] = update_flag_addition(R[Rdn], imm8);
					}
					break;

				// SUBS Subtract 8-bit immediate
				case 0b111:
					{
						uint8_t Rdn = (opcode >> 8) & 0b111;
						uint8_t imm8 = opcode & 0xFF;
						R[Rdn] = update_flag_subtraction(R[Rdn], imm8);
					}
					break;

				case 0b011:
					switch (opcode >> 9) {
						// ADDS Add register
						case 0b01100:
							{
								uint8_t Rm = (opcode >> 6) & 0b111;
								uint8_t Rn = (opcode >> 3) & 0b111;
								uint8_t Rd = opcode & 0b111;
								R[Rd] = update_flag_addition(R[Rn], R[Rm]);
							}
							break;

						// SUBS Subtract register
						case 0b01101:
							{
								uint8_t Rm = (opcode >> 6) & 0b111;
								uint8_t Rn = (opcode >> 3) & 0b111;
								uint8_t Rd = opcode & 0b111;
								R[Rd] = update_flag_subtraction(R[Rn], R[Rm]);
							}
							break;

						// ADDS Add 3-bit immediate
						case 0b01110:
							{
								uint8_t imm3 = (opcode >> 6) & 0b111;
								uint8_t Rn = (opcode >> 3) & 0b111;
								uint8_t Rd = opcode & 0b111;
								R[Rd] = update_flag_addition(R[Rn], imm3);
							}
							break;

						// SUBS Subtract 3-bit immediate
						case 0b01111:
							{
								uint8_t imm3 = (opcode >> 6) & 0b111;
								uint8_t Rn = (opcode >> 3) & 0b111;
								uint8_t Rd = opcode & 0b111;
								R[Rd] = update_flag_subtraction(R[Rn], imm3);
							}
							break;
					}
					break;

			}
			break;

		// A5.2.2 Data Processing
		case 0b010000:
			switch ((opcode >> 6) & 0xF) {
				// ANDS Bitwise AND
				case 0b0000:
					{
						uint8_t Rm = (opcode >> 3) & 0b111;
						uint8_t Rdn = opcode & 0b111;
						R[Rdn] &= R[Rm];
						update_flag('N', R[Rdn] >> 31);	// Value of MSb
						update_flag('Z', R[Rdn] == 0);
					}
					break;

				// EORS Exclusive OR
				case 0b0001:
					{
						uint8_t Rm = (opcode >> 3) & 0b111;
						uint8_t Rdn = opcode & 0b111;
						R[Rdn] ^= R[Rm];
						update_flag('N', R[Rdn] >> 31);	// Value of MSb
						update_flag('Z', R[Rdn] == 0);
					}
					break;

				// LSLS Logical Shift Left Register
				case 0b0010:
					{
						uint8_t Rm = (opcode >> 3) & 0b111;
						uint8_t Rdn = opcode & 0b111;
						// Shift by least significant byte in register
						uint32_t result = R[Rdn] << (R[Rm] & 0xFF);
						// If shift by 32 or more bits, clear all bits in result to 0
						if ((R[Rm] & 0xFF) >= 32)
							result = 0;
						update_flag('N', result >> 31);	// Value of MSb
						update_flag('Z', result == 0);
						// If shift 0 bits carry flag is unaffected
						if ((R[Rm] & 0xFF) != 0) {
							// If shift by 33 or more bits and update carry flag, set to 0
							if ((R[Rm] & 0xFF) >= 33)
								update_flag('C', 0);
							else
								// Updated to last bit shifted out
								update_flag('C', (R[Rdn] >> (32-R[Rm])) & 1);
						}
						R[Rdn] = result;
					}
					break;

				// LSRS Logical Shift Right Register
				case 0b0011:
					{
						uint8_t Rm = (opcode >> 3) & 0b111;
						uint8_t Rdn = opcode & 0b111;
						// Shift by least significant byte in register
						uint32_t result = R[Rdn] >> (R[Rm] & 0xFF);
						// If shift by 32 or more bits, clear all bits in result to 0
						if ((R[Rm] & 0xFF) >= 32)
							result = 0;
						update_flag('N', result >> 31);	// Value of MSb
						update_flag('Z', result == 0);
						// If shift 0 bits carry flag is unaffected
						if ((R[Rm] & 0xFF) != 0) {
							// If shift by 33 or more bits and update carry flag, set to 0
							if ((R[Rm] & 0xFF) >= 33)
								update_flag('C', 0);
							else
								// Updated to last bit shifted out
								update_flag('C', (R[Rdn] >> ((R[Rm]&0xFF) - 1)) & 1);
						}
						R[Rdn] = result;
					}
					break;

				// ASRS Arithmetic Shift Right Register
				// TODO: Check correct
				case 0b0100:
					{
						uint8_t Rm = (opcode >> 3) & 0b111;
						uint8_t Rdn = opcode & 0b111;
						uint8_t shiftLen = (R[Rm] & 0xFF);
						// Shift by least significant byte in register
						uint32_t result = R[Rdn] >> shiftLen;
						// Shift in copied of sign bit
						if (R[Rdn] >> 31)
							result |= ((1<<shiftLen) - 1) << (32-shiftLen);
						// If shift by 32 or more bits, clear all bits in result to 0
						if ((R[Rm] & 0xFF) >= 32)
							result = 0;
						update_flag('N', result >> 31);	// Value of MSb
						update_flag('Z', result == 0);
						// If shift 0 bits carry flag is unaffected
						if ((R[Rm] & 0xFF) != 0) {
							// If shift by 33 or more bits and update carry flag, set to 0
							if ((R[Rm] & 0xFF) >= 33)
								update_flag('C', 0);
							else
								// Updated to last bit shifted out
								update_flag('C', (R[Rdn] >> ((R[Rm]&0xFF) - 1)) & 1);
						}
						R[Rdn] = result;
					}
					break;

				// ADCS Add With Carry Register
				case 0b0101:
					{
						uint8_t Rm = (opcode >> 3) & 0b111;
						uint8_t Rdn = opcode & 0b111;
						R[Rdn] = update_flag_addition(R[Rdn], R[Rm] + get_flag('C'));
					}
					break;

				// SBCS Subtract With Carry Register
				case 0b0110:
					{
						uint8_t Rm = (opcode >> 3) & 0b111;
						uint8_t Rdn = opcode & 0b111;
						R[Rdn] = update_flag_subtraction(R[Rdn], R[Rm] + get_flag('C'));
					}
					break;

				// RORS Rotate Right Register
				// TODO
				case 0b0111:
					{
						uint8_t Rm = (opcode >> 3) & 0b111;
						uint8_t Rdn = opcode & 0b111;
					}
					break;

				// TST Set Flags on bitwise AND
				case 0b1000:
					{
						uint8_t Rm = (opcode >> 3) & 0b111;
						uint8_t Rn = opcode & 0b111;
						uint32_t result = R[Rm] & R[Rn];
						update_flag('N', result >> 31);	// Value of MSb
						update_flag('Z', result == 0);
					}
					break;

				// RSBS Reverse Subract from 0 Register
				case 0b1001:
					{
						uint8_t Rn = (opcode >> 3) & 0b111;
						uint8_t Rd = opcode & 0b111;
						update_flag_subtraction(0, R[Rn]);
					}
					break;

				// CMP Compare Registers
				case 0b1010:
					{
						uint8_t Rm = (opcode >> 3) & 0b111;
						uint8_t Rn = opcode & 0b111;
						update_flag_subtraction(R[Rn], R[Rm]);
					}
					break;

				// CMN Compare Negative Registers
				case 0b1011:
					{
						uint8_t Rm = (opcode >> 3) & 0b111;
						uint8_t Rn = opcode & 0b111;
						update_flag_addition(R[Rn], R[Rm]);
					}
					break;

				// ORRS Logical OR Register
				case 0b1100:
					{
						uint8_t Rm = (opcode >> 3) & 0b111;
						uint8_t Rdn = opcode & 0b111;
						R[Rdn] |= R[Rm];
						update_flag('N', R[Rdn] >> 31);	// Value of MSb
						update_flag('Z', R[Rdn] == 0);
					}
					break;

				// MULS Multiply Two Registers
				case 0b1101:
					{
						uint8_t Rn = (opcode >> 3) & 0b111;
						uint8_t Rdm = opcode & 0b111;
						R[Rdm] *= R[Rn];
						update_flag('N', R[Rdm] >> 31);	// Value of MSb
						update_flag('Z', R[Rdm] == 0);
					}
					break;

				// BICS Bit Clear Register
				case 0b1110:
					{
						uint8_t Rm = (opcode >> 3) & 0b111;
						uint8_t Rdn = opcode & 0b111;
						R[Rdn] &= ~R[Rm];
						update_flag('N', R[Rdn] >> 31);	// Value of MSb
						update_flag('Z', R[Rdn] == 0);
					}
					break;

				// MVN Bitwise NOT Register
				case 0b1111:
					{
						uint8_t Rm = (opcode >> 3) & 0b111;
						uint8_t Rd = opcode & 0b111;
						R[Rd] = ~R[Rm];
						update_flag('N', R[Rd] >> 31);	// Value of MSb
						update_flag('Z', R[Rd] == 0);
					}
					break;

			}
			break;

		// A5.2.3 Special data instructions and branch and exchange
		case 0b010001:
			switch ((opcode >> 8) & 0b11) {
				// Add Registers
				case 0b00:
					{
						uint8_t Rm = (opcode >> 3) & 0xF;
						uint8_t Rdn = opcode & 0b111;
					}
					break;

				// Compare Registers / Unpredictable
				case 0b01:
					switch((opcode >> 6) & 0b11) {
						// Unpredictable
						case 0b00:
							break;

						case 0b01:
							break;
					}
					break;

				// MOV Move Registers
				// TODO
				case 0b10:
					{
						uint8_t Rm = (opcode >> 3) & 0xF;
						uint8_t Rd = opcode & 0b111;
					}
					break;

				// Branch
				case 0b11:
					// BLX Branch with Link and Exchange Register
					if ((opcode >> 7) & 1) {
						uint8_t Rm = (opcode >> 3) & 0xF;
						// If bit[0] of Rm is 0
						if (~(R[Rm] & 1)) {
							// TODO: Hardfault exception
						}
					}
					// BX Branch and Exchange
					else {
						uint8_t Rm = (opcode >> 3) & 0xF;
						// If bit[0] of Rm is 0
						if (~(R[Rm] & 1)) {
							// TODO: Hardfault exception
						}
					}
					break;
			}
			break;

		// LDR (Literal)
		case 0b010010 ... 0b010011:
			{
				uint8_t imm8 = opcode & 0xFF;
				uint8_t Rt = (opcode >> 8) & 0b111;
			}
			break;

		// A5.2.4 Load/Store single data item
		case 0b010100 ... 0b100111:
			switch (opcode >> 12) {
				case 0b0101:
					switch ((opcode >> 9) & 0b111) {
						// STR (register) - Store Register
						case 0b000:
							{
								uint8_t Rm = (opcode >> 6) & 0b111;
								uint8_t Rn = (opcode >> 3) & 0b111;
								uint8_t Rt = opcode & 0b111;
								memory.write_word(R[Rm] + R[Rn], R[Rt]);
							}
							break;
						// STRH (register) - Store Register Halfword
						case 0b001:
							{
								uint8_t Rm = (opcode >> 6) & 0b111;
								uint8_t Rn = (opcode >> 3) & 0b111;
								uint8_t Rt = opcode & 0b111;
								memory.write_halfword(R[Rm] + R[Rn], R[Rt]);
							}
							break;
						// STRB (register) - Store Register Byte
						case 0b010:
							{
								uint8_t Rm = (opcode >> 6) & 0b111;
								uint8_t Rn = (opcode >> 3) & 0b111;
								uint8_t Rt = opcode & 0b111;
								memory.write_byte(R[Rm] + R[Rn], R[Rt]);
							}
							break;
						// LDRSB (register) - Load Register Signed Byte
						case 0b011:
							{
								uint8_t Rm = (opcode >> 6) & 0b111;
								uint8_t Rn = (opcode >> 3) & 0b111;
								uint8_t Rt = opcode & 0b111;
								uint32_t data = memory.read_byte(R[Rm] + R[Rn]);
								// If most significant bit is set
								if (data & 0x80)
									data |= 0xFFFFFF00;
								R[Rt] = data;
							}
							break;
						// LDR (register) - Load Register
						case 0b100:
							{
								uint8_t Rm = (opcode >> 6) & 0b111;
								uint8_t Rn = (opcode >> 3) & 0b111;
								uint8_t Rt = opcode & 0b111;
								R[Rt] = memory.read_word(R[Rm] + R[Rn]);
							}
							break;
						// LDRH (register) - Load Register Halfword
						case 0b101:
							{
								uint8_t Rm = (opcode >> 6) & 0b111;
								uint8_t Rn = (opcode >> 3) & 0b111;
								uint8_t Rt = opcode & 0b111;
								R[Rt] = memory.read_halfword(R[Rm] + R[Rn]);
							}
							break;
						// LDRB (register) - Load Register Byte
						case 0b110:
							{
								uint8_t Rm = (opcode >> 6) & 0b111;
								uint8_t Rn = (opcode >> 3) & 0b111;
								uint8_t Rt = opcode & 0b111;
								R[Rt] = memory.read_byte(R[Rm] + R[Rn]);
							}
							break;
						// LDRSH (register) - Load Register Signed Halfword
						case 0b111:
							{
								uint8_t Rm = (opcode >> 6) & 0b111;
								uint8_t Rn = (opcode >> 3) & 0b111;
								uint8_t Rt = opcode & 0b111;
								uint32_t data = memory.read_halfword(R[Rm] + R[Rn]);
								if (data & 0x8000) {
									data |= 0xFFFF0000;
								}
								R[Rt] = data;
							}
							break;
					}
					break;
				case 0b0110:
					// LDR (immediate) - Load Register
					if ((opcode >> 11) & 1) {
						uint8_t imm5 = (opcode >> 6) & 0b11111;
						uint8_t Rn = (opcode >> 3) & 0b111;
						uint8_t Rt = opcode & 0b111;
						R[Rt] = memory.read_word(imm5 + R[Rn]);
					}
					// STR (immediate) - Store Register
					else {
						uint8_t imm5 = (opcode >> 6) & 0b11111;
						uint8_t Rn = (opcode >> 3) & 0b111;
						uint8_t Rt = opcode & 0b111;
						memory.write_word(imm5 + R[Rn], R[Rt]);
					}
					break;
				case 0b0111:
					// LDRB (immediate) - Load Register Byte
					if ((opcode >> 11) & 1) {
						uint8_t imm5 = (opcode >> 6) & 0b11111;
						uint8_t Rn = (opcode >> 3) & 0b111;
						uint8_t Rt = opcode & 0b111;
						R[Rt] = memory.read_byte(imm5 + R[Rn]);
					}
					// STRB (immediate) - Store Register Byte
					else {
						uint8_t imm5 = (opcode >> 6) & 0b11111;
						uint8_t Rn = (opcode >> 3) & 0b111;
						uint8_t Rt = opcode & 0b111;
						memory.write_byte(imm5 + R[Rn], R[Rt]);
					}
					break;
				case 0b1000:
					// LDRH (immediate) - Load Register Halfword
					if ((opcode >> 11) & 1) {
						uint8_t imm5 = (opcode >> 6) & 0b11111;
						uint8_t Rn = (opcode >> 3) & 0b111;
						uint8_t Rt = opcode & 0b111;
						R[Rt] = memory.read_halfword(imm5 + R[Rn]);
					}
					// STRH (immediate) - Store Register Halfword
					else {
						uint8_t imm5 = (opcode >> 6) & 0b11111;
						uint8_t Rn = (opcode >> 3) & 0b111;
						uint8_t Rt = opcode & 0b111;
						memory.write_halfword(imm5 + R[Rn], R[Rt]);
					}
					break;
				case 0b1001:
					// LDR (immediate) - Load Register SP Relative
					if ((opcode >> 11) & 1) {
						uint8_t Rt = (opcode >> 8) & 0b111;
						uint8_t imm8 = opcode & 0xFF;
						R[Rt] = memory.read_word(imm8 + *SP);
					}
					// STR (immediate) - Store Register SP Relative
					else {
						uint8_t Rt = (opcode >> 8) & 0b111;
						uint8_t imm8 = opcode & 0xFF;
						memory.write_word(imm8 + *SP, R[Rt]);
					}
					break;
			}
			break;

		// ADR (Generate PC-Relative Address)
		case 0b101000 ... 0b101001:
			{
				uint8_t Rd = (opcode >> 8) & 0b111;
				uint8_t imm8 = opcode & 0xFF;
				R[Rd] = *PC + imm8;
			}
			break;

		// ADD (SP Plus Immediate)
		case 0b101010 ... 0b101011:
			{
				uint8_t Rd = (opcode >> 8) & 0b111;
				uint8_t imm8 = opcode & 0xFF;

				R[Rd] = *SP + imm8;
			}
			break;

		// Miscellaneous 16-bit instructions
		case 0b101100 ... 0b101111:
			switch ((opcode >> 6) & 0b111111) {
				// ADD (SP plus immediate) - Add immediate to SP
				case 0b000000 ... 0b000001:
					{
						uint8_t imm7 = opcode & 0b1111111;
						*SP += imm7;
					}
					break;
				// SUB (SP minus immediate) - Subtract Immediate from SP
				case 0b000010 ... 0b000011:
					{
						uint8_t imm7 = opcode & 0b1111111;
						*SP -= imm7;
					}
					break;
				// SXTH - Signed Extend Halfword
				case 0b001000:
					{
						uint8_t Rm = (opcode >> 3) & 0b111;
						uint8_t Rd = opcode & 0b111;
						uint32_t data = memory.read_halfword(R[Rm]);
						if (data & 0x8000)
							data |= 0xFFFF0000;
						else
							// Take lower 16-bits only
							data &= 0xFFFF;
						R[Rd] = data;
					}
					break;
				// SXTB - Signed Extend Byte
				case 0b001001:
					{
						uint8_t Rm = (opcode >> 3) & 0b111;
						uint8_t Rd = opcode & 0b111;
						uint32_t data = memory.read_byte(R[Rm]);
						if (data & 0x80)
							data |= 0xFFFFFF00;
						else
							// Take lower 8-bits only
							data &= 0xFF;
						R[Rd] = data;
					}
					break;
				// UXTH - Unsigned Extend Halfword
				case 0b001010:
					{
						uint8_t Rm = (opcode >> 3) & 0b111;
						uint8_t Rd = opcode & 0b111;
						// Take lower 16-bits only
						R[Rd] = R[Rm] & 0xFFFF;
					}
					break;
				// UXTB - Unsigned Extend Byte
				case 0b001011:
					{
						uint8_t Rm = (opcode >> 3) & 0b111;
						uint8_t Rd = opcode & 0b111;
						// Take lower 8-bits only
						R[Rd] = opcode & 0xFF;
					}
					break;
				// PUSH - Push Multiple Registers
				case 0b010000 ... 0b010111:
					{
						uint8_t register_list = opcode & 0xFF;
						for (int i=0; i<8; i++) {
							// Check if bit is set
							if (register_list >> i & 1) {
								// stackPush(R[i+1]);
								// TODO
							}
						}
					}
					break;
				// CPS - Change Processor State
				case 0b011001:
					break;
				// REV - Byte-Reverse Word
				case 0b101000:
					{
						uint8_t Rm = (opcode >> 3) & 0b111;
						uint8_t Rd = opcode & 0b111;
						R[Rd] = 
							// Swap first and last byte
							(R[Rm] >> 24) | ((R[Rm] & 0xFF) << 24) |
							// Swap middle bytes
							(((R[Rm] >> 8) & 0xFF) << 16) | (((R[Rm] >> 16) & 0xFF) << 8);
					}
					break;
				// REV16 - Byte-Reverse Packed Halfword
				case 0b101001:
					{
						uint8_t Rm = (opcode >> 3) & 0b111;
						uint8_t Rd = opcode & 0b111;
						R[Rd] = 
							// Swap lower bytes
							((R[Rm] >> 8) & 0xFF) | ((R[Rm] & 0xFF) << 8) |
							// Swap higher bytes
							(((R[Rm]>>16) & 0xFF) << 24) | (((R[Rm]>>24) & 0xFF) << 16);
					}
					break;
				// REVSH - Byte-Reverse Signed Halfword
				case 0b101011:
					{
						uint8_t Rm = (opcode >> 3) & 0b111;
						uint8_t Rd = opcode & 0b111;
						R[Rd] = ((R[Rm] & 0xFF) << 8) | ((R[Rm] >> 8) & 0xFF);
						// If 15th bit is set
						if (R[Rd] & 0x8000)
							R[Rd] |= 0xFFFF0000;
					}
					break;
				// POP - Pop Multiple Registers
				case 0b110000 ... 0b110111:
					{
						if (opcode & 1) {
							// TODO Hardfault exception
						}
						uint8_t register_list = opcode & 0xFF;
						uint8_t P = (opcode >> 8) & 1;
					}
					break;
				// BKPT - Breakpoint
				case 0b111000 ... 0b111011:
					break;
				// Hints
				case 0b111100 ... 0b111111:
					break;
			}
			break;
		
		// STM - Store multiple registers
		case 0b110000 ... 0b110001:
			{
				uint8_t Rn = (opcode >> 8) & 0b111;
				uint8_t register_list = opcode & 0xFF;
				uint8_t address = R[Rn];
				for (int i=7; i>-1; i--) {
					if ((register_list >> i) & 1) {
						memory.write_word(address, R[8-i]);
						address += 4;
					}
				}
				// If Rn is unset
				if (~((register_list >> Rn) & 1)) {
					// Write back address
					R[Rn] = address;
				}
			}
			break;

		// LDM - Load multiple registers
		case 0b110010 ... 0b110011:
			{
				uint8_t Rn = (opcode >> 8) & 0b111;
				uint8_t register_list = opcode & 0xFF;
				uint8_t address = R[Rn];
				for (int i=7; i>-1; i--) {
					if ((register_list >> i) & 1) {
						R[8-i] = memory.read_word(address);
						address += 4;
					}
				}
				// If Rn is unset
				if (~((register_list >> Rn) & 1)) {
					// Write back address
					R[Rn] = address;
				}
			}
			break;

		// Conditional branch, and Supervisor Call
		case 0b110100 ... 0b110111:
			{
				switch ((opcode >> 8) & 0xF) {
					// UDF - Permanently Undefined
					case 1110:
						{
							// TODO
						}
						break;
					// SVC - Supervisor Call
					case 1111:
						{
							uint8_t imm8 = opcode & 0xFF;
							// TODO
						}
						break;
					// B - Conditional Branch - A6.7.10
					default:
						{
							uint8_t cond = (opcode >> 8) & 0xF;
							uint8_t imm8 = opcode & 0xFF;
							bool condMet = 0;
							// ARMv6-M Reference Manual A6.3
							switch (cond) {
								// EQ - Equal
								case 0b0000:
									{
										if (get_flag('Z'))
											condMet = 1;
									}
									break;
								// NE - Not Equal
								case 0b0001:
									{
										if (!get_flag('Z'))
											condMet = 1;
									}
									break;
								// CS - Carry Set
								case 0b0010:
									{
										if (get_flag('C'))
											condMet = 1;
									}
									break;
								// CC - Carry Clear
								case 0b0011:
									{
										if (!get_flag('C'))
											condMet = 1;
									}
									break;
								// MI - Minus, Negative
								case 0b0100:
									{
										if (get_flag('N'))
											condMet = 1;
									}
									break;
								// PL - Plus, Positive or Zero
								case 0b0101:
										if (!get_flag('N'))
											condMet = 1;
									break;
								// VS - Overflow
								case 0b0110:
									{
										if (get_flag('V'))
											condMet = 1;
									}
									break;
								// VC - No Overflow
								case 0b0111:
									{
										if (!get_flag('V'))
											condMet = 1;
									}
									break;
								// HI - Unsigned Higher
								case 0b1000:
									{
										if(get_flag('C') && !get_flag('Z'))
											condMet = 1;
									}
									break;
								// LS - Unsigned Lower or Same
								case 0b1001:
									{
										if(!get_flag('C') && get_flag('Z'))
											condMet = 1;
									}
									break;
								// GE - Signed Greater Than or Equal
								case 0b1010:
									{
										if(get_flag('N') == get_flag('V'))
											condMet = 1;
									}
									break;
								// LT - Signed Less Than
								case 0b1011:
									{
										if(get_flag('N') != get_flag('V'))
											condMet = 1;
									}
									break;
								// GT - Signed Greater Than
								case 0b1100:
									{
										if(!get_flag('Z') && (get_flag('N') == get_flag('V')))
											condMet = 1;
									}
									break;
								// LE - Signed Less Than or Equal
								case 0b1101:
									{
										if(get_flag('Z') || (get_flag('N') != get_flag('V')))
											condMet = 1;
									}
									break;
								// None (AL) - Always (Unconditional)
								// Should never be run as AL only uses T2, undefined behaviour
								case 0b1110:
									{
										condMet = 0;
										*PC += (opcode&0x7FF) * 2 - 2048;
										incrementPC = 0;
									}
									break;
							}
							if (condMet) {
								*PC += imm8 * 2 - 256;	// Keep number between -256 and 254
								incrementPC = 0;
							}
						}
						break;
				}
			}
			break;

		// Unconditional Branch
		case 0b111000 ... 0b111001:
			{
				uint16_t imm11 = opcode & 0x7FF;
				*PC += imm11 * 2 - 2048;
				incrementPC = 0;
			}
			break;
	}
	if (incrementPC)
		*PC += 2;
}

void CM0P_Core::setPC(uint32_t addr) {
	*PC = addr;
}

uint32_t* CM0P_Core::getCoreRegisters() {
	uint32_t* out = (uint32_t*)malloc(sizeof(uint32_t) * 16);
	for (int i=0; i<16; i++) {
		out[i] = R[i];
	}
	return out;
}

CM0P_Memory* CM0P_Core::getMemPtr() {
	return &memory;
}

