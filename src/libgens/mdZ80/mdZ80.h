/**********************************************************/
/*                                                        */
/* Z80 emulator 0.99                                      */
/* Copyright 2002 Stéphane Dallongeville                  */
/* Used for the genesis emulation in Gens                 */
/*                                                        */
/**********************************************************/

#ifndef __MDZ80_H
#define __MDZ80_H


#include "../macros/fastcall.h"
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif


/****************************/
/* Structures & definitions */
/****************************/


typedef uint8_t FASTCALL Z80_RB(uint32_t adr);
typedef void FASTCALL Z80_WB(uint32_t adr, uint8_t data);


struct _Z80_context
{
	union
	{
		struct
		{
			uint8_t A;
			uint8_t F;
			uint8_t x;
			uint8_t FXY;
		} b;
		struct
		{
			uint16_t AF;
			uint16_t FXYW;
		} w;
		uint32_t d;
	} AF;
	union
	{
		struct
		{
			uint8_t C;
			uint8_t B;
		} b;
		uint16_t w;
	} BC;
	union
	{
		struct
		{
			uint8_t E;
			uint8_t D;
		} b;
		uint16_t w;
	} DE;
	union
	{
		struct
		{
			uint8_t L;
			uint8_t H;
		} b;
		uint16_t w;
	} HL;
	union
	{
		struct
		{
			uint8_t IXL;
			uint8_t IXH;
		} b;
		uint16_t w;
	} IX;
	union
	{
		struct
		{
			uint8_t IYL;
			uint8_t IYH;
		} b;
		uint16_t w;
	} IY;
	uint16_t reserved_reg;	// Reserved for struct alignment.
	
	uint32_t PC;	// PC == BasePC + Z80 PC [x86 pointer!]
	
	union
	{
		struct
		{
			uint8_t SPL;
			uint8_t SPH;
		} b;
		uint16_t w;
	} SP;
	uint16_t reserved_sp;	// Reserved for struct alignment.
	
	union
	{
		struct
		{
			uint8_t A2;
			uint8_t F2;
			uint8_t x;
			uint8_t FXY2;
		} b;
		struct
		{
			uint16_t AF2;
			uint16_t FXYW2;
		} w;
		uint32_t d;
	} AF2;
	uint16_t BC2;
	uint16_t DE2;
	uint16_t HL2;
	uint16_t reserved_reg2;	// Reserved for struct alignment.
	
	// Internal registers.
	uint8_t IFF;		// Interrupt flip-flops.
	uint8_t R;		// Refresh register.
	
	// Interrupt registers.
	uint8_t I;		// Interrupt vector page. (IM 2)
	uint8_t IM;		// Interrupt mode.
	uint8_t IntVect;	// Interrupt vector. (IM 0, IM 2)
	uint8_t IntLine;	// Interrupt line. (0x01 == INT; 0x80 == NMI)
	
	// Z80 status flags.
	uint8_t Status;
	uint8_t reserved_stat;	// Reserved for struct alignment.
	
	uint32_t BasePC;	// Pointer to x86 memory location where Z80 RAM starts.
	
	uint32_t CycleCnt;
	uint32_t CycleTD;
	uint32_t CycleIO;
	uint32_t CycleSup;
	uint8_t *Fetch[0x100];
	
	Z80_RB *ReadB;
	Z80_WB *WriteB;
	
	Z80_RB *IN_C;
	Z80_WB *OUT_C;
};

typedef struct _Z80_context Z80_CONTEXT;

// Z80 status flags.
#define Z80_RUNNING	0x01
#define Z80_HALTED	0x02
#define Z80_FAULTED	0x10


/**
 * Z80 functions (asm)
 */


//uint32_t z80_Init(Z80_CONTEXT *z80);
//uint32_t z80_Reset(Z80_CONTEXT *z80);

void mdZ80_Set_ReadB(Z80_CONTEXT *z80, Z80_RB *func);
void mdZ80_Set_WriteB(Z80_CONTEXT *z80, Z80_WB *func);
void mdZ80_Set_In(Z80_CONTEXT *z80, Z80_RB *func);
void mdZ80_Set_Out(Z80_CONTEXT *z80, Z80_WB *func);
void mdZ80_Add_Fetch(Z80_CONTEXT *z80, uint8_t low_adr, uint8_t high_adr, uint8_t *region);

//uint32_t z80_Read_Odo(Z80_CONTEXT *z80);
//void z80_Clear_Odo(Z80_CONTEXT *z80);
//void z80_Set_Odo(Z80_CONTEXT *z80, uint32_t Odo);
//void z80_Add_Cycles(Z80_CONTEXT *z80, uint32_t cycles);

uint32_t z80_Exec(Z80_CONTEXT *z80, int odo);


/**
 * Z80 functions (ported to C)
 */


void mdZ80_init(Z80_CONTEXT *z80);
void mdZ80_reset(Z80_CONTEXT *z80);


unsigned int mdZ80_get_PC(Z80_CONTEXT *z80);
unsigned int mdZ80_get_AF(Z80_CONTEXT *z80);
unsigned int mdZ80_get_AF2(Z80_CONTEXT *z80);
void mdZ80_set_PC(Z80_CONTEXT *z80, unsigned int PC);
void mdZ80_set_AF(Z80_CONTEXT *z80, uint32_t AF);
void mdZ80_set_AF2(Z80_CONTEXT *z80, uint32_t AF2);

unsigned int mdZ80_read_odo(Z80_CONTEXT *z80);
static inline void mdZ80_clear_odo(Z80_CONTEXT *z80)
{
	z80->CycleCnt = 0;
}
void mdZ80_set_odo(Z80_CONTEXT *z80, unsigned int odo);
void mdZ80_add_cycles(Z80_CONTEXT *z80, uint32_t cycles);

//uint32_t FASTCALL mdZ80_Exec(Z80_CONTEXT *z80, int odo);

void mdZ80_nmi(Z80_CONTEXT *z80);
void mdZ80_interrupt(Z80_CONTEXT *z80, unsigned char vector);


/**
 * Default read/write functions.
 */
uint8_t FASTCALL mdZ80_def_ReadB(uint32_t address);
uint8_t FASTCALL mdZ80_def_In(uint32_t address);
void FASTCALL mdZ80_def_WriteB(uint32_t address, uint8_t data);
void FASTCALL mdZ80_def_Out(uint32_t address, uint8_t data);


#ifdef __cplusplus
}
#endif

#endif /* __MDZ80_H */
