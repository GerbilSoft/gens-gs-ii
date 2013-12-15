/***************************************************************************
 * mdZ80: Gens Z80 Emulator                                                *
 * mdZ80.h: Main Z80 emulation functions.                                  *
 *                                                                         *
 * Copyright (c) 1999-2002 by Stéphane Dallongeville                       *
 * Copyright (c) 2003-2004 by Stéphane Akhoun                              *
 * Copyright (c) 2008-2013 by David Korth                                  *
 *                                                                         *
 * This program is free software; you can redistribute it and/or modify it *
 * under the terms of the GNU General Public License as published by the   *
 * Free Software Foundation; either version 2 of the License, or (at your  *
 * option) any later version.                                              *
 *                                                                         *
 * This program is distributed in the hope that it will be useful, but     *
 * WITHOUT ANY WARRANTY; without even the implied warranty of              *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the           *
 * GNU General Public License for more details.                            *
 *                                                                         *
 * You should have received a copy of the GNU General Public License along *
 * with this program; if not, write to the Free Software Foundation, Inc., *
 * 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.           *
 ***************************************************************************/

#ifndef __MDZ80_MDZ80_H__
#define __MDZ80_MDZ80_H__

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

// Z80 context forward-declaration.
struct _mdZ80_context;
typedef struct _mdZ80_context mdZ80_context;

// Z80 function pointer definitions.
typedef uint8_t Z80_RB(uint32_t adr);
typedef void Z80_WB(uint32_t adr, uint8_t data);

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
void mdZ80_Add_Fetch(mdZ80_context *z80, uint8_t low_adr, uint8_t high_adr, const uint8_t *region);

/*! Odometer (clock cycle) functions. **/
unsigned int mdZ80_read_odo(mdZ80_context *z80);
void mdZ80_clear_odo(mdZ80_context *z80);
void mdZ80_set_odo(mdZ80_context *z80, unsigned int odo);
void mdZ80_add_cycles(mdZ80_context *z80, uint32_t cycles);

/*! Interrupt request functions. **/
void mdZ80_nmi(mdZ80_context *z80);
void mdZ80_interrupt(mdZ80_context *z80, uint8_t vector);

/*! Z80 register access. (mdZ80_reg.c) **/

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

int mdZ80_exec(mdZ80_context *z80, int odo);

#ifdef __cplusplus
}
#endif

#endif /* __MDZ80_MDZ80_H__ */
