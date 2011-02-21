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


PsgHq::PsgHq()
{
	//m_enabled = true; // TODO: Make this customizable.
}


PsgHq::PsgHq(int clock, int rate)
{
	//m_enabled = true; // TODO: Make this customizable.
	reInit(clock, rate);
}


void PsgHq::reInit(int clock, int rate)
{
	// Initialize the Blip Buffer.
	m_output.clock_rate(clock);
	m_output.sample_rate(rate);
	m_output.clear();
	
	// Initialize the synths.
	// MAX_OUTPUT in Psg == 0x4FFF
	// 0x5000 / 0x8000 == 0.625
	// NOTE: Sms_Apu uses (0.85 / osc_count).
	// Verify if this is correct for MD.
	const double vol = (0.85 / 4.0);
	for (int i = 0; i < 3; i++)
	{
		m_synthTone[i].volume(vol);
		m_synthTone[i].output(&m_output);
	}
	m_synthNoise.volume(vol);
	m_synthNoise.output(&m_output);
	
	// Clear PSG registers.
	m_curChan = 0;
	m_curReg = 0;
	m_lfsr = 0;		// TODO: Should this be LFSR_INIT?
	m_lfsrMask = 0;		// TODO: Should this be LFSR_MASK_WHITE or LFSR_MASK_PERIODIC?
	
	// Clear all variables.
	memset(m_volume, 0x00, sizeof(m_volume));
	memset(m_counter, 0x00, sizeof(m_counter));
	memset(m_isOutput, 0x00, sizeof(m_isOutput));
	
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
	
	if (m_curReg & 1)
	{
		// Volume register.
		// Update m_volume[m_curChan] with the associated volume.
		m_volume[m_curChan] = ms_VolTable[data & 0x0F];
	}
	else if (m_curChan == 3)
	{
		// TONE register: NOISE channel.
		// TODO: Update NOISE if TONE 2 is changed and NOISE is set to use TONE 2.
		
		// Reset the noise LFSR.
		m_lfsr = LFSR_INIT;
		
		// Check if we should use white noise or periodic noise.
		if (data & 4)
			m_lfsrMask = LFSR_MASK_WHITE;
		else
			m_lfsrMask = LFSR_MASK_PERIODIC;
	}
}


/**
 * runCycles(): Run the PSG for a given number of cycles.
 * @param cycles Number of cycles to run.
 * @return Cycles remaining in the current frame.
 */
void PsgHq::runCycles(int cycles)
{
	// PSG uses 16 "real" cycles per sample.
	int end_cycles = (m_cycles + cycles);
	int div16_cycles = (end_cycles & ~0xF);
	
	for (; m_cycles < div16_cycles; m_cycles += 16)
	{
		// Process the PSG channels.
		// TODO: Optimize this! (e.g. only run one iteration per transition)
		
		// TONE channels 0-2 (in reverse order)
		for (int j = 2; j >= 0; j--)
		{
			m_counter[j]--; // Decrement the tone counter.
			if (m_counter[j] <= 0)
			{
				// Counter has hit 0. Flip the output bit.
				// TODO: Optimize tone=0 checking.
				m_isOutput[j] = !m_isOutput[j];
				
				// Reset the tone counter.
				m_counter[j] = m_reg[j << 1];
				if (m_counter[j] == 0)
					m_isOutput[j] = 0;
				
				// Update the synth buffer.
				m_synthTone[j].update(m_cycles,
						(m_isOutput[j] ? m_volume[j] : 0));
			}
		}
		
		// NOISE channel (TODO)
	}
	
	// Store the overflow cycles.
	m_cycles = end_cycles;
}


/**
 * render(): Render the PSG output.
 * This function should be called *after* the entire frame is processed.
 * @param bufL Left audio buffer.
 * @param bufR Right audio buffer.
 */
void PsgHq::render(int32_t *bufL, int32_t *bufR)
{
	// End the current output frame.
	m_output.end_frame(m_cycles);
	m_cycles = 0;
	
	// Convert the buffer to split format.
	// TODO: Don't do this. Instead, have a global Blip_Buffer.
	// A global Blip_Buffer is also needed because the number of
	// samples read may not be exactly 735 for 44,100 Hz.
	blip_sample_t sample_buf[65536];
	long samples_read = m_output.read_samples(sample_buf, sizeof(sample_buf)/sizeof(sample_buf[0]), 0);
	for (long i = 0; i < samples_read; i++)
	{
		*bufL++ += sample_buf[i];
		*bufR++ += sample_buf[i];
	}
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
