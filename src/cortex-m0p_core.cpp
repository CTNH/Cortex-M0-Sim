#include "cortex-m0p_core.h"

CM0P_Core::CM0P_Core() {}

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

void CM0P_Core::update_flag(char flag, bool bit) {
	switch(flag) {
		// Negative Flag
		case 'N':
			break;
		// Zero Flag
		case 'Z':
			break;
		// Carry Flag
		case 'C':
			break;
		// Overflow Flag
		case 'V':
			break;
	}
}

void CM0P_Core::step_inst() {
	uint16_t opcode = memory.read_halfword(R[15]);

	// If first 2-bits are 00
	if (opcode >> 14 == 0) {
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
	}
	// A5.2.2 Data Processing
	else if (opcode >> 10 == 0b010000) {
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
	}
	// A5.2.3 Special data instructions and branch and exchange
	else if (opcode >> 10 == 0b010001) {
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
	}
	// LDR (Literal)
	else if (opcode >> 11 == 0b01001) {
		uint8_t imm8 = opcode & 0xFF;
		uint8_t Rt = (opcode >> 8) & 0b111;
	}
	// A5.2.4 Load/Store single data item
	else if (opcode >> 12 <= 0b1001) {
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
	}
	// ADR (Generate PC-Relative Address)
	else if ((opcode >> 11) == 0b10100) {
		uint8_t Rd = (opcode >> 8) & 0b111;
		uint8_t imm8 = opcode & 0xFF;
		R[Rd] = *PC + imm8;
	}
	// ADD (SP Plus Immediate)
	else if ((opcode >> 11) == 0b10101) {
	}
	else if (opcode >> 12 == 0b1011) {
		switch ((opcode >> 6) & 0b111111) {
			// ADD (SP plus immediate) - Add immediate to SP
			case 0b000000 ... 0b000001:
				break;
			// SUB (SP minus immediate) - Subtract Immediate from SP
			case 0b000010 ... 0b000011:
				break;
			// SXTH - Signed Extend Halfword
			case 0b001000:
				break;
			// SXTB - Signed Extend Byte
			case 0b001001:
				break;
			// UXTH - Unsigned Extend Halfword
			case 0b001010:
				break;
			// UXTB - Unsigned Extend Byte
			case 0b001011:
				break;
			// PUSH - Push Multiple Registers
			case 0b010000 ... 0b010111:
				break;
			// CPS - Change Processor State
			case 0b011001:
				break;
			// REV - Byte-Reverse Word
			case 0b101000:
				break;
			// REV16 - Byte-Reverse Packed Halfword
			case 0b101001:
				break;
			// REVSH - Byte-Reverse Signed Halfword
			case 0b101011:
				break;
			// POP - Pop Multiple Registers
			case 0b110000 ... 0b110111:
				break;
			// BKPT - Breakpoint
			case 0b111000 ... 0b111011:
				break;
			// Hints
			case 0b111100 ... 0b111111:
				break;
		}
	}
}

