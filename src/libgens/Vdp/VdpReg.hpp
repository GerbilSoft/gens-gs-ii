/***************************************************************************
 * libgens: Gens Emulation Library.                                        *
 * VdpReg.hpp: VDP registers.                                              *
 *                                                                         *
 * Copyright (c) 1999-2002 by Stéphane Dallongeville.                      *
 * Copyright (c) 2003-2004 by Stéphane Akhoun.                             *
 * Copyright (c) 2008-2015 by David Korth.                                 *
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

/**
 * References:
 * - "VDP 128Kb Extended VRAM mode"
 *   - http://gendev.spritesmind.net/forum/viewtopic.php?t=1368
 * - http://md.squee.co/VDP
 */

#ifndef __LIBGENS_MD_VDPREG_HPP__
#define __LIBGENS_MD_VDPREG_HPP__

#include <stdint.h>

namespace LibGens {

namespace VdpTypes {
	// VDP registers.
	union VdpReg_t {
		uint8_t reg[24];
		struct {
			/**
			 * Mode 5 (MD) registers.
			 */

			/**
			 * Register 0: Mode Set 1.
			 * [   0    0   LCB  IE1 HSTG PSEL   M3 EXTV]
			 * 
			 * LCB: Left Column Blank. SMS VDP leftover; if set, masks the first 8 pixels
			 *      with the background color. (SG-1000 MkII and later)
			 * IE1: Enable H interrupt. (1 == on; 0 == off)
			 * M4/PSEL: Palette Select. If clear, masks high two bits of each CRam color component.
			 *          If M5 is off, acts like M4 instead of PSEL.
			 * M3: HV counter latch. (1 == stop HV counter; 0 == enable read, H, V counter)
			 *
			 * NOTE: The following bits are not emulated.
			 *
			 * HSTG: Toggle HSync every line. This causes shenanigans and breaks H40.
			 *       Set this bit to 0.
			 * EXTV: 1 == Enables the EXTernal Video function.
			 *       Uses CSync pin as external video signal.
			 *       If enabled and nothing is connected to CSync, screen will blank.
			 */
			uint8_t Set1;
			#define VDP_REG_M5_SET1_LCB	(1 << 5)
			#define VDP_REG_M5_SET1_IE1	(1 << 4)
			#define VDP_REG_M5_SET1_HSTG	(1 << 3)
			#define VDP_REG_M5_SET1_PSEL	(1 << 2)
			#define VDP_REG_M5_SET1_M3	(1 << 1)
			#define VDP_REG_M5_SET1_EXTV	(1 << 0)

			/**
			 * Register 1: Mode Set 2.
			 * [128K DISP   IE0   M1   M2   M5    0    0]
			 * 
			 * 128K: Extended VRAM mode. (1 == on; 0 == off)
			 * DISP: Display Enable. (1 == on; 0 == off)
			 * IE0: Enable V interrupt. (1 == on; 0 == off)
			 * M1: DMA Enable. (1 == on; 0 == off)
			 * M2: V resolution. (1 == V30 [PAL only]; 0 == V28)
			 * M5: Mode 4/5 toggle. (1 == M5; 0 == M4)
			 */
			uint8_t Set2;
			#define VDP_REG_M5_SET2_128K	(1 << 7)
			#define VDP_REG_M5_SET2_DISP	(1 << 6)
			#define VDP_REG_M5_SET2_IE0	(1 << 5)
			#define VDP_REG_M5_SET2_M1	(1 << 4)
			#define VDP_REG_M5_SET2_M2	(1 << 3)
			#define VDP_REG_M5_SET2_M5	(1 << 2)

			/**
			 * Register 2: Pattern name table base address for Scroll A.
			 * NOTE: SA16 is only valid in 128 KB mode.
			 * [   x SA16 SA15 SA14 SA13    x    x    x]
			 */
			uint8_t Pat_ScrA_Adr;

			/**
			 * Register 3: Pattern name table base address for Window.
			 * NOTE: WD16 is only valid in 128 KB mode.
			 * NOTE: WD11 is ignored in H40.
			 * [   x WD16 WD15 WD14 WD13 WD12 WD11    x]
			 */
			uint8_t Pat_Win_Adr;

			/**
			 * Register 4: Pattern name table base address for Scroll B.
			 * NOTE: SB16 is only valid in 128 KB mode.
			 * [   x    x    x    x SB16 SB15 SB14 SB13]
			 */
			uint8_t Pat_ScrB_Adr;

			/**
			 * Register 5: Sprite Attribute Table base address.
			 * NOTE: AT16 is only valid in 128 KB mode.
			 * NOTE: AT9 is ignored in H40.
			 * [AT16 AT15 AT14 AT13 AT12 AT11 AT10  AT9]
			 */
			uint8_t Spr_Att_Adr;

			/**
			 * Register 6: Sprite Pattern Generator base address.
			 * NOTE: ONLY used in 128 KB VRAM mode!
			 * [   x    x AP16    x    x    x    x    x]
			 */
			uint8_t Spr_Pat_Adr;

			/**
			 * Register 7: Background color.
			 * [   x    x CPT1 CPT0 COL3 COL2 COL1 COL0]
			 */
			uint8_t BG_Color;

			/**
			 * Register 8: Unused.
			 */
			uint8_t Reg8;

			/**
			 * Register 9: Unused.
			 */
			uint8_t Reg9;

			/**
			 * Register 10: H Interrupt register.
			 * [HIT7 HIT6 HIT5 HIT4 HIT3 HIT2 HIT1 HIT0]
			 */
			uint8_t H_Int;

			/**
			 * Register 11: Mode Set 3.
			 * [PBUS LOCK    0    0  IE2 VSCR HSCR LSCR]
			 * 
			 * IE2: Enable external interrupt. (1 == on; 0 == off)
			 * VSCR: V Scroll mode. (0 == full; 1 == 2-cell)
			 * HSCR/LSCR: H Scroll mode.
			 *   - 00 == full
			 *   - 01 == invalid (first 8 entries used for whole screen)
			 *   - 10 == per-cell
			 *   - 11 == per-line
			 *
			 * NOTE: The following bits are not emulated.
			 *
			 * PBUS: 1 == Enables the external pixel bus.
			 *       When enabled, the 6-bit palette index is placed on
			 *       the pixel bus pins when rendering a pixel. This is
			 *       used for Sega System C, C2, and 18.
			 *       Has no effect on stock Mega Drive hardware.
			 * LOCK: Locks up the system when set to 1.
			 */
			uint8_t Set3;
			#define VDP_REG_M5_SET3_PBUS	(1 << 7)
			#define VDP_REG_M5_SET3_LOCK	(1 << 6)
			#define VDP_REG_M5_SET3_IE2	(1 << 3)
			#define VDP_REG_M5_SET3_VSCR	(1 << 2)
			#define VDP_REG_M5_SET3_HSCR	(1 << 1)
			#define VDP_REG_M5_SET3_LSCR	(1 << 0)

			/**
			 * Register 12: Mode Set 4.
			 * [ RS0  VSY  HSY  SPR S/TE LSM1 LSM0  RS1]
			 * 
			 * RS0/RS1: H resolution. (00 == H32; 11 == H40; others == invalid)
			 * S/TE: Shadow/Highlight enable. (1 == on; 0 == off)
			 * LSM1/LSM0: Interlace mode.
			 *   - 00 == normal
			 *   - 01 == interlace mode 1
			 *   - 10 == invalid (acts like 00)
			 *   - 11 == interlace mode 2
			 *
			 * NOTE: The following bits are not emulated.
			 *
			 * SPR: 1 == Enables the SPA/B pin. Used when implementing
			 *      external CRAM, e.g. on Sega System C, C2, and 18.
			 *      SPA/B value is 0 for sprite pixel, 1 for non-sprite pixel.
			 * HSY: 1 == Locks HSync to 1. Results in ~16 kHz H40 mode.
			 * VSY: 1 == Replaces the VSync signal with the pixel clock.
			 */
			uint8_t Set4;
			#define VDP_REG_M5_SET4_RS0	(1 << 7)
			#define VDP_REG_M5_SET4_VSY	(1 << 6)
			#define VDP_REG_M5_SET4_HSY	(1 << 5)
			#define VDP_REG_M5_SET4_SPR	(1 << 4)
			#define VDP_REG_M5_SET4_STE	(1 << 3)
			#define VDP_REG_M5_SET4_LSM1	(1 << 2)
			#define VDP_REG_M5_SET4_LSM0	(1 << 1)
			#define VDP_REG_M5_SET4_RS1	(1 << 0)

			/**
			 * Register 13: H Scroll Data Table base address.
			 * NOTE: HS16 is only valid in 128 KB mode.
			 * [   x HS16 HS15 HS14 HS13 HS12 HS11 HS10]
			 */
			uint8_t H_Scr_Adr;

			/**
			 * Register 14: Nametable Pattern Generator base address.
			 * NOTE: ONLY used in 128 KB VRAM mode!
			 * [   x    x    x PB16    x    x    x PA16]
			 *
			 * PA16: When set, layer A is rebased to the upper 64 KB. [includes window]
			 * PB16: When this and PA16 are set, layer B is rebased to the upper 64KB.
			 */
			uint8_t Pat_Data_Adr;
			#define VDP_REG_M5_PAT_DATA_PB16	(1 << 4)
			#define VDP_REG_M5_PAT_DATA_PA16	(1 << 0)

			/**
			 * Register 15: Auto Increment Data.
			 * [INC7 INC6 INC5 INC4 INC3 INC2 INC1 INC0]
			 */
			uint8_t Auto_Inc;

			/**
			 * Register 16: Scroll Size.
			 * [   0    0 VSZ1 VSZ0    0    0 HSZ1 HSZ0]
			 * 
			 * VSZ1/VSZ0: Vertical scroll size.
			 * HSZ1/HSZ0: Horizontal scroll size.
			 * 
			 * Scroll sizes:
			 * - 00 == 32 cells
			 * - 01 == 64 cells
			 * - 10 == invalid
			 * - 11 == 128 cells
			 */
			uint8_t Scr_Size;

			/**
			 * Register 17: Window H position.
			 * [RIGT    0    0 WHP5 WHP4 WHP3 WHP2 WHP1]
			 */
			uint8_t Win_H_Pos;
			#define VDP_REG_M5_WIN_H_RIGT	(1 << 7)

			/**
			 * Register 18: Window V position.
			 * [DOWN    0    0 WVP5 WVP4 WVP3 WVP2 WVP1]
			 */
			uint8_t Win_V_Pos;
			#define VDP_REG_M5_WIN_V_DOWN	(1 << 7)

			/**
			 * Registers 19, 20: DMA Length counter.
			 */
			uint8_t DMA_Length_L;
			uint8_t DMA_Length_H;

			/**
			 * Registers 21, 22, 23: DMA Source address.
			 */
			uint8_t DMA_Src_Adr_L;
			uint8_t DMA_Src_Adr_M;
			uint8_t DMA_Src_Adr_H;
		} m5;

		struct {
			/**
			 * Mode 4 (SMS) registers.
			 * NOTE: Mode 4 is currently not implemented.
			 * This is here for future use.
			 *
			 * On SMS1, address bits with asterisks are bitwise-AND'ed
			 * with the requested cell address. On SMS2/GG, these bits
			 * are ignored.
			 */

			/**
			 * Register 0: Mode Set 1.
			 * [ VSI  HSI   LCB  IE1   EC   M4   M3 EXTV]
			 *
			 * VSI: Vertical Scroll Inhibit. Disables vertical scrolling for
			 *      columns 24-31. (Similar to MD "Window" plane.)
			 * HSI: Horizontal Scroll Inhibit. Disables horizontal scrolling for
			 *      rows 0-1. (Similar to MD "Window" plane.)
			 * LCB: Left Column Blank. SMS VDP leftover; if set, masks the first 8 pixels
			 *      with the background color. (SG-1000 MkII and later)
			 * IE1: Enable H interrupt. (1 == on; 0 == off)
			 * M4: Mode 4 enable. Set to 1 for Mode 4; set to 0 for TMS9918 modes.
			 * M3: M3 bit for TMS9918A modes. Also used for screen height on SMS2.
			 * EXTV: If set to 1, causes loss of sync and color. Leftover EXTVID bit
			 *       from TMS9918A; always set to 0 on SMS.
			 *       (TODO: May function the same as on MD?)
			 */
			uint8_t Set1;
			#define VDP_REG_M4_SET1_VSI	(1 << 7)
			#define VDP_REG_M4_SET1_HSI	(1 << 6)
			#define VDP_REG_M4_SET1_LCB	(1 << 5)
			#define VDP_REG_M4_SET1_IE1	(1 << 4)
			#define VDP_REG_M4_SET1_EC	(1 << 3)
			#define VDP_REG_M4_SET1_M4	(1 << 2)
			#define VDP_REG_M4_SET1_M3	(1 << 1)
			#define VDP_REG_M4_SET1_EXTV	(1 << 0)

			/**
			 * Register 1: Mode Set 2.
			 * [416K DISP   IE0   M1   M2    0 SIZE  MAG]
			 * 
			 * 416K: Select 4/16K addressing mode.
			 * DISP: Display Enable. (1 == on; 0 == off)
			 * IE0: Enable V interrupt. (1 == on; 0 == off)
			 * M1: (SMS2) Selects 224-line mode for M4 if M3=1.
			 * M2: (SMS2) Selects 240-line mode for M4 if M3=1.
			 * SIZE: Sprite size. 0=8x8, 1=16x16 (TMS) or 8x16 (SMS).
			 * MAG: Double-size sprites. 0=normal, 1=double
			 *
			 * M1 and M2 have no effect on SMS1 VDPs if M4=1.
			 * If M4=0, M1, M2, and M3 select TMS9918A modes.
			 *
			 * 416K: On all SMS1 and later VDPs, this bit is ignored.
			 *
			 * MAG: On SMS1, the first four sprites will be zoomed both
			 * horizontally and vertically; the next four sprites will
			 * only be zoomed vertically. On SMS2 and GG, all eight sprites
			 * on the line will be zoomed both horizontally and vertically.
			 */
			uint8_t Set2;
			#define VDP_REG_M4_SET2_128K	(1 << 7)
			#define VDP_REG_M4_SET2_DISP	(1 << 6)
			#define VDP_REG_M4_SET2_IE0	(1 << 5)
			#define VDP_REG_M4_SET2_M1	(1 << 4)
			#define VDP_REG_M4_SET2_M2	(1 << 3)
			#define VDP_REG_M4_SET2_SIZE	(1 << 1)
			#define VDP_REG_M4_SET2_MAG	(1 << 0)

			/**
			 * Register 2: Name Table base address.
			 * [   x    x    x    x  A13  A12 +A11 *A10]
			 *
			 * SMS1: A10 acts as a mask for the cell address.
			 * SMS2: A10 is ignored; A11 is ignored in 224-line and 240-line modes.
			 */
			uint8_t Bkg_Tbl_Adr;

			/**
			 * Register 3: Color Table base address.
			 * [   1    1    1    1    1    1    1    1]
			 *
			 * SMS1: All bits should be set to 1. Otherwise, the VDP will
			 *       fetch pattern and name table data incorrectly.
			 *       (Can be used normally in TMS modes.)
			 */
			uint8_t Color_Tbl_Adr;

			/**
			 * Register 4: Background Pattern Generator base address.
			 * [   x    x    x    x    x *G13 *G12 *G11]
			 *
			 * SMS1: Bits 2-0 should be set. Otherwise, the VDP will
			 *       fetch pattern and name table data incorrectly.
			 *       (Can be used normally in TMS modes.)
			 */
			// TODO: Rename Pat_*_Adr to *_Pat_Adr for both M4 and M5?
			uint8_t Pat_Bkg_Adr;

			/**
			 * Register 5: Sprite Attribute Table base address.
			 * [   x SA13 SA12 SA11 SA10  SA9  SA8 *SA7]
			 *
			 * SMS1: A7 acts as a mask for the sprite attribute table address.
			 *       If it's set to 0, the sprite X position and tile index
			 *       will be fetched from the lower 128 bytes of the sprite
			 *       attribute table instead of the upper 128 bytes.
			 *       It should be set to 1 for normal operation.
			 */
			uint8_t Spr_Att_Adr;

			/**
			 * Register 6: Sprite Pattern Generator base address.
			 * [   x    x    x    x    x SG13 *G12 *G11]
			 *
			 * SMS1: A12-A11 act as a mask over bits 8 and 6 of the
			 *       tile index if cleared. They should be set for
			 *       normal operation.
			 */
			uint8_t Spr_Pat_Adr;

			/**
			 * Register 7: Background color.
			 * [   x    x    x    x COL3 COL2 COL1 COL0]
			 *
			 * Background color is taken from the sprite palette.
			 * (Second palette, aka Palette 1)
			 */
			uint8_t BG_Color;

			/**
			 * Register 8: Background X scroll.
			 * [HSC7 HSC6 HSC5 HSC4 HSC3 HSC2 HSC1 HSC0]
			 */
			uint8_t H_Scroll;

			/**
			 * Register 9: Background Y scroll.
			 * [VSC7 VSC6 VSC5 VSC4 VSC3 VSC2 VSC1 VSC0]
			 */
			uint8_t V_Scroll;

			/**
			 * Register 10: H Interrupt register.
			 * [HIT7 HIT6 HIT5 HIT4 HIT3 HIT2 HIT1 HIT0]
			 */
			uint8_t H_Int;
		} m4;

		struct {
			/**
			 * TMS9918A registers.
			 * NOTE: TMS9918A modes are currently not implemented.
			 * This is here for future use.
			 */

			/**
			 * Register 0: Mode Set 1.
			 * [   x    x     x    x    x    x   M3 EXTV]
			 *
			 * M3: Graphic II mode bit.
			 * EXTV: Set to 1 to enable external video input. Since this
			 *       isn't connected on Sega systems, setting this bit
			 *       will result in loss of color and sync.
			 */
			uint8_t Set1;
			#define VDP_REG_TMS_SET1_M3	(1 << 1)
			#define VDP_REG_TMS_SET1_EXTV	(1 << 0)

			/**
			 * Register 1: Mode Set 2.
			 * [416K DISP   IE0   M1   M2    0 SIZE  MAG]
			 * 
			 * 416K: Select 4/16K addressing mode.
			 * DISP: Display Enable. (1 == on; 0 == off)
			 * IE0: Enable V interrupt. (1 == on; 0 == off)
			 * M1: Text mode bit.
			 * M2: Multicolor mode bit.
			 * SIZE: Sprite size. 0=8x8, 1=16x16 (TMS) or 8x16 (SMS).
			 * MAG: Double-size sprites. 0=normal, 1=double
			 *
			 * 416K: If cleared, VRAM addresses will shift around.
			 * See tms9918a.txt for more information.
			 */
			uint8_t Set2;
			#define VDP_REG_TMS_SET2_128K	(1 << 7)
			#define VDP_REG_TMS_SET2_DISP	(1 << 6)
			#define VDP_REG_TMS_SET2_IE0	(1 << 5)
			#define VDP_REG_TMS_SET2_M1	(1 << 4)
			#define VDP_REG_TMS_SET2_M2	(1 << 3)
			#define VDP_REG_TMS_SET2_SIZE	(1 << 1)
			#define VDP_REG_TMS_SET2_MAG	(1 << 0)

			/**
			 * Register 2: Name Table base address.
			 * [   x    x    x    x PN13 PN12 PN11 PN10]
			 */
			uint8_t Bkg_Tbl_Adr;

			/**
			 * Register 3: Color Table base address.
			 * [CT13 CT12 CT11 CT10  CT9  CT8  CT7  CT6]
			 */
			uint8_t Color_Tbl_Adr;

			/**
			 * Register 4: Background Pattern Generator base address.
			 * [   x    x    x    x    x PG13 PG12 PG11]
			 */
			// TODO: Rename Pat_*_Adr to *_Pat_Adr for both M4 and M5?
			uint8_t Pat_Bkg_Adr;

			/**
			 * Register 5: Sprite Attribute Table base address.
			 * [   x SA13 SA12 SA11 SA10  SA9  SA8  SA7]
			 */
			uint8_t Spr_Att_Adr;

			/**
			 * Register 6: Sprite Pattern Generator base address.
			 * [   x    x    x    x    x SG13 SG12 SG11]
			 */
			uint8_t Spr_Pat_Adr;

			/**
			 * Register 7: Background color.
			 * [ TC3  TC2  TC1  TC0  BD3  BD2  BD1  BD0]
			 *
			 * BD: Background color.
			 * TC: Text color. (Text mode only)
			 */
			uint8_t BG_Color;
		} tms;
	};

	/**
	  * VDP Codeword register.
	  * MD: [CD5 CD4 CD3 CD2 CD1 CD0]
	  * - CD5: DMA enable.
	  * - CD4: Busy.
	  * - CD3-CD1: Destination.
	  * - CD0: Read/Write. (0 = read, 1 = write)
	  */
	enum VDP_CodeWord {
		// CD5: DMA enable.
		CD_DMA_ENABLE		= (1 << 5),

		// CD4: Done. (if 0, VDP has work to do)
		CD_DONE			= (1 << 4),

		// CD3-1: Destination.
		// NOTE: CRAM has different destinations
		// depending on read/write mode.
		CD_DEST_VRAM		= 0x00,
		CD_DEST_REGISTER_INT_W	= 0x02, // CD0=0
		CD_DEST_CRAM_INT_W	= 0x02, // CD0=1
		CD_DEST_VSRAM		= 0x04,
		CD_DEST_CRAM_INT_R	= 0x08,
		CD_DEST_VRAM_8BIT	= 0x0C,	// undocumented
		CD_DEST_MASK		= 0x0E,

		// CD0: Read/write.
		CD_MODE_READ		= 0x00,
		CD_MODE_WRITE		= 0x01,
		CD_MODE_MASK		= 0x01,

		// CD3-0 combined.
		CD_DEST_VRAM_READ	= (CD_DEST_VRAM | CD_MODE_READ),
		CD_DEST_VRAM_WRITE	= (CD_DEST_VRAM | CD_MODE_WRITE),
		CD_DEST_CRAM_READ	= (CD_DEST_CRAM_INT_R | CD_MODE_READ),
		CD_DEST_CRAM_WRITE	= (CD_DEST_CRAM_INT_W | CD_MODE_WRITE),
		CD_DEST_VSRAM_READ	= (CD_DEST_VSRAM | CD_MODE_READ),
		CD_DEST_VSRAM_WRITE	= (CD_DEST_VSRAM | CD_MODE_WRITE),
		// NOTE: REGISTER_WRITE looks like a READ, but it isn't!
		CD_DEST_REGISTER_WRITE	= (CD_DEST_REGISTER_INT_W | CD_MODE_READ),
		CD_DEST_MODE_MASK	= 0x0F,
		CD_DEST_MODE_CD4_MASK	= 0x1F,
	};
}

}

#endif /* __LIBGENS_MD_VDPREG_HPP__ */
