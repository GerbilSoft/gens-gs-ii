/***************************************************************************
 * gens-qt4: Gens Qt4 UI.                                                  *
 * EmuManager.hpp: Emulation manager.                                      *
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

#ifndef __GENS_QT4_HPP__
#define __GENS_QT4_HPP__

// Qt includes.
#include <QtCore/QObject>
#include <QtCore/QQueue>
#include <QtGui/QWidget>

// LibGens includes.
#include "libgens/Rom.hpp"
#include "libgens/IO/IoBase.hpp"

// Audio backend.
#include "Audio/GensPortAudio.hpp"

namespace GensQt4
{

class EmuManager : public QObject
{
	Q_OBJECT
	
	public:
		EmuManager();
		~EmuManager();
		
		// TODO: Move the parent argument to EmuManager()?
		int openRom(QWidget *parent = 0);
		int closeRom(void);
		
		inline bool isRomOpen(void) const { return (m_rom != NULL); }
		
		/** Controller settings. **/
		void setController(int port, LibGens::IoBase::IoType type);
		
		/** Rom class passthrough functions. **/
		
		inline LibGens::Rom::RomFormat romFormat(void) const
		{
			if (!m_rom)
				return LibGens::Rom::RFMT_UNKNOWN;
			return m_rom->romFormat();
		}
		
		inline LibGens::Rom::MDP_SYSTEM_ID sysId(void) const
		{
			if (!m_rom)
				return LibGens::Rom::MDP_SYSTEM_UNKNOWN;
			return m_rom->sysId();
		}
	
	signals:
		void updateFps(double fps);
		void stateChanged(void);		// Emulation state changed. Update the Gens title.
		void updateVideo(void);			// Update the video widget in GensWindow.
		
		/**
		 * osdPrintMsg(): Print a message on the OSD.
		 * @param duration Duration for the message to appear, in milliseconds.
		 * @param msg Message to print.
		 */
		void osdPrintMsg(int duration, const QString& msg);
	
	protected:
		// Timing management.
		double m_lastTime;
		int m_frames;
		
		// ROM object.
		LibGens::Rom *m_rom;
		
		// Audio backend.
		GensPortAudio *m_audio;
		
		// Controller change requests.
		struct CtrlChange_t { int port; LibGens::IoBase::IoType type; };
		QQueue<CtrlChange_t> m_qCtrlChange;
		void processQCtrlChange(void);
	
	protected slots:
		// Frame done from EmuThread.
		void emuFrameDone(void);
};

}

#endif /* __GENS_QT4_HPP__ */
