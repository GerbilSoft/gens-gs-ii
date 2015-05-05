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

// Needed for FORCE_INLINE.
#include "../macros/common.h"

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
		unsigned int H_Cell;	// [16, 20]
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

		/**
		 * Check if 128 KB VRAM mode is enabled.
		 * This checks both the register value and
		 * whether or not 128 KB is enabled in the emulator.
		 * @return True if 128 KB VRAM mode is enabled.
		 */
		inline bool is128KB(void) const
		{
			// TODO: Implement 128 KB mode.
			return false;
		}

		// Window row shift. (H40 == 6, H32 == 5)
		uint8_t H_Win_Shift;

		// Scroll mode masks. (Reg.11)
		uint8_t V_Scroll_MMask;
		uint8_t H_Scroll_Mask;

		// Scroll size masks. (Reg.16)
		uint8_t H_Scroll_CMul;
		uint8_t H_Scroll_CMask;
		uint8_t V_Scroll_CMask;

		// VDP window. (convenience values)
		unsigned int Win_X_Pos;
		unsigned int Win_Y_Pos;

		// Is Interlaced Mode 2 set?
		// Cached flag that can be overridden
		// by VDP options.
		bool im2_flag;

		/**
		 * Is Interlaced Mode 1 set?
		 * @return True if IM1 is set; false if not.
		 */
		inline bool isIM1(void) const
			{ return ((VDP_Reg.m5.Set4 & 0x06) == 0x02); }

		/**
		 * Is Interlaced Mode 2 set?
		 * @return True if IM2 is set; false if not.
		 */
		inline bool isIM2(void) const
			{ return ((VDP_Reg.m5.Set4 & 0x06) == 0x06); }

		/**
		 * Is any Interlaced mode set?
		 * @return True if either IM1 or IM2 is set; false if not.
		 */
		inline bool isIM1orIM2(void) const
			{ return ((VDP_Reg.m5.Set4 & 0x02) == 0x02); }

		// VDP mode.
		VdpTypes::VDP_Mode_t VDP_Mode;
		void updateVdpMode(void);

		// VDP model.
		// TODO: Allow user to change this.
		VdpTypes::VDP_Model_t VDP_Model;

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
		 * Reset the VDP registers.
		 * @param bootRomFix If true, set the registers as the boot ROM would.
		 */
		void resetRegisters(bool bootRomFix);

		/**
		 * Internal VDP data write function.
		 * Used by writeDataMD() and DMA.
		 * @param data Data word.
		 */
		void vdpDataWrite_int(uint16_t data);

		/**
		 * VDP address pointers.
		 * These are relative to VRam[] and are based on register values.
		 */
		uint32_t ScrA_Tbl_Addr;		// Scroll A Name Table base address.
		uint32_t ScrB_Tbl_Addr;		// Scroll B Name Table base address.
		uint32_t Win_Tbl_Addr;		// Window Name Table bas address.
		uint32_t Spr_Tbl_Addr;		// Sprite Table base address.
		uint32_t H_Scroll_Tbl_Addr;	// Horizontal Scroll Table base address.

		// Pattern base addresses.
		uint32_t ScrA_Gen_Addr;		// Scroll A Pattern Generator base address.
		uint32_t ScrB_Gen_Addr;		// Scroll B Pattern Generator base address.
		uint32_t Spr_Gen_Addr;		// Sprite Pattern Generator base address.

		/**
		 * VDP address masks.
		 * H40 mode typically has one bit masked compared to H32
		 * for certain registers. This mask also handles the extra
		 * address bit for 128 KB mode.
		 */
		uint32_t VRam_Mask;		// Based on 128 KB.
		uint32_t ScrA_Tbl_Mask;		// Based on 128 KB.
		uint32_t ScrB_Tbl_Mask;		// Based on 128 KB.
		uint32_t Win_Tbl_Mask;		// Based on 128 KB and H32/H40.
		uint32_t Spr_Tbl_Mask;		// Based on 128 KB and H32/H40.
		uint32_t H_Scroll_Tbl_Mask;	// Based on 128 KB.

		/** VDP address functions: Get Pointers. **/
		// FIXME: "& VRam_Mask" is probably not needed,
		// since the table addresses are already masked.
		// Name Tables
		inline uint16_t *ScrA_Tbl_Addr_Ptr16(uint32_t offset)
			{ return &VRam.u16[((ScrA_Tbl_Addr + offset) & VRam_Mask) >> 1]; }
		inline uint16_t *ScrB_Tbl_Addr_Ptr16(uint32_t offset)
			{ return &VRam.u16[((ScrB_Tbl_Addr + offset) & VRam_Mask) >> 1]; }
		inline uint16_t *Win_Tbl_Addr_Ptr16(uint32_t offset)
			{ return &VRam.u16[((Win_Tbl_Addr + offset) & VRam_Mask) >> 1]; }
		inline VdpStructs::SprEntry_m5 *Spr_Tbl_Addr_PtrM5(uint32_t link)
			{ return (VdpStructs::SprEntry_m5*)&VRam.u16[((Spr_Tbl_Addr + (link*8)) & VRam_Mask) >> 1]; }
		inline uint16_t *H_Scroll_Tbl_Addr_Ptr16(uint32_t offset)
			{ return &VRam.u16[((H_Scroll_Tbl_Addr + offset) & VRam_Mask) >> 1]; }
		// Pattern Generators
		inline uint16_t *ScrA_Gen_Addr_Ptr16(uint32_t offset)
			{ return &VRam.u16[((ScrA_Gen_Addr + offset) & VRam_Mask) >> 1]; }
		inline uint16_t *ScrB_Gen_Addr_Ptr16(uint32_t offset)
			{ return &VRam.u16[((ScrB_Gen_Addr + offset) & VRam_Mask) >> 1]; }
		inline uint16_t *Win_Gen_Addr_Ptr16(uint32_t offset)	// Same as Scroll A.
			{ return ScrA_Gen_Addr_Ptr16(offset); }
		inline uint16_t *Spr_Gen_Addr_Ptr16(uint32_t offset)
			{ return &VRam.u16[((Spr_Gen_Addr + offset) & VRam_Mask) >> 1]; }

		/** VDP address functions: Get Values. **/
		// Name Tables
		inline uint16_t ScrA_Tbl_Addr_u16(uint32_t offset) const
			{ return VRam.u16[((ScrA_Tbl_Addr + offset) & VRam_Mask) >> 1]; }
		inline uint16_t ScrB_Tbl_Addr_u16(uint32_t offset) const
			{ return VRam.u16[((ScrB_Tbl_Addr + offset) & VRam_Mask) >> 1]; }
		inline uint16_t Win_Tbl_Addr_u16(uint32_t offset) const
			{ return VRam.u16[((Win_Tbl_Addr + offset) & VRam_Mask) >> 1]; }
		inline uint16_t Spr_Tbl_Addr_u16(uint32_t offset) const
			{ return VRam.u16[((Spr_Tbl_Addr + offset) & VRam_Mask) >> 1]; }
		inline uint16_t H_Scroll_Tbl_Addr_u16(uint32_t offset) const
			{ return VRam.u16[((H_Scroll_Tbl_Addr + offset) & VRam_Mask) >> 1]; }
		// Pattern Generators
		inline uint16_t ScrA_Gen_Addr_u16(uint32_t offset) const
			{ return VRam.u16[((ScrA_Gen_Addr + offset) & VRam_Mask) >> 1]; }
		inline uint16_t ScrB_Gen_Addr_u16(uint32_t offset) const
			{ return VRam.u16[((ScrB_Gen_Addr + offset) & VRam_Mask) >> 1]; }
		inline uint16_t Win_Gen_Addr_u16(uint32_t offset) const	// Same as Scroll A.
			{ return ScrA_Gen_Addr_u16(offset); }
		inline uint32_t Spr_Gen_Addr_u32(uint32_t offset) const
			{ return VRam.u32[((Spr_Gen_Addr + offset) & VRam_Mask) >> 2]; }

		/**
		 * Update the VDP address cache. (Mode 5)
		 * @param updateMask 1 == 64KB/128KB; 2 == H32/H40; 3 == both
		 */
		void updateVdpAddrCache_m5(unsigned int updateMask);

		/**
		 * Set the scroll plane size. (Mode 5 only!)
		 * @param val Register value.
		 * Format: [   0    0 VSZ1 VSZ0    0    0 HSZ1 HSZ0]
		 */
		void setScrollSize_m5(uint8_t val);

		// VDP control struct.
		struct {
			/**
			 * ctrl_latch: Control word latch.
			 * 0: Next control word is FIRST word.
			 * 1: Next control word is SECOND word, which triggers an action.
			 */
			uint8_t ctrl_latch;	// Control word latch.

			// DMA values.
			uint8_t DMA_Mode;	// (DMA ADDRESS HIGH & 0xC0) [reg 23]
			//int DMAT_Length;	// TODO: Move it here.
			uint16_t reserved;

			// VDP internal registers.
			// addr_hi_latch is stored with the bits in the
			// shifted A16-A14 positions for performance reasons.
			// Reference: http://gendev.spritesmind.net/forum/viewtopic.php?t=1277&p=17430#17430
			uint32_t address;	// Address counter.
			uint32_t addr_hi_latch;	// Latch for A16-A14.
			uint8_t code;		// Access code. (CD5-CD0)

			// TODO: Address LSB latch for MD Mode 4?

			/**
			 * Data latch.
			 * - Mega Drive in SMS mode: Control word latch.
			 *   First byte of the control word is latched,
			 *   compared to SMS where it's applied immediately.
			 * - Game Gear: CRAM latch.
			 *   Writes to even CRAM addresses go here.
			 *   Writes to odd CRAM addresses act as a word write,
			 *   with the written data as the even byte and
			 *   this latch as the odd byte.
			 *   NOTE: Game Gear CRAM is LE16.
			 */
			uint8_t data_latch;

			void reset(void)
			{
				ctrl_latch = 0;
				DMA_Mode = 0;
				reserved = 0;
				address = 0;
				addr_hi_latch = 0;
				code = 0;
				data_latch = 0;
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

		void processDmaCtrlWrite(void);

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
		union LineBuf_t {
			struct LineBuf_px_t {
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

	private:
		// Sprite Attribute Table cache. (Mode 5)
		// NOTE: Only 80 entries are present on the actual VDP,
		// but we have 128 here to prevent overflows.
		static const int SprAttrTbl_sz = (80 * sizeof(VdpStructs::SprEntry_m5));
		union {
			// Direct byte/word access for SAT caching.
			uint8_t b[128*8];
			uint16_t w[128*4];
			// Sprite entries.
			VdpStructs::SprEntry_m5 spr[128];
		} SprAttrTbl_m5;

		// Sprite line cache.
		// Caches the current line and the next line.
		// TODO: Reduce to a power-of-two size?
		struct SprLineCache_t {
			int16_t Pos_X;
			int16_t Pos_Y;
			uint8_t Size_X;
			uint8_t Size_Y;
			int16_t Pos_Y_Max;
			union {
				uint16_t Num_Tile;	// M5: Includes palette, priority, and flip bits.
				struct {
					// M4/TMS sprite information.
#if GENS_BYTEORDER == GENS_LIL_ENDIAN
					uint8_t sprite;	// TMS/M4: Sprite pattern number.
					uint8_t color;	// TMS:    Color number.
#else /* GENS_BYTEORDER == GENS_BIG_ENDIAN */
					uint8_t color;	// TMS:    Color number.
					uint8_t sprite;	// TMS/M4: Sprite pattern number.
#endif
				};
			};
		};

		/**
		 * Sprite line cache array.
		 * - TMS9918A: 4 sprites per line.
		 * - SMS/GG: 8 sprites per line.
		 * - MD: 16 or 20 sprites per line.
		 * - Full 80 is used if sprite limits are disabled.
		 *
		 * NOTE: This was previously a union of variously-sized
		 * arrays, e.g. tms[2][4], sms[2][8], etc, but it has
		 * been changed to prevent issues on mode switch, since
		 * the previous approach would result in wrong addresses:
		 * &sms[1][0] == &md[0][8]
		 */
		SprLineCache_t sprLineCache[2][80];

		// Sprite count cache.
		// Includes both the current line and the next line.
		uint8_t sprCountCache[2];

	/*!*****************************************
	 * VdpRend_m5: Mode 5 rendering functions. *
	 *******************************************/
	public:
		// Sprite dot overflow flag.
		// If set, the previous line had a sprite dot overflow.
		// This is needed to properly implement Sprite Masking in S1.
		bool sprDotOverflow;

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
		FORCE_INLINE uint16_t T_Get_Nametable_Word(unsigned int x, unsigned int y);

		template<bool interlaced>
		FORCE_INLINE uint32_t T_Get_Pattern_Data(uint16_t pattern, unsigned int y_fine_offset);

		template<bool plane, bool interlaced, bool vscroll, bool h_s>
		FORCE_INLINE void T_Render_Line_Scroll(int cell_start, int cell_length);

		template<bool interlaced, bool vscroll, bool h_s>
		FORCE_INLINE void T_Render_Line_ScrollA_Window(void);

		FORCE_INLINE void Update_Sprite_Line_Cache_m5(int line);

		template<bool interlaced>
		FORCE_INLINE unsigned int T_Update_Sprite_Line_Cache_m5(int line);

		template<bool interlaced, bool h_s>
		FORCE_INLINE void T_Render_Line_Sprite(void);

		template<bool interlaced, bool h_s>
		FORCE_INLINE void T_Render_Line_m5(void);

		template<typename pixel>
		FORCE_INLINE void T_Render_LineBuf(pixel *dest, pixel *md_palette);

		template<typename pixel>
		FORCE_INLINE void T_Apply_SMS_LCB(pixel *dest, pixel border_color);

	/*!*****************************************
	 * VdpRend_m4: Mode 4 rendering functions. *
	 *******************************************/
	public:
		FORCE_INLINE unsigned int Update_Sprite_Line_Cache_m4(int line);

	/*!******************************************************
	 * VdpRend_tms: TMS9918A Modes 0-3 rendering functions. *
	 ********************************************************/
	public:
		FORCE_INLINE unsigned int Update_Sprite_Line_Cache_tms(int line);

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
