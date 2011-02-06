/***************************************************************************
 * gens-qt4: Gens Qt4 UI.                                                  *
 * GensConfig.hpp: Gens configuration.                                     *
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

#ifndef __GENS_QT4_GENSCONFIG_HPP__
#define __GENS_QT4_GENSCONFIG_HPP__

// Qt includes.
#include <QtCore/QObject>
#include <QtCore/QString>
#include <QtGui/QColor>

// Key configuration.
#include "actions/GensKeyConfig.hpp"

namespace GensQt4
{

class GensConfig : public QObject
{
	Q_OBJECT
	
	/** Properties. **/
	
	/** Onscreen display. **/
	Q_PROPERTY(bool osdFpsEnabled READ osdFpsEnabled WRITE setOsdFpsEnabled NOTIFY osdFpsEnabled_changed)
	Q_PROPERTY(QColor osdFpsColor READ osdFpsColor WRITE setOsdFpsColor NOTIFY osdFpsColor_changed)
	Q_PROPERTY(bool osdMsgEnabled READ osdMsgEnabled WRITE setOsdMsgEnabled NOTIFY osdMsgEnabled_changed)
	Q_PROPERTY(QColor osdMsgColor READ osdMsgColor WRITE setOsdMsgColor NOTIFY osdMsgColor_changed)
	
	/** Sega CD Boot ROMs. **/
	Q_PROPERTY(QString mcdRomUSA READ mcdRomUSA WRITE setMcdRomUSA NOTIFY mcdRomUSA_changed)
	Q_PROPERTY(QString mcdRomEUR READ mcdRomEUR WRITE setMcdRomEUR NOTIFY mcdRomEUR_changed)
	Q_PROPERTY(QString mcdRomJPN READ mcdRomJPN WRITE setMcdRomJPN NOTIFY mcdRomJPN_changed)
	
	/** External programs. **/
	Q_PROPERTY(QString extprgUnRAR READ extprgUnRAR WRITE setExtPrgUnRAR NOTIFY extprgUnRAR_changed)
	
	/** Graphics settings. **/
	Q_PROPERTY(bool aspectRatioConstraint READ aspectRatioConstraint WRITE setAspectRatioConstraint NOTIFY aspectRatioConstraint_changed)
	Q_PROPERTY(bool fastBlur READ fastBlur WRITE setFastBlur NOTIFY fastBlur_changed)
	Q_PROPERTY(int contrast READ contrast WRITE setContrast NOTIFY contrast_changed)
	Q_PROPERTY(int brightness READ brightness WRITE setBrightness NOTIFY brightness_changed)
	Q_PROPERTY(bool grayscale READ grayscale WRITE setGrayscale NOTIFY grayscale_changed)
	Q_PROPERTY(bool inverted READ inverted WRITE setInverted NOTIFY inverted_changed)
	
	/** General settings. **/
	Q_PROPERTY(bool autoFixChecksum READ autoFixChecksum WRITE setAutoFixChecksum NOTIFY autoFixChecksum_changed)
	Q_PROPERTY(bool autoPause READ autoPause WRITE setAutoPause NOTIFY autoPause_changed)
	Q_PROPERTY(bool borderColor READ borderColor WRITE setBorderColor NOTIFY borderColor_changed)
	Q_PROPERTY(bool pauseTint READ pauseTint WRITE setPauseTint NOTIFY pauseTint_changed)
	Q_PROPERTY(bool ntscV30Rolling READ ntscV30Rolling WRITE setNtscV30Rolling NOTIFY ntscV30Rolling_changed)
	
	/** Savestates. **/
	Q_PROPERTY(int saveSlot READ saveSlot WRITE setSaveSlot NOTIFY saveSlot_changed)
	
	public:
		GensConfig();
		~GensConfig();
		
		void save(void);
		void reload(void);
		
		/** Onscreen display. **/
		bool osdFpsEnabled(void) const
			{ return m_osdFpsEnabled; }
		void setOsdFpsEnabled(bool enable);
		QColor osdFpsColor(void) const
			{ return m_osdFpsColor; }
		void setOsdFpsColor(const QColor& color);
		bool osdMsgEnabled(void) const
			{ return m_osdMsgEnabled; }
		void setOsdMsgEnabled(bool enable);
		QColor osdMsgColor(void) const
			{ return m_osdMsgColor; }
		void setOsdMsgColor(const QColor& color);
		
		/** Sega CD Boot ROMs. **/
		QString mcdRomUSA(void) const
			{ return m_mcdRomUSA; }
		void setMcdRomUSA(const QString& filename);
		QString mcdRomEUR(void) const
			{ return m_mcdRomEUR; }
		void setMcdRomEUR(const QString& filename);
		QString mcdRomJPN(void) const
			{ return m_mcdRomJPN; }
		void setMcdRomJPN(const QString& filename);
		
		/** External programs. **/
		QString extprgUnRAR(void) const
			{ return m_extprgUnRAR; }
		void setExtPrgUnRAR(const QString& filename);
		
		/** Graphics settings. **/
		bool aspectRatioConstraint(void) const
			{ return m_aspectRatioConstraint; }
		void setAspectRatioConstraint(bool newAspectRatioConstraint);
		bool fastBlur(void) const
			{ return m_fastBlur; }
		void setFastBlur(bool newFastBlur);
		int contrast(void) const
			{ return m_contrast; }
		void setContrast(int newContrast);
		int brightness(void) const
			{ return m_brightness; }
		void setBrightness(int newBrightness);
		bool grayscale(void) const
			{ return m_grayscale; }
		void setGrayscale(bool newGrayscale);
		bool inverted(void) const
			{ return m_inverted; }
		void setInverted(bool newInverted);
		// TODO: Color Scale Method.
		
		/** General settings. **/
		bool autoFixChecksum(void) const
			{ return m_autoFixChecksum; }
		void setAutoFixChecksum(bool newAutoFixChecksum);
		bool autoPause(void) const
			{ return m_autoPause; }
		void setAutoPause(bool newAutoPause);
		bool borderColor(void) const
			{ return m_borderColor; }
		void setBorderColor(bool newBorderColor);
		bool pauseTint(void) const
			{ return m_pauseTint; }
		void setPauseTint(bool newPauseTint);
		bool ntscV30Rolling(void) const
			{ return m_ntscV30Rolling; }
		void setNtscV30Rolling(bool newNtscV30Rolling);
		
		/** Savestates. **/
		int saveSlot(void) const
			{ return m_saveSlot; }
		void setSaveSlot(int newSaveSlot);
		void setSaveSlot_Prev(void)
			{ setSaveSlot((m_saveSlot + 1) % 10); }
		void setSaveSlot_Next(void)
			{ setSaveSlot((m_saveSlot + 9) % 10); }
		
		/** Key configuration. **/
		int keyToAction(GensKey_t key)
			{ return m_keyConfig.keyToAction(key); }
	
	protected:
		/** Onscreen display. **/
		bool m_osdFpsEnabled;
		QColor m_osdFpsColor;
		bool m_osdMsgEnabled;
		QColor m_osdMsgColor;
		
		/** Sega CD Boot ROMs. **/
		QString m_mcdRomUSA;
		QString m_mcdRomEUR;
		QString m_mcdRomJPN;
		
		/** External programs. **/
		QString m_extprgUnRAR;
		
		/** Graphics settings. **/
		bool m_aspectRatioConstraint;
		bool m_fastBlur;
		int m_contrast;
		int m_brightness;
		bool m_grayscale;
		bool m_inverted;
		// TODO: Color Scale Method.
		
		/** General settings. **/
		bool m_autoFixChecksum;
		bool m_autoPause;
		bool m_borderColor;
		bool m_pauseTint;
		bool m_ntscV30Rolling;
		
		/** Savestates. **/
		int m_saveSlot;
		
		/** Key configuration. **/
		GensKeyConfig m_keyConfig;
	
	signals:
		/** Onscreen display. **/
		void osdFpsEnabled_changed(bool enable);
		void osdFpsColor_changed(const QColor& color);
		void osdMsgEnabled_changed(bool enable);
		void osdMsgColor_changed(const QColor& color);
		
		/** Sega CD Boot ROMs. **/
		void mcdRomUSA_changed(const QString& filename);
		void mcdRomEUR_changed(const QString& filename);
		void mcdRomJPN_changed(const QString& filename);
		
		/** External programs. **/
		void extprgUnRAR_changed(const QString& filename);
		
		/** Graphics settings. **/
		void aspectRatioConstraint_changed(bool newAspectRatioConstraint);
		void fastBlur_changed(bool newFastBlur);
		void contrast_changed(int newContrast);
		void brightness_changed(int newBrightness);
		void grayscale_changed(bool newGrayscale);
		void inverted_changed(bool newInverted);
		// TODO: Color Scale Method.
		
		/** General settings. **/
		void autoFixChecksum_changed(bool newAutoFixChecksum);
		void autoPause_changed(bool newAutoPause);
		void borderColor_changed(bool newBorderColor);
		void pauseTint_changed(bool newPauseTint);
		void ntscV30Rolling_changed(bool newNtscV30Rolling);
		
		/** Savestates. **/
		void saveSlot_changed(int newSaveSlot);
};

}

#endif /* __GENS_QT4_GENSCONFIG_HPP__ */
