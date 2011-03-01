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

// C includes.
#include <stdint.h>

// Qt includes.
#include <QtCore/QObject>
#include <QtCore/QString>
#include <QtGui/QColor>

// Key configuration.
#include "actions/GensKeyConfig.hpp"

// Property function macro.
// NOTE: We can't include Q_PROPERTY() or signals here due to moc limitations.
#define GC_PROPERTY(propType, propName, setPropType, setPropName) \
		propType propName(void) const; \
		void set##setPropName(setPropType new##setPropName); \

namespace GensQt4
{

class GensConfigPrivate;

class GensConfig : public QObject
{
	Q_OBJECT
	
	public:
		GensConfig();
		~GensConfig();
		
		/**
		 * reload(): Load the user's configuration file.
		 * @param filename Filename. (If not specified, uses the default filename.)
		 * @return 0 on success; non-zero on error.
		 */
		int reload(void);
		int reload(const QString& filename);
		
		/**
		 * save(): Save the user's configuration file.
		 * @param filename Filename. (If not specified, uses the default filename.)
		 * @return 0 on success; non-zero on error.
		 */
		int save(void);
		int save(const QString& filename);
		
		/**
		 * emitAll(): Emit all configuration settings.
		 * Useful when starting the emulator.
		 */
		void emitAll(void);
		
	/** Properties. **/
		
		/** Configuration path. **/
		// TODO: Mark cfgPath as CONSTANT?
		// NOTE: This uses Qt directory separators.
		Q_PROPERTY(QString cfgPath READ cfgPath)
		QString cfgPath(void) const;
		
		/** User configuration paths. **/
		// TODO: Make these configurable?
		// NOTE: These use Qt directory separators.
		public:
			enum ConfigPath
			{
				GCPATH_CONFIG		= 0,	// Same as cfgPath.
				GCPATH_SAVESTATES	= 1,	// Savestates.
				GCPATH_SRAM		= 2,	// Cartridge SRAM.
				GCPATH_BRAM		= 3,	// Sega CD BRAM.
				GCPATH_WAV		= 4,	// WAV dumping.
				GCPATH_VGM		= 5,	// VGM dumping.
				GCPATH_SCREENSHOTS	= 6,	// Screemshots.
				
				GCPATH_MAX
			};
			QString userPath(ConfigPath pathID);
		
		/** Onscreen display. **/
		Q_PROPERTY(bool osdFpsEnabled READ osdFpsEnabled WRITE setOsdFpsEnabled NOTIFY osdFpsEnabled_changed)
		GC_PROPERTY(bool, osdFpsEnabled, bool, OsdFpsEnabled);
		Q_PROPERTY(QColor osdFpsColor READ osdFpsColor WRITE setOsdFpsColor NOTIFY osdFpsColor_changed)
		GC_PROPERTY(QColor, osdFpsColor, const QColor&, OsdFpsColor);
		Q_PROPERTY(bool osdMsgEnabled READ osdMsgEnabled WRITE setOsdMsgEnabled NOTIFY osdMsgEnabled_changed)
		GC_PROPERTY(bool, osdMsgEnabled, bool, OsdMsgEnabled);
		Q_PROPERTY(QColor osdMsgColor READ osdMsgColor WRITE setOsdMsgColor NOTIFY osdMsgColor_changed)
		GC_PROPERTY(QColor, osdMsgColor, const QColor&, OsdMsgColor);
		
		/** Intro effect. **/
		Q_PROPERTY(int introStyle READ introStyle WRITE setIntroStyle NOTIFY introStyle_changed)
		GC_PROPERTY(int, introStyle, int, IntroStyle);
		Q_PROPERTY(int introColor READ introColor WRITE setIntroColor NOTIFY introColor_changed)
		GC_PROPERTY(int, introColor, int, IntroColor);
		
		/** System. **/
		
		enum ConfRegionCode_t
		{
			CONFREGION_AUTODETECT = -1,
			CONFREGION_JP_NTSC    = 0,
			CONFREGION_ASIA_PAL   = 1,
			CONFREGION_US_NTSC   = 2,
			CONFREGION_EU_PAL     = 3
		};
		Q_ENUMS(ConfRegionCode_t);
		Q_PROPERTY(ConfRegionCode_t regionCode READ regionCode WRITE setRegionCode NOTIFY regionCode_changed);
		GC_PROPERTY(ConfRegionCode_t, regionCode, ConfRegionCode_t, RegionCode);
		
		// Region code auto-detection order.
		Q_PROPERTY(uint16_t regionCodeOrder READ regionCodeOrder WRITE setRegionCodeOrder NOTIFY regionCodeOrder_changed);
		GC_PROPERTY(uint16_t, regionCodeOrder, uint16_t, RegionCodeOrder);
		
		/** Sega CD Boot ROMs. **/
		Q_PROPERTY(QString mcdRomUSA READ mcdRomUSA WRITE setMcdRomUSA NOTIFY mcdRomUSA_changed)
		GC_PROPERTY(QString, mcdRomUSA, const QString&, McdRomUSA);
		Q_PROPERTY(QString mcdRomEUR READ mcdRomEUR WRITE setMcdRomEUR NOTIFY mcdRomEUR_changed)
		GC_PROPERTY(QString, mcdRomEUR, const QString&, McdRomEUR);
		Q_PROPERTY(QString mcdRomJPN READ mcdRomJPN WRITE setMcdRomJPN NOTIFY mcdRomJPN_changed)
		GC_PROPERTY(QString, mcdRomJPN, const QString&, McdRomJPN);
		Q_PROPERTY(QString mcdRomAsia READ mcdRomJPN WRITE setMcdRomAsia NOTIFY mcdRomAsia_changed)
		GC_PROPERTY(QString, mcdRomAsia, const QString&, McdRomAsia);
		
		/** External programs. **/
		Q_PROPERTY(QString extprgUnRAR READ extprgUnRAR WRITE setExtPrgUnRAR NOTIFY extprgUnRAR_changed)
		GC_PROPERTY(QString, extprgUnRAR, const QString&, ExtPrgUnRAR);
		
		/** Graphics settings. **/
		Q_PROPERTY(bool aspectRatioConstraint READ aspectRatioConstraint WRITE setAspectRatioConstraint NOTIFY aspectRatioConstraint_changed)
		GC_PROPERTY(bool, aspectRatioConstraint, bool, AspectRatioConstraint);
		Q_PROPERTY(bool fastBlur READ fastBlur WRITE setFastBlur NOTIFY fastBlur_changed)
		GC_PROPERTY(bool, fastBlur, bool, FastBlur);
		Q_PROPERTY(bool bilinearFilter READ bilinearFilter WRITE setBilinearFilter NOTIFY bilinearFilter_changed)
		GC_PROPERTY(bool, bilinearFilter, bool, BilinearFilter);
		
		enum InterlacedMode_t
		{
			INTERLACED_EVEN		= 0,
			INTERLACED_ODD		= 1,
			INTERLACED_FLICKER	= 2,
			INTERLACED_2X		= 3,
		};
		Q_ENUMS(InterlacedMode_t);
		GC_PROPERTY(InterlacedMode_t, interlacedMode, InterlacedMode_t, InterlacedMode);
		Q_PROPERTY(InterlacedMode_t interlacedMode READ interlacedMode WRITE setInterlacedMode NOTIFY interlacedMode_changed)
		
		Q_PROPERTY(int contrast READ contrast WRITE setContrast NOTIFY contrast_changed)
		GC_PROPERTY(int, contrast, int, Contrast);
		Q_PROPERTY(int brightness READ brightness WRITE setBrightness NOTIFY brightness_changed)
		GC_PROPERTY(int, brightness, int, Brightness);
		Q_PROPERTY(bool grayscale READ grayscale WRITE setGrayscale NOTIFY grayscale_changed)
		GC_PROPERTY(bool, grayscale, bool, Grayscale);
		Q_PROPERTY(bool inverted READ inverted WRITE setInverted NOTIFY inverted_changed)
		GC_PROPERTY(bool, inverted, bool, Inverted);
		Q_PROPERTY(int colorScaleMethod READ colorScaleMethod WRITE setColorScaleMethod NOTIFY colorScaleMethod_changed)
		GC_PROPERTY(int, colorScaleMethod, int, ColorScaleMethod);
		
		public:
			enum StretchMode_t
			{
				STRETCH_NONE	= 0,
				STRETCH_H	= 1,
				STRETCH_V	= 2,
				STRETCH_FULL	= 3
			};
		Q_ENUMS(StretchMode_t);
		Q_PROPERTY(StretchMode_t stretchMode READ stretchMode WRITE setStretchMode NOTIFY stretchMode_changed)
		GC_PROPERTY(StretchMode_t, stretchMode, StretchMode_t, StretchMode);
		
		/** General settings. **/
		Q_PROPERTY(bool autoFixChecksum READ autoFixChecksum WRITE setAutoFixChecksum NOTIFY autoFixChecksum_changed)
		GC_PROPERTY(bool, autoFixChecksum, bool, AutoFixChecksum);
		Q_PROPERTY(bool autoPause READ autoPause WRITE setAutoPause NOTIFY autoPause_changed)
		GC_PROPERTY(bool, autoPause, bool, AutoPause);
		Q_PROPERTY(bool borderColor READ borderColor WRITE setBorderColor NOTIFY borderColor_changed)
		GC_PROPERTY(bool, borderColor, bool, BorderColor);
		Q_PROPERTY(bool pauseTint READ pauseTint WRITE setPauseTint NOTIFY pauseTint_changed)
		GC_PROPERTY(bool, pauseTint, bool, PauseTint);
		Q_PROPERTY(bool ntscV30Rolling READ ntscV30Rolling WRITE setNtscV30Rolling NOTIFY ntscV30Rolling_changed)
		GC_PROPERTY(bool, ntscV30Rolling, bool, NtscV30Rolling);
		
		/** Savestates. **/
		Q_PROPERTY(int saveSlot READ saveSlot WRITE setSaveSlot NOTIFY saveSlot_changed)
		GC_PROPERTY(int, saveSlot, int, SaveSlot);
		void setSaveSlot_Prev(void);
		void setSaveSlot_Next(void);
		
		/** GensWindow configuration. **/
		Q_PROPERTY(bool showMenuBar READ showMenuBar WRITE setShowMenuBar NOTIFY showMenuBar_changed)
		GC_PROPERTY(bool, showMenuBar, bool, ShowMenuBar);
		
		/** Key configuration. **/
		int keyToAction(GensKey_t key);
		GensKey_t actionToKey(int action);
	
	signals:
		/** Onscreen display. **/
		void osdFpsEnabled_changed(bool enable);
		void osdFpsColor_changed(const QColor& color);
		void osdMsgEnabled_changed(bool enable);
		void osdMsgColor_changed(const QColor& color);
		
		/** System. **/
		void regionCode_changed(GensConfig::ConfRegionCode_t newRegionCode);
		void regionCodeOrder_changed(uint16_t newRegionCodeOrder);
		
		/** Intro effect. **/
		void introStyle_changed(int style);
		void introColor_changed(int color);
		
		/** Sega CD Boot ROMs. **/
		void mcdRomUSA_changed(const QString& filename);
		void mcdRomEUR_changed(const QString& filename);
		void mcdRomJPN_changed(const QString& filename);
		void mcdRomAsia_changed(const QString& filename);
		
		/** External programs. **/
		void extprgUnRAR_changed(const QString& filename);
		
		/** Graphics settings. **/
		void aspectRatioConstraint_changed(bool newAspectRatioConstraint);
		void fastBlur_changed(bool newFastBlur);
		void bilinearFilter_changed(bool newBilinearFilter);
		void interlacedMode_changed(GensConfig::InterlacedMode_t newInterlacedMode);
		void contrast_changed(int newContrast);
		void brightness_changed(int newBrightness);
		void grayscale_changed(bool newGrayscale);
		void inverted_changed(bool newInverted);
		void colorScaleMethod_changed(int newColorScaleMethod);
		void stretchMode_changed(GensConfig::StretchMode_t newStretchMode);
		
		/** General settings. **/
		void autoFixChecksum_changed(bool newAutoFixChecksum);
		void autoPause_changed(bool newAutoPause);
		void borderColor_changed(bool newBorderColor);
		void pauseTint_changed(bool newPauseTint);
		void ntscV30Rolling_changed(bool newNtscV30Rolling);
		
		/** Savestates. **/
		void saveSlot_changed(int newSaveSlot);
		
		/** GensWindow configuration. **/
		void showMenuBar_changed(bool newShowMenuBar);
	
	private:
		friend class GensConfigPrivate;
		GensConfigPrivate *d;
		Q_DISABLE_COPY(GensConfig)
};

}

#endif /* __GENS_QT4_GENSCONFIG_HPP__ */
