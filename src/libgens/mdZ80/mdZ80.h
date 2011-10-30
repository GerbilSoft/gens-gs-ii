/**********************************************************/
/*                                                        */
/* Z80 emulator 0.99                                      */
/* Copyright 2002 Stéphane Dallongeville                  */
/* Used for the genesis emulation in Gens                 */
/*                                                        */
/**********************************************************/

#ifndef __MDZ80_H__
#define __MDZ80_H__

#include "../macros/fastcall.h"
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

// Z80 context forward-declaration.
struct _mdZ80_context;
typedef struct _mdZ80_context mdZ80_context;

// Z80 function pointer definitions.
typedef uint8_t FASTCALL Z80_RB(uint32_t adr);
typedef void FASTCALL Z80_WB(uint32_t adr, uint8_t data);

/*! Z80 context allocation. **/

/**
 * Create a new Z80 context.
 * @return New Z80 context, or NULL on error.
 */
mdZ80_context *mdZ80_new(void);

/**
 * Free a Z80 context.
 * @param z80 Pointer to Z80 context.
 */
void mdZ80_free(mdZ80_context *z80);

/**
 * Reset the Z80 CPU. (Hard Reset)
 * This resets *all* registers to their initial states.
 * @param z80 Pointer to Z80 context.
 */
void mdZ80_hard_reset(mdZ80_context *z80);

/**
 * Reset the Z80 CPU.
 * This is equivalent to asserting the !RESET line.
 * @param z80 Pointer to Z80 context.
 */
void mdZ80_soft_reset(mdZ80_context *z80);


/*! Set memory and I/O read/write functions. **/
void mdZ80_Set_ReadB(mdZ80_context *z80, Z80_RB *func);
void mdZ80_Set_WriteB(mdZ80_context *z80, Z80_WB *func);
void mdZ80_Set_In(mdZ80_context *z80, Z80_RB *func);
void mdZ80_Set_Out(mdZ80_context *z80, Z80_WB *func);
void mdZ80_Add_Fetch(mdZ80_context *z80, uint8_t low_adr, uint8_t high_adr, uint8_t *region);

/*! Odometer (clock cycle) functions. **/
unsigned int mdZ80_read_odo(mdZ80_context *z80);
void mdZ80_clear_odo(mdZ80_context *z80);
void mdZ80_set_odo(mdZ80_context *z80, unsigned int odo);
void mdZ80_add_cycles(mdZ80_context *z80, uint32_t cycles);

/*! Interrupt request functions. **/
void mdZ80_nmi(mdZ80_context *z80);
void mdZ80_interrupt(mdZ80_context *z80, unsigned char vector);

/*! Default read/write functions. **/
uint8_t FASTCALL mdZ80_def_ReadB(uint32_t address);
uint8_t FASTCALL mdZ80_def_In(uint32_t address);
void FASTCALL mdZ80_def_WriteB(uint32_t address, uint8_t data);
void FASTCALL mdZ80_def_Out(uint32_t address, uint8_t data);

/*! Z80 register access. **/

uint16_t mdZ80_get_AF(mdZ80_context *z80);
uint16_t mdZ80_get_BC(mdZ80_context *z80);
uint16_t mdZ80_get_DE(mdZ80_context *z80);
uint16_t mdZ80_get_HL(mdZ80_context *z80);
uint16_t mdZ80_get_IX(mdZ80_context *z80);
uint16_t mdZ80_get_IY(mdZ80_context *z80);
uint16_t mdZ80_get_PC(mdZ80_context *z80);
uint16_t mdZ80_get_SP(mdZ80_context *z80);
uint16_t mdZ80_get_AF2(mdZ80_context *z80);
uint16_t mdZ80_get_BC2(mdZ80_context *z80);
uint16_t mdZ80_get_DE2(mdZ80_context *z80);
uint16_t mdZ80_get_HL2(mdZ80_context *z80);
uint8_t  mdZ80_get_IFF(mdZ80_context *z80);
uint8_t  mdZ80_get_R(mdZ80_context *z80);
uint8_t  mdZ80_get_I(mdZ80_context *z80);
uint8_t  mdZ80_get_IM(mdZ80_context *z80);
uint8_t  mdZ80_get_IntVect(mdZ80_context *z80);
uint8_t  mdZ80_get_IntLine(mdZ80_context *z80);
uint8_t  mdZ80_get_Status(mdZ80_context *z80);

void mdZ80_set_AF(mdZ80_context *z80, uint16_t data);
void mdZ80_set_BC(mdZ80_context *z80, uint16_t data);
void mdZ80_set_DE(mdZ80_context *z80, uint16_t data);
void mdZ80_set_HL(mdZ80_context *z80, uint16_t data);
void mdZ80_set_IX(mdZ80_context *z80, uint16_t data);
void mdZ80_set_IY(mdZ80_context *z80, uint16_t data);
void mdZ80_set_PC(mdZ80_context *z80, uint16_t data);
void mdZ80_set_SP(mdZ80_context *z80, uint16_t data);
void mdZ80_set_AF2(mdZ80_context *z80, uint16_t data);
void mdZ80_set_BC2(mdZ80_context *z80, uint16_t data);
void mdZ80_set_DE2(mdZ80_context *z80, uint16_t data);
void mdZ80_set_HL2(mdZ80_context *z80, uint16_t data);
void mdZ80_set_IFF(mdZ80_context *z80, uint8_t data);
void mdZ80_set_R(mdZ80_context *z80, uint8_t data);
void mdZ80_set_I(mdZ80_context *z80, uint8_t data);
void mdZ80_set_IM(mdZ80_context *z80, uint8_t data);
void mdZ80_set_IntVect(mdZ80_context *z80, uint8_t data);
void mdZ80_set_IntLine(mdZ80_context *z80, uint8_t data);
void mdZ80_set_Status(mdZ80_context *z80, uint8_t data);

/*! Z80 main execution loop. **/

uint32_t z80_Exec(mdZ80_context *z80, int odo);

#ifdef __cplusplus
}
#endif

#endif /* __MDZ80_H__ */
