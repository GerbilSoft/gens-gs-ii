/***************************************************************************
 * libgens: Gens Emulation Library.                                        *
 * Psg.cpp: TI SN76489 (PSG) emulator.                                     *
 *                                                                         *
 * Copyright (c) 1999-2002 by Stéphane Dallongeville                       *
 * Copyright (c) 2003-2004 by Stéphane Akhoun                              *
 * Copyright (c) 2008-2010 by David Korth                                  *
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

/***********************************************************/
/*                                                         */
/* PSG.C : SN76489 emulator                                */
/*                                                         */
/* Noise define constantes taken from MAME                 */
/*                                                         */
/* This source is a part of Gens project                   */
/* Written by Stéphane Dallongeville (gens@consolemul.com) */
/* Copyright (c) 2002 by Stéphane Dallongeville            */
/*                                                         */
/***********************************************************/

#include "Psg.hpp"

// C includes.
#include <stdint.h>
#include <string.h>

// Sound Manager.
#include "SoundMgr.hpp"
#include "MD/VdpIo.hpp"

namespace LibGens
{

/** Static class variables. **/
const uint32_t Psg::ms_psgStateInit[8] = {0x00, 0x0F, 0x00, 0x0F, 0x00, 0x0F, 0x00, 0x0F};

/** Gens-specific externs and variables **/

/* Message logging. */
#include "macros/log_msg.h"

#if 0
/* GSX v7 savestate functionality. */
#include "util/file/gsx_v7.h"
#include "libgsft/gsft_byteswap.h"

/* GYM dumping. */
#include "util/sound/gym.hpp"

#include "audio/audio.h"
#endif


/** Functions **/


Psg::Psg()
{
	// TODO: Some initialization should go here!
	m_writeLen = 0;
	m_enabled = true; // TODO: Make this customizable.
	resetBufferPtrs();
}


Psg::Psg(int clock, int rate)
{
	// Initialize the PSG.
	m_writeLen = 0;
	m_enabled = true; // TODO: Make this customizable.
	resetBufferPtrs();
	reinit(clock, rate);
}


/**
 * reinit(): Initialize the PSG chip.
 * @param clock Clock frequency.
 * @param rate Sound rate.
 */
void Psg::reinit(int clock, int rate)
{
	double out;
	
	// Clear the write length.
	m_writeLen = 0;
	
	// Step calculation
	for (int i = 1; i < 1024; i++)
	{
		out = (double)(clock) / (double)(i << 4);	// out = frequency
		out /= (double)(rate);
		out *= 65536.0;
		
		m_stepTable[i] = (unsigned int)out;
	}
	m_stepTable[0] = m_stepTable[1];
	
	// Step calculation [noise table]
	for (int i = 0; i < 3; i++)
	{
		out = (double)(clock) / (double)(1 << (9 + i));
		out /= (double)(rate);
		out *= 65536.0;
		
		m_noiseStepTable[i] = (unsigned int)out;
	}
	m_noiseStepTable[3] = 0;
	
	// Volume table
	out = (double)MAX_OUTPUT / 3.0;
	for (int i = 0; i < 15; i++)
	{
		m_volumeTable[i] = (unsigned int)out;
		out /= 1.258925412;	// = 10 ^ (2/20) = 2dB
	}
	m_volumeTable[15] = 0;
	
	// Clear PSG registers.
	m_curChan = 0;
	m_curReg = 0;
	m_lfsr = 0;		// TODO: Should this be LFSR_INIT?
	m_lfsrMask = 0;		// TODO: Should this be LFSR_MASK_WHITE or LFSR_MASK_PERIODIC?
	
	memset(m_volume, 0x00, sizeof(m_volume));
	memset(m_counter, 0x00, sizeof(m_counter));
	memset(m_cntStep, 0x00, sizeof(m_cntStep));
	
	// Initialize the PSG state.
	restoreState(ms_psgStateInit);
}


/**
 * write(): Write a value to the PSG.
 * @param data Data value.
 */
void Psg::write(uint8_t data)
{
	#if 0 // TODO: GYM
	if (GYM_Dumping)
		gym_dump_update(3, (uint8_t)data, 0);
	#endif
	
	if (data & 0x80)
	{
		// LATCH/DATA byte.
		m_curReg = ((data & 0x70) >> 4);
		m_curChan = (m_curReg >> 1);
		
		// Save the data value in the register.
		m_reg[m_curReg] = ((m_reg[m_curReg] & 0x3F0) | (data & 0x0F));
	}
	else
	{
		// DATA byte.
		if (!(m_curReg & 1) && m_curChan != 3)
		{
			// TONE channel: Upper 6 bits.
			m_reg[m_curReg] = (m_reg[m_curReg] & 0x0F) | ((data & 0x3F) << 4);
		}
		else
		{
			// NOISE channel or Volume Register: Lower 4 bits.
			m_reg[m_curReg] = (data & 0x0F);
		}
	}
	
	if (m_curReg & 1)
	{
		// Volume register.
		specialUpdate();
		m_volume[m_curChan] = m_volumeTable[data & 0x0F];
		
		LOG_MSG(psg, LOG_MSG_LEVEL_DEBUG1,
			"channel %d    volume = %.8X",
			m_curChan, m_volume[m_curChan]);
	}
	else
	{
		// Frequency
		specialUpdate();
		
		if (m_curChan != 3)
		{
			// Normal channel
			m_cntStep[m_curChan] = m_stepTable[m_reg[m_curReg]];
			
			// If NOISE is set to use TONE channel 2, update it.
			if ((m_curChan == 2) && ((m_reg[6] & 3) == 3))
				m_cntStep[3] = (m_cntStep[2] >> 1);
			
			LOG_MSG(psg, LOG_MSG_LEVEL_DEBUG1,
				"channel %d    step = %.8X",
				m_curChan, m_cntStep[m_curChan]);
		}
		else
		{
			// Noise channel
			m_lfsr = LFSR_INIT;
			m_noiseStepTable[3] = (m_cntStep[2] >> 1);	// Update noise step table for type 3 (use TONE channel 2)
			m_cntStep[3] = m_noiseStepTable[data & 3];
			
			// Check if we should use white noise or periodic noise.
			if (data & 4)
				m_lfsrMask = LFSR_MASK_WHITE;
			else
				m_lfsrMask = LFSR_MASK_PERIODIC;
			
			LOG_MSG(psg, LOG_MSG_LEVEL_DEBUG1,
				"channel N    type = %.2X", data);
		}
	}
}


/**
 * update(): Update the PSG audio output using square waves.
 * @param bufL Left audio buffer. (16-bit; int32_t is used for saturation.)
 * @param bufR Right audio buffer. (16-bit; int32_t is used for saturation.)
 * @param start Starting position in the buffer.
 * @param length Length to write.
 */
void Psg::update(int32_t *bufL, int32_t *bufR, int length)
{
	int cur_cnt, cur_step, cur_vol;
	
	// Channels 0-2 (in reverse order)
	for (int j = 2; j >= 0; j--)
	{
		// Get the volume for this channel.
		cur_vol = m_volume[j];
		if (cur_vol == 0)
			continue;
		
		if (cur_vol != 0)
		{
			// Current channel's volume is non-zero.
			// Apply the appropriate square wave.
			cur_step = m_cntStep[j];
			if (cur_step < 0x10000)
			{
				// Current channel's tone is audible.
				cur_cnt = m_counter[j];
				
				for (int i = 0; i < length; i++)
				{
					cur_cnt += cur_step;
					
					if (cur_cnt & 0x10000)
					{
						// Overflow. Apply +1 tone.
						bufL[i] += cur_vol;
						bufR[i] += cur_vol;
					}
				}
				
				// Update the counter for this channel.
				m_counter[j] = cur_cnt;
			}
			else
			{
				// Current channel's tone is not audible.
				// Always apply a +1 tone.
				// (TODO: Is this correct?)
				for (int i = 0; i < length; i++)
				{
					bufL[i] += cur_vol;
					bufR[i] += cur_vol;
				}
				
				// Update the counter for this channel.
				m_counter[j] += (m_cntStep[j] * length);
			}
		}
		else
		{
			// Current channel's volume is zero.
			// Simply increase the channel's counter.
			m_counter[j] += m_cntStep[j] * length;
		}
	}
	
	// Channel 3 - Noise
	cur_vol = m_volume[3];
	if (cur_vol != 0)
	{
		cur_cnt = m_counter[3];
		cur_step = m_cntStep[3];
		
		for (int i = 0; i < length; i++)
		{
			cur_cnt += cur_step;
			
			if (m_lfsr & 1)
			{
				bufL[i] += cur_vol;
				bufR[i] += cur_vol;
			}
			
			// Check if the LFSR should be shifted.
			if (cur_cnt & 0x10000)
			{
				cur_cnt &= 0xFFFF;
				m_lfsr = LFSR16_Shift(m_lfsr, m_lfsrMask);
			}
		}
		
		m_counter[3] = cur_cnt;
	}
	else
	{
		m_counter[3] += (m_cntStep[3] * length);
	}
}


/**
 * saveState(): Save the PSG state.
 * @param state 8-DWORD array to save the state to.
 */
void Psg::saveState(uint32_t state[8])
{
	for (int i = 0; i < 8; i++)
		state[i] = m_reg[i];
}


/**
 * restoreState(): Restore the PSG state.
 * @param state 8-DWORD array to load the state from.
 */
void Psg::restoreState(const uint32_t state[8])
{
	for (int i = 0; i < 8; i++)
	{
		this->write(0x80 | (i << 4) | (state[i] & 0xF));
		
		// Only write DATA bytes for tone registers.
		// Don't write DATA bytes for volume or noise registers.
		if (!(i & 1) && i < 6)
			this->write((state[i] >> 4) & 0x3F);
	}
}


/** Gens-specific code **/


/**
 * getReg(): Get the value of a register.
 * @param regID Register ID.
 * @return Value of the register.
 */
int Psg::getReg(int regID)
{
	if (regID < 0 || regID >= 8)
		return -1;
	
	return (int)m_reg[regID];
}


/**
 * specialUpdate(): Update the PSG buffer.
 */
void Psg::specialUpdate(void)
{
	if (!(m_writeLen > 0 && m_enabled))
		return;
	
	// Update the sound buffer.
	update(m_bufPtrL, m_bufPtrR, m_writeLen);
	m_writeLen = 0;
	
	// Determine the new starting position.
	int writePos = SoundMgr::GetWritePos(VdpIo::VDP_Lines.Display.Current + 1);
	
	// Update the PSG buffer pointers.
	m_bufPtrL = &SoundMgr::ms_SegBufL[writePos];
	m_bufPtrR = &SoundMgr::ms_SegBufR[writePos];
}


/**
 * resetBufferPtrs(): Reset the PSG buffer pointers.
 */
void Psg::resetBufferPtrs(void)
{
	m_bufPtrL = &SoundMgr::ms_SegBufL[0];
	m_bufPtrR = &SoundMgr::ms_SegBufR[0];
}


// TODO: Eliminate the GSXv7 stuff.
// TODO: Add the counter state to the ZOMG save format.
#if 0
/**
 * GSX v7 PSG save/restore functions.
 * Ported from Gens Rerecording.
 */


/**
 * PSG_Save_State_GSX_v7(): Restore the PSG state. (GSX v7)
 * @param save GSX v7 PSG struct to save to.
 */
void PSG_Save_State_GSX_v7(struct _gsx_v7_psg *save)
{
	save->current_channel	= cpu_to_le32(PSG.Current_Channel);
	save->current_reg	= cpu_to_le32(PSG.Current_Register);
	
	save->reg[0]		= cpu_to_le32(PSG.Register[0]);
	save->reg[1]		= cpu_to_le32(PSG.Register[1]);
	save->reg[2]		= cpu_to_le32(PSG.Register[2]);
	save->reg[3]		= cpu_to_le32(PSG.Register[3]);
	save->reg[4]		= cpu_to_le32(PSG.Register[4]);
	save->reg[5]		= cpu_to_le32(PSG.Register[5]);
	save->reg[6]		= cpu_to_le32(PSG.Register[6]);
	save->reg[7]		= cpu_to_le32(PSG.Register[7]);
	
	save->counter[0]	= cpu_to_le32(PSG.Counter[0]);
	save->counter[1]	= cpu_to_le32(PSG.Counter[1]);
	save->counter[2]	= cpu_to_le32(PSG.Counter[2]);
	save->counter[3]	= cpu_to_le32(PSG.Counter[3]);
	
	save->cntstep[0]	= cpu_to_le32(PSG.CntStep[0]);
	save->cntstep[1]	= cpu_to_le32(PSG.CntStep[1]);
	save->cntstep[2]	= cpu_to_le32(PSG.CntStep[2]);
	save->cntstep[3]	= cpu_to_le32(PSG.CntStep[3]);
	
	save->volume[0]		= cpu_to_le32(PSG.Volume[0]);
	save->volume[1]		= cpu_to_le32(PSG.Volume[1]);
	save->volume[2]		= cpu_to_le32(PSG.Volume[2]);
	save->volume[3]		= cpu_to_le32(PSG.Volume[3]);
	
	save->noise_type	= cpu_to_le32(PSG.LFSR_Mask == LFSR_MASK_PERIODIC ? 0x08000 : 0x12000);
	save->noise		= cpu_to_le32(PSG.LFSR);
}


/**
 * PSG_Restore_State_GSX_v7(): Restore the PSG state. (GSX v7)
 * @param save GSX v7 PSG struct to restore from.
 */
void PSG_Restore_State_GSX_v7(struct _gsx_v7_psg *save)
{
	PSG.Current_Channel	= le32_to_cpu(save->current_channel);
	PSG.Current_Register	= le32_to_cpu(save->current_reg);
	
	PSG.Register[0]		= le32_to_cpu(save->reg[0]);
	PSG.Register[1]		= le32_to_cpu(save->reg[1]);
	PSG.Register[2]		= le32_to_cpu(save->reg[2]);
	PSG.Register[3]		= le32_to_cpu(save->reg[3]);
	PSG.Register[4]		= le32_to_cpu(save->reg[4]);
	PSG.Register[5]		= le32_to_cpu(save->reg[5]);
	PSG.Register[6]		= le32_to_cpu(save->reg[6]);
	PSG.Register[7]		= le32_to_cpu(save->reg[7]);
	
	PSG.Counter[0]		= le32_to_cpu(save->counter[0]);
	PSG.Counter[1]		= le32_to_cpu(save->counter[1]);
	PSG.Counter[2]		= le32_to_cpu(save->counter[2]);
	PSG.Counter[3]		= le32_to_cpu(save->counter[3]);
	
	PSG.CntStep[0]		= le32_to_cpu(save->cntstep[0]);
	PSG.CntStep[1]		= le32_to_cpu(save->cntstep[1]);
	PSG.CntStep[2]		= le32_to_cpu(save->cntstep[2]);
	PSG.CntStep[3]		= le32_to_cpu(save->cntstep[3]);
	
	PSG.Volume[0]		= le32_to_cpu(save->volume[0]);
	PSG.Volume[1]		= le32_to_cpu(save->volume[1]);
	PSG.Volume[2]		= le32_to_cpu(save->volume[2]);
	PSG.Volume[3]		= le32_to_cpu(save->volume[3]);
	
	PSG.LFSR_Mask		= (le32_to_cpu(save->noise_type) == 0x08000
					? LFSR_MASK_PERIODIC
					: LFSR_MASK_WHITE);
	PSG.LFSR		= le32_to_cpu(save->noise);
}
#endif

/** end **/

}
