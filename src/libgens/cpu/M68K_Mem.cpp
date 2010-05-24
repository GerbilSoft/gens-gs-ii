/***************************************************************************
 * libgens: Gens Emulation Library.                                        *
 * M68K_Mem.cpp: Main 68000 memory handler.                                *
 *                                                                         *
 * Copyright (c) 1999-2002 by Stéphane Dallongeville.                      *
 * Copyright (c) 2003-2004 by Stéphane Akhoun.                             *
 * Copyright (c) 2008-2010 by David Korth.                                 *
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

#include "M68K_Mem.hpp"
#include "MD/VdpIo.hpp"

namespace LibGens
{

/** ROM and RAM variables. **/
M68K_Mem::Ram_68k_t M68K_Mem::Ram_68k;
M68K_Mem::Rom_Data_t M68K_Mem::Rom_Data;

/** SRam variables. **/
uint8_t  M68K_Mem::SRam[64 * 1024];
uint32_t M68K_Mem::SRam_Start;
uint32_t M68K_Mem::SRam_End;

M68K_Mem::SRam_State_t M68K_Mem::SRam_State;


/** Read Byte functions. **/


/**
 * M68K_Read_Byte_Default(): Default M68K read byte handler.
 * @param address Address.
 * @return 0x00.
 */
uint8_t M68K_Mem::M68K_Read_Byte_Default(uint32_t address)
{
	((void)address);
	return 0x00;
}


/**
 * T_M68K_Read_Byte_RomX(): Read a byte from ROM bank X.
 * M68K memory space is split into 32 512 KB banks. (16 MB total)
 * TODO: XOR by 1 on little-endian systems only.
 * @param bank ROM bank number.
 */
template<uint8_t bank>
uint8_t M68K_Mem::T_M68K_Read_Byte_RomX(uint32_t address)
{
	address &= 0x7FFFF;
	address ^= ((bank << 19) | 1);
	return Rom_Data.u8[address];
}
// TODO: Add banks C, D, E, and F for 8 MB ROM support.
// For now, they will return 0x00.


/**
 * M68K_Read_Byte_Rom4(): Read a byte from ROM bank 4. (0x200000 - 0x27FFFF)
 * This ROM bank may be SRam.
 * @param address Address.
 * @return Byte from ROM or SRam.
 */
uint8_t M68K_Mem::M68K_Read_Byte_Rom4(uint32_t address)
{
	// Mask off the high byte of the address.
	address &= 0xFFFFFF;
	
	// Check if this is an SRam data request.
	if (SRam_State.on && SRam_State.enabled &&
	    address >= SRam_Start && address <= SRam_End)
	{
		// SRam data request.
		if (SRam_State.custom)
		{
			// Custom SRam.
			// TODO: The original Gens code simply returns 0.
			// I have no idea what this is supposed to do.
			return 0;
		}
		
		// Return the byte from SRam.
		// Note: SRam is NOT byteswapped.
		// TODO: Check boundaries.
		return SRam[address - SRam_Start];
	}
	
	// ROM data request.
	address &= 0x7FFFF;
	address ^= ((4 << 19) | 1);	// TODO: LE only!
	return Rom_Data.u8[address];
}


/**
 * M68K_Read_Byte_Ram(): Read a byte from RAM. (0xE00000 - 0xFFFFFF)
 * RAM is 64 KB, mirrored throughout the entire range.
 * @param address Address.
 * @return Byte from RAM.
 */
uint8_t M68K_Mem::M68K_Read_Byte_Ram(uint32_t address)
{
	address &= 0xFFFF;
	address ^= 1;	// TODO: LE only!
	return Ram_68k.u8[address];
}


/**
 * M68K_Read_Byte_Misc(): Read a byte from the miscellaneous data bank. (0xA00000 - 0xA7FFFF)
 * This includes Z80 memory, Z80 control registers, and gamepads.
 * @param address Address.
 * @return Miscellaneous data byte.
 */
uint8_t M68K_Mem::M68K_Read_Byte_Misc(uint32_t address)
{
	// Mask off the high byte of the address.
	address &= 0xFFFFFF;
	
	if (address <= 0xA0FFFF)
	{
		// Z80 memory space.
		if (Z80_State & (Z80_STATE_BUSREQ | Z80_STATE_RESET))
		{
			// Z80 is either running or has the bus.
			// Don't do anything.
			// TODO: I don't think the Z80 needs to be stopped here...
			return 0;
		}
		
		// Call the Z80 read function.
		// TODO
#if 0
		push	ecx
		push	edx
		mov	ecx, ebx
		and	ebx, 0x7000
		and	ecx, 0x7FFF
		shr	ebx, 10
		call	[SYM(Z80_ReadB_Table) + ebx]
		pop	edx
		pop	ecx
		ret
#endif
		return 0;
	}
	else if (address == 0xA11100)
	{
		// Z80 BUSREQ. (0xA11100)
		if (Z80_State & Z80_STATE_BUSREQ)
		{
			// Z80 is currently running.
			return 0x81;
		}
		
		// Z80 is not running.
		// TODO
#if 0
	.z80_off:
		call	SYM(main68k_readOdometer)
		sub	eax, [SYM(Last_BUS_REQ_Cnt)]
		cmp	eax, CYCLE_FOR_TAKE_Z80_BUS_GENESIS
		ja	short .bus_taken
		
		movzx	eax, byte [SYM(Last_BUS_REQ_St)]
		pop	ebx
		or	al, 0x80
		ret
		
	.bus_taken:
		mov	eax, 0x80
		pop	ebx
		ret
#endif
		return 0;
	}
	else if (address > 0xA1000D)
	{
		// Invalid address.
		return 0;
	}
	
	// MD miscellaneous registers.
	switch (address & 0x00000E)
	{
		case 0x00:
			/**
			 * 0xA10000/0xA10001: Genesis version register.
			 * Format: [MODE VMOD DISK RSV VER3 VER2 VER1 VER0]
			 * MODE: Region. (0 == East; 1 == West)
			 * VMOD: Video mode. (0 == NTSC; 1 == PAL)
			 * DISK: Floppy disk drive. (0 == connected; 1 == not connected.)
			 * RSV: Reserved. (I think this is used for SegaCD.)
			 * VER 3-0: HW version. (0 == no TMSS; 1 = TMSS)
			 * TODO: Set VER to 1 once TMSS support is added, if TMSS is enabled.
			 */
			return ((Game_Mode << 7) | (CPU_Mode << 6) | 0x20);
		
		case 0x02:
			/**
			 * 0xA10002/0xA10003: Control Port 1: Data.
			 */
			// TODO
			//return RD_Controller_1();
			return 0xFF;
		
		case 0x04:
			/**
			 * 0xA10004/0xA10005: Control Port 2: Data.
			 */
			// TODO
			//return RD_Controller_2();
			return 0xFF;
		
		case 0x06:
			/**
			 * 0xA10006/0xA10007: Control Port 3: Data. (EXT)
			 * NOTE: NOT IMPLEMENTED!
			 */
			return 0xFF;
		
		case 0x08:
			/**
			 * 0xA10008/0xA10009: Control Port 1: CTRL.
			 */
			// TODO
			//return Controller_1_COM;
			return 0xFF;
		
		case 0x0A:
			/**
			 * 0xA1000A/0xA1000B: Control Port 2: CTRL.
			 */
			// TODO
			//return Controller_2_COM;
			return 0xFF;
		
		case 0x0C:
			/**
			 * 0xA1000C/0xA1000D: Control Port 3: CTRL. (EXT)
			 * NOTE: NOT IMPLEMENTED!
			 */
			return 0x00;
		
		default:
			// Unknown register.
			return 0x00;
	}
}


/**
 * M68K_Read_Byte_VDP(): Read a byte from the VDP data banks. (0xC00000 - 0xDFFFFF)
 * @param address Address.
 * @return VDP data byte.
 */
uint8_t M68K_Mem::M68K_Read_Byte_VDP(uint32_t address)
{
	// Valid address: ((address & 0xE700E0) == 0xC00000)
	// Information from vdppin.txt, (c) 2008 Charles MacDonald.
	
	// Since this function is only called if address is in the
	// VDP data banks, we can just check if ((address & 0x700E0) == 0).
	if ((address & 0x700E0) != 0)
	{
		// Not a valid VDP address.
		return 0x00;
	}
	
	// Check the VDP address.
	address &= 0x1F;
	if (address < 0x04)
	{
		// Invalid address.
		// NOTE: VDP Data port should still be readable...
		return 0x00;
	}
	else if (address < 0x08)
	{
		// 0xC00004 - 0xC00007: VDP Control Port.
		uint16_t vdp_status = VdpIo::Read_Status();
		if (address & 0x01)
		{
			// Odd address; return the high byte.
			return ((vdp_status >> 8) & 0xFF);
		}
		else
		{
			// Even address; return the low byte.
			return (vdp_status & 0xFF);
		}
	}
	else if (address == 0x08)
	{
		// 0xC00008: V counter.
		return VdpIo::Read_V_Counter();
	}
	else if (address == 0x09)
	{
		// 0xC00009: H counter.
		return VdpIo::Read_H_Counter();
	}
	
	// Invalid VDP address.
	return 0x00;
}


/** Read Word functions. **/


/**
 * M68K_Read_Word_Default(): Default M68K read word handler.
 * @param address Address.
 * @return 0x0000.
 */
uint16_t M68K_Mem::M68K_Read_Word_Default(uint32_t address)
{
	((void)address);
	return 0x0000;
}


/**
 * T_M68K_Read_Byte_RomX(): Read a word from ROM bank X.
 * M68K memory space is split into 32 512 KB banks. (16 MB total)
 * @param bank ROM bank number.
 */
template<uint8_t bank>
uint16_t M68K_Mem::T_M68K_Read_Word_RomX(uint32_t address)
{
	address &= 0x7FFFE;
	address ^= ((bank << 19) >> 1);
	return Rom_Data.u16[address];
}
// TODO: Add banks C, D, E, and F for 8 MB ROM support.
// For now, they will return 0x00.


/**
 * M68K_Read_Word_Rom4(): Read a word from ROM bank 4. (0x200000 - 0x27FFFF)
 * This ROM bank may be SRam.
 * @param address Address.
 * @return Word from ROM or SRam.
 */
uint16_t M68K_Mem::M68K_Read_Word_Rom4(uint32_t address)
{
	// Mask off the high byte of the address.
	address &= 0xFFFFFF;
	
	// Check if this is an SRam data request.
	if (SRam_State.on && SRam_State.enabled &&
	    address >= SRam_Start && address <= SRam_End)
	{
		// SRam data request.
		if (SRam_State.custom)
		{
			// Custom SRam.
			// TODO: The original Gens code simply returns 0.
			// I have no idea what this is supposed to do.
			return 0;
		}
		
		// Return the byte from SRam.
		// Note: SRam is NOT byteswapped.
		// TODO: Check boundaries.
		// TODO: Proper byteswapping.
		address -= SRam_Start;
		return (SRam[address] << 8) | SRam[address+1];
	}
	
	// ROM data request.
	address &= 0x7FFFF;
	address ^= ((4 << 19) | 1);	// TODO: LE only!
	return Rom_Data.u16[address>>1];
}


/**
 * M68K_Read_Word_Ram(): Read a word from RAM. (0xE00000 - 0xFFFFFF)
 * RAM is 64 KB, mirrored throughout the entire range.
 * @param address Address.
 * @return Word from RAM.
 */
uint16_t M68K_Mem::M68K_Read_Word_Ram(uint32_t address)
{
	address &= 0xFFFE;
	return Ram_68k.u16[address >> 1];
}


/**
 * M68K_Read_Word_Misc(): Read a word from the miscellaneous data bank. (0xA00000 - 0xA7FFFF)
 * This includes Z80 memory, Z80 control registers, and gamepads.
 * @param address Address.
 * @return Miscellaneous data word.
 */
uint16_t M68K_Mem::M68K_Read_Word_Misc(uint32_t address)
{
	// Mask off the high byte of the address.
	address &= 0xFFFFFF;
	
	if (address <= 0xA0FFFF)
	{
		// Z80 memory space.
		if (Z80_State & (Z80_STATE_BUSREQ | Z80_STATE_RESET))
		{
			// Z80 is either running or has the bus.
			// Don't do anything.
			// TODO: I don't think the Z80 needs to be stopped here...
			return 0;
		}
		
		// Call the Z80 read function.
		// NOTE: Z80 doesn't support word reads.
		// TODO
#if 0
		push	ecx
		push	edx
		mov	ecx, ebx
		and	ebx, 0x7000
		and	ecx, 0x7FFF
		shr	ebx, 10
		call	[SYM(Z80_ReadB_Table) + ebx]
		pop	edx
		pop	ecx
		pop	ebx
		ret
#endif
		return 0;
	}
	else if (address == 0xA11100)
	{
		// Z80 BUSREQ. (0xA11100)
		if (Z80_State & Z80_STATE_BUSREQ)
		{
			// Z80 is currently running.
			// NOTE: Low byte is supposed to be from
			// the next fetched instruction.
			Fake_Fetch ^= 0xFF;
			return (0x8100 | (Fake_Fetch & 0xFF));
		}
		
		// Z80 is not running.
		// TODO
#if 0
	.z80_off:
		call	SYM(main68k_readOdometer)
		sub	eax, [SYM(Last_BUS_REQ_Cnt)]
		cmp	eax, CYCLE_FOR_TAKE_Z80_BUS_GENESIS
		ja	short .bus_taken
		
		movzx	eax, byte [SYM(Fake_Fetch)]	; mov al, [SYM(Fake_Fetch)]
		mov	ah, [SYM(Last_BUS_REQ_St)]
		xor	al, 0xFF
		add	ah, 0x80
		mov	[SYM(Fake_Fetch)], al		; fake the next fetched instruction (random)
		pop	ebx
		ret
	
	align 16
	
	.bus_taken:
		movzx	eax, byte [SYM(Fake_Fetch)]	; mov al, [SYM(Fake_Fetch)]
		mov	ah, 0x80
		xor	al, 0xFF
		pop	ebx
		mov	[SYM(Fake_Fetch)], al		; fake the next fetched instruction (random)
		ret
#endif
		return 0;
	}
	else if (address > 0xA1000D)
	{
		// Invalid address.
		return 0;
	}
	
	// MD miscellaneous registers.
	switch (address & 0x00000E)
	{
		case 0x00:
			/**
			 * 0xA10000/0xA10001: Genesis version register.
			 * Format: [MODE VMOD DISK RSV VER3 VER2 VER1 VER0]
			 * MODE: Region. (0 == East; 1 == West)
			 * VMOD: Video mode. (0 == NTSC; 1 == PAL)
			 * DISK: Floppy disk drive. (0 == connected; 1 == not connected.)
			 * RSV: Reserved. (I think this is used for SegaCD.)
			 * VER 3-0: HW version. (0 == no TMSS; 1 = TMSS)
			 * TODO: Set VER to 1 once TMSS support is added, if TMSS is enabled.
			 */
			return ((Game_Mode << 7) | (CPU_Mode << 6) | 0x20);
		
		case 0x02:
			/**
			 * 0xA10002/0xA10003: Control Port 1: Data.
			 */
			// TODO
			//return RD_Controller_1();
			return 0xFF;
		
		case 0x04:
			/**
			 * 0xA10004/0xA10005: Control Port 2: Data.
			 */
			// TODO
			//return RD_Controller_2();
			return 0xFF;
		
		case 0x06:
			/**
			 * 0xA10006/0xA10007: Control Port 3: Data. (EXT)
			 * NOTE: NOT IMPLEMENTED!
			 */
			return 0xFF00;
		
		case 0x08:
			/**
			 * 0xA10008/0xA10009: Control Port 1: CTRL.
			 */
			// TODO
			//return Controller_1_COM;
			return 0xFF;
		
		case 0x0A:
			/**
			 * 0xA1000A/0xA1000B: Control Port 2: CTRL.
			 */
			// TODO
			//return Controller_2_COM;
			return 0xFF;
		
		case 0x0C:
			/**
			 * 0xA1000C/0xA1000D: Control Port 3: CTRL. (EXT)
			 * NOTE: NOT IMPLEMENTED!
			 */
			return 0x0000;
		
		default:
			// Unknown register.
			return 0x0000;
	}
}


/**
 * M68K_Read_Word_VDP(): Read a word from the VDP data banks. (0xC00000 - 0xDFFFFF)
 * @param address Address.
 * @return VDP data byte.
 */
uint16_t M68K_Mem::M68K_Read_Word_VDP(uint32_t address)
{
	// Valid address: ((address & 0xE700E0) == 0xC00000)
	// Information from vdppin.txt, (c) 2008 Charles MacDonald.
	
	// Since this function is only called if address is in the
	// VDP data banks, we can just check if ((address & 0x700E0) == 0).
	if ((address & 0x700E0) != 0)
	{
		// Not a valid VDP address.
		return 0x00;
	}
	
	// Check the VDP address.
	address &= 0x1F;
	if (address < 0x04)
	{
		// 0xC00000 - 0xC00003: VDP Data Port.
		return VdpIo::Read_Data();
	}
	else if (address < 0x08)
	{
		// 0xC00004 - 0xC00007: VDP Control port.
		return VdpIo::Read_Status();
	}
	else if (address < 0x0A)
	{
		// 0xC00008 - 0xC00009: HV counter.
		return ((VdpIo::Read_V_Counter() << 8) | VdpIo::Read_H_Counter());
	}
	
	// Invalid VDP address.
	return 0x00;
}


/** Write Byte functions. **/


/**
 * M68K_Write_Byte_Default(): Default M68K write byte handler.
 * @param address Address.
 * @param data Byte to write.
 */
void M68K_Mem::M68K_Write_Byte_Default(uint32_t address, uint8_t data)
{
	// Do nothing!
	((void)address);
	((void)data);
}


/**
 * M68K_Write_Byte_SRam(): Write a byte to SRam.
 * @param address Address.
 * @param data Byte to write.
 */
void M68K_Mem::M68K_Write_Byte_SRam(uint32_t address, uint8_t data)
{
	// Mask off the high byte of the address.
	address &= 0xFFFFFF;
	
	// Check if this is an SRam data request.
	if (SRam_State.on && SRam_State.enabled &&
	    address >= SRam_Start && address <= SRam_End)
	{
		// SRam data request.
		
		// Write the byte to SRam.
		// Note: SRam is NOT byteswapped.
		// TODO: Check boundaries.
		SRam[address - SRam_Start] = data;
	}
}


/**
 * M68K_Write_Byte_Ram(): Write a byte to RAM. (0xE00000 - 0xFFFFFF)
 * RAM is 64 KB, mirrored throughout the entire range.
 * @param address Address.
 * @param data Byte to write.
 */
void M68K_Mem::M68K_Write_Byte_Ram(uint32_t address, uint8_t data)
{
	address &= 0xFFFF;
	address ^= 1;	// TODO: LE only!
	Ram_68k.u8[address] = data;
}


/**
 * M68K_Write_Byte_Misc(): Write a byte to the miscellaneous data bank. (0xA00000 - 0xA7FFFF)
 * This includes Z80 memory, Z80 control registers, and gamepads.
 * @param address Address.
 * @param data Byte to write.
 */
void M68K_Mem::M68K_Write_Byte_Misc(uint32_t address, uint8_t data)
{
	// Mask off the high byte of the address.
	address &= 0xFFFFFF;
	
	if (address <= 0xA0FFFF)
	{
		// Z80 memory space.
		if (Z80_State & (Z80_STATE_BUSREQ | Z80_STATE_RESET))
		{
			// Z80 is either running or has the bus.
			// Don't do anything.
			// TODO: I don't think the Z80 needs to be stopped here...
			return;
		}
		
		// Call the Z80 write function.
		// TODO
#if 0
		push	edx
		mov	ecx, ebx
		and	ebx, 0x7000
		and	ecx, 0x7FFF
		shr	ebx, 10
		mov	edx, eax
		call	[SYM(Z80_WriteB_Table) + ebx]
		pop	edx
		pop	ecx
		pop	ebx
		ret
#endif
		return;
	}
	else if (address == 0xA11100)
	{
		// Z80 BUSREQ. (0xA11100)
		// TODO
		
#if 0
		xor	ecx, ecx
		mov	ah, [SYM(Z80_State)]
		mov	dword [SYM(Controller_1_Counter)], ecx
		test	al, 1	; TODO: Should this be ah, Z80_STATE_ENABLED ?
		mov	dword [SYM(Controller_1_Delay)], ecx
		mov	dword [SYM(Controller_2_Counter)], ecx
		mov	dword [SYM(Controller_2_Delay)], ecx
		jnz	short .deactivated
		
		test	ah, Z80_STATE_BUSREQ
		jnz	short .already_activated
		
		or	ah, Z80_STATE_BUSREQ
		push	edx
		mov	[SYM(Z80_State)], ah
		mov	ebx, [SYM(Cycles_M68K)]
		call	SYM(main68k_readOdometer)
		sub	ebx, eax
		mov	edx, [SYM(Cycles_Z80)]
		mov	ebx, [SYM(Z80_M68K_Cycle_Tab) + ebx * 4]
		sub	edx, ebx
		
		push	edx
		push	SYM(M_Z80)
		call	SYM(mdZ80_set_odo)
		add	esp, byte 8
		pop	edx
	
	.already_activated:
		pop	ecx
		pop	ebx
		ret
	
	align 16
	
	.deactivated:
		call	SYM(main68k_readOdometer)
		mov	cl, [SYM(Z80_State)]
		mov	[SYM(Last_BUS_REQ_Cnt)], eax
		test	cl, Z80_STATE_BUSREQ
		setnz	[SYM(Last_BUS_REQ_St)]
		jz	short .already_deactivated
		
		push	edx
		mov	ebx, [SYM(Cycles_M68K)]
		and	cl, ~Z80_STATE_BUSREQ
		sub	ebx, eax
		mov	[SYM(Z80_State)], cl
		mov	edx, [SYM(Cycles_Z80)]
		mov	ebx, [SYM(Z80_M68K_Cycle_Tab) + ebx * 4]
		mov	ecx, SYM(M_Z80)
		sub	edx, ebx
		call	z80_Exec
		pop	edx
	
	.already_deactivated:
		pop	ecx
		pop	ebx
		ret
#endif
		return;
	}
	else if (address == 0xA11200)
	{
		// Z80 RESET. (0xA11200)
		if (data & 1)
		{
			// RESET is high. Start the Z80.
			Z80_State &= ~Z80_STATE_RESET;
		}
		else
		{
			// RESET is low. Stop the Z80.
			// TODO
			//mdZ80_reset();
			Z80_State |= Z80_STATE_RESET;
			
			// YM2612's RESET line is tied to the Z80's RESET line.
			// TODO
			//YM2612_Reset();
		}
	}
	else if (address == 0xA130F1)
	{
		// SRam control register. (0xA130F1)
		SRam_State.on = (data & 1);
		SRam_State.write = !(data & 2);
	}
	else if (address >= 0xA130F2 && address <= 0xA130FF)
	{
		// Super Street Fighter II (SSF2) bankswitching system.
		// TODO
#if 0
		and	ebx, 0xF
		and	eax, 0x1F
		shr	ebx, 1
		mov	ecx, [SYM(Genesis_M68K_Read_Byte_Table) + eax * 4]
		mov	[SYM(M68K_Read_Byte_Table) + ebx * 4], ecx
		mov	ecx, [SYM(Genesis_M68K_Read_Word_Table) + eax * 4]
		mov	[SYM(M68K_Read_Word_Table) + ebx * 4], ecx
#endif
	}
	else if (address > 0xA1000D)
	{
		// Invalid address.
		return;
	}
	
	// MD miscellaneous registers.
	switch (address & 0x00000E)
	{
		// Non-writable and not-implemented registers first.
		case 0x00: /// 0xA10000/0xA10001: Genesis version register.
		case 0x06: /// 0xA10006/0xA10007: Control Port 3: Data. (EXT)
		case 0x0C: /// 0xA1000C/0xA1000D: Control Port 3: CTRL. (EXT)
		default:
			break;
		
		case 0x02:
			/**
			 * 0xA10002/0xA10003: Control Port 1: Data.
			 */
			// TODO
			//WR_Controller_1(data);
			break;
		
		case 0x04:
			/**
			 * 0xA10004/0xA10005: Control Port 2: Data.
			 */
			// TODO
			//WR_Controller_2(data);
			break;
		
		case 0x08:
			/**
			 * 0xA10008/0xA10009: Control Port 1: CTRL.
			 */
			// TODO
			//Controller_1_COM = data;
			break;
		
		case 0x0A:
			/**
			 * 0xA1000A/0xA1000B: Control Port 2: CTRL.
			 */
			// TODO
			//Controller_2_COM = data;
			break;
	}
}


/**
 * M68K_Write_Byte_VDP(): Write a byte to the VDP data banks. (0xC00000 - 0xDFFFFF)
 * @param address Address.
 * @param data Byte to write.
 */
void M68K_Mem::M68K_Write_Byte_VDP(uint32_t address, uint8_t data)
{
	// Valid address: ((address & 0xE700E0) == 0xC00000)
	// Information from vdppin.txt, (c) 2008 Charles MacDonald.
	
	// Since this function is only called if address is in the
	// VDP data banks, we can just check if ((address & 0x700E0) == 0).
	if ((address & 0x700E0) != 0)
	{
		// Not a valid VDP address.
		return;
	}
	
	// Check the VDP address.
	address &= 0x1F;
	if (address < 0x04)
	{
		// 0xC00000 - 0xC00003: VDP Data Port.
		VdpIo::Write_Data_Byte(data);
	}
	else if (address < 0x08)
	{
		// 0xC00004 - 0xC00007: VDP Control Port.
		// TODO: This should still be writable.
		// Gens' mem_m68k.asm doesn't implement this.
	}
	else if (address == 0x11)
	{
		// 0xC00011: PSG control port.
		// TODO
		//PSG_Write(data);
	}
}


/** Default function tables. **/


const M68K_Mem::M68K_Read_Byte_fn M68K_Mem::MD_M68K_Read_Byte_Table[0x20] =
{
	T_M68K_Read_Byte_RomX<0x0>,	// 0x000000 - 0x07FFFF [Bank 0x00]
	T_M68K_Read_Byte_RomX<0x1>,	// 0x080000 - 0x0FFFFF [Bank 0x01]
	T_M68K_Read_Byte_RomX<0x2>,	// 0x100000 - 0x17FFFF [Bank 0x02]
	T_M68K_Read_Byte_RomX<0x3>,	// 0x180000 - 0x1FFFFF [Bank 0x03]
	M68K_Read_Byte_Rom4,		// 0x200000 - 0x27FFFF [Bank 0x04]
	T_M68K_Read_Byte_RomX<0x5>,	// 0x280000 - 0x2FFFFF [Bank 0x05]
	T_M68K_Read_Byte_RomX<0x6>,	// 0x300000 - 0x37FFFF [Bank 0x06]
	T_M68K_Read_Byte_RomX<0x7>,	// 0x380000 - 0x3FFFFF [Bank 0x07]
	T_M68K_Read_Byte_RomX<0x8>,	// 0x400000 - 0x47FFFF [Bank 0x08]
	T_M68K_Read_Byte_RomX<0x9>,	// 0x480000 - 0x4FFFFF [Bank 0x09]
	T_M68K_Read_Byte_RomX<0xA>,	// 0x500000 - 0x57FFFF [Bank 0x0A]
	T_M68K_Read_Byte_RomX<0xB>,	// 0x580000 - 0x5FFFFF [Bank 0x0B]
	M68K_Read_Byte_Default,		// 0x600000 - 0x67FFFF [Bank 0x0C]
	M68K_Read_Byte_Default,		// 0x680000 - 0x6FFFFF [Bank 0x0D]
	M68K_Read_Byte_Default,		// 0x700000 - 0x77FFFF [Bank 0x0E]
	M68K_Read_Byte_Default,		// 0x780000 - 0x7FFFFF [Bank 0x0F]
	M68K_Read_Byte_Default,		// 0x800000 - 0x87FFFF [Bank 0x10]
	M68K_Read_Byte_Default,		// 0x880000 - 0x8FFFFF [Bank 0x11]
	M68K_Read_Byte_Default,		// 0x900000 - 0x97FFFF [Bank 0x12]
	M68K_Read_Byte_Default,		// 0x980000 - 0x9FFFFF [Bank 0x13]
	M68K_Read_Byte_Misc,		// 0xA00000 - 0xA7FFFF [Bank 0x14]
	M68K_Read_Byte_Default,		// 0xA80000 - 0xAFFFFF [Bank 0x15]
	M68K_Read_Byte_Default,		// 0xB00000 - 0xB7FFFF [Bank 0x16]
	M68K_Read_Byte_Default,		// 0xB80000 - 0xBFFFFF [Bank 0x17]
	M68K_Read_Byte_VDP,		// 0xC00000 - 0xC7FFFF [Bank 0x18]
	M68K_Read_Byte_VDP,		// 0xC80000 - 0xCFFFFF [Bank 0x19]
	M68K_Read_Byte_VDP,		// 0xD00000 - 0xD7FFFF [Bank 0x1A]
	M68K_Read_Byte_VDP,		// 0xD80000 - 0xDFFFFF [Bank 0x1B]
	M68K_Read_Byte_Ram,		// 0xE00000 - 0xE7FFFF [Bank 0x1C]
	M68K_Read_Byte_Ram,		// 0xE80000 - 0xEFFFFF [Bank 0x1D]
	M68K_Read_Byte_Ram,		// 0xF00000 - 0xF7FFFF [Bank 0x1E]
	M68K_Read_Byte_Ram,		// 0xF80000 - 0xFFFFFF [Bank 0x1F]
};


const M68K_Mem::M68K_Read_Word_fn M68K_Mem::MD_M68K_Read_Word_Table[0x20] =
{
	T_M68K_Read_Word_RomX<0x0>,	// 0x000000 - 0x07FFFF [Bank 0x00]
	T_M68K_Read_Word_RomX<0x1>,	// 0x080000 - 0x0FFFFF [Bank 0x01]
	T_M68K_Read_Word_RomX<0x2>,	// 0x100000 - 0x17FFFF [Bank 0x02]
	T_M68K_Read_Word_RomX<0x3>,	// 0x180000 - 0x1FFFFF [Bank 0x03]
	M68K_Read_Word_Rom4,		// 0x200000 - 0x27FFFF [Bank 0x04]
	T_M68K_Read_Word_RomX<0x5>,	// 0x280000 - 0x2FFFFF [Bank 0x05]
	T_M68K_Read_Word_RomX<0x6>,	// 0x300000 - 0x37FFFF [Bank 0x06]
	T_M68K_Read_Word_RomX<0x7>,	// 0x380000 - 0x3FFFFF [Bank 0x07]
	T_M68K_Read_Word_RomX<0x8>,	// 0x400000 - 0x47FFFF [Bank 0x08]
	T_M68K_Read_Word_RomX<0x9>,	// 0x480000 - 0x4FFFFF [Bank 0x09]
	T_M68K_Read_Word_RomX<0xA>,	// 0x500000 - 0x57FFFF [Bank 0x0A]
	T_M68K_Read_Word_RomX<0xB>,	// 0x580000 - 0x5FFFFF [Bank 0x0B]
	M68K_Read_Word_Default,		// 0x600000 - 0x67FFFF [Bank 0x0C]
	M68K_Read_Word_Default,		// 0x680000 - 0x6FFFFF [Bank 0x0D]
	M68K_Read_Word_Default,		// 0x700000 - 0x77FFFF [Bank 0x0E]
	M68K_Read_Word_Default,		// 0x780000 - 0x7FFFFF [Bank 0x0F]
	M68K_Read_Word_Default,		// 0x800000 - 0x87FFFF [Bank 0x10]
	M68K_Read_Word_Default,		// 0x880000 - 0x8FFFFF [Bank 0x11]
	M68K_Read_Word_Default,		// 0x900000 - 0x97FFFF [Bank 0x12]
	M68K_Read_Word_Default,		// 0x980000 - 0x9FFFFF [Bank 0x13]
	M68K_Read_Word_Misc,		// 0xA00000 - 0xA7FFFF [Bank 0x14]
	M68K_Read_Word_Default,		// 0xA80000 - 0xAFFFFF [Bank 0x15]
	M68K_Read_Word_Default,		// 0xB00000 - 0xB7FFFF [Bank 0x16]
	M68K_Read_Word_Default,		// 0xB80000 - 0xBFFFFF [Bank 0x17]
	M68K_Read_Word_VDP,		// 0xC00000 - 0xC7FFFF [Bank 0x18]
	M68K_Read_Word_VDP,		// 0xC80000 - 0xCFFFFF [Bank 0x19]
	M68K_Read_Word_VDP,		// 0xD00000 - 0xD7FFFF [Bank 0x1A]
	M68K_Read_Word_VDP,		// 0xD80000 - 0xDFFFFF [Bank 0x1B]
	M68K_Read_Word_Ram,		// 0xE00000 - 0xE7FFFF [Bank 0x1C]
	M68K_Read_Word_Ram,		// 0xE80000 - 0xEFFFFF [Bank 0x1D]
	M68K_Read_Word_Ram,		// 0xF00000 - 0xF7FFFF [Bank 0x1E]
	M68K_Read_Word_Ram,		// 0xF80000 - 0xFFFFFF [Bank 0x1F]
};

}
