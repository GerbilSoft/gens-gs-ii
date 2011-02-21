/***************************************************************************
 * libgens: Gens Emulation Library.                                        *
 * PsgHqHq.cpp: TI SN76489 (PSG) emulator. (High Quality)                    *
 *                                                                         *
 * Copyright (c) 1999-2002 by Stéphane Dallongeville                       *
 * Copyright (c) 2003-2004 by Stéphane Akhoun                              *
 * Copyright (c) 2008-2011 by David Korth                                  *
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

#include "PsgHq.hpp"

// Sound Manager.
#include "SoundMgr.hpp"
#include "MD/VdpIo.hpp"

// C includes.
#include <string.h>

namespace LibGens
{

/** Static class variables. **/

/**
 * ms_psgStateInit: Initial PSG state.
 */
const Zomg_PsgSave_t PsgHq::ms_psgStateInit =
{
	{0x000, 0x000, 0x000, 0x000},	// TONE registers.
	{  0xF,   0xF,   0xF,   0xF},	// VOLUME registers.
	
	{0, 0, 0, 0},			// TONE counters. (TODO)
	
	LFSR_INIT,	// LFSR mask
	0xFF		// Game Gear stereo register.
};

// Lookup tables.
// volumes [i] = 64 * pow( 1.26, 15 - i ) / pow( 1.26, 15 )
const int8_t PsgHq::ms_VolTable[16] =
	{64, 50, 39, 31, 24, 19, 15, 12, 9, 7, 5, 4, 3, 2, 1, 0};


/** Gens-specific externs and variables **/

/* Message logging. */
#include "macros/log_msg.h"


/** Functions **/


PsgHq::PsgHq(Blip_Buffer *blipBuffer)
{
	//m_enabled = true; // TODO: Make this customizable.
	
	// Save the Blip_Buffer pointer.
	// TODO: Is this necessary?
	m_blipBuffer = blipBuffer;
	
	// Initialize the synths.
	// MAX_OUTPUT in Psg == 0x4FFF
	// 0x5000 / 0x8000 == 0.625
	// NOTE: Sms_Apu uses (0.85 / osc_count).
	// Verify if this is correct for MD.
	const double vol = (0.85 / 4.0);
	for (int i = 0; i < 3; i++)
	{
		m_synthTone[i].volume(vol);
		m_synthTone[i].output(m_blipBuffer);
	}
	m_synthNoise.volume(vol);
	m_synthNoise.output(m_blipBuffer);
	
	// Clear PSG registers.
	m_curChan = 0;
	m_curReg = 0;
	m_lfsr = 0;		// TODO: Should this be LFSR_INIT?
	m_lfsrMask = 0;		// TODO: Should this be LFSR_MASK_WHITE or LFSR_MASK_PERIODIC?
	
	// Clear all variables.
	memset(m_volume, 0x00, sizeof(m_volume));
	memset(m_counter, 0x00, sizeof(m_counter));
	memset(m_isOutput, 0x00, sizeof(m_isOutput));
	
	// Clear the registers.
	memset(m_reg, 0x00, sizeof(m_reg));
	m_noise_resetVal = 0;
	
	// Clear the cycle counter.
	m_cycles = 0;
	
	// Reset the PSG state.
	reset();
}


/**
 * write(): Write a value to the PSG.
 * @param data Data value.
 */
void PsgHq::write(uint8_t data)
{
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
	
	// TODO: Optimize this?
	if (m_curReg & 1)
	{
		// Volume register.
		// Update m_volume[m_curChan] with the associated volume.
		m_volume[m_curChan] = ms_VolTable[data & 0x0F];
	}
	else
	{
		// TONE register.
		// Special updates need to be done for registers 2 and 3.
		
		if (m_curChan == 2)
		{
			// TONE register 2.
			// If NOISE is set to use Tone2, update m_noise_resetVal.
			if ((m_reg[3<<1] & 0x3) == 3)
			{
				m_noise_resetVal = m_reg[2<<1];
				
				// NOTE: Classic Psg treats a noise tone of 0 like 1.
				// SMS Power shows tones 0 and 1 result in +1 output.
				// Figure out the correct value.
				// (This fixes white noise on the Sonic 1 title screen.)
				if (m_noise_resetVal == 0)
					m_noise_resetVal = 1;
			}
		}
		else if (m_curChan == 3)
		{
			// TONE register: NOISE channel.
			switch (m_reg[3<<1] & 0x3)
			{
				case 0:		m_noise_resetVal = 0x10; break;
				case 1:		m_noise_resetVal = 0x20; break;
				case 2:		m_noise_resetVal = 0x40; break;
				case 3:
					m_noise_resetVal = m_reg[2<<1];
					
					// NOTE: Classic Psg treats a noise tone of 0 like 1.
					// SMS Power shows tones 0 and 1 result in +1 output.
					// Figure out the correct value.
					// (This fixes white noise on the Sonic 1 title screen.)
					if (m_noise_resetVal == 0)
						m_noise_resetVal = 1;
					break;
			}
			
			// Reset the noise LFSR.
			m_lfsr = LFSR_INIT;
			
			// Check if we should use white noise or periodic noise.
			if (data & 4)
				m_lfsrMask = LFSR_MASK_WHITE;
			else
				m_lfsrMask = LFSR_MASK_PERIODIC;
		}
	}
}


/**
 * runCycles(): Run the PSG for a given number of cycles.
 * @param cycles Number of cycles to run.
 * @return Cycles remaining in the current frame.
 */
void PsgHq::runCycles(int cycles)
{
	const int end_cycles = (m_cycles + cycles);
	
	// PSG uses 16 "real" cycles per sample.
	const int sample_count = ((end_cycles / 16) - (m_cycles / 16));
	
	// TONE channels.
	for (int i = 2; i >= 0; i--)
	{
		int sample_cycles = sample_count;
		int real_cycle_pos = m_cycles;
		
		while (m_counter[i] <= sample_cycles)
		{
			// Timer for this channel will overflow.
			sample_cycles -= m_counter[i];
			real_cycle_pos += (m_counter[i] * 16);
			
			// Flip the output bit.
			m_isOutput[i] = !m_isOutput[i];
			
			// Reset the tone counter.
			m_counter[i] = m_reg[i << 1];
			if (m_counter[i] == 0)
			{
				// Reset value is 0.
				// Don't output anything.
				m_isOutput[i] = 0;
				sample_cycles = 0;
				m_synthTone[i].update(real_cycle_pos, 0);
				break;
			}
			
			// Update the synth buffer.
			m_synthTone[i].update(real_cycle_pos,
					(m_isOutput[i] ? m_volume[i] : 0));
		}
		
		// Subtract the remaining cycles from the tone counter.
		m_counter[i] -= sample_cycles;
	}
	
	// NOISE channel.
	{
		int sample_cycles = sample_count;
		int real_cycle_pos = m_cycles;
		
		while (m_counter[3] <= sample_cycles)
		{
			// Timer for this channel will overflow.
			sample_cycles -= m_counter[3];
			real_cycle_pos += (m_counter[3] * 16);
			
			// Shift the LFSR.
			m_lfsr = LFSR16_Shift(m_lfsr, m_lfsrMask);
			m_isOutput[3] = (m_lfsr & 1);
			
			// Reset the tone counter.
			m_counter[3] = m_noise_resetVal;
			if (m_counter[3] == 0)
			{
				// Reset value is 0.
				// Don't output anything.
				m_isOutput[3] = 0;
				sample_cycles = 0;
				m_synthNoise.update(real_cycle_pos, 0);
				break;
			}
			
			// Update the synth buffer.
			m_synthNoise.update(real_cycle_pos,
					(m_isOutput[3] ? m_volume[3] : 0));
		}
		
		// Subtract the remaining cycles from the tone counter.
		m_counter[3] -= sample_cycles;
	}
	
	// Store the overflow cycles.
	m_cycles = end_cycles;
}


/** ZOMG savestate functions. **/


/**
 * zomgSave(): Save the PSG state.
 * @param state Zomg_PsgSave_t struct to save to.
 */
void PsgHq::zomgSave(Zomg_PsgSave_t *state)
{
	// TONE channels.
	state->tone_reg[0] = (m_reg[0] & 0x3FF);
	state->vol_reg[0]  = (m_reg[1] & 0xF);
	state->tone_reg[1] = (m_reg[2] & 0x3FF);
	state->vol_reg[1]  = (m_reg[3] & 0xF);
	state->tone_reg[2] = (m_reg[4] & 0x3FF);
	state->vol_reg[2]  = (m_reg[5] & 0xF);
	
	// NOISE channel.
	state->tone_reg[3] = (m_reg[6] & 0x7); // Only 3 bits are relevant for NOISE.
	state->vol_reg[3]  = (m_reg[7] & 0xF);
	
	// TONE counters.
	state->tone_ctr[0] = m_counter[0];
	state->tone_ctr[1] = m_counter[1];
	state->tone_ctr[2] = m_counter[2];
	state->tone_ctr[3] = m_counter[3];
	
	// LFSR state.
	state->lfsr_state = m_lfsr;
	
	// Game Gear stereo register.
	// TODO: Implement Game Gear stereo.
	state->gg_stereo = 0x00;
}


/**
 * zomgRestore(): Restore the PSG state.
 * @param state Zomg_PsgSave_t struct to restore from.
 */
void PsgHq::zomgRestore(const Zomg_PsgSave_t *state)
{
	for (int i = 0; i < 4; i++)
	{
		// Register latch and data value.
		this->write(0x80 | ((i*2) << 4) | (state->tone_reg[i] & 0xF));
		
		// Second data value for TONE registers.
		if (i < 3)
			this->write((state->tone_reg[i] >> 4) & 0x3F);
		
		// VOLUME register.
		this->write(0x80 | (((i*2)+1) << 4) | (state->vol_reg[i] & 0xF));
		
		// TONE counter.
		m_counter[i] = state->tone_ctr[i];
	}
	
	// LFSR state.
	m_lfsr = state->lfsr_state;
	
	// Game Gear stereo register.
	// TODO: Implement Game Gear stereo.
}


/** Gens-specific code **/


/**
 * getReg(): Get the value of a register.
 * @param regID Register ID.
 * @return Value of the register.
 */
int PsgHq::getReg(int regID)
{
	if (regID < 0 || regID >= 8)
		return -1;
	
	return (int)m_reg[regID];
}

}
