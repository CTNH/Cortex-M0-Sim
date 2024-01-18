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
				break;

			// Compare Registers / Unpredictable
			case 0b01:
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
		}
	}
}

