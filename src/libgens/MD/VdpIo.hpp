/***************************************************************************
 * libgens: Gens Emulation Library.                                        *
 * VdpIo.hpp: VDP I/O class.                                               *
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

#ifndef __LIBGENS_MD_VDPIO_HPP__
#define __LIBGENS_MD_VDPIO_HPP__

#include <stdint.h>

namespace LibGens
{

class VdpIo
{
	public:
		static void Init(void);
		static void End(void);
		
		/** VDP I/O registers and variables. **/
		
		// VDP registers.
		union VDP_Reg_t
		{
			uint8_t reg[24];
			struct m5_t
			{
				/**
				* Mode 5 (MD) registers.
				* DISP == Display Enable. (1 == on; 0 == off)
				* IE0 == Enable V interrupt. (1 == on; 0 == off)
				* IE1 == Enable H interrupt. (1 == on; 0 == off)
				* IE2 == Enable external interrupt. (1 == on; 0 == off)
				* M1 == DMA Enable. (1 == on; 0 == off)
				* M2 == V cell mode. (1 == V30 [PAL only]; 0 == V28)
				* M3 == HV counter latch. (1 == stop HV counter; 0 == enable read, H, V counter)
				* M4/PSEL == Palette Select; if clear, masks high two bits of each CRam color component.
				*            If M5 is off, acts like M4 instead of PSEL.
				* M5 == Mode 4/5 toggle; set for Mode 5, clear for Mode 4.
				* VSCR == V Scroll mode. (0 == full; 1 == 2-cell)
				* HSCR/LSCR == H Scroll mode. (00 == full; 01 == invalid; 10 == 1-cell; 11 == 1-line)
				* RS0/RS1 == H cell mode. (11 == H40; 00 == H32; others == invalid)
				* LSM1/LSM0 == Interlace mode. (00 == normal; 01 == interlace mode 1; 10 == invalid; 11 == interlace mode 2)
				* S/TE == Highlight/Shadow enable. (1 == on; 0 == off)
				* VSZ1/VSZ2 == Vertical scroll size. (00 == 32 cells; 01 == 64 cells; 10 == invalid; 11 == 128 cells)
				* HSZ1/HSZ2 == Vertical scroll size. (00 == 32 cells; 01 == 64 cells; 10 == invalid; 11 == 128 cells)
				*/
				uint8_t Set1;		// Mode Set 1.  [   0    0    0  IE1    0 PSEL   M3    0]
				uint8_t Set2;		// Mode Set 2.  [   0 DISP  IE0   M1   M2   M5    0    0]
				uint8_t Pat_ScrA_Adr;	// Pattern name table base address for Scroll A.
				uint8_t Pat_Win_Adr;	// Pattern name table base address for Window.
				uint8_t Pat_ScrB_Adr;	// Pattern name table base address for Scroll B.
				uint8_t Spr_Att_Adr;	// Sprite Attribute Table base address.
				uint8_t Reg6;		// unused
				uint8_t BG_Color;	// Background color.
				uint8_t Reg8;		// unused
				uint8_t Reg9;		// unused
				uint8_t H_Int;		// H Interrupt.
				uint8_t Set3;		// Mode Set 3.  [   0    0    0    0  IE2 VSCR HSCR LSCR]
				uint8_t Set4;		// Mode Set 4.  [ RS0    0    0    0 S/TE LSM1 LSM0  RS1]
				uint8_t H_Scr_Adr;	// H Scroll Data Table base address.
				uint8_t Reg14;		// unused
				uint8_t Auto_Inc;	// Auto Increment.
				uint8_t Scr_Size;	// Scroll Size. [   0    0 VSZ1 VSZ0    0    0 HSZ1 HSZ0]
				uint8_t Win_H_Pos;	// Window H position.
				uint8_t Win_V_Pos;	// Window V position.
				uint8_t DMA_Length_L;	// DMA Length Counter Low.
				uint8_t DMA_Length_H;	// DMA Length Counter High.
				uint8_t DMA_Src_Adr_L;	// DMA Source Address Low.
				uint8_t DMA_Src_Adr_M;	// DMA Source Address Mid.
				uint8_t DMA_Src_Adr_H;	// DMA Source Address High.
			};
			m5_t m5;
			struct m4_t
			{
				/**
				* Mode 4 (SMS) registers.
				* NOTE: Mode 4 is currently not implemented.
				* This is here for future use.
				*/
				uint8_t Set1;		// Mode Set 1.
				uint8_t Set2;		// Mode Set 2.
				uint8_t NameTbl_Addr;	// Name table base address.
				uint8_t ColorTbl_Addr;	// Color table base address.
				uint8_t	Pat_BG_Addr;	// Background Pattern Generator base address.
				uint8_t Spr_Att_Addr;	// Sprite Attribute Table base address.
				uint8_t Spr_Pat_addr;	// Sprite Pattern Generator base address.
				uint8_t BG_Color;	// Background color.
				uint8_t H_Scroll;	// Horizontal scroll.
				uint8_t V_Scroll;	// Vertical scroll.
				uint8_t H_Int;		// H Interrupt.
			};
			m4_t m4;
		};
		static VDP_Reg_t VDP_Reg;
		
		// These two variables are internal to Gens.
		// They don't map to any actual VDP registers.
		static int DMA_Length;
		static unsigned int DMA_Address;
		
		// DMAT variables.
		static unsigned int DMAT_Tmp;
		static int DMAT_Length;
		static unsigned int DMAT_Type;
		
		// VDP address pointers.
		// These are relative to VRam[] and are based on register values.
		static uint16_t *ScrA_Addr;
		static uint16_t *ScrB_Addr;
		static uint16_t *Win_Addr;
		static uint16_t *Spr_Addr;
		static uint16_t *H_Scroll_Addr;
		
		// VDP convenience values: Horizontal.
		// NOTE: These must be signed for VDP arithmetic to work properly!
		static int H_Cell;
		static int H_Pix;
		static int H_Pix_Begin;
		
		// Window row shift.
		// H40: 6. (64x32 window)
		// H32: 5. (32x32 window)
		static unsigned int H_Win_Shift;
		
		// VDP convenience values: Scroll.
		static unsigned int V_Scroll_MMask;
		static unsigned int H_Scroll_Mask;
		
		static unsigned int H_Scroll_CMul;
		static unsigned int H_Scroll_CMask;
		static unsigned int V_Scroll_CMask;
		
		// TODO: Eliminate these.
		static int Win_X_Pos;
		static int Win_Y_Pos;
		
		// Interlaced mode.
		struct Interlaced_t
		{
			unsigned int HalfLine  :1;	// Half-line is enabled. [LSM0]
			unsigned int DoubleRes :1;	// 2x resolution is enabled. [LSM1]
		};
		static Interlaced_t Interlaced;
		
		// Sprite dot overflow.
		// If set, the previous line had a sprite dot overflow.
		// This is needed to properly implement Sprite Masking in S1.
		static int SpriteDotOverflow;
		
		// Horizontal Interrupt Counter.
		static int HInt_Counter;
		
		/**
		 * VDP_Ctrl: VDP control struct.
		 */
		struct VDP_Ctrl_t
		{
			unsigned int Flag;	// Data latch.
			union Data_t
			{
				uint16_t w[2];	// Control words.
				uint32_t d;	// Control DWORD. (TODO: Endianness conversion.)
			};
			Data_t Data;
			unsigned int Write;
			unsigned int Access;
			unsigned int Address;
			unsigned int DMA_Mode;
			unsigned int DMA;
		};
		static VDP_Ctrl_t VDP_Ctrl;
		
		/**
		* VDP_Mode: Current VDP mode.
		*/
		#define VDP_MODE_M1	(1 << 0)
		#define VDP_MODE_M2	(1 << 1)
		#define VDP_MODE_M3	(1 << 2)
		#define VDP_MODE_M4	(1 << 3)
		#define VDP_MODE_M5	(1 << 4)
		static unsigned int VDP_Mode;
		
		/**
		 * VRam: Video RAM.
		 * SMS/GG: 16 KB.
		 * MD: 64 KB. (32 KW)
		 */
		union VDP_VRam_t
		{
			uint8_t  u8[64*1024];
			uint16_t u16[(64*1024)>>1];
			uint32_t u32[(64*1024)>>2];
		};
		static VDP_VRam_t VRam;
		
		/**
		 * CRam: Color RAM.
		 * SMS: 32 bytes.
		 * MD: 128 bytes. (64 words)
		 * GG: 64 bytes. (32 words)
		 */
		union VDP_CRam_t
		{
			uint8_t  u8[64<<1];
			uint16_t u16[64];
			uint32_t u32[64>>1];
		};
		static VDP_CRam_t CRam;
		
		/**
		 * VSRam: Vertical Scroll RAM.
		 * MD: 40 words.
		 */
		union VSRam_t
		{
			uint8_t  u8[40<<1];
			uint16_t u16[40];
			
			uint8_t  reserved[128];		// TODO: Figure out how to remove this.
		};
		static VSRam_t VSRam;
		
		static int VDP_Int;
		static int VDP_Status;
		
		// VDP line counters.
		// NOTE: Gens/GS currently uses 312 lines for PAL. It should use 313!
		struct VDP_Lines_t
		{
			/** Total lines using NTSC/PAL line numbering. **/
			struct Display_t
			{
				unsigned int Total;	// Total number of lines on the display. (262, 313)
				unsigned int Current;	// Current display line.
			};
			Display_t Display;
			
			/** Visible lines using VDP line numbering. **/
			struct Visible_t
			{
				int Total;		// Total number of visible lines. (192, 224, 240)
				int Current;		// Current visible line. (May be negative for top border.)
				int Border_Size;	// Size of the border. (192 lines == 24; 224 lines == 8)
			};
			Visible_t Visible;
			
			/** NTSC V30 handling. **/
			struct NTSC_V30_t
			{
				int Offset;		// Current NTSC V30 roll offset.
				int VBlank_Div;		// VBlank divider. (0 == VBlank is OK; 1 == no VBlank allowed)
			};
			NTSC_V30_t NTSC_V30;
		};
		static VDP_Lines_t VDP_Lines;
		
		// Flags.
		union VDP_Flags_t
		{
			unsigned int flags;
			struct
			{
				unsigned int VRam	:1;	// VRam was modified. (Implies VRam_Spr.)
				unsigned int VRam_Spr	:1;	// Sprite Attribute Table was modified.
				unsigned int CRam	:1;	// CRam was modified.
			};
		};
		static VDP_Flags_t VDP_Flags;
		
		// Set this to 1 to enable zero-length DMA requests.
		// Default is 0. (hardware-accurate)
		static int Zero_Length_DMA;
		
		// System status.
		// TODO: Move this to a more relevant file.
		struct SysStatus_t
		{
			unsigned int Genesis	:1;
			unsigned int SegaCD	:1;
			unsigned int _32X	:1;
		};
		static SysStatus_t SysStatus;
		
		/** VDP tables. **/
		static uint8_t H_Counter_Table[512][2];
		static const uint32_t CD_Table[64];
		static const uint8_t DMA_Timing_Table[4][4];
		
		/** VDP functions. **/
		static void Reset(void);
		static uint8_t Int_Ack(void);
		static void Update_IRQ_Line(void);
		
		static void Set_Visible_Lines(void);
		static void Check_NTSC_V30_VBlank(void);
		
		/**
		 * Scroll_Size_t: Convenience enum for dealing with scroll plane sizes.
		 */
		enum Scroll_Size_t
		{
			V32_H32 = 0, V32_H64,  V32_HXX,  V32_H128,
			V64_H32,     V64_H64,  V64_HXX,  V64_H128,
			VXX_H32,     VXX_H64,  VXX_HXX,  VXX_H128,
			V128_H32,    V128_H64, V128_HXX, V128_H128
		};
		
		static void Set_Reg(int reg_num, uint8_t val);
		
		static uint8_t Read_H_Counter(void);
		static uint8_t Read_V_Counter(void);
		static uint16_t Read_Status(void);
		static uint16_t Read_Data(void);
		
		static unsigned int Update_DMA(void);
		
		static void Write_Data_Byte(uint8_t data);
		static void Write_Data_Word(uint16_t data);
		static void Write_Ctrl(uint16_t data);
		
		/**
		 * GetHPix(): Get the current horizontal resolution.
		 * This should only be used for non-VDP code.
		 * VDP code should access VDP_Reg.H_Pix directly.
		 * NOTE: Do NOT use this if a ROM isn't loaded!
		 * @return Horizontal resolution, in pixels.
		 */
		static inline int GetHPix(void) { return H_Pix; }
		
		/**
		 * GetHPixBegin(): Get the first horizontal pixel number.
		 * This should only be used for non-VDP code.
		 * VDP code should access VDP_Reg.H_Pix_Begin directly.
		 * NOTE: Do NOT use this if a ROM isn't loaded!
		 * @return First horizontal pixel number.
		 */
		static inline int GetHPixBegin(void) { return H_Pix_Begin; }
		
		/**
		 * vdp_getVPix(): Get the current vertical resolution.
		 * This should only be used for non-VDP code.
		 * VDP code should access VDP_Reg.Set2 directly.
		 * NOTE: Do NOT use this if a ROM isn't loaded!
		 * @return Vertical resolution, in pixels.
		 */
		static inline int GetVPix(void) { return VDP_Lines.Visible.Total; }
	
	protected:
		static inline void Update_Mode(void);
		
		/** DMA **/
		
		static void DMA_Fill(uint16_t data);
	
		enum DMA_Dest_t
		{
			DMA_DEST_VRAM	= 1,
			DMA_DEST_CRAM	= 2,
			DMA_DEST_VSRAM	= 3,
		};
		
		enum DMA_Src_t
		{
			DMA_SRC_ROM			= 0,
			DMA_SRC_M68K_RAM		= 1,
			DMA_SRC_PRG_RAM			= 2,
			DMA_SRC_WORD_RAM_2M		= 3,
			DMA_SRC_WORD_RAM_1M_0		= 5,
			DMA_SRC_WORD_RAM_1M_1		= 6,
			DMA_SRC_WORD_RAM_CELL_1M_0	= 7,
			DMA_SRC_WORD_RAM_CELL_1M_1	= 8,
		};
		
		// NOTE: This needs to be a macro, since it's used in case statements.
		#define DMA_TYPE(src, dest) (((int)src << 2) | ((int)dest))
		
		template<DMA_Src_t src_component, DMA_Dest_t dest_component>
		static inline void T_DMA_Loop(unsigned int src_address, unsigned int dest_address, int length);
	
	private:
		VdpIo() { }
		~VdpIo() { }
};

}

#endif /* __LIBGENS_MD_VDPIO_HPP__ */
