/***************************************************************************
 * gens-qt4: Gens Qt4 UI.                                                  *
 * ABackend_Write.hpp: Audio Write functions.                              *
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

#include "GensPortAudio.hpp"

// C includes.
#include <string.h>
#include <unistd.h>

// LOG_MSG() subsystem.
#include "libgens/macros/log_msg.h"

// LibGens Sound Manager.
#include "libgens/sound/SoundMgr.hpp"

// CPU flags.
#include "libgens/Util/cpuflags.h"

// Timing. (TODO: Get rid of this when finished debugging!)
#include "libgens/Util/Timing.hpp"

namespace GensQt4
{

/**
 * write(): Write the current segment to the audio buffer.
 * @return 0 on success; non-zero on error.
 */
int GensPortAudio::write(void)
{
	if (!m_open)
		return 1;
	
	// Lock the ring buffer for writing.
	int16_t *buf = m_buffer.writeLock();
	
	// TODO: MMX versions.
	int ret;
#ifdef HAVE_MMX
	if (CPU_Flags & MDP_CPUFLAG_X86_MMX)
	{
		// MMX is supported.
		if (m_stereo)
			ret = writeStereoMMX(buf);
		else
			ret = writeMonoMMX(buf);
	}
	else
#endif /* HAVE_MMX */
	if (m_stereo)
		ret = writeStereo(buf);
	else
		ret = writeMono(buf);
	
	// Unlock the ring buffer.
	m_buffer.writeUnlock();
	return ret;
}


/**
 * writeStereo(): Write the current segment to the audio buffer. (Stereo output)
 * @param dest Destination buffer.
 * @return 0 on success; non-zero on error.
 */
int GensPortAudio::writeStereo(int16_t *dest)
{
	// Segment length.
	const int SegLength = LibGens::SoundMgr::GetSegLength();
	
	// Source buffer pointers.
	int32_t *srcL = &LibGens::SoundMgr::ms_SegBufL[0];
	int32_t *srcR = &LibGens::SoundMgr::ms_SegBufR[0];
	
	for (unsigned int i = SegLength; i != 0; i--, srcL++, srcR++, dest += 2)
	{
		if (*srcL < -0x8000)
			*dest = -0x8000;
		else if (*srcL > 0x7FFF)
			*dest = 0x7FFF;
		else
			*dest = (int16_t)(*srcL);
		
		if (*srcR < -0x8000)
			*(dest+1) = -0x8000;
		else if (*srcR > 0x7FFF)
			*(dest+1) = 0x7FFF;
		else
			*(dest+1) = (int16_t)(*srcR);
	}
	
	// Clear the segment buffers.
	memset(LibGens::SoundMgr::ms_SegBufL, 0x00, SegLength*sizeof(LibGens::SoundMgr::ms_SegBufL[0]));
	memset(LibGens::SoundMgr::ms_SegBufR, 0x00, SegLength*sizeof(LibGens::SoundMgr::ms_SegBufR[0]));
	return 0;
}


/**
 * writeMono(): Write the current segment to the audio buffer. (Monaural output)
 * @param dest Destination buffer.
 * @return 0 on success; non-zero on error.
 */
int GensPortAudio::writeMono(int16_t *dest)
{
	// Segment length.
	const int SegLength = LibGens::SoundMgr::GetSegLength();
	
	// Source buffer pointers.
	int32_t *srcL = &LibGens::SoundMgr::ms_SegBufL[0];
	int32_t *srcR = &LibGens::SoundMgr::ms_SegBufR[0];
	
	for (unsigned int i = SegLength; i != 0; i--, srcL++, srcR++, dest++)
	{
		int32_t out = ((*srcL + *srcR) >> 1);
		
		if (out < -0x8000)
			*dest = -0x8000;
		else if (out > 0x7FFF)
			*dest = 0x7FFF;
		else
			*dest = (int16_t)out;
	}
	
	// Clear the segment buffers.
	memset(LibGens::SoundMgr::ms_SegBufL, 0x00, SegLength*sizeof(LibGens::SoundMgr::ms_SegBufL[0]));
	memset(LibGens::SoundMgr::ms_SegBufR, 0x00, SegLength*sizeof(LibGens::SoundMgr::ms_SegBufR[0]));
	return 0;
}


#ifdef HAVE_MMX
/**
 * writeStereoMMX(): Write the current segment to the audio buffer. (Stereo output; MMX-optimized)
 * @param dest Destination buffer.
 * @return 0 on success; non-zero on error.
 */
int GensPortAudio::writeStereoMMX(void)
{
	// Lock the audio buffer.
	m_mtxBuf.lock();
	
	// Segment length.
	const int SegLength = LibGens::SoundMgr::GetSegLength();
	
	// Destination buffer pointer.
	int16_t *dest = (int16_t*)&m_buf[(m_bufPos * m_sampleSize) / sizeof(m_buf[0])];
	
	// Source buffer pointers.
	int32_t *srcL = &LibGens::SoundMgr::ms_SegBufL[0];
	int32_t *srcR = &LibGens::SoundMgr::ms_SegBufR[0];
	
	// If the segment length is odd, write the first sample without MMX.
	if (SegLength & 1)
	{
		if (*srcL < -0x8000)
			*dest = -0x8000;
		else if (*srcL > 0x7FFF)
			*dest = 0x7FFF;
		else
			*dest = (int16_t)(*srcL);
		
		if (*srcR < -0x8000)
			*(dest+1) = -0x8000;
		else if (*srcR > 0x7FFF)
			*(dest+1) = 0x7FFF;
		else
			*(dest+1) = (int16_t)(*srcR);
		
		// Next sample.
		srcL++;
		srcR++;
		dest += 2;
	}
	
	// Load the shift value.
	__asm__ (
		"movl	$32, %%eax\n"
		"movd	%%eax, %%mm5\n"
		: // output
		: // input
		: "eax" // clobber
		);
	
	// Write two samples at once using MMX.
	for (unsigned int i = (SegLength / 2); i != 0; i--, srcL += 2, srcR += 2, dest += 4)
	{
		__asm__ (
			/* Get source data. */
			"movd		 (%0), %%mm0\n"		// %mm0 = [ 0, R1]
			"movd		4(%0), %%mm1\n"		// %mm1 = [ 0, R2]
			"psllq		%%mm5, %%mm0\n"		// %mm0 = [R1,  0]
			"movd		 (%1), %%mm2\n"		// %mm2 = [ 0, L1]
			"psllq		%%mm5, %%mm1\n"		// %mm1 = [R2,  0]
			"movd		4(%1), %%mm3\n"		// %mm3 = [ 0, L2]
			"por		%%mm2, %%mm0\n"		// %mm0 = [R1, L1]
			"por		%%mm3, %%mm1\n"		// %mm1 = [R2, L2]
			"packssdw	%%mm1, %%mm0\n"		// %mm0 = [R2, L2, R1, L1]
			"movq		%%mm0, (%2)\n"
			: // output (dest is a ptr; it's not written to!)
			: "r" (srcR), "r" (srcL), "r" (dest)	// input
			);
	}
	
	// Reset the FPU state.
	__asm__ ("emms");
	
	// Clear the segment buffers.
	memset(LibGens::SoundMgr::ms_SegBufL, 0x00, SegLength*sizeof(LibGens::SoundMgr::ms_SegBufL[0]));
	memset(LibGens::SoundMgr::ms_SegBufR, 0x00, SegLength*sizeof(LibGens::SoundMgr::ms_SegBufR[0]));
	
	// Increase the buffer position.
	m_bufPos += SegLength;
	
	// Unlock the audio buffer.
	m_mtxBuf.unlock();
	return 0;
}


/**
 * writeMonoMMX(): Write the current segment to the audio buffer. (Monaural output; MMX-optimized)
 * @param dest Destination buffer.
 * @return 0 on success; non-zero on error.
 */
int GensPortAudio::writeMonoMMX(void)
{
	// Lock the audio buffer.
	m_mtxBuf.lock();
	
	// Segment length.
	const int SegLength = LibGens::SoundMgr::GetSegLength();
	
	// Destination buffer pointer.
	int16_t *dest = (int16_t*)&m_buf[(m_bufPos * m_sampleSize) / sizeof(m_buf[0])];
	
	// Source buffer pointers.
	int32_t *srcL = &LibGens::SoundMgr::ms_SegBufL[0];
	int32_t *srcR = &LibGens::SoundMgr::ms_SegBufR[0];
	
	// If the segment length is odd, write the first sample without MMX.
	if (SegLength & 1)
	{
		int32_t out = ((*srcL + *srcR) >> 1);
		
		if (out < -0x8000)
			*dest = -0x8000;
		else if (out > 0x7FFF)
			*dest = 0x7FFF;
		else
			*dest = (int16_t)out;
		
		// Next sample.
		srcL++;
		srcR++;
		dest++;
	}
	
	// NOTE: Using a shift value in a register
	// seems to be slower for monaural...
	
	// Write two samples at once using MMX.
	for (unsigned int i = (SegLength / 2); i != 0; i--, srcL += 2, srcR += 2, dest += 2)
	{
		__asm__ (
			/* Get source data. */
			"movq		 (%0), %%mm0\n"		// %mm0 = [R2, R1]
			"movq		 (%1), %%mm1\n"		// %mm1 = [L2, L1]
			"packssdw	%%mm0, %%mm0\n"		// %mm0 = [0, 0, R2, R1]
			"packssdw	%%mm1, %%mm1\n"		// %mm0 = [0, 0, L2, L1]
			"psraw		   $1, %%mm0\n"		// NOTE: Slight loss of precision...
			"psraw		   $1, %%mm1\n"		// NOTE: Slight loss of precision...
			"paddw		%%mm1, %%mm0\n"		// %mm0 = [0, 0, L2+R2, L1+R1]
			"movd		%%mm0, (%2)\n"
			: // output (dest is a ptr; it's not written to!)
			: "r" (srcR), "r" (srcL), "r" (dest)	// input
			);
	}
	
	// Reset the FPU state.
	__asm__ ("emms");
	
	// Clear the segment buffers.
	memset(LibGens::SoundMgr::ms_SegBufL, 0x00, SegLength*sizeof(LibGens::SoundMgr::ms_SegBufL[0]));
	memset(LibGens::SoundMgr::ms_SegBufR, 0x00, SegLength*sizeof(LibGens::SoundMgr::ms_SegBufR[0]));
	
	// Increase the buffer position.
	m_bufPos += SegLength;
	
	// Unlock the audio buffer.
	m_mtxBuf.unlock();
	return 0;
}
#endif /* HAVE_MMX */

}
