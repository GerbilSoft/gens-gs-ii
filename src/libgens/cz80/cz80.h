/********************************************************************************/
/*                                                                              */
/* CZ80 include file                                                            */
/* C Z80 emulator version 0.91 	                                                */
/* Copyright 2004 Stephane Dallongeville                                        */
/*                                                                              */
/********************************************************************************/

/*
  2011-10-18: modified for DGen/SDL.
*/

#ifndef _CZ80_H_
#define _CZ80_H_

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif




/******************************/
/* Compiler dependant defines */
/******************************/

#ifdef _MSC_VER

// MSVC: Enable FASTCALL on i386.
// (Copied from MinGW-w64's winnt.h.)
#if defined (_M_IX86)
#define CZ80CALL __fastcall
#else
#define CZ80CALL
#endif

#else
#define CZ80CALL
#endif



/*************************************/
/* Z80 core Structures & definitions */
/*************************************/

#include "cz80_conf.h"

typedef uint8_t CZ80CALL CZ80_READ(void *ctx, uint16_t adr);
typedef void CZ80CALL CZ80_WRITE(void *ctx, uint16_t adr, uint8_t data);

#if CZ80_USE_WORD_HANDLER
typedef uint16_t CZ80CALL CZ80_READ_WORD(void *ctx, uint16_t adr);
typedef void CZ80CALL CZ80_WRITE_WORD(void *ctx, uint16_t adr, uint16_t data);
#endif

typedef void CZ80CALL CZ80_RETI_CALLBACK(void *ctx);
typedef uint8_t CZ80CALL CZ80_INT_CALLBACK(void *ctx, uint8_t param);

struct _cz80_struc;
typedef struct _cz80_struc cz80_struc;

/*************************/
/* Publics Z80 functions */
/*************************/

void    Cz80_Init(cz80_struc *cpu);
uint8_t Cz80_Reset(cz80_struc *cpu);
uint8_t Cz80_Soft_Reset(cz80_struc *cpu);

/**
 * Set an instruction fetch handler.
 * @param cpu Z80 CPU object.
 * @param low_adr Low address. (16-bit)
 * @param high_adr High address. (16-bit)
 * @param fetch Memory location.
 */
void    Cz80_Set_Fetch(cz80_struc *cpu, uint16_t low_adr, uint16_t high_adr, void *fetch_adr);

void    Cz80_Set_Ctx(cz80_struc *cpu, void *ctx);
void    Cz80_Set_ReadB(cz80_struc *cpu, CZ80_READ *Func);
void    Cz80_Set_WriteB(cz80_struc *cpu, CZ80_WRITE *Func);
#if CZ80_USE_WORD_HANDLER
void    Cz80_Set_ReadW(cz80_struc *cpu, CZ80_READ_WORD *Func);
void    Cz80_Set_WriteW(cz80_struc *cpu, CZ80_WRITE_WORD *Func);
#endif

void    Cz80_Set_INPort(cz80_struc *cpu, CZ80_READ *Func);
void    Cz80_Set_OUTPort(cz80_struc *cpu, CZ80_WRITE *Func);

void    Cz80_Set_IRQ_Callback(cz80_struc *cpu, CZ80_INT_CALLBACK *Func);
void    Cz80_Set_RETI_Callback(cz80_struc *cpu, CZ80_RETI_CALLBACK *Func);

uint8_t Cz80_Read_Byte(cz80_struc *cpu, uint16_t adr);
uint16_t Cz80_Read_Word(cz80_struc *cpu, uint16_t adr);
void    Cz80_Write_Byte(cz80_struc *cpu, uint16_t adr, uint8_t data);
void    Cz80_Write_Word(cz80_struc *cpu, uint16_t adr, uint16_t data);

int     CZ80CALL Cz80_Exec(cz80_struc *cpu, int cycles);

void    CZ80CALL Cz80_Set_IRQ(cz80_struc *cpu, uint8_t vector);
void    CZ80CALL Cz80_Set_NMI(cz80_struc *cpu);
void    CZ80CALL Cz80_Clear_IRQ(cz80_struc *cpu);
void    CZ80CALL Cz80_Clear_NMI(cz80_struc *cpu);

int     CZ80CALL Cz80_Get_CycleToDo(cz80_struc *cpu);
int     CZ80CALL Cz80_Get_CycleRemaining(cz80_struc *cpu);
int     CZ80CALL Cz80_Get_CycleDone(cz80_struc *cpu);
void    CZ80CALL Cz80_Release_Cycle(cz80_struc *cpu);
void    CZ80CALL Cz80_Add_Cycle(cz80_struc *cpu, unsigned int cycle);

uint16_t     CZ80CALL Cz80_Get_BC(cz80_struc *cpu);
uint16_t     CZ80CALL Cz80_Get_DE(cz80_struc *cpu);
uint16_t     CZ80CALL Cz80_Get_HL(cz80_struc *cpu);
uint16_t     CZ80CALL Cz80_Get_AF(cz80_struc *cpu);

uint16_t     CZ80CALL Cz80_Get_BC2(cz80_struc *cpu);
uint16_t     CZ80CALL Cz80_Get_DE2(cz80_struc *cpu);
uint16_t     CZ80CALL Cz80_Get_HL2(cz80_struc *cpu);
uint16_t     CZ80CALL Cz80_Get_AF2(cz80_struc *cpu);

uint16_t     CZ80CALL Cz80_Get_IX(cz80_struc *cpu);
uint16_t     CZ80CALL Cz80_Get_IY(cz80_struc *cpu);
uint16_t     CZ80CALL Cz80_Get_SP(cz80_struc *cpu);

#if CZ80_EXACT
uint16_t     CZ80CALL Cz80_Get_WZ(cz80_struc *cpu);
#endif
uint16_t     CZ80CALL Cz80_Get_PC(cz80_struc *cpu);

uint8_t      CZ80CALL Cz80_Get_R(cz80_struc *cpu);
uint16_t     CZ80CALL Cz80_Get_IFF(cz80_struc *cpu);
uint8_t      CZ80CALL Cz80_Get_IM(cz80_struc *cpu);
uint8_t      CZ80CALL Cz80_Get_I(cz80_struc *cpu);

void    CZ80CALL Cz80_Set_BC(cz80_struc *cpu, uint16_t value);
void    CZ80CALL Cz80_Set_DE(cz80_struc *cpu, uint16_t value);
void    CZ80CALL Cz80_Set_HL(cz80_struc *cpu, uint16_t value);
void    CZ80CALL Cz80_Set_AF(cz80_struc *cpu, uint16_t value);

void    CZ80CALL Cz80_Set_BC2(cz80_struc *cpu, uint16_t value);
void    CZ80CALL Cz80_Set_DE2(cz80_struc *cpu, uint16_t value);
void    CZ80CALL Cz80_Set_HL2(cz80_struc *cpu, uint16_t value);
void    CZ80CALL Cz80_Set_AF2(cz80_struc *cpu, uint16_t value);

void    CZ80CALL Cz80_Set_IX(cz80_struc *cpu, uint16_t value);
void    CZ80CALL Cz80_Set_IY(cz80_struc *cpu, uint16_t value);
void    CZ80CALL Cz80_Set_SP(cz80_struc *cpu, uint16_t value);

#if CZ80_EXACT
void    CZ80CALL Cz80_Set_WZ(cz80_struc *cpu, uint16_t value);
#endif
void    CZ80CALL Cz80_Set_PC(cz80_struc *cpu, uint16_t value);

void    CZ80CALL Cz80_Set_R(cz80_struc *cpu, uint8_t value);
void    CZ80CALL Cz80_Set_IFF(cz80_struc *cpu, uint16_t value);
void    CZ80CALL Cz80_Set_IM(cz80_struc *cpu, uint8_t value);
void    CZ80CALL Cz80_Set_I(cz80_struc *cpu, uint8_t value);

#ifdef __cplusplus
}
#endif

#endif  // _CZ80_H_
