/********************************************************************************/
/*                                                                              */
/* CZ80 (Z80 CPU emulator) version 0.91                                         */
/* Compiled with Dev-C++                                                        */
/* Copyright 2004-2005 Stephane Dallongeville                                   */
/*                                                                              */
/********************************************************************************/

//#ifdef CPUZ80_CZ80_CORE

#include <stdio.h>
#include <string.h>
#include <stddef.h>
#include <stdlib.h>

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
static uint8_t Cz80_dummy_fetch[CZ80_FETCH_BANK_SIZE];

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

/**
 * Allocate and initialize a CZ80 structure.
 * @return CZ80 structure.
 */
cz80_struc *Cz80_Alloc(void)
{
    cz80_struc *cpu = (cz80_struc*)malloc(sizeof(*cpu));
    Cz80_Init(cpu);
    return cpu;
}

/**
 * Free a CZ80 structure.
 * @param cpu CZ80 structure.
 */
void Cz80_Free(cz80_struc *cpu)
{
    free(cpu);
}

/////////////////////////////////

#include "cz80exec.inc.c"

/////////////////////////////////

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

//#endif // CPUZ80_CZ80_CORE
