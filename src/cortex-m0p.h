#ifndef CORTEXM0P_H
#define CORTEXM0P_H

/*
 *	The following instructions list is from the ARM developer
 *	https://developer.arm.com/documentation/dui0662/b/The-Cortex-M0--Instruction-Set/
 */
void adcs();	// Add with carry
void add();		// Add
void adr();		// PC-relative Address to Register
void ands();	// Bitwise AND
void asrs();	// Arithmetic Shift Right
void b();		// Branch (conditionally)
void bics();	// Bit clear
void bkpt();	// Breakpoint
void bl();		// Branch with link
void blx();		// Branch indirect with link
void bx();		// Branch indirect
void cmn();		// Compare negative
void CMP();		// Compare
void CPSID();	// Change processor state, disable interrupts
void CPSIE();	// Change processor state, enable interrupts
void DMB();		// Data memory barrier
void DSB();		// Data synchronization barrier
void EORS();	// Exclusive OR
void ISB();		// Instruction synchronization barrier
void LDM();		// Load multiple registers, increment after
void LDR();		// Load register from PC-relative address
void LDRB();	// Load register with byte
void LDRH();	// Load register with halfword
void LDRSB();	// Load register with signed byte
void LDRSH();	// Load register with signed halfword
void LSLS();	// Logical Shift Left
void LSRS();	// Logical Shift Right
void MOV();		// Move
void MRS();		// Move to general register from speical register
void MSR();		// Move to speical register from general register
void MULS();	// Multiply, 32-bit result
void MVNS();	// Bitwise NOT
void NOP();		// No Operation
void ORRS();	// Logical OR
void POP();		// Pop registers from stack
void PUSH();	// Push registers onto stack
void REV();		// Byte-Reverse word
void REV16();	// Byte-Reverse packed halfwords
void REVSH();	// Byte-Reverse signed halfword
void RORS();	// Rotate Right
void RSBS();	// Rotate Subtract
void SBCS();	// Subtract with Carry
void SEV();		// Send Event
void STM();		// Store Multiple registers, increment after
void STR();		// Store Register as word
void STRB();	// Store Register as byte
void STRH();	// Store Register as halfword
void SUB();		// Subtract
void SVC();		// Supervisor Call
void SXTB();	// Sign extend byte
void SXTH();	// Sign extend halfword
void TST();		// Logical AND based test
void UXTB();	// Zero extend a byte
void UXTH();	// Zero extend a halfword
void WFE();		// Wait For Event
void WFI();		// Wait For Interrupt


// Produce a unique hash for each instruction
int instructionHash(char* inst);
// Checks if the instruction hash will produce unique hashes
int hashUniqueCheck();
// Executes the instruction passed through
int execInstruction(char* inst);

#endif

