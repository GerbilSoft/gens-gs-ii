/********************************************************************************/
/*                                                                              */
/* CZ80 (Z80 CPU emulator) version 0.91                                          */
/* Compiled with Dev-C++                                                        */
/* Copyright 2004-2005 Stephane Dallongeville                                   */
/*                                                                              */
/********************************************************************************/

//#ifdef CPUZ80_CZ80_CORE

#include <stdio.h>
#include <string.h>
#include <stddef.h>

#include "cz80.h"
#include "cz80_int.h"
#include "cz80_flags.h"
#include "cz80_context.h"

// Enable Jump Table optimizations on gcc.
#ifdef __GNUC__
#define CZ80_USE_JUMPTABLE      1
#else
#define CZ80_USE_JUMPTABLE      0
#endif


// include macro file
//////////////////////

#define IS_IN_CZ80 1
#include "cz80.inc.c"

// shared global variable
//////////////////////////

static uint8_t flags_init = 0;

static uint8_t SZXY[256];            // zero and sign flags
static uint8_t SZXYP[256];           // zero, sign and parity flags
static uint8_t SZXY_BIT[256];        // zero, sign and parity/overflow (=zero) flags for BIT opcode
static uint8_t SZXYHV_inc[256];      // zero, sign, half carry and overflow flags INC R8
static uint8_t SZXYHV_dec[256];      // zero, sign, half carry and overflow flags DEC R8

// Dummy fetch bank.
static uint8_t Cz80_dummy_fetch[CZ80_FETCH_BANK];

// prototype
/////////////

static uint8_t CZ80CALL Cz80_Read_Dummy(void *ctx, const uint16_t adr);
static void CZ80CALL Cz80_Write_Dummy(void *ctx, const uint16_t adr, uint8_t data);

static uint8_t CZ80CALL Cz80_Interrupt_Ack_Dummy(void *ctx, uint8_t param);
static void CZ80CALL Cz80_RetI_Dummy(void *ctx);

// core main functions
///////////////////////

/**
 * Initialize global flag variables.
 */
static void Cz80_Init_Flags(void)
{
    unsigned int i, j, p;

    // flags tables initialisation
    for (i = 0; i < 256; i++)
    {
        SZXY[i] = i & (CZ80_SF | CZ80_YF | CZ80_XF);
        if (!i) SZXY[i] |= CZ80_ZF;

        SZXY_BIT[i] = i & (CZ80_SF | CZ80_YF | CZ80_XF);
        if (!i) SZXY_BIT[i] |= CZ80_ZF | CZ80_PF;
        
        for (j = 0, p = 0; j < 8; j++) if (i & (1 << j)) p++;
        SZXYP[i] = SZXY[i];
        if (!(p & 1)) SZXYP[i] |= CZ80_PF;
        
        SZXYHV_inc[i] = SZXY[i];
        if(i == 0x80) SZXYHV_inc[i] |= CZ80_VF;
        if((i & 0x0F) == 0x00) SZXYHV_inc[i] |= CZ80_HF;
        
        SZXYHV_dec[i] = SZXY[i] | CZ80_NF;
        if (i == 0x7F) SZXYHV_dec[i] |= CZ80_VF;
        if ((i & 0x0F) == 0x0F) SZXYHV_dec[i] |= CZ80_HF;
    }

    // Dummy fetch bank.
    // Contains all HALT opcodes.
    memset(Cz80_dummy_fetch, 0x76, sizeof(Cz80_dummy_fetch));
}

void Cz80_Init(cz80_struc *cpu)
{
    memset(cpu, 0, sizeof(cz80_struc));

    if (!flags_init)
    {
        // Initialize the flag tables and dummy fetch bank.
        flags_init = 1;
        Cz80_Init_Flags();
    }

    Cz80_Set_Fetch(cpu, 0x0000, 0xFFFF, Cz80_dummy_fetch);

    Cz80_Set_ReadB(cpu, Cz80_Read_Dummy);
    Cz80_Set_WriteB(cpu, Cz80_Write_Dummy);
    Cz80_Set_INPort(cpu, Cz80_Read_Dummy);
    Cz80_Set_OUTPort(cpu, Cz80_Write_Dummy);

    cpu->Interrupt_Ack = Cz80_Interrupt_Ack_Dummy;
    cpu->RetI = Cz80_RetI_Dummy;
}

uint8_t Cz80_Reset(cz80_struc *cpu)
{
    cz80_struc *CPU = cpu;

    // Clear all registers.
    // TODO: Initialize all other registers to random values?
    memset(CPU, 0, offsetof(cz80_struc, CycleSup));

    // Initialize registers as if /RESET was pulsed.
    return Cz80_Soft_Reset(cpu);
}

uint8_t Cz80_Soft_Reset(cz80_struc *cpu)
{
    cz80_struc *CPU = cpu;

    // TODO: Should take 3 clock cycles if running?
    Cz80_Set_PC(CPU, 0);
    EXACT_ONLY(CPU->WZ = 0); /* TODO: Is this correct? */
    zFA = 0xFFFF;
    zSP = 0xFFFF;
    zwR = 0;
    zIFF = 0;
    zI = 0;
    zIM = 0;

    return CPU->Status;
}

/////////////////////////////////

#include "cz80exec.inc.c"

/////////////////////////////////

void CZ80CALL Cz80_Set_IRQ(cz80_struc *cpu, uint8_t vector)
{
    cpu->IntVect = vector;
    cpu->Status |= CZ80_HAS_INT;
    cpu->CycleSup = cpu->CycleIO;
    cpu->CycleIO = 0;
}

void CZ80CALL Cz80_Set_NMI(cz80_struc *cpu)
{
    cpu->Status |= CZ80_HAS_NMI;
    cpu->CycleSup = cpu->CycleIO;
    cpu->CycleIO = 0;
}

void CZ80CALL Cz80_Clear_IRQ(cz80_struc *cpu)
{
    cpu->Status &= ~CZ80_HAS_INT;
}

void CZ80CALL Cz80_Clear_NMI(cz80_struc *cpu)
{
    cpu->Status &= ~CZ80_HAS_NMI;
}

/////////////////////////////////

int CZ80CALL Cz80_Get_CycleToDo(cz80_struc *cpu)
{
    if (!(cpu->Status & CZ80_RUNNING)) return -1;

    return cpu->CycleToDo;
}

int CZ80CALL Cz80_Get_CycleRemaining(cz80_struc *cpu)
{
    if (!(cpu->Status & CZ80_RUNNING)) return -1;

    return (cpu->CycleIO + cpu->CycleSup);
}

int CZ80CALL Cz80_Get_CycleDone(cz80_struc *cpu)
{
    if (!(cpu->Status & CZ80_RUNNING)) return -1;

    return (cpu->CycleToDo - (cpu->CycleIO + cpu->CycleSup));
}

void CZ80CALL Cz80_Release_Cycle(cz80_struc *cpu)
{
    if (cpu->Status & CZ80_RUNNING) cpu->CycleIO = cpu->CycleSup = 0;
}

void CZ80CALL Cz80_Add_Cycle(cz80_struc *cpu, unsigned int cycle)
{
    if (cpu->Status & CZ80_RUNNING) cpu->CycleIO -= cycle;
}

// Read / Write dummy functions
////////////////////////////////

static uint8_t CZ80CALL Cz80_Read_Dummy(void *ctx, const uint16_t adr)
{
    (void)ctx;
    (void)adr;
    return 0;
}

static void CZ80CALL Cz80_Write_Dummy(void *ctx, const uint16_t adr, uint8_t data)
{
	(void)ctx;
	(void)adr;
	(void)data;
}

static uint8_t CZ80CALL Cz80_Interrupt_Ack_Dummy(void *ctx, uint8_t param)
{
    (void)ctx;
    (void)param;

    // return vector
    return -1;
}

static void CZ80CALL Cz80_RetI_Dummy(void *ctx)
{
    (void)ctx;
}


// Read / Write core functions
///////////////////////////////

void Cz80_Set_Ctx(cz80_struc *cpu, void *ctx)
{
    cpu->ctx = ctx;
}

// FIXME: The Word functions use big-endian memory...
uint8_t Cz80_Read_Byte(cz80_struc *cpu, uint16_t adr)
{
    return cpu->Read_Byte(cpu->ctx, adr);
}

uint16_t Cz80_Read_Word(cz80_struc *cpu, uint16_t adr)
{
#if CZ80_USE_WORD_HANDLER
    return cpu->Read_Word(cpu->ctx, adr);
#elif CZ80_LITTLE_ENDIAN
    return (cpu->Read_Byte(cpu->ctx, adr) |
	    (cpu->Read_Byte(cpu->ctx, (adr + 1)) << 8));
#else
    return ((cpu->Read_Byte(cpu->ctx, adr) << 8) |
	    cpu->Read_Byte(cpu->ctx, (adr + 1)));
#endif
}

void Cz80_Write_Byte(cz80_struc *cpu, uint16_t adr, uint8_t data)
{
    cpu->Write_Byte(cpu->ctx, adr, data);
}

void Cz80_Write_Word(cz80_struc *cpu, uint16_t adr, uint16_t data)
{
#if CZ80_USE_WORD_HANDLER
    cpu->Write_Word(cpu->ctx, adr, data);
#elif CZ80_LITTLE_ENDIAN
    cpu->Write_Byte(cpu->ctx, adr, (data & 0xFF));
    cpu->Write_Byte(cpu->ctx, (adr + 1), (data >> 8));
#else
    cpu->Write_Byte(cpu->ctx, adr, (data >> 8));
    cpu->Write_Byte(cpu->ctx, (adr + 1), (data & 0xFF));
#endif
}

// setting core functions
//////////////////////////

void Cz80_Set_Fetch(cz80_struc *cpu, uint16_t low_adr, uint16_t high_adr, void *fetch_adr)
{
    uint16_t i, j;

    i = low_adr >> CZ80_FETCH_SFT;
    j = high_adr >> CZ80_FETCH_SFT;
    fetch_adr = (void *)((uintptr_t)fetch_adr - (i << CZ80_FETCH_SFT));
    while (i <= j) cpu->Fetch[i++] = (uint8_t*) fetch_adr;
}

void Cz80_Set_ReadB(cz80_struc *cpu, CZ80_READ *Func)
{
    cpu->Read_Byte = Func;
}

void Cz80_Set_WriteB(cz80_struc *cpu, CZ80_WRITE *Func)
{
    cpu->Write_Byte = Func;
}

#if CZ80_USE_WORD_HANDLER
void Cz80_Set_ReadW(cz80_struc *cpu, CZ80_READ_WORD *Func)
{
    cpu->Read_Word = Func;
}

void Cz80_Set_WriteW(cz80_struc *cpu, CZ80_WRITE_WORD *Func)
{
    cpu->Write_Word = Func;
}
#endif

void Cz80_Set_INPort(cz80_struc *cpu, CZ80_READ *Func)
{
    cpu->IN_Port = Func;
}

void Cz80_Set_OUTPort(cz80_struc *cpu, CZ80_WRITE *Func)
{
    cpu->OUT_Port = Func;
}

void Cz80_Set_IRQ_Callback(cz80_struc *cpu, CZ80_INT_CALLBACK *Func)
{
    cpu->Interrupt_Ack = Func;
}

void Cz80_Set_RETI_Callback(cz80_struc *cpu, CZ80_RETI_CALLBACK *Func)
{
    cpu->RetI = Func;
}

// externals main functions
////////////////////////////

uint16_t CZ80CALL Cz80_Get_BC(cz80_struc *cpu)
{
    cz80_struc *CPU = cpu;
    return zBC;
}

uint16_t CZ80CALL Cz80_Get_DE(cz80_struc *cpu)
{
    cz80_struc *CPU = cpu;
    return zDE;
}

uint16_t CZ80CALL Cz80_Get_HL(cz80_struc *cpu)
{
    cz80_struc *CPU = cpu;
    return zHL;
}

uint16_t CZ80CALL Cz80_Get_AF(cz80_struc *cpu)
{
    cz80_struc *CPU = cpu;
    return (zF | (zA << 8));
}

uint16_t CZ80CALL Cz80_Get_BC2(cz80_struc *cpu)
{
    cz80_struc *CPU = cpu;
    return zBC2;
}

uint16_t CZ80CALL Cz80_Get_DE2(cz80_struc *cpu)
{
    cz80_struc *CPU = cpu;
    return zDE2;
}

uint16_t CZ80CALL Cz80_Get_HL2(cz80_struc *cpu)
{
    cz80_struc *CPU = cpu;
    return zHL2;
}

uint16_t CZ80CALL Cz80_Get_AF2(cz80_struc *cpu)
{
    cz80_struc *CPU = cpu;
    return (zF2 | (zA2 << 8));
}

uint16_t CZ80CALL Cz80_Get_IX(cz80_struc *cpu)
{
    cz80_struc *CPU = cpu;
    return zIX;
}

uint16_t CZ80CALL Cz80_Get_IY(cz80_struc *cpu)
{
    cz80_struc *CPU = cpu;
    return zIY;
}

uint16_t CZ80CALL Cz80_Get_SP(cz80_struc *cpu)
{
    cz80_struc *CPU = cpu;
    return zSP;
}

#if CZ80_EXACT
uint16_t CZ80CALL Cz80_Get_WZ(cz80_struc *cpu)
{
    cz80_struc *CPU = cpu;
    return CPU->WZ;
}
#endif

uint16_t CZ80CALL Cz80_Get_PC(cz80_struc *cpu)
{
    cz80_struc *CPU = cpu;
    return CPU->PC;
}

uint8_t CZ80CALL Cz80_Get_R(cz80_struc *cpu)
{
    cz80_struc *CPU = cpu;
    return (zR & 0x7F) | (zR2 & 0x80);
}

uint16_t CZ80CALL Cz80_Get_IFF(cz80_struc *cpu)
{
    cz80_struc *CPU = cpu;
    uint16_t value = 0;

    if (zIFF1 & CZ80_IFF) value |= 1;
    if (zIFF2 & CZ80_IFF) value |= 2;
    return value;
}

uint8_t CZ80CALL Cz80_Get_IM(cz80_struc *cpu)
{
    cz80_struc *CPU = cpu;
    return zIM;
}

uint8_t CZ80CALL Cz80_Get_I(cz80_struc *cpu)
{
    cz80_struc *CPU = cpu;
    return zI;
}

uint8_t CZ80CALL Cz80_Get_IntVect(cz80_struc *cpu)
{
    return cpu->IntVect;
}

uint8_t CZ80CALL Cz80_Get_Status(cz80_struc *cpu)
{
    return cpu->Status;
}


void CZ80CALL Cz80_Set_BC(cz80_struc *cpu, uint16_t value)
{
    cz80_struc *CPU = cpu;
    zBC = value;
}

void CZ80CALL Cz80_Set_DE(cz80_struc *cpu, uint16_t value)
{
    cz80_struc *CPU = cpu;
    zDE = value;
}

void CZ80CALL Cz80_Set_HL(cz80_struc *cpu, uint16_t value)
{
    cz80_struc *CPU = cpu;
    zHL = value;
}

void CZ80CALL Cz80_Set_AF(cz80_struc *cpu, uint16_t val)
{
    cz80_struc *CPU = cpu;
    zF = (uint8_t)(val & 0xFF);
    zA = (uint8_t)(val >> 8);
}

void CZ80CALL Cz80_Set_BC2(cz80_struc *cpu, uint16_t value)
{
    cz80_struc *CPU = cpu;
    zBC2 = value;
}

void CZ80CALL Cz80_Set_DE2(cz80_struc *cpu, uint16_t value)
{
    cz80_struc *CPU = cpu;
    zDE2 = value;
}

void CZ80CALL Cz80_Set_HL2(cz80_struc *cpu, uint16_t value)
{
    cz80_struc *CPU = cpu;
    zHL2 = value;
}

void CZ80CALL Cz80_Set_AF2(cz80_struc *cpu, uint16_t val)
{
    cz80_struc *CPU = cpu;
    zF2 = (uint8_t)(val & 0xFF);
    zA2 = (uint8_t)(val >> 8);
}

void CZ80CALL Cz80_Set_IX(cz80_struc *cpu, uint16_t value)
{
    cz80_struc *CPU = cpu;
    zIX = value;
}

void CZ80CALL Cz80_Set_IY(cz80_struc *cpu, uint16_t value)
{
    cz80_struc *CPU = cpu;
    zIY = value;
}

void CZ80CALL Cz80_Set_SP(cz80_struc *cpu, uint16_t value)
{
    cz80_struc *CPU = cpu;
    zSP = value;
}

#if CZ80_EXACT
void CZ80CALL Cz80_Set_WZ(cz80_struc *cpu, uint16_t value)
{
    cz80_struc *CPU = cpu;
    CPU->WZ = value;
}
#endif

void CZ80CALL Cz80_Set_PC(cz80_struc *cpu, uint16_t val)
{
    cpu->BasePC = (uintptr_t) cpu->Fetch[val >> CZ80_FETCH_SFT];
    cpu->PC = val;
}


void CZ80CALL Cz80_Set_R(cz80_struc *cpu, uint8_t value)
{
    cz80_struc *CPU = cpu;
    zR = value & 0x7F;
    zR2 = value & 0x80;
}

void CZ80CALL Cz80_Set_IFF(cz80_struc *cpu, uint16_t value)
{
    cz80_struc *CPU = cpu;
    zIFF = 0;
    if (value & 1) zIFF1 = CZ80_IFF;
    if (value & 2) zIFF2 = CZ80_IFF;
}

void CZ80CALL Cz80_Set_IM(cz80_struc *cpu, uint8_t value)
{
    cz80_struc *CPU = cpu;
    zIM = value & 3;
}

void CZ80CALL Cz80_Set_I(cz80_struc *cpu, uint8_t value)
{
    cz80_struc *CPU = cpu;
    zI = value & 0xFF;
}

void CZ80CALL Cz80_Set_IntVect(cz80_struc *cpu, uint8_t value)
{
    cpu->IntVect = value;
}

void CZ80CALL Cz80_Set_Status(cz80_struc *cpu, uint8_t value)
{
    cpu->Status = value;
}

//#endif // CPUZ80_CZ80_CORE
