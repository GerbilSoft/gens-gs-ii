/***************************************************************************
 * libgens: Gens Emulation Library.                                        *
 * Vdp_p.hpp: VDP emulation class. (Private class)                         *
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

#ifndef __LIBGENS_MD_VDP_P_HPP__
#define __LIBGENS_MD_VDP_P_HPP__

#include <stdint.h>
#include "Util/byteswap.h"

// VDP types.
#include "VdpTypes.hpp"
#include "VdpReg.hpp"
#include "VdpPalette.hpp"
#include "VdpStatus.hpp"
#include "VdpStructs.hpp"

#include "VdpRend_Err_p.hpp"

namespace LibGens {

class Vdp;
class VdpPrivate
{
	public:
		VdpPrivate(Vdp *q);
		~VdpPrivate();

	protected:
		friend class Vdp;
		Vdp *const q;
	private:
		// Q_DISABLE_COPY() equivalent.
		// TODO: Add LibGens-specific version of Q_DISABLE_COPY().
		VdpPrivate(const VdpPrivate &);
		VdpPrivate &operator=(const VdpPrivate &);

	public:
		// Default VDP emulation options.
		static const VdpTypes::VdpEmuOptions_t def_vdpEmuOptions;

		// Screen mode. [H32, H40]
		// NOTE: Signed, not unsigned, due to
		// the way some things are calculated.
		int H_Cell;		// [16, 20]
		int H_Pix;		// [256, 320]
		int H_Pix_Begin;	// [32, 0] (may not be necessary?)

		/**
		 * Determine if the display mode is H40. (320px wide)
		 * @return True if H40; false otherwise.
		 */
		inline bool isH40(void) const
		{
			// H40 mode is activated by setting VDP_Reg.m5.Set4, bit 0 (0x01, RS1).
			// Bit 7 (0x80, RS0) is also needed, but RS1 is what tells the VDP
			// to increase the pixel counters to 320px per line.
			// Source: http://wiki.megadrive.org/index.php?title=VDPRegs_Addendum (Jorge)
			return (VDP_Reg.m5.Set4 & 0x01);
		}

		// Window row shift. (H40 == 6, H32 == 5)
		unsigned int H_Win_Shift;

		// VDP scroll. (convenience values)
		unsigned int V_Scroll_MMask;
		unsigned int H_Scroll_Mask;

		unsigned int H_Scroll_CMul;
		unsigned int H_Scroll_CMask;
		unsigned int V_Scroll_CMask;

		/**
		 * Scroll_Size_t: Convenience enum for dealing with scroll plane sizes.
		 */
		enum Scroll_Size_t {
			V32_H32 = 0, V32_H64,  V32_HXX,  V32_H128,
			V64_H32,     V64_H64,  V64_HXX,  V64_H128,
			VXX_H32,     VXX_H64,  VXX_HXX,  VXX_H128,
			V128_H32,    V128_H64, V128_HXX, V128_H128
		};

		// VDP window. (convenience values)
		unsigned int Win_X_Pos;
		unsigned int Win_Y_Pos;

		// Interlaced mode.
		VdpTypes::Interlaced_t Interlaced;

		// Sprite dot overflow.
		// If set, the previous line had a sprite dot overflow.
		// This is needed to properly implement Sprite Masking in S1.
		int SpriteDotOverflow;

		/**
		 * VDP_Mode: Current VDP mode.
		 * TODO: Mark as private after integrating VdpRend_Err within the Vdp class.
		 */
		enum VDP_Mode_t {
			// Individual mode bits.
			VDP_MODE_M1 = (1 << 0),
			VDP_MODE_M2 = (1 << 1),
			VDP_MODE_M3 = (1 << 2),
			VDP_MODE_M4 = (1 << 3),
			VDP_MODE_M5 = (1 << 4),

			// TMS9918 modes.
			// TODO: Add invalid modes?
			VDP_MODE_TMS_GRAPHIC_I = 0,
			VDP_MODE_TMS_TEXT = 1,
			VDP_MODE_TMS_GRAPHIC_II = 2,
			VDP_MODE_TMS_MULTICOLOR = 4,

			// Sega Master System II modes.
			VDP_MODE_M4_224 = 0xB,
			VDP_MODE_M4_240 = 0xE,
		};
		VDP_Mode_t VDP_Mode;
		void updateVdpMode(void);

		// VRam, VSRam.
		// NOTE: VdpSpriteMaskingTest requires access to VRam.
		// ZOMG functions can be used as a workaround.
		VdpTypes::VRam_t VRam;
		VdpTypes::VSRam_t VSRam;

		int HInt_Counter;	// Horizontal Interrupt Counter.
		int VDP_Int;		// VDP interrupt state.
		VdpStatus Reg_Status;	// VDP status register.
		uint16_t testReg;	// Test register.

		// VDP registers.
		VdpTypes::VdpReg_t VDP_Reg;

		/** VDP tables. **/
		// TODO: Precalculate H_Counter_Table?
		uint8_t H_Counter_Table[512][2];

		/**
		 * Set a VDP register.
		 * @param reg_num Register number. (0-10 for M4; 0-23 for M5)
		 * @param val New value for the register.
		 */
		void setReg(int reg_num, uint8_t val);

		/**
		 * Internal VDP data write function.
		 * Used by writeDataMD() and DMA.
		 * @param data Data word.
		 */
		void vdpDataWrite_int(uint16_t data);

		// Update flags.
		union {
			uint8_t flags;
			struct {
				bool VRam	:1;	// VRam was modified. (Implies VRam_Spr.)
				bool VRam_Spr	:1;	// Sprite Attribute Table was modified.
			};
		} m_updateFlags;
		inline void markVRamDirty(void)
			{ m_updateFlags.VRam = true; }

		/**
		 * VDP address pointers.
		 * These are relative to VRam[] and are based on register values.
		 * TODO: Convert to uint32_t for 128 KB support.
		 */
		uint16_t ScrA_Addr;
		uint16_t ScrB_Addr;
		uint16_t Win_Addr;
		uint16_t Spr_Addr;
		uint16_t H_Scroll_Addr;

		/** VDP address functions: Get Pointers. **/
		inline uint16_t *ScrA_Addr_Ptr16(uint16_t offset)
			{ return &VRam.u16[((ScrA_Addr + offset) & 0xFFFF) >> 1]; }
		inline uint16_t *ScrB_Addr_Ptr16(uint16_t offset)
			{ return &VRam.u16[((ScrB_Addr + offset) & 0xFFFF) >> 1]; }
		inline uint16_t *Win_Addr_Ptr16(uint16_t offset)
			{ return &VRam.u16[((Win_Addr + offset) & 0xFFFF) >> 1]; }
		inline VdpStructs::SprEntry_m5 *Spr_Addr_PtrM5(uint16_t offset)
			{ return (VdpStructs::SprEntry_m5*)&VRam.u16[((Spr_Addr + offset) & 0xFFFF) >> 1]; }
		inline uint16_t *H_Scroll_Addr_Ptr16(uint16_t offset)
			{ return &VRam.u16[((H_Scroll_Addr + offset) & 0xFFFF) >> 1]; }

		/** VDP address functions: Get Values. **/
		inline uint16_t ScrA_Addr_u16(uint16_t offset) const
			{ return VRam.u16[((ScrA_Addr + offset) & 0xFFFF) >> 1]; }
		inline uint16_t ScrB_Addr_u16(uint16_t offset) const
			{ return VRam.u16[((ScrB_Addr + offset) & 0xFFFF) >> 1]; }
		inline uint16_t Win_Addr_u16(uint16_t offset) const
			{ return VRam.u16[((Win_Addr + offset) & 0xFFFF) >> 1]; }
		inline uint16_t Spr_Addr_u16(uint16_t offset) const
			{ return VRam.u16[((Spr_Addr + offset) & 0xFFFF) >> 1]; }
		inline uint16_t H_Scroll_Addr_u16(uint16_t offset) const
			{ return VRam.u16[((H_Scroll_Addr + offset) & 0xFFFF) >> 1]; }

		// VDP control struct.
		struct {
			/**
			 * ctrl_latch: Control word latch.
			 * 0: Next control word is FIRST word.
			 * 1: Next control word is SECOND word, which triggers an action.
			 */
			uint8_t ctrl_latch;	// Control word latch.

			// VDP internal registers.
			uint8_t code;		// Access code. (CD5-CD0)
			uint16_t address;	// Address counter.

			// DMA values.
			uint8_t DMA_Mode;	// (DMA ADDRESS HIGH & 0xC0) [reg 23]
			//int DMAT_Length;	// TODO: Move it here.

			void reset(void)
			{
				ctrl_latch = 0;
				code = 0;
				address = 0;
				DMA_Mode = 0;
			}
		} VDP_Ctrl;

		/** DMA helper functions. **/

		/**
		 * Get the DMA length from the VDP Mode 5 registers.
		 * @return DMA length.
		 */
		inline uint16_t DMA_Length(void) const
		{
			return (VDP_Reg.m5.DMA_Length_H << 8) |
			       (VDP_Reg.m5.DMA_Length_L);
		}

		/**
		 * Set the DMA length in the VDP Mode 5 registers.
		 * @param length DMA length.
		 */
		inline void set_DMA_Length(uint16_t length)
		{
			VDP_Reg.m5.DMA_Length_H = (length >> 8);
			VDP_Reg.m5.DMA_Length_L = (length & 0xFF);
		}

		/**
		 * Get the DMA source address from the VDP Mode 5 registers.
		 * @return DMA source address, divided by 2.
		 */
		inline uint32_t DMA_Src_Adr(void) const
		{
			return ((VDP_Reg.m5.DMA_Src_Adr_H & 0x7F) << 16) |
				(VDP_Reg.m5.DMA_Src_Adr_M << 8) |
				(VDP_Reg.m5.DMA_Src_Adr_L);
		}

		/**
		 * Increment the DMA source address from the VDP Mode 5 registers.
		 * Implements 128 KB wrapping, so only use this from DMA functions!
		 * @param length Number of words to increment the source address by.
		 */
		inline void inc_DMA_Src_Adr(uint16_t length)
		{
			uint16_t src_adr = (VDP_Reg.m5.DMA_Src_Adr_M << 8) |
					   (VDP_Reg.m5.DMA_Src_Adr_L);

			// Increment the address with 16-bit overflow.
			src_adr += length;

			// Save the new address.
			VDP_Reg.m5.DMA_Src_Adr_M = (src_adr >> 8);
			VDP_Reg.m5.DMA_Src_Adr_L = (src_adr & 0xFF);
		}

		/**
		 * DMA transfer type:
		 * - 0: External ROM/RAM to VRAM
		 * - 1: External ROM/RAM to CRAM/VSRAM
		 * - 2: DMA FILL
		 * - 3: DMA COPY
		 */
		enum DMAT_Type_t {
			DMAT_MEM_TO_VRAM = 0,
			DMAT_MEM_TO_CRAM_VSRAM,
			DMAT_FILL,
			DMAT_COPY,
		};
		DMAT_Type_t DMAT_Type;

		void DMA_Fill(uint16_t data);

		enum DMA_Dest_t {
			DMA_DEST_VRAM	= 1,
			DMA_DEST_CRAM	= 2,
			DMA_DEST_VSRAM	= 3,
		};

		enum DMA_Src_t {
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
		inline void T_DMA_Loop(void);

	/*!**************************************************************
	 * VdpRend: Rendering functions and variables.                  *
	 ****************************************************************/
	public:
		/**
		 * NOTE: rend_init(), rend_end(), and rend_reset() should ONLY be called from
		 * Vdp::Vdp(), Vdp::~Vdp(), and Vdp::Reset()!
		 */
		void rend_init(void);
		void rend_end(void);
		void rend_reset(void);

		// Palette manager.
		VdpPalette palette;

		// VDP layer control.
		unsigned int VDP_Layers;

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
		void renderLine_m5(void);

		// Sprite structs.
		struct {
			int Pos_X;
			int Pos_Y;
			unsigned int Size_X;
			unsigned int Size_Y;
			int Pos_X_Max;
			int Pos_Y_Max;
			unsigned int Num_Tile;	// Includes palette, priority, and flip bits.
			int Pos_X_Max_Vis;	// Number of visible horizontal pixels. (Used for Sprite Limit.)
		} Sprite_Struct[80];
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
	public:
		friend class VdpRend_Err_Private;
		VdpRend_Err_Private *const d_err;

		void renderLine_Err(void);
		void updateErr(void);
};

}

#endif /* __LIBGENS_MD_VDP_P_HPP__ */
