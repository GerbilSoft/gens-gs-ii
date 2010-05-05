/***************************************************************************
 * libgens: Gens Emulation Library.                                        *
 * EmuMD.cpp: MD emulation code.                                           *
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

#include "EmuMD.hpp"

// VDP I/O and renderer.
#include "VdpIo.hpp"
#include "VdpRend.hpp"
#include "VdpPalette.hpp"

namespace LibGens
{

/**
 * EmuMD::Init_TEST(): Test function for the MD VDP.
 */
void EmuMD::Init_TEST(void)
{
	// Initialize the VDP.
	VdpIo::Reset();
	
	// Recalculate the full MD palette.
	// TODO: This would usually be done at program startup.
	VdpPalette::Recalc();
	
	// Set the background color.
	VdpIo::CRam.u16[0] = 0x888;
	VdpIo::VDP_Flags.CRam = 1;
	
	// TODO: VdpIo::VDP_Lines.Display.Total isn't being set properly...
	VdpIo::VDP_Lines.Display.Total = 262;
	
	// Run a frame with the VDP enabled.
	T_Do_Frame<true>();
}


/**
 * T_Do_Line(): Run a scanline.
 * @param LineType Line type.
 * @param VDP If true, VDP is updated.
 */
template<EmuMD::LineType_t LineType, bool VDP>
FORCE_INLINE void EmuMD::T_Do_Line(void)
{
	// TODO: Sound, I/O, CPU. (LibGens)
#if 0
	int *buf[2];
	buf[0] = Seg_L + Sound_Extrapol[VDP_Lines.Display.Current][0];
	buf[1] = Seg_R + Sound_Extrapol[VDP_Lines.Display.Current][0];
	YM2612_DacAndTimers_Update(buf, Sound_Extrapol[VDP_Lines.Display.Current][1]);
	YM_Len += Sound_Extrapol[VDP_Lines.Display.Current][1];
	PSG_Len += Sound_Extrapol[VDP_Lines.Display.Current][1];
		
	Fix_Controllers();
	Cycles_M68K += CPL_M68K;
	Cycles_Z80 += CPL_Z80;
	if (VDP_Reg.DMAT_Length)
		main68k_addCycles(VDP_Update_DMA());
#endif
	
	switch (LineType)
	{
		case LINETYPE_ACTIVEDISPLAY:
			// In visible area.
			VdpIo::VDP_Status |=  0x0004;	// HBlank = 1
#if 0
			// TODO: CPU. (LibGens)
			main68k_exec(Cycles_M68K - 404);
#endif
			VdpIo::VDP_Status &= ~0x0004;	// HBlank = 0
			
			if (--VdpIo::HInt_Counter < 0)
			{
				VdpIo::VDP_Int |= 0x4;
				VdpIo::Update_IRQ_Line();
				VdpIo::HInt_Counter = VdpIo::VDP_Reg.m5.H_Int;
			}
			
			break;
		
		case LINETYPE_VBLANKLINE:
		{
			// VBlank line!
			if (--VdpIo::HInt_Counter < 0)
			{
				VdpIo::VDP_Int |= 0x4;
				VdpIo::Update_IRQ_Line();
			}
			
#if 0
			// TODO: Congratulations! (LibGens)
			CONGRATULATIONS_PRECHECK();
#endif
			VdpIo::VDP_Status |= 0x000C;	// VBlank = 1 et HBlank = 1 (retour de balayage vertical en cours)
			if (VdpIo::VDP_Lines.NTSC_V30.VBlank_Div != 0)
				VdpIo::VDP_Status &= ~0x0008;
			
#if 0
			// TODO: CPU, Congratulations! (LibGens)
			main68k_exec(Cycles_M68K - 360);
			Z80_EXEC(168);
			CONGRATULATIONS_POSTCHECK();
#endif
			
			VdpIo::VDP_Status &= ~0x0004;	// HBlank = 0
			if (VdpIo::VDP_Lines.NTSC_V30.VBlank_Div == 0)
			{
				VdpIo::VDP_Status |=  0x0080;		// V Int happened
				
				VdpIo::VDP_Int |= 0x8;
				VdpIo::Update_IRQ_Line();
#if 0
				// TODO: CPU. (LibGens)
				mdZ80_interrupt(&M_Z80, 0xFF);
#endif
			}
			
			break;
		}
		
		case LINETYPE_BORDER:
		default:
			break;
	}
	
	if (VDP)
	{
		// VDP needs to be updated.
		VdpRend::Render_Line();
	}
	
#if 0
	// TODO: CPU. (LibGens)
	main68k_exec(Cycles_M68K);
	Z80_EXEC(0);
#endif
}


/**
 * T_Do_Frame(): Run a frame.
 * @param VDP If true, VDP is updated.
 */
template<bool VDP>
FORCE_INLINE void EmuMD::T_Do_Frame(void)
{
	// Initialize VDP_Lines.Display.
	VdpIo::Set_Visible_Lines();
	
	// Check if VBlank is allowed.
	VdpIo::Check_NTSC_V30_VBlank();
	
	// TODO: Sound, CPU. (LibGens)
#if 0
	YM_Buf[0] = PSG_Buf[0] = Seg_L;
	YM_Buf[1] = PSG_Buf[1] = Seg_R;
	YM_Len = PSG_Len = 0;
	
	Cycles_M68K = Cycles_Z80 = 0;
	Last_BUS_REQ_Cnt = -1000;
	main68k_tripOdometer();
	mdZ80_clear_odo(&M_Z80);
#endif
	
	// TODO: MDP. (LibGens)
#if 0
	// Raise the MDP_EVENT_PRE_FRAME event.
	EventMgr::RaiseEvent(MDP_EVENT_PRE_FRAME, NULL);
#endif
	
	// Set the VRam flag to force a VRam update.
	VdpIo::VDP_Flags.VRam = 1;
	
	// Interlaced frame status.
	// Both Interlaced Modes 1 and 2 set this bit on odd frames.
	// This bit is cleared on even frames and if not running in interlaced mode.
	if (VdpIo::VDP_Reg.m5.Set4 & 0x06)
		VdpIo::VDP_Status ^= 0x0010;
	else
		VdpIo::VDP_Status &= ~0x0010;
	
	/** Main execution loops. **/
	
	/** Loop 0: Top border. **/
	for (VdpIo::VDP_Lines.Display.Current = 0;
	     VdpIo::VDP_Lines.Visible.Current < 0;
	     VdpIo::VDP_Lines.Display.Current++, VdpIo::VDP_Lines.Visible.Current++)
	{
		T_Do_Line<LINETYPE_BORDER, VDP>();
	}
	
	/** Visible line 0. **/
	VdpIo::HInt_Counter = VdpIo::VDP_Reg.m5.H_Int;	// Initialize HInt_Counter.
	VdpIo::VDP_Status &= ~0x0008;			// Clear VBlank status.
	
	/** Loop 1: Active display. **/
	for (;
	     VdpIo::VDP_Lines.Visible.Current < VdpIo::VDP_Lines.Visible.Total;
	     VdpIo::VDP_Lines.Display.Current++, VdpIo::VDP_Lines.Visible.Current++)
	{
		T_Do_Line<LINETYPE_ACTIVEDISPLAY, VDP>();
	}
	
	/** Loop 2: VBlank line. **/
	T_Do_Line<LINETYPE_VBLANKLINE, VDP>();
	
	/** Loop 3: Bottom border. **/
	for (VdpIo::VDP_Lines.Display.Current++, VdpIo::VDP_Lines.Visible.Current++;
	     VdpIo::VDP_Lines.Display.Current < VdpIo::VDP_Lines.Display.Total;
	     VdpIo::VDP_Lines.Display.Current++, VdpIo::VDP_Lines.Visible.Current++)
	{
		T_Do_Line<LINETYPE_BORDER, VDP>();
	}
	
	// TODO: Sound. (LibGens)
#if 0
	// Update the PSG and YM2612 output.
	PSG_Special_Update();
	YM2612_Special_Update();
	
	// If WAV or GYM is being dumped, update the WAV or GYM.
	// TODO: VGM dumping
	if (WAV_Dumping)
		wav_dump_update();
	if (GYM_Dumping)
		gym_dump_update(0, 0, 0);
#endif
	
	// TODO: MDP. (LibGens)
#if 0
	// Raise the MDP_EVENT_POST_FRAME event.
	mdp_event_post_frame_t post_frame;
	post_frame.width = vdp_getHPix();
	post_frame.height = VDP_Lines.Visible.Total;
	post_frame.pitch = 336;
	post_frame.bpp = bppMD;
	
	int screen_offset = (TAB336[VDP_Lines.Visible.Border_Size] + 8);
	if (post_frame.width < 320)
		screen_offset += ((320 - post_frame.width) / 2);
	
	if (bppMD == 32)
		post_frame.md_screen = &MD_Screen.u32[screen_offset];
	else
		post_frame.md_screen = &MD_Screen.u16[screen_offset];
	
	EventMgr::RaiseEvent(MDP_EVENT_POST_FRAME, &post_frame);
#endif
}

}
