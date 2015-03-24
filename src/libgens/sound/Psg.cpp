/***************************************************************************
 * libgens: Gens Emulation Library.                                        *
 * Psg.cpp: TI SN76489 (PSG) emulator.                                     *
 *                                                                         *
 * Copyright (c) 1999-2002 by Stéphane Dallongeville                       *
 * Copyright (c) 2003-2004 by Stéphane Akhoun                              *
 * Copyright (c) 2008-2015 by David Korth                                  *
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
#include "Psg_p.hpp"

// C includes.
#include <stdint.h>
// C includes. (C++ namespace)
#include <cstring>

// Sound Manager.
#include "SoundMgr.hpp"
#include "Vdp/Vdp.hpp"

// TODO: Get rid of EmuContext.
#include "../EmuContext.hpp"

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

namespace LibGens {

/** PsgPrivate **/

// Initial PSG state.
const Zomg_PsgSave_t PsgPrivate::psgStateInit =
{
	{0x000, 0x000, 0x000, 0x000},	// TONE registers.
	{  0xF,   0xF,   0xF,   0xF},	// VOLUME registers.

	{0, 0, 0, 0},			// TONE counters. (TODO)

	LFSR_INIT,	// LFSR mask
	0xFF		// Game Gear stereo register.
};

PsgPrivate::PsgPrivate(Psg *q)
	: q(q)
	, writeLen(0)
	, enabled(true)	// TODO: Make this customizable.
{
	// TODO: Move this here?
	// (It's currently initialized in the Psg constructors.)
	//resetBufferPtrs();
}

/**
 * Update the PSG audio output using square waves.
 * @param bufL Left audio buffer. (16-bit; int32_t is used for saturation.)
 * @param bufR Right audio buffer. (16-bit; int32_t is used for saturation.)
 * @param length Length to write.
 */
void PsgPrivate::update(int32_t *bufL, int32_t *bufR, int length)
{
	int cur_cnt, cur_step, cur_vol;

	// Channels 0-2 (in reverse order)
	for (int j = 2; j >= 0; j--) {
		// Get the volume for this channel.
		cur_vol = volume[j];
		/* FIXME: Is this correct?
		if (cur_vol == 0)
			continue;
		*/

		if (cur_vol != 0) {
			// Current channel's volume is non-zero.
			// Apply the appropriate square wave.
			cur_step = cntStep[j];
			if (cur_step < 0x10000) {
				// Current channel's tone is audible.
				cur_cnt = counter[j];

				for (int i = 0; i < length; i++) {
					cur_cnt += cur_step;
					if (cur_cnt & 0x10000) {
						// Overflow. Apply +1 tone.
						bufL[i] += cur_vol;
						bufR[i] += cur_vol;
					}
				}

				// Update the counter for this channel.
				counter[j] = cur_cnt;
			} else {
				// Current channel's tone is not audible.
				// Always apply a +1 tone.
				// (TODO: Is this correct?)
				for (int i = 0; i < length; i++) {
					bufL[i] += cur_vol;
					bufR[i] += cur_vol;
				}

				// Update the counter for this channel.
				counter[j] += (cntStep[j] * length);
			}
		} else {
			// Current channel's volume is zero.
			// Simply increase the channel's counter.
			counter[j] += cntStep[j] * length;
		}
	}

	// Channel 3 - Noise
	cur_vol = volume[3];
	if (cur_vol != 0) {
		cur_cnt = counter[3];
		cur_step = cntStep[3];

		for (int i = 0; i < length; i++) {
			cur_cnt += cur_step;

			if (lfsr & 1) {
				bufL[i] += cur_vol;
				bufR[i] += cur_vol;
			}

			// Check if the LFSR should be shifted.
			if (cur_cnt & 0x10000) {
				cur_cnt &= 0xFFFF;
				lfsr = LFSR16_Shift(lfsr, lfsrMask);
			}
		}

		counter[3] = cur_cnt;
	} else {
		// Current channel's volume is zero.
		// Simply increase the channel's counter.
		counter[3] += (cntStep[3] * length);
	}
}

/** Psg **/

Psg::Psg()
	: d(new PsgPrivate(this))
{
	// TODO: Some initialization should go here!
	resetBufferPtrs();
}

Psg::Psg(int clock, int rate)
	: d(new PsgPrivate(this))
{
	resetBufferPtrs();

	// Initialize the PSG.
	reInit(clock, rate);
}

Psg::~Psg()
{
	delete d;
}

/**
 * Reset the PSG state.
 */
void Psg::reset(void)
{
	// Quick & dirty hack:
	// Use the ZOMG restore function
	// to restore the initial state.
	zomgRestore(&d->psgStateInit);
}

/**
 * reInit(): (Re-)Initialize the PSG chip.
 * @param clock Clock frequency.
 * @param rate Sound rate.
 */
void Psg::reInit(int clock, int rate)
{
	double out;

	// Clear the write length.
	d->writeLen = 0;

	// Step calculation
	for (int i = 1; i < 1024; i++) {
		out = (double)(clock) / (double)(i << 4);	// out = frequency
		out /= (double)(rate);
		out *= 65536.0;
		d->stepTable[i] = (unsigned int)out;
	}
	d->stepTable[0] = d->stepTable[1];

	// Step calculation [noise table]
	for (int i = 0; i < 3; i++) {
		out = (double)(clock) / (double)(1 << (9 + i));
		out /= (double)(rate);
		out *= 65536.0;
		d->noiseStepTable[i] = (unsigned int)out;
	}
	d->noiseStepTable[3] = 0;

	// Volume table
	out = (double)d->MAX_OUTPUT / 3.0;
	for (int i = 0; i < 15; i++) {
		d->volumeTable[i] = (unsigned int)out;
		out /= 1.258925412;	// = 10 ^ (2/20) = 2dB
	}
	d->volumeTable[15] = 0;

	// Clear PSG registers.
	d->curChan = 0;
	d->curReg = 0;
	d->lfsr = 0;		// TODO: Should this be LFSR_INIT?
	d->lfsrMask = 0;	// TODO: Should this be LFSR_MASK_WHITE or LFSR_MASK_PERIODIC?

	memset(d->volume, 0x00, sizeof(d->volume));
	memset(d->counter, 0x00, sizeof(d->counter));
	memset(d->cntStep, 0x00, sizeof(d->cntStep));

	// Reset the PSG state.
	reset();
}

/**
 * Write to the PSG's data port.
 * @param data Data value.
 */
void Psg::write(uint8_t data)
{
#if 0 // TODO: GYM
	if (GYM_Dumping)
		gym_dump_update(3, (uint8_t)data, 0);
#endif

	if (data & 0x80) {
		// LATCH/DATA byte.
		d->curReg = ((data & 0x70) >> 4);
		d->curChan = (d->curReg >> 1);

		// Save the data value in the register.
		d->reg[d->curReg] = ((d->reg[d->curReg] & 0x3F0) | (data & 0x0F));
	} else {
		// DATA byte.
		if (!(d->curReg & 1) && d->curChan != 3) {
			// TONE channel: Upper 6 bits.
			d->reg[d->curReg] = (d->reg[d->curReg] & 0x0F) | ((data & 0x3F) << 4);
		} else {
			// NOISE channel or Volume Register: Lower 4 bits.
			d->reg[d->curReg] = (data & 0x0F);
		}
	}

	if (d->curReg & 1) {
		// Volume register.
		specialUpdate();
		d->volume[d->curChan] = d->volumeTable[data & 0x0F];

		LOG_MSG(psg, LOG_MSG_LEVEL_DEBUG1,
			"channel %d    volume = %.8X",
			d->curChan, d->volume[d->curChan]);
	} else {
		// Frequency.
		specialUpdate();

		if (d->curChan != 3) {
			// Normal channel
			d->cntStep[d->curChan] = d->stepTable[d->reg[d->curReg]];

			// If NOISE is set to use TONE channel 2, update it.
			if ((d->curChan == 2) && ((d->reg[6] & 3) == 3)) {
				d->cntStep[3] = (d->cntStep[2] >> 1);
			}

			LOG_MSG(psg, LOG_MSG_LEVEL_DEBUG1,
				"channel %d    step = %.8X",
				d->curChan, d->cntStep[d->curChan]);
		} else {
			// Noise channel
			d->lfsr = d->LFSR_INIT;
			d->noiseStepTable[3] = (d->cntStep[2] >> 1);	// Update noise step table for type 3 (use TONE channel 2)
			d->cntStep[3] =d->noiseStepTable[data & 3];

			// Check if we should use white noise or periodic noise.
			if (data & 4) {
				d->lfsrMask = d->LFSR_MASK_WHITE;
			} else {
				d->lfsrMask = d->LFSR_MASK_PERIODIC;
			}

			LOG_MSG(psg, LOG_MSG_LEVEL_DEBUG1,
				"channel N    type = %.2X", data);
		}
	}
}

/** ZOMG savestate functions. **/

/**
 * zomgSave(): Save the PSG state.
 * @param state Zomg_PsgSave_t struct to save to.
 */
void Psg::zomgSave(Zomg_PsgSave_t *state)
{
	// TONE channels.
	state->tone_reg[0] = (d->reg[0] & 0x3FF);
	state->vol_reg[0]  = (d->reg[1] & 0xF);
	state->tone_reg[1] = (d->reg[2] & 0x3FF);
	state->vol_reg[1]  = (d->reg[3] & 0xF);
	state->tone_reg[2] = (d->reg[4] & 0x3FF);
	state->vol_reg[2]  = (d->reg[5] & 0xF);
	
	// NOISE channel.
	state->tone_reg[3] = (d->reg[6] & 0x7); // Only 3 bits are relevant for NOISE.
	state->vol_reg[3]  = (d->reg[7] & 0xF);
	
	// TODO: TONE counters.
	// Psg uses an interpolated counter that overflows at 65,536.
	// ZOMG counter should use the same value as the original PSG.
	state->tone_ctr[0] = 0xFFFF;
	state->tone_ctr[1] = 0xFFFF;
	state->tone_ctr[2] = 0xFFFF;
	state->tone_ctr[3] = 0xFFFF;
	
	// LFSR state.
	state->lfsr_state = d->lfsr;
	
	// Game Gear stereo register.
	// TODO: Implement Game Gear stereo.
	state->gg_stereo = 0x00;
}


/**
 * zomgRestore(): Restore the PSG state.
 * @param state Zomg_PsgSave_t struct to restore from.
 */
void Psg::zomgRestore(const Zomg_PsgSave_t *state)
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
		
		// TODO: TONE counters.
		// Psg uses an interpolated counter that overflows at 65,536.
		// ZOMG counter should use the same value as the original PSG.
	}
	
	// LFSR state.
	d->lfsr = state->lfsr_state;
	
	// Game Gear stereo register.
	// TODO: Implement Game Gear stereo.
}

/** Gens-specific code **/

/**
 * Get the value of a register.
 * @param regID Register ID.
 * @return Value of the register.
 */
int Psg::getReg(int regID)
{
	if (regID < 0 || regID >= 8)
		return -1;
	
	return (int)d->reg[regID];
}

/**
 * Update the PSG buffer.
 */
void Psg::specialUpdate(void)
{
	if (d->writeLen <= 0 || !d->enabled)
		return;

	// Update the sound buffer.
	d->update(d->bufPtrL, d->bufPtrR, d->writeLen);
	d->writeLen = 0;

	// TODO: Don't use EmuContext here...
	int line_num = 1;
	EmuContext *context = EmuContext::Instance();
	if (context != nullptr) {
		line_num = (context->m_vdp->VDP_Lines.currentLine + 1);
	}

	// Determine the new starting position.
	int writePos = SoundMgr::GetWritePos(line_num);

	// Update the PSG buffer pointers.
	d->bufPtrL = &SoundMgr::ms_SegBufL[writePos];
	d->bufPtrR = &SoundMgr::ms_SegBufR[writePos];
}

/** PSG write length. **/

void Psg::addWriteLen(int len)
{
	d->writeLen += len;
}

void Psg::clearWriteLen(void)
{
	d->writeLen = 0;
}

/**
 * Reset the PSG buffer pointers.
 */
void Psg::resetBufferPtrs(void)
{
	d->bufPtrL = &SoundMgr::ms_SegBufL[0];
	d->bufPtrR = &SoundMgr::ms_SegBufR[0];
}

// TODO: Eliminate the GSXv7 stuff.
// TODO: Add the counter state to the ZOMG save format.
#if 0
/**
 * GSX v7 PSG save/restore functions.
 * Ported from Gens Rerecording.
 */

/**
 * Restore the PSG state. (GSX v7)
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
 * Restore the PSG state. (GSX v7)
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

}
