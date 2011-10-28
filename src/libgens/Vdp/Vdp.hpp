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
#include "VdpReg.hpp"
#include "VdpPalette.hpp"
#include "VdpStatus.hpp"

namespace LibGens
{

class VdpRend_Err_Private;

class Vdp
{
	public:
		Vdp();
		~Vdp();
	
	private:
		// Q_DISABLE_COPY() equivalent.
		// TODO: Add LibGens-specific version of Q_DISABLE_COPY().
		Vdp(const Vdp &);
		Vdp &operator=(const Vdp &);
	
	public:
		/**
		 * reset(): Reset the VDP.
		 */
		void reset(void);
		
		/**
		 * VDP emulation options.
		 * TODO: Move somewhere else?
		 * TODO: Keep static?
		 */
		static VdpTypes::VdpEmuOptions_t VdpEmuOptions;
		
		// Update flags.
	public:
		void MarkVRamDirty(void);
	private:
		VdpTypes::UpdateFlags_t ms_UpdateFlags;
	
	/*!**************************************************************
	 * VdpIo: I/O registers and variables.                          *
	 ****************************************************************/
	
	public:
		// VDP registers.
		VdpTypes::VdpReg_t VDP_Reg;
	
		/** DMA variables. **/
	private:
		// These two variables are internal to Gens.
		// They don't map to any actual VDP registers.
		// TODO: Do we need separate DMA_Length variables,
		// or are the DMA length and address registers used by the VDP?
		int DMA_Length;
		unsigned int DMA_Address;
	
		// DMAT variables.
		// TODO: Mark DMAT_Length private.
	public:	
		int DMAT_Length;
	private:
		unsigned int DMAT_Tmp;
		unsigned int DMAT_Type;
	
	private:
		/** VDP address functions: Get Pointers. **/
		uint16_t *ScrA_Addr_Ptr16(uint16_t offset);
		uint16_t *ScrB_Addr_Ptr16(uint16_t offset);
		uint16_t *Win_Addr_Ptr16(uint16_t offset);
		uint16_t *Spr_Addr_Ptr16(uint16_t offset);
		uint16_t *H_Scroll_Addr_Ptr16(uint16_t offset);
		
		/** VDP address functions: Get Values. **/
		uint16_t ScrA_Addr_u16(uint16_t offset);
		uint16_t ScrB_Addr_u16(uint16_t offset);
		uint16_t Win_Addr_u16(uint16_t offset);
		uint16_t Spr_Addr_u16(uint16_t offset);
		uint16_t H_Scroll_Addr_u16(uint16_t offset);
		
		/**
		 * VDP address pointers.
		 * These are relative to VRam[] and are based on register values.
		 */
		uint16_t ScrA_Addr;
		uint16_t ScrB_Addr;
		uint16_t Win_Addr;
		uint16_t Spr_Addr;
		uint16_t H_Scroll_Addr;
		
		/**
		 * Window row shift.
		 * H40: 6. (64x32 window)
		 * H32: 5. (32x32 window)
		 */
		unsigned int H_Win_Shift;
		
		// VDP convenience values: Scroll.
		unsigned int V_Scroll_MMask;
		unsigned int H_Scroll_Mask;
		
		unsigned int H_Scroll_CMul;
		unsigned int H_Scroll_CMask;
		unsigned int V_Scroll_CMask;
		
		// TODO: Eliminate these.
		int Win_X_Pos;
		int Win_Y_Pos;
		
		// Interlaced mode.
		VdpTypes::Interlaced_t Interlaced;
		
		// Sprite dot overflow.
		// If set, the previous line had a sprite dot overflow.
		// This is needed to properly implement Sprite Masking in S1.
		int SpriteDotOverflow;
	
	public:
		/**
		* VDP_Mode: Current VDP mode.
		* TODO: Mark as private after integrating VdpRend_Err within the Vdp class.
		*/
		#define VDP_MODE_M1	(1 << 0)
		#define VDP_MODE_M2	(1 << 1)
		#define VDP_MODE_M3	(1 << 2)
		#define VDP_MODE_M4	(1 << 3)
		#define VDP_MODE_M5	(1 << 4)
		unsigned int VDP_Mode;
	
		// Horizontal Interrupt Counter.
		int HInt_Counter;
		
		// VRam, VSRam.
		VdpTypes::VRam_t VRam;
		VdpTypes::VSRam_t VSRam;
		
		int VDP_Int;
		
		// VDP status register.
		VdpStatus Reg_Status;
	
	public:
		// VDP line counters.
		// NOTE: Gens/GS currently uses 312 lines for PAL. It should use 313!
		VdpTypes::VdpLines_t VDP_Lines;
		
		// System status.
		// TODO: Move this to a more relevant file.
		union SysStatus_t
		{
			struct
			{
				unsigned int Genesis	:1;
				unsigned int SegaCD	:1;
				unsigned int _32X	:1;
			};
			unsigned int data;
		};
		SysStatus_t SysStatus;
	
	private:
		/** VDP tables. **/
		uint8_t H_Counter_Table[512][2];
		static const uint8_t DMA_Timing_Table[4][4];
	
	public:
		/** Interrupt functions. **/
		uint8_t Int_Ack(void);
		void Update_IRQ_Line(void);
		
		// Lines.
		void updateVdpLines(bool resetCurrent);
		void Check_NTSC_V30_VBlank(void);
		
		void Set_Reg(int reg_num, uint8_t val);
		
		uint8_t Read_H_Counter(void);
		uint8_t Read_V_Counter(void);
		uint16_t Read_Status(void);
		uint16_t Read_Data(void);
		
		unsigned int Update_DMA(void);
		
		void Write_Data_Byte(uint8_t data);
		void Write_Data_Word(uint16_t data);
		void Write_Ctrl(uint16_t data);
		
		/**
		 * GetHPix(): Get the current horizontal resolution.
		 * NOTE: Do NOT use this if a ROM isn't loaded!
		 * @return Horizontal resolution, in pixels.
		 */
		int GetHPix(void);
		
		/**
		 * GetHPixBegin(): Get the first horizontal pixel number.
		 * NOTE: Do NOT use this if a ROM isn't loaded!
		 * @return First horizontal pixel number.
		 */
		int GetHPixBegin(void);
		
		/**
		 * GetHCells(): Get the current horizontal resolution, in cells.
		 * @return Horizontal resolution, in cells.
		 */
		int GetHCells(void);
		
		/**
		 * GetVPix(): Get the current vertical resolution.
		 * NOTE: Do NOT use this if a ROM isn't loaded!
		 * @return Vertical resolution, in pixels.
		 */
		int GetVPix(void);
	
	private:
		/**
		 * Vdp::Update_Mode(): Update VDP_Mode.
		 */
		void Update_Mode(void);
		
		// VDP convenience values: Horizontal.
		// NOTE: These must be signed for VDP arithmetic to work properly!
		int H_Cell;
		int H_Pix;
		int H_Pix_Begin;

		/**
		 * Determine if the display mode is H40. (320px wide)
		 * @return True if H40; false otherwise.
		 */
		bool isH40(void) const;
		
		/** DMA **/
		
		void DMA_Fill(uint16_t data);
		
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
		inline void T_DMA_Loop(unsigned int src_address, uint16_t dest_address, int length);
		
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
		VDP_Ctrl_t VDP_Ctrl;
		
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
	
	private:
		/**
		 * NOTE: rend_init(), rend_end(), and rend_reset() should ONLY be called from
		 * Vdp::Vdp(), Vdp::~Vdp(), and Vdp::Reset()!
		 */
		void rend_init(void);
		void rend_end(void);
		void rend_reset(void);
	
	public:
		// Palette manager.
		VdpPalette m_palette;
		
		// MD framebuffer.
		MdFb MD_Screen;
	
	public:
		// VDP layer control.
		unsigned int VDP_Layers;
		
		/** Line rendering functions. **/
		void Render_Line(void);
	
	private:
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
		LineBuf_t LineBuf;
		
		template<bool hs, typename pixel>
		inline void T_Update_Palette(pixel *MD_palette, const pixel *palette);
	
	/*!**************************************************************
	 * VdpRend_m5: Mode 5 rendering functions and variables.        *
	 ****************************************************************/
	
	public:
		/** Line rendering functions. **/
		void Render_Line_m5(void);
	
	private:
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
		Sprite_Struct_t Sprite_Struct[80];
		uint8_t Sprite_Visible[80];	// List of visible sprites. (element == sprite idx in Sprite_Struct[])
		uint8_t TotalSprites;		// Total number of visible sprites.
		
		template<bool interlaced>
		FORCE_INLINE int T_GetLineNumber(void) const;
		
		template<bool plane, bool h_s, int pat_pixnum, uint32_t mask, int shift>
		FORCE_INLINE void T_PutPixel_P0(int disp_pixnum, uint32_t pattern, unsigned int palette);
		
		template<bool plane, bool h_s, int pat_pixnum, uint32_t mask, int shift>
		FORCE_INLINE void T_PutPixel_P1(int disp_pixnum, uint32_t pattern, unsigned int palette);
		
		template<bool priority, bool h_s, int pat_pixnum, uint32_t mask, int shift>
		FORCE_INLINE uint8_t T_PutPixel_Sprite(int disp_pixnum, uint32_t pattern, unsigned int palette);
		
		template<bool plane, bool h_s, bool flip>
		FORCE_INLINE void T_PutLine_P0(int disp_pixnum, uint32_t pattern, int palette);
		
		template<bool plane, bool h_s, bool flip>
		FORCE_INLINE void T_PutLine_P1(int disp_pixnum, uint32_t pattern, int palette);
		
		template<bool priority, bool h_s, bool flip>
		FORCE_INLINE void T_PutLine_Sprite(int disp_pixnum, uint32_t pattern, int palette);
		
		template<bool plane>
		FORCE_INLINE uint16_t T_Get_X_Offset(void);
		
		template<bool plane, bool interlaced>
		FORCE_INLINE unsigned int T_Get_Y_Offset(int cell_cur);
		
		template<bool interlaced, bool vscroll_mask>
		FORCE_INLINE unsigned int T_Get_Y_Cell_Offset(unsigned int y_offset);
		
		template<bool interlaced>
		FORCE_INLINE unsigned int T_Get_Y_Fine_Offset(unsigned int y_offset);
		
		template<bool plane>
		FORCE_INLINE uint16_t T_Get_Pattern_Info(unsigned int x, unsigned int y);
		
		template<bool interlaced>
		FORCE_INLINE uint32_t T_Get_Pattern_Data(uint16_t pattern, unsigned int y_fine_offset);
		
		template<bool plane, bool interlaced, bool vscroll, bool h_s>
		FORCE_INLINE void T_Render_Line_Scroll(int cell_start, int cell_length);
		
		template<bool interlaced, bool vscroll, bool h_s>
		FORCE_INLINE void T_Render_Line_ScrollA_Window(void);
		
		template<bool interlaced, bool partial>
		FORCE_INLINE void T_Make_Sprite_Struct(void);
		
		template<bool sprite_limit, bool interlaced>
		FORCE_INLINE unsigned int T_Update_Mask_Sprite(void);
		
		template<bool interlaced, bool h_s>
		FORCE_INLINE void T_Render_Line_Sprite(void);
		
		template<bool interlaced, bool h_s>
		FORCE_INLINE void T_Render_Line_m5(void);
		
		template<typename pixel>
		FORCE_INLINE void T_Render_LineBuf(pixel *dest, pixel *md_palette);
		
		template<typename pixel>
		FORCE_INLINE void T_Apply_SMS_LCB(pixel *dest, pixel border_color);
	
	/*!***************************************************************
	 * VdpRend_Err: Error message rendering functions and variables. *
	 *****************************************************************/
	
	private:
		friend class VdpRend_Err_Private;
		VdpRend_Err_Private *const d_err;
		
		void Render_Line_Err(void);
		void Update_Err(void);
};

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

/**
 * Determine if the display mode is H40. (320px wide)
 * @return True if H40; false otherwise.
 */
inline bool Vdp::isH40(void) const
{
	// H40 mode is activated by setting VDP_Reg.m5.Set4, bit 0 (0x01, RS1).
	// Bit 7 (0x80, RS0) is also needed, but RS1 is what tells the VDP
	// to increase the pixel counters to 320px per line.
	// Source: http://wiki.megadrive.org/index.php?title=VDPRegs_Addendum (Jorge)
	return (VDP_Reg.m5.Set4 & 0x01);
}

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
