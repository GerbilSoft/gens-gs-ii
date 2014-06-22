/***************************************************************************
 * gens-qt4: Gens Qt4 UI.                                                  *
 * GensQApplication.cpp: QApplication subclass.                            *
 *                                                                         *
 * Copyright (c) 1999-2002 by Stéphane Dallongeville.                      *
 * Copyright (c) 2003-2004 by Stéphane Akhoun.                             *
 * Copyright (c) 2008-2014 by David Korth.                                 *
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

#include "GensQApplication.hpp"

// Qt includes.
#include <QtCore/QTranslator>
#include <QtCore/QLocale>
#include <QtCore/QLibraryInfo>
#include <QtCore/QDir>
#include <QtGui/QIcon>
#include <QtGui/QStyle>

// LibGens includes.
#include "libgens/lg_main.hpp"
#include "libgens/macros/common.h"

namespace GensQt4
{

class GensQApplicationPrivate
{
	public:
		GensQApplicationPrivate(GensQApplication *q);
		void init(void);

		// Initialize GensQApplication.
		void gqaInit(void);

		/**
		 * Set the Gens translation.
		 * @param locale Locale name, e.g. "en_US".
		 */
		void setGensTranslation(const QString &locale);

	private:
		GensQApplication *const q;
		Q_DISABLE_COPY(GensQApplicationPrivate)

	public:
		// GUI thread.
		QThread *guiThread;

	private:
		// Qt translators.
		QTranslator *qtTranslator;
		QTranslator *gensTranslator;
};


/**************************************
 * GensQApplicationPrivate functions. *
 **************************************/

GensQApplicationPrivate::GensQApplicationPrivate(GensQApplication *q)
	: q(q)
	, guiThread(nullptr)
	, qtTranslator(nullptr)
	, gensTranslator(nullptr)
{ }

/**
 * GensQApplication initialization function.
 * The same code is used in all three GensQApplication() constructors.
 */
void GensQApplicationPrivate::gqaInit(void)
{
	// Save the GUI thread pointer for later.
	guiThread = QThread::currentThread();

	// Set application information.
	QCoreApplication::setOrganizationName(QLatin1String("GerbilSoft"));
	QCoreApplication::setApplicationName(QLatin1String("Gens/GS II"));

	// Version number.
	// TODO: Use MDP version macros.
	// NOTE: gens-qt4's version is currently tied to LibGens.
	const QString sVersion = QString::fromLatin1("%1.%2.%3")
					.arg((LibGens::version >> 24) & 0xFF)
					.arg((LibGens::version >> 16) & 0xFF)
					.arg(LibGens::version & 0xFFFF);
	QCoreApplication::setApplicationVersion(sVersion);

	// Set the application icon.
	QIcon iconApp;
	iconApp.addFile(QLatin1String(":/gens/gensgs_48x48.png"), QSize(48, 48));
	iconApp.addFile(QLatin1String(":/gens/gensgs_32x32.png"), QSize(32, 32));
	iconApp.addFile(QLatin1String(":/gens/gensgs_16x16.png"), QSize(16, 16));
	q->setWindowIcon(iconApp);

#if QT_VERSION >= 0x040600
	// Check if an icon theme is available.
	if (!QIcon::hasThemeIcon(QLatin1String("application-exit"))) {
		// Icon theme is not available.
		// Use built-in Oxygen icon theme.
		// Reference: http://tkrotoff.blogspot.com/2010/02/qiconfromtheme-under-windows.html
		QIcon::setThemeName(QLatin1String("oxygen"));
	}
#endif

#ifdef Q_OS_WIN32
	// Set the application font.
	q->SetFont_Win32();
#endif /* Q_OS_WIN32 */

	// Initialize Qt translators.
	qtTranslator = new QTranslator(q);
	q->installTranslator(qtTranslator);
	gensTranslator = new QTranslator(q);
	q->installTranslator(gensTranslator);

	// Initialize the Gens translation.
	setGensTranslation(QLocale::system().name());

	// Connect the crash handler.
#ifdef HAVE_SIGACTION
	QObject::connect(q, SIGNAL(signalCrash(int,siginfo_t*,void*)),
			 q, SLOT(slotCrash(int,siginfo_t*,void*)));
#else /* !HAVE_SIGACTION */
	QObject::connect(q, SIGNAL(signalCrash(int)),
			 q, SLOT(slotCrash(int)));
#endif /* HAVE_SIGACTION */
}


/**
 * Set the Gens translation.
 * @param locale Locale name, e.g. "en_US".
 */
void GensQApplicationPrivate::setGensTranslation(const QString &locale)
{
	// Initialize the Qt translation system.
	// TODO: Allow switching languages on the fly?
	// TODO: Check in the following directories:
	// * Qt library directory
	// * Application/translations/
	// * Application/
	// * config/
	qtTranslator->load(
		QLatin1String("qt_") + locale,
		QLibraryInfo::location(QLibraryInfo::TranslationsPath));

	// Initialize the Gens translator.
	// TODO: Check in the following directories:
	// * Application/translations/
	// * Application/
	// * config/
	QDir appDir(QApplication::applicationDirPath());
	gensTranslator->load(
		QLatin1String("gens-qt4_") + locale,
		appDir.absoluteFilePath(QLatin1String("translations/")));

	/** Translation file information. **/

	//: Translation file author. Put your name here.
	QString tsAuthor = GensQApplication::tr("David Korth", "ts-author");
	Q_UNUSED(tsAuthor)

	// TODO: Allow the program to access the translation file information.
}


/*******************************
 * GensQApplication functions. *
 *******************************/

GensQApplication::GensQApplication(int &argc, char **argv)
	: QApplication(argc, argv)
	, d(new GensQApplicationPrivate(this))
{
	d->gqaInit();
}

GensQApplication::GensQApplication(int &argc, char **argv, bool GUIenabled)
	: QApplication(argc, argv, GUIenabled)
	, d(new GensQApplicationPrivate(this))
{
	d->gqaInit();
}

GensQApplication::GensQApplication(int &argc, char **argv, Type type)
	: QApplication(argc, argv, type)
	, d(new GensQApplicationPrivate(this))
{
	d->gqaInit();
}

GensQApplication::~GensQApplication()
	{ delete d; }

/**
 * Check if the current thread is the GUI thread.
 * @return True if it is; false if it isn't.
 */
bool GensQApplication::isGuiThread(void)
{
	return (QThread::currentThread() == d->guiThread);
}

/**
 * Get an icon from the system theme.
 * @param name Icon name.
 * @return QIcon.
 */
QIcon GensQApplication::IconFromTheme(const QString &name)
{
#if QT_VERSION >= 0x040600
	if (QIcon::hasThemeIcon(name))
		return QIcon::fromTheme(name);
#endif

	// System icon doesn't exist.
	// Get the built-in icon.
	QString startDir = QLatin1String(":/oxygen/");
	static const int iconSizes[] = {256, 128, 64, 48, 32, 24, 22, 16};

	QIcon icon;
	for (int i = 0; i < ARRAY_SIZE(iconSizes); i++) {
		int sz = iconSizes[i];
		QString num = QString::number(sz);
		QString filename = startDir + num + QChar(L'x') + num + QChar(L'/') + name;
		QPixmap pxm(filename);
		if (!pxm.isNull())
			icon.addPixmap(pxm);
	}

	return icon;
}

/**
 * Get an icon from the Gens/GS II icon set.
 * @param name Icon name.
 * @param subcategory Subcategory.
 * @return QIcon.
 */
QIcon GensQApplication::IconFromProgram(const QString &name, const QString &subcategory)
{
	// System icon doesn't exist.
	// Get the fallback icon.
	struct IconSz_t {
		QString path;
		int sz;
	};

	QString startDir = QLatin1String(":/gens/");
	if (!subcategory.isEmpty())
		startDir += subcategory + QChar(L'/');

	static const int iconSizes[] = {256, 128, 64, 48, 32, 24, 22, 16};

	QIcon icon;
	for (int i = 0; i < ARRAY_SIZE(iconSizes); i++) {
		int sz = iconSizes[i];
		QString num = QString::number(sz);
		QString filename = startDir + num + QChar(L'x') + num + QChar(L'/') + name;
		QPixmap pxm(filename);
		if (!pxm.isNull())
			icon.addPixmap(pxm);
	}

	return icon;
}

/**
 * Get an icon from the Gens/GS II icon set.
 * @param name Icon name.
 * @return QIcon.
 */
QIcon GensQApplication::IconFromProgram(const QString &name)
{
	return IconFromProgram(name, QString());
}

/**
 * Get a standard icon.
 * @param standardIcon Standard pixmap.
 * @param option QStyleOption.
 * @param widget QWidget.
 * @return QIcon.
 */
QIcon GensQApplication::StandardIcon(QStyle::StandardPixmap standardIcon,
				const QStyleOption *option,
				const QWidget *widget)
{
	QStyle *style = QApplication::style();
	QIcon icon;
	const char *xdg_icon = nullptr;

#if defined(Q_OS_UNIX) && !defined(Q_OS_MAC)
	// Always use StandardPixmap.
	// Qt will use the xdg icon if the StandardPixmap isn't found.
	// TODO: Verify this behavior on old systems.
	switch (standardIcon) {
		case QStyle::SP_MessageBoxQuestion:
			xdg_icon = "dialog-question";
			break;
		default:
			xdg_icon = nullptr;
			break;
	}
#else
	// Other systems.
	// TODO: Check icons on Mac.
	switch (standardIcon) {
		// Windows only.
		case QStyle::SP_MessageBoxQuestion:
#if defined(Q_OS_WIN)
			icon = style->standardIcon(standardIcon, option, widget);
#else /* !Q_OS_WIN */
			xdg_icon = "dialog-question";
#endif /* Q_OS_WIN */
			break;

		// Neither Windows nor Mac OS X.
		case QStyle::SP_DialogApplyButton:
			xdg_icon = "dialog-ok-apply";
			break;
		case QStyle::SP_DialogCloseButton:
			xdg_icon = "dialog-close";
			break;

		// Available on all systems.
		case QStyle::SP_MessageBoxInformation:
		case QStyle::SP_MessageBoxWarning:
		case QStyle::SP_MessageBoxCritical:
		default:
			// TODO: Add more icons.
			icon = style->standardIcon(standardIcon, option, widget);
			break;
	}
#endif /* defined(Q_OS_UNIX) && !defined(Q_OS_MAC) */

	if (icon.isNull()) {
		if (xdg_icon)
			icon = IconFromTheme(QLatin1String(xdg_icon));
		if (icon.isNull()) {
			// We don't have a custom icon for this one.
			return style->standardIcon(standardIcon, option, widget);
		}
	}

	return icon;
}

}
