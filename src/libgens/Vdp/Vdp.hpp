/***************************************************************************
 * libgens: Gens Emulation Library.                                        *
 * Vdp.hpp: VDP emulation class.                                           *
 *                                                                         *
 * Copyright (c) 1999-2002 by Stéphane Dallongeville.                      *
 * Copyright (c) 2003-2004 by Stéphane Akhoun.                             *
 * Copyright (c) 2008-2011 by David Korth.                                 *
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

#ifndef __LIBGENS_MD_VDP_HPP__
#define __LIBGENS_MD_VDP_HPP__

#include <stdint.h>

// VdpRend includes.
#include "../Util/MdFb.hpp"

// Needed for FORCE_INLINE.
#include "../macros/common.h"

// VDP types.
#include "VdpTypes.hpp"
#include "VdpPalette.hpp"

namespace LibGens
{

class Vdp
{
	public:
		static void Init(void);
		static void End(void);
		static void Reset(void);
		
		// VDP emulation options.
		static VdpTypes::VdpEmuOptions_t VdpEmuOptions;
		
		// Update flags.
	public:
		static void MarkCRamDirty(void);
		static void MarkVRamDirty(void);
	private:
		static VdpTypes::UpdateFlags_t ms_UpdateFlags;
	
	/*!**************************************************************
	 * VdpIo: I/O registers and variables.                          *
	 ****************************************************************/
	
	public:
		// VDP registers.
		static VdpTypes::VdpReg_t VDP_Reg;
		
		// These two variables are internal to Gens.
		// They don't map to any actual VDP registers.
		static int DMA_Length;
		static unsigned int DMA_Address;
		
		// DMAT variables.
		static unsigned int DMAT_Tmp;
		static int DMAT_Length;
		static unsigned int DMAT_Type;
		
		/** VDP address functions: Get Pointers. **/
		static uint16_t *ScrA_Addr_Ptr16(uint16_t offset);
		static uint16_t *ScrB_Addr_Ptr16(uint16_t offset);
		static uint16_t *Win_Addr_Ptr16(uint16_t offset);
		static uint16_t *Spr_Addr_Ptr16(uint16_t offset);
		static uint16_t *H_Scroll_Addr_Ptr16(uint16_t offset);
		
		/** VDP address functions: Get Values. **/
		static uint16_t ScrA_Addr_u16(uint16_t offset);
		static uint16_t ScrB_Addr_u16(uint16_t offset);
		static uint16_t Win_Addr_u16(uint16_t offset);
		static uint16_t Spr_Addr_u16(uint16_t offset);
		static uint16_t H_Scroll_Addr_u16(uint16_t offset);
		
		/**
		 * Window row shift.
		 * H40: 6. (64x32 window)
		 * H32: 5. (32x32 window)
		 */
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
		static VdpTypes::Interlaced_t Interlaced;
		
		// Sprite dot overflow.
		// If set, the previous line had a sprite dot overflow.
		// This is needed to properly implement Sprite Masking in S1.
		static int SpriteDotOverflow;
		
		// Horizontal Interrupt Counter.
		static int HInt_Counter;
		
		/**
		* VDP_Mode: Current VDP mode.
		*/
		#define VDP_MODE_M1	(1 << 0)
		#define VDP_MODE_M2	(1 << 1)
		#define VDP_MODE_M3	(1 << 2)
		#define VDP_MODE_M4	(1 << 3)
		#define VDP_MODE_M5	(1 << 4)
		static unsigned int VDP_Mode;
		
		// VRam, CRam, VSRam.
		static VdpTypes::VRam_t VRam;
		static VdpTypes::CRam_t CRam;
		static VdpTypes::VSRam_t VSRam;
		
		static int VDP_Int;
		static int VDP_Status;
		
		// VDP line counters.
		// NOTE: Gens/GS currently uses 312 lines for PAL. It should use 313!
		static VdpTypes::VdpLines_t VDP_Lines;
		
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
		static const uint8_t DMA_Timing_Table[4][4];
		
		/** Interrupt functions. **/
		static uint8_t Int_Ack(void);
		static void Update_IRQ_Line(void);
		
		// Lines.
		static void Set_Visible_Lines(void);
		static void Check_NTSC_V30_VBlank(void);
		
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
		 * NOTE: Do NOT use this if a ROM isn't loaded!
		 * @return Horizontal resolution, in pixels.
		 */
		static int GetHPix(void);
		
		/**
		 * GetHPixBegin(): Get the first horizontal pixel number.
		 * NOTE: Do NOT use this if a ROM isn't loaded!
		 * @return First horizontal pixel number.
		 */
		static int GetHPixBegin(void);
		
		/**
		 * GetHCells(): Get the current horizontal resolution, in cells.
		 * @return Horizontal resolution, in cells.
		 */
		static int GetHCells(void);
		
		/**
		 * GetVPix(): Get the current vertical resolution.
		 * NOTE: Do NOT use this if a ROM isn't loaded!
		 * @return Vertical resolution, in pixels.
		 */
		static int GetVPix(void);
	
	private:
		/**
		 * Vdp::Update_Mode(): Update VDP_Mode.
		 */
		static void Update_Mode(void);
		
		// VDP convenience values: Horizontal.
		// NOTE: These must be signed for VDP arithmetic to work properly!
		static int H_Cell;
		static int H_Pix;
		static int H_Pix_Begin;
		
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
		static inline void T_DMA_Loop(unsigned int src_address, uint16_t dest_address, int length);
		
		// VDP address pointers.
		// These are relative to VRam[] and are based on register values.
		static uint16_t ScrA_Addr;
		static uint16_t ScrB_Addr;
		static uint16_t Win_Addr;
		static uint16_t Spr_Addr;
		static uint16_t H_Scroll_Addr;
		
		/**
		 * VDP_Ctrl: VDP control struct.
		 */
		struct VDP_Ctrl_t
		{
			// Control word buffer.
			union Data_t
			{
				uint16_t w[2];	// Control words.
				uint32_t d;	// Control DWORD. (TODO: Endianness conversion.)
			};
			Data_t Data;
			
			// VDP memory access mode.
			uint16_t Access;	// Uses VDEST_t values to determine VDP destination.
			uint16_t Address;	// Address counter.
			
			// DMA values.
			uint8_t DMA_Mode;	// (DMA ADDRESS HIGH & 0xC0) [reg 23]
			uint8_t DMA;		// HIGH byte from CD_Table[].
			
			/**
			 * ctrl_latch: Control word latch.
			 * False: Next control word is FIRST word.
			 * True: Next control word is SECOND word, which triggers an action.
			 */
			bool ctrl_latch;	// Control word latch.
		};
		static VDP_Ctrl_t VDP_Ctrl;
		
		/**
		 * VDEST_t: VDP memory destination constants.
		 */
		enum VDEST_t
		{
			// 0x0000: INVALID.
			VDEST_INVALID		= 0x0000,
			
			// Bits 0-2: Location. 
			VDEST_LOC_INVALID	= 0x0000,
			VDEST_LOC_VRAM		= 0x0001,
			VDEST_LOC_CRAM		= 0x0002,
			VDEST_LOC_VSRAM		= 0x0003,
			
			// Bits 3-4: Access. (R/W)
			VDEST_ACC_INVALID	= 0x0000,
			VDEST_ACC_READ		= 0x0004,
			VDEST_ACC_WRITE		= 0x0008,
			
			// Bits 8-11: DMA MEM to VRAM: destination
			VDEST_DMA_MEM_TO_INVALID	= 0x0000,
			VDEST_DMA_MEM_TO_VRAM		= 0x0100,
			VDEST_DMA_MEM_TO_CRAM		= 0x0200,
			VDEST_DMA_MEM_TO_VSRAM		= 0x0300,
			
			// Bit 12: DMA VRAM FILL.
			VDEST_DMA_NO_FILL	= 0x0000,
			VDEST_DMA_FILL		= 0x0400,
			
			// Bit 13: DMA VRAM COPY.
			VDEST_DMA_NO_COPY	= 0x0000,
			VDEST_DMA_COPY		= 0x0800,
		};
		
		/**
		 * CD_Table[]: VDP memory destination table.
		 * Maps VDP control word destinations to Gens destinations.
		 */
		static const uint16_t CD_Table[64];
		
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
	
	/*!**************************************************************
	 * VdpRend: Rendering functions and variables.                  *
	 ****************************************************************/
	
	protected:
		/** NOTE: Init(), End(), and Reset() should ONLY be called from VdpIo()! **/
		static void Rend_Init(void);
		static void Rend_End(void);
		static void Rend_Reset(void);
	
	public:
		// Palette manager.
		static VdpPalette m_palette;
		
		// MD framebuffer.
		static MdFb MD_Screen;

		// Sprite structs.
		struct Sprite_Struct_t
		{
			int Pos_X;
			int Pos_Y;
			unsigned int Size_X;
			unsigned int Size_Y;
			int Pos_X_Max;
			int Pos_Y_Max;
			unsigned int Num_Tile;	// Includes palette, priority, and flip bits.
			int Pos_X_Max_Vis;	// Number of visible horizontal pixels. (Used for Sprite Limit.)
		};
		static Sprite_Struct_t Sprite_Struct[128];
		static unsigned int Sprite_Visible[128];
		
		// VDP layer control.
		static unsigned int VDP_Layers;
		
		/** Line rendering functions. **/
		static void Render_Line(void);
	
	protected:
		// Line buffer for current line.
		union LineBuf_t
		{
			struct LineBuf_px_t
			{
#if GENS_BYTEORDER == GENS_LIL_ENDIAN
				uint8_t pixel;
				uint8_t layer;
#else /* GENS_BYTEORDER == GENS_BIG_ENDIAN */
				uint8_t layer;
				uint8_t pixel;
#endif
			};
			LineBuf_px_t px[336];
			uint8_t  u8[336<<1];
			uint16_t u16[336];
			uint32_t u32[336>>1];
		};
		static LineBuf_t LineBuf;
		
		template<bool hs, typename pixel>
		static inline void T_Update_Palette(pixel *MD_palette, const pixel *palette);
	
	/*!**************************************************************
	 * VdpRend_m5: Mode 5 rendering functions and variables.        *
	 ****************************************************************/
	
	public:
		static VdpTypes::IntRend_Mode_t IntRend_Mode;
		
		/** Line rendering functions. **/
		static void Render_Line_m5(void);
	
	protected:
		// Temporary VDP data.
		static unsigned int Y_FineOffset;
		static unsigned int TotalSprites;
		
		template<bool interlaced>
		static FORCE_INLINE int T_GetLineNumber(void);
		
		template<bool plane, bool h_s, int pat_pixnum, uint32_t mask, int shift>
		static FORCE_INLINE void T_PutPixel_P0(int disp_pixnum, uint32_t pattern, unsigned int palette);
		
		template<bool plane, bool h_s, int pat_pixnum, uint32_t mask, int shift>
		static FORCE_INLINE void T_PutPixel_P1(int disp_pixnum, uint32_t pattern, unsigned int palette);
		
		template<bool priority, bool h_s, int pat_pixnum, uint32_t mask, int shift>
		static FORCE_INLINE uint8_t T_PutPixel_Sprite(int disp_pixnum, uint32_t pattern, unsigned int palette);
		
		template<bool plane, bool h_s, bool flip>
		static FORCE_INLINE void T_PutLine_P0(int disp_pixnum, uint32_t pattern, int palette);
		
		template<bool plane, bool h_s, bool flip>
		static FORCE_INLINE void T_PutLine_P1(int disp_pixnum, uint32_t pattern, int palette);
		
		template<bool priority, bool h_s, bool flip>
		static FORCE_INLINE void T_PutLine_Sprite(int disp_pixnum, uint32_t pattern, int palette);
		
		template<bool plane>
		static FORCE_INLINE uint16_t T_Get_X_Offset(void);
		
		template<bool plane, bool interlaced>
		static FORCE_INLINE unsigned int T_Update_Y_Offset(int cell_cur);
		
		template<bool plane>
		static FORCE_INLINE uint16_t T_Get_Pattern_Info(unsigned int x, unsigned int y);
		
		template<bool interlaced>
		static FORCE_INLINE unsigned int T_Get_Pattern_Data(uint16_t pattern);
		
		template<bool plane, bool interlaced, bool vscroll, bool h_s>
		static FORCE_INLINE void T_Render_Line_Scroll(int cell_start, int cell_length);
		
		template<bool interlaced, bool vscroll, bool h_s>
		static FORCE_INLINE void T_Render_Line_ScrollA(void);
		
		template<bool interlaced, bool partial>
		static FORCE_INLINE void T_Make_Sprite_Struct(void);
		
		template<bool sprite_limit, bool interlaced>
		static FORCE_INLINE unsigned int T_Update_Mask_Sprite(void);
		
		template<bool interlaced, bool h_s>
		static FORCE_INLINE void T_Render_Line_Sprite(void);
		
		template<bool interlaced, bool h_s>
		static FORCE_INLINE void T_Render_Line_m5(void);
		
		template<typename pixel>
		static FORCE_INLINE void T_Render_LineBuf(pixel *dest, pixel *md_palette);
	
	private:
		Vdp() { }
		~Vdp() { }
};

/**
 * MarkCRamDirty(): Mark CRam as dirty.
 */
inline void Vdp::MarkCRamDirty(void)
	{ ms_UpdateFlags.CRam = 1; }

/**
 * MarkVRamDirty(): Mark VRam as dirty.
 */
inline void Vdp::MarkVRamDirty(void)
	{ ms_UpdateFlags.VRam = 1; }

inline int Vdp::GetHPix(void)
	{ return H_Pix; }

inline int Vdp::GetHPixBegin(void)
	{ return H_Pix_Begin; }

inline int Vdp::GetHCells(void)
	{ return H_Cell; }

inline int Vdp::GetVPix(void)
	{ return VDP_Lines.Visible.Total; }

/** VDP address functions: Get Pointers. **/
inline uint16_t *Vdp::ScrA_Addr_Ptr16(uint16_t offset)
	{ return &VRam.u16[((ScrA_Addr + offset) & 0xFFFF) >> 1]; }
inline uint16_t *Vdp::ScrB_Addr_Ptr16(uint16_t offset)
	{ return &VRam.u16[((ScrB_Addr + offset) & 0xFFFF) >> 1]; }
inline uint16_t *Vdp::Win_Addr_Ptr16(uint16_t offset)
	{ return &VRam.u16[((Win_Addr + offset) & 0xFFFF) >> 1]; }
inline uint16_t *Vdp::Spr_Addr_Ptr16(uint16_t offset)
	{ return &VRam.u16[((Spr_Addr + offset) & 0xFFFF) >> 1]; }
inline uint16_t *Vdp::H_Scroll_Addr_Ptr16(uint16_t offset)
	{ return &VRam.u16[((H_Scroll_Addr + offset) & 0xFFFF) >> 1]; }

/** VDP address functions: Get Values. **/
inline uint16_t Vdp::ScrA_Addr_u16(uint16_t offset)
	{ return VRam.u16[((ScrA_Addr + offset) & 0xFFFF) >> 1]; }
inline uint16_t Vdp::ScrB_Addr_u16(uint16_t offset)
	{ return VRam.u16[((ScrB_Addr + offset) & 0xFFFF) >> 1]; }
inline uint16_t Vdp::Win_Addr_u16(uint16_t offset)
	{ return VRam.u16[((Win_Addr + offset) & 0xFFFF) >> 1]; }
inline uint16_t Vdp::Spr_Addr_u16(uint16_t offset)
	{ return VRam.u16[((Spr_Addr + offset) & 0xFFFF) >> 1]; }
inline uint16_t Vdp::H_Scroll_Addr_u16(uint16_t offset)
	{ return VRam.u16[((H_Scroll_Addr + offset) & 0xFFFF) >> 1]; }

}

#endif /* __LIBGENS_MD_VDPIO_HPP__ */
