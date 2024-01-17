#include "cortex-m0p_core.h"

CM0P_Core::CM0P_Core() {}


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
			// Logical Shift Left
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

			// CMP Compare
			case 0b101:
				{
					uint8_t Rn = (opcode >> 8) & 0b111;
					uint8_t imm8 = opcode & 0xFF;
					uint32_t result = R[Rn] - imm8;
					update_flag('N', result >> 31);	// Value of MSb
					update_flag('Z', result == 0);
					update_flag('C', R[Rn] >= imm8);	// Result of subtraction >= 0
					update_flag(
						'V',
						((result >> 31) && ~(R[Rn] >> 31) && (imm8 >> 31)) ||
						(~(result >> 31) && (R[Rn] >> 31) && ~(imm8 >> 31))
					);
				}
				break;

			// ADDS Add 8-bit immediate
			case 0b110:
				{
					uint8_t Rdn = (opcode >> 8) & 0b111;
					uint8_t imm8 = opcode & 0xFF;
					uint32_t result = R[Rdn] + imm8;

					update_flag('N', result >> 31);	// Value of MSb
					update_flag('Z', result == 0);
					update_flag('C', R[Rdn] + imm8 > 0xFFFFFFFF);
					update_flag(
						'V',
						((result >> 31) && ~(R[Rdn] >> 31) && ~(imm8 >> 31)) ||
						(~(result >> 31) && (R[Rdn] >> 31) && (imm8 >> 31))
					);
					R[Rdn] = result;
				}
				break;

			// SUBS Subtract 8-bit immediate
			case 0b111:
				{
					uint8_t Rdn = (opcode >> 8) & 0b111;
					uint8_t imm8 = opcode & 0xFF;
					uint32_t result = R[Rdn] - imm8;

					update_flag('N', result >> 31);	// Value of MSb
					update_flag('Z', result == 0);
					update_flag('C', R[Rdn] >= imm8);	// Result of subtraction >= 0
					update_flag(
						'V',
						((result >> 31) && ~(R[Rdn] >> 31) && (imm8 >> 31)) ||
						(~(result >> 31) && (R[Rdn] >> 31) && ~(imm8 >> 31))
					);
					R[Rdn] = result;
				}
				break;

			case 0b011:
				switch (opcode >> 9) {
					// Add register
					case 0b01100:
						break;

					// Subtract register
					case 0b01101:
						break;

					// Add 3-bit immediate
					case 0b01110:
						break;

					// Subtract 3-bit immediate
					case 0b01111:
						break;
				}
				break;

		}
	}
	// A5.2.2 Data Processing
	else if (opcode >> 10 == 0b010000) {
		switch ((opcode >> 6) & 0xF) {
			// Bitwise AND
			case 0b0000:
				break;

			// Exclusive OR
			case 0b0001:
				break;

			// Logical Shift Left
			case 0b0010:
				break;

			case 0b0011:
				break;

			case 0b0100:
				break;

			case 0b0101:
				break;

			case 0b0110:
				break;

			case 0b0111:
				break;

			case 0b1000:
				break;

			case 0b1001:
				break;

			case 0b1010:
				break;

			case 0b1011:
				break;

			case 0b1100:
				break;

			case 0b1101:
				break;

			case 0b1110:
				break;

			case 0b1111:
				break;

		}
	}
	else if (opcode >> 10 == 0b010001) {
		switch ((opcode >> 6) & 0xF) {
		}
	}
}

