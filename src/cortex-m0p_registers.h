// #include <cstdint>
#include <stdint.h>

typedef struct {
	// dgug Page 3-9
	uint32_t APSR;		// Application Program Status Register
						// Bit 31:	N (Negative Flag)		1 when result of operation is negative, 0 if not
						// Bit 30:	Z (Zero Flag)			1 when result of operation is 0, 0 if not
						// Bit 29:	C (Carry/Borrow Flag)	1 if operation resulted in carry, 0 if not
						// Bit 28:	V (Overflow Flag)		1 if operation caused overflow, 0 if not
						// Bit 0-27: Reserved
	uint32_t IPSR;		// Interrupt Program Status Register
	uint32_t EPSR;		// Execution Program Status Register
} ProgramStatus_Register;

typedef struct {
	// R0-R12 General Purpose Registers for Data Operations
	uint32_t R0;
	uint32_t R1;
	uint32_t R2;
	uint32_t R3;
	uint32_t R4;
	uint32_t R5;
	uint32_t R6;
	uint32_t R7;
	uint32_t R8;
	uint32_t R9;
	uint32_t R10;
	uint32_t R11;
	uint32_t R12;
	// Stack Pointers (R13); In Thread mode CONTROL register indicates which to use
	uint32_t MSP;	// Main Stack Pointer
	uint32_t PSP;	// Process Stack Pointer
	uint32_t LR;	// R14 Link Register; Stores return info of subroutines, function calls and exceptions
	uint32_t PC;	// R15 Program Counter
	ProgramStatus_Register PSR;
	uint32_t PRIMASK;	// Prevents activation of all exception with configurable priority
	uint32_t CONTROL;	// Controls stack used, optionally code privilege level when in thread mode
} ProcessorCore_Register;

// Cortex-M0+ Technical Reference Manual
typedef struct __attribute__((__packed__)) {
	uint32_t SYST_CSR;		// SysTick Control and Status Register
	uint32_t SYST_RVR;		// SysTick Reload Value Register
	uint32_t SYST_CVR;		// SysTick Current Value Register
	uint32_t SYST_CALIB;	// SysTick Calibration value Register
	uint32_t CPUID;
	uint32_t ICSR;			// Interrupt Control State Register
	uint32_t AIRCR;			// Application Interrupt and Reset Control Register
	uint32_t CCR;			// Configuration and Control Register
	uint32_t SHPR2;			// System Handler Priority Register 2
	uint32_t SHPR3;			// System Handler Priority Register 3
	uint32_t SHCSR;			// System Handler Control and State Register
	uint32_t VTOR;			// Vector Table Offset Register
	uint32_t ACTLR;			// Auxiliary Control Register
} SystemControl_Register;


// Nested Vectored Interrupt Controller
typedef struct __attribute__((__packed__)) {
	uint32_t ISER;	// Interrupt Set-Enable Register
	uint32_t ICER;	// Interrupt Clear-Enable Register
	uint32_t ISPR;	// Interrupt Set-Pending Register
	uint32_t ICPR;	// Interrupt Clear-Pending Register
	// Interrupt Priority Registers
	uint32_t IPR0;
	uint32_t IPR1;
	uint32_t IPR2;
	uint32_t IPR3;
	uint32_t IPR4;
	uint32_t IPR5;
	uint32_t IPR6;
	uint32_t IPR7;
} NVIC_Register;


// Memory Protection Unit
typedef struct __attribute__((__packed__)) {
	uint32_t TYPE;
	uint32_t CTRL;
	uint32_t RNR;
	uint32_t RBAR;
	uint32_t RASR;
} MPU_Register;

