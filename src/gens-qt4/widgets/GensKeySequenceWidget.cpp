/***************************************************************************
 * gens-qt4: Gens Qt4 UI.                                                  *
 * GensKeySequenceWidget.cpp: Key sequence input widget.                   *
 * Minimal reimplementation of KKeySequenceWidget from KDE 4.6.0.          *
 *                                                                         *
 * Copyright (c) 1998 Mark Donohoe <donohoe@kde.org>                       *
 * Copyright (c) 2001 Ellis Whitehead <ellis@kde.org>                      *
 * Copyright (c) 2007 Andreas Hartmetz <ahartmetz@gmail.com>               *
 * Copyright (c) 2011 by David Korth.                                      *
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

#include "GensKeySequenceWidget.hpp"
#include "GensKeySequenceWidget_p.hpp"

// Qt includes.
#include <QtCore/QTimer>
#include <QtGui/QApplication>
#include <QtGui/QHBoxLayout>
#include <QtGui/QKeySequence>
#include <QtGui/QToolButton>
#include <QtGui/QIcon>

// Qt events.
#include <QtGui/QKeyEvent>
#include <QtGui/QFocusEvent>

// GensQApplication::IconFromTheme()
#include "../GensQApplication.hpp"


namespace GensQt4
{

class GensKeySequenceWidgetPrivate
{
	public:
		GensKeySequenceWidgetPrivate(GensKeySequenceWidget *q);
		
		void init(void);
		static QIcon GetClearButtonIcon(void);
		
		static QKeySequence appendToSequence(const QKeySequence& seq, int keyQt);
		
		void updateShortcutDisplay(void);
		void startRecording(void);
		
		void controlModifierlessTimout()
		{
			if (nKey != 0 && !modifierKeys)
			{
				// No modifier key pressed currently. Start the timout
				modifierlessTimeout.start(ms_TimeoutValue);
			}
			else
			{
				// A modifier is pressed. Stop the timeout
				modifierlessTimeout.stop();
			}
		}
		
		void cancelRecording(void)
		{
			keySequence = oldKeySequence;
			doneRecording();
		}
		
		bool isShiftAsModifierAllowed(int keyQt)
		{
			// Shift only works as a modifier with certain keys. It's not possible
			// to enter the SHIFT+5 key sequence for me because this is handled as
			// '%' by qt on my keyboard.
			// The working keys are all hardcoded here :-(
			if (keyQt >= Qt::Key_F1 && keyQt <= Qt::Key_F35)
				return true;

			if (QChar(keyQt).isLetter())
				return true;

			switch (keyQt)
			{
				case Qt::Key_Return:
				case Qt::Key_Space:
				case Qt::Key_Backspace:
				case Qt::Key_Escape:
				case Qt::Key_Print:
				case Qt::Key_ScrollLock:
				case Qt::Key_Pause:
				case Qt::Key_PageUp:
				case Qt::Key_PageDown:
				case Qt::Key_Insert:
				case Qt::Key_Delete:
				case Qt::Key_Home:
				case Qt::Key_End:
				case Qt::Key_Up:
				case Qt::Key_Down:
				case Qt::Key_Left:
				case Qt::Key_Right:
					return true;
				
				default:
					return false;
			}
		}
		
		// private slot
		void doneRecording(void);
		
		// members
		GensKeySequenceWidget *const q;
		QHBoxLayout *layout;
		GensKeySequenceButton *keyButton;
		QToolButton *clearButton;
		
		QTimer modifierlessTimeout;
		static const int ms_TimeoutValue = 600;
		
		// key sequence values
		QKeySequence keySequence;
		QKeySequence oldKeySequence;
		uint nKey;
		uint modifierKeys;
		bool isRecording;
};


/*******************************************
 * GensKeySequenceWidgetPrivate functions. *
 *******************************************/

GensKeySequenceWidgetPrivate::GensKeySequenceWidgetPrivate(GensKeySequenceWidget *q)
	: q(q)
	, layout(NULL)
	, keyButton(NULL)
	, clearButton(NULL)
	, nKey(0)
	, modifierKeys(0)
	, isRecording(false)
{}


/**
 * GensKeySequenceWidgetPrivate::GetClearButtonIcon(): Get the clear button icon.
 * @return Clear button icon.
 */
inline QIcon GensKeySequenceWidgetPrivate::GetClearButtonIcon(void)
{
	// Determine which icon to use for the clear button.
	if (QApplication::isLeftToRight())
	{
		return GensQApplication::IconFromTheme(
				QLatin1String("edit-clear-locationbar-rtl"));
	}
	else
	{
		return GensQApplication::IconFromTheme(
				QLatin1String("edit-clear-locationbar-ltr"));
	}
	
	// Should not get here...
	return QIcon();
}


/**
 * GensKeySequenceWidgetPrivate::init(): Initialize the private data members.
 */
void GensKeySequenceWidgetPrivate::init(void)
{
	layout = new QHBoxLayout(q);
	layout->setMargin(0);
	
	// Create the key button.
	keyButton = new GensKeySequenceButton(this, q);
	keyButton->setFocusPolicy(Qt::StrongFocus);
	keyButton->setIcon(GensQApplication::IconFromTheme(QLatin1String("configure")));
	layout->addWidget(keyButton);
	
	// Create the clear button.
	clearButton = new QToolButton(q);
	layout->addWidget(clearButton);
	
	// Set the clear button icon.
	clearButton->setIcon(GetClearButtonIcon());
}


QKeySequence GensKeySequenceWidgetPrivate::appendToSequence(const QKeySequence& seq, int keyQt)
{
	switch (seq.count())
	{
		case 0:
			return QKeySequence(keyQt);
		case 1:
			return QKeySequence(seq[0], keyQt);
		case 2:
			return QKeySequence(seq[0], seq[1], keyQt);
		case 3:
			return QKeySequence(seq[0], seq[1], seq[2], keyQt);
		default:
			return seq;
	}
}


/**
 * GensKeySequenceWidgetPrivate::startRecording(): Start recording a key sequence.
 */
void GensKeySequenceWidgetPrivate::startRecording()
{
	nKey = 0;
	modifierKeys = 0;
	oldKeySequence = keySequence;
	keySequence = QKeySequence();
	
	isRecording = true;
	keyButton->grabKeyboard();
	
	if (!QWidget::keyboardGrabber())
	{
		// TODO: Output a warning.
		//kWarning() << "Failed to grab the keyboard! Most likely qt's nograb option is active";
	}
	
	keyButton->setDown(true);
	updateShortcutDisplay();
}


/**
 * GensKeySequenceWidgetPrivate::doneRecording(): Done recording a key sequence.
 */
void GensKeySequenceWidgetPrivate::doneRecording(void)
{
	modifierlessTimeout.stop();
	isRecording = false;
	keyButton->releaseKeyboard();
	keyButton->setDown(false);
	
	if (keySequence == oldKeySequence)
	{
		// The sequence hasn't changed
		updateShortcutDisplay();
		return;
	}
	
	// Key sequence has changed.
        emit q->keySequenceChanged(keySequence);
	updateShortcutDisplay();
}


/**
 * GensKeySequenceWidgetPrivate::updateShortcutDisplay(): Update shortcut display.
 */
void GensKeySequenceWidgetPrivate::updateShortcutDisplay(void)
{
	// NOTE: QKeySequence with just a modifier does NOT print a trailing '+'
	// on Mac OS X,but does print a trailing '+' on other systems.
	// Hence, we're using built-in strings for now.
#if defined(Q_WS_MAC)
	// http://macbiblioblog.blogspot.com/2005/05/special-key-symbols.html
	static const QString sModCtrl  = QChar(0x2318);	// Command symbol.
	static const QString sModAlt   = QChar(0x2325);	// Option symbol.
	static const QString sModMeta  = QChar(0x2303);	// Control symbol.
	static const QString sModShift = QChar(0x21E7); // Shift symbol.
#elif defined(Q_WS_WIN)
	static const QString sModCtrl  = GensKeySequenceWidget::tr("Ctrl");
	static const QString sModAlt   = GensKeySequenceWidget::tr("Alt");
	static const QString sModMeta  = GensKeySequenceWidget::tr("Win");
	static const QString sModShift = GensKeySequenceWidget::tr("Shift");
#else
	static const QString sModCtrl  = GensKeySequenceWidget::tr("Ctrl");
	static const QString sModAlt   = GensKeySequenceWidget::tr("Alt");
	static const QString sModMeta  = GensKeySequenceWidget::tr("Meta");
	static const QString sModShift = GensKeySequenceWidget::tr("Shift");
#endif
	
	// Empty string if no non-modifier was pressed.
	QString s = keySequence.toString(QKeySequence::NativeText);
	s.replace(QChar(L'&'), QLatin1String("&&"));
	
	if (isRecording)
	{
		if (modifierKeys)
		{
			
#if defined(Q_WS_MAC)
			if (modifierKeys & Qt::META)	s += sModMeta;
			if (modifierKeys & Qt::ALT)	s += sModAlt;
			if (modifierKeys & Qt::SHIFT)	s += sModShift;
			if (modifierKeys & Qt::CTRL)	s += sModCtrl;
			
			if (modifierKeys & (Qt::META | Qt::ALT | Qt::CTRL | Qt::SHIFT))
				 s += QChar(L'+');
#else /* !Q_WS_MAC */
			if (modifierKeys & Qt::META)	s += sModMeta + QChar(L'+');
			if (modifierKeys & Qt::CTRL)	s += sModCtrl + QChar(L'+');
			if (modifierKeys & Qt::ALT)	s += sModAlt + QChar(L'+');
			if (modifierKeys & Qt::SHIFT)	s += sModShift + QChar(L'+');
#endif /* Q_WS_MAC */
		}
		else if (nKey == 0)
		{
			s = GensKeySequenceWidget::tr("Input");
		}
		
		//make it clear that input is still going on
		s.append(QLatin1String(" ..."));
	}
	
	if (s.isEmpty())
		s = GensKeySequenceWidget::tr("None");
	
	s.prepend(QChar(L' '));
	s.append(QChar(L' '));
	keyButton->setText(s);
}


/************************************
 * GensKeySequenceWidget functions. *
 ************************************/


GensKeySequenceWidget::GensKeySequenceWidget(QWidget *parent)
	: QWidget(parent)
	, d(new GensKeySequenceWidgetPrivate(this))
{
	// Initialize the private members.
	d->init();
	setFocusProxy(d->keyButton);
	
	// Connect signals to the buttons.
	connect(d->keyButton, SIGNAL(clicked()), this, SLOT(captureKeySequence()));
	connect(d->clearButton, SIGNAL(clicked()), this, SLOT(clearKeySequence()));
	connect(&d->modifierlessTimeout, SIGNAL(timeout()), this, SLOT(doneRecording()));

	//TODO: how to adopt style changes at runtime?
	/*QFont modFont = d->clearButton->font();
	modFont.setStyleHint(QFont::TypeWriter);
	d->clearButton->setFont(modFont);*/
	d->updateShortcutDisplay();
}


GensKeySequenceWidget::~GensKeySequenceWidget()
{
	delete d;
}


/**
 * GensKeySequenceWidget::changeEvent(): Widget state has changed.
 * @param event State change event.
 */
void GensKeySequenceWidget::changeEvent(QEvent *event)
{
	if (event->type() == QEvent::LayoutDirectionChange)
	{
		// Update the clear button icon.
		d->clearButton->setIcon(GensKeySequenceWidgetPrivate::GetClearButtonIcon());
	}
	
	// Pass the event to the base class.
	this->QWidget::changeEvent(event);
}


/**
 * GensKeySequenceWidget::doneRecording(): Done recording a sequence.
 * Forwards to GensKeySequenceWidgetPrivate::doneRecording().
 * We can't use Q_PRIVATE_SLOT() because the private class isn't available,
 * and we're not using KDE's automoc4 to generate mocs without linking
 * them into the executable separately.
 * 
 * (automoc4 links mocs using #include.)
 */
void GensKeySequenceWidget::doneRecording(void)
{
	d->doneRecording();
}


/**
 * GensKeySequenceWidget::keySequence(): Return the currently selected key sequence.
 * @return Currently selected key sequence.
 */
QKeySequence GensKeySequenceWidget::keySequence() const
{
	return d->keySequence;
}


/**
 * GensKeySequenceWidget::captureKeySequence(): Capture a key sequence.
 */
void GensKeySequenceWidget::captureKeySequence(void)
{
	d->startRecording();
}


/**
 * GensKeySequenceWidget::setKeySequence(): Set the key sequence.
 * @param seq Key sequence.
 */
void GensKeySequenceWidget::setKeySequence(const QKeySequence& seq)
{
	// oldKeySequence holds the key sequence before recording started, if setKeySequence()
	// is called while not recording then set oldKeySequence to the existing sequence so
	// that the keySequenceChanged() signal is emitted if the new and previous key
	// sequences are different
	if (!d->isRecording)
		d->oldKeySequence = d->keySequence;
	
	d->keySequence = seq;
	d->doneRecording();
}


/**
 * GensKeySequenceWidget::clearKeySequence(): Clear the current key sequence.
 */
void GensKeySequenceWidget::clearKeySequence()
{
	setKeySequence(QKeySequence());
}


/************************************
 * GensKeySequenceButton functions. *
 ************************************/


GensKeySequenceButton::~GensKeySequenceButton()
{
}


void GensKeySequenceButton::setText(const QString& text)
{
	QPushButton::setText(text);
	//setFixedSize( sizeHint().width()+12, sizeHint().height()+8 );
}


/**
 * GensKeySequenceButton::event(): Handle events.
 * This is mainly to prevent Qt from special-casing Tab and Backtab.
 * @param e QEvent.
 */
bool GensKeySequenceButton::event(QEvent* e)
{
	if (d->isRecording && e->type() == QEvent::KeyPress)
	{
		keyPressEvent(static_cast<QKeyEvent*>(e));
		return true;
	}
	
	// The shortcut 'alt+c' ( or any other dialog local action shortcut )
	// ended the recording and triggered the action associated with the
	// action. In case of 'alt+c' ending the dialog.  It seems that those
	// ShortcutOverride events get sent even if grabKeyboard() is active.
	if (d->isRecording && e->type() == QEvent::ShortcutOverride)
	{
		e->accept();
		return true;
	}
	
	return QPushButton::event(e);
}


/**
 * GensKeySequenceButton::keyPressEvent(): Key press event.
 * @param e QKeyEvent.
 */
void GensKeySequenceButton::keyPressEvent(QKeyEvent *e)
{
	int keyQt = e->key();
	if (keyQt == -1)
	{
		// Qt sometimes returns garbage keycodes, I observed -1, if it doesn't know a key.
		// We cannot do anything useful with those (several keys have -1, indistinguishable)
		// and QKeySequence.toString() will also yield a garbage string.
		
		// TODO: Show an error message.
#if 0
		KMessageBox::sorry(this,
			i18n("The key you just pressed is not supported by Qt."),
			i18n("Unsupported Key"));
#endif
		return d->cancelRecording();
	}
	
	uint newModifiers = e->modifiers() & (Qt::SHIFT | Qt::CTRL | Qt::ALT | Qt::META);
	
	//don't have the return or space key appear as first key of the sequence when they
	//were pressed to start editing - catch and them and imitate their effect
	if (!d->isRecording && ((keyQt == Qt::Key_Return || keyQt == Qt::Key_Space)))
	{
		d->startRecording();
		d->modifierKeys = newModifiers;
		d->updateShortcutDisplay();
		return;
	}
	
	// We get events even if recording isn't active.
	if (!d->isRecording)
		return QPushButton::keyPressEvent(e);
	
	e->accept();
	d->modifierKeys = newModifiers;
	
	switch (keyQt)
	{
		case Qt::Key_AltGr: //or else we get unicode salad
			return;
		
		case Qt::Key_Shift:
		case Qt::Key_Control:
		case Qt::Key_Alt:
		case Qt::Key_Meta:
		case Qt::Key_Menu: //unused (yes, but why?)
			d->controlModifierlessTimout();
			d->updateShortcutDisplay();
			break;
		
		default:
			// All keys are allowed with or without modifiers.
			
			// TODO: Figure out how to get e.g. Shift+2 working.
			// Shift+2 shows up as '@' on US keyboards,
			// but may be '"' on international keyboards.
			// FIXME: Ignore those shifted keys for now.
			if (keyQt != 0)
			{
				if ((keyQt == Qt::Key_Backtab) && (d->modifierKeys & Qt::SHIFT))
					keyQt = Qt::Key_Tab | d->modifierKeys;
				else if (!(d->modifierKeys & Qt::SHIFT) || d->isShiftAsModifierAllowed(keyQt))
					keyQt |= d->modifierKeys;
				else
				{
					// TODO: Fix shift modifiers with number keys.
					//keyQt |= (d->modifierKeys & ~Qt::SHIFT);
					keyQt = 0;
				}
				
				if (d->nKey == 0)
					d->keySequence = QKeySequence(keyQt);
				else
					d->keySequence =
						GensKeySequenceWidgetPrivate::appendToSequence(d->keySequence, keyQt);
				
				d->nKey++;
				
				// We're not allowing multi-key shortcuts in GensKeySequenceWidget.
				//if ((!d->multiKeyShortcutsAllowed) || (d->nKey >= 4))
				{
					d->doneRecording();
					return;
				}
				
				d->controlModifierlessTimout();
				d->updateShortcutDisplay();
			}
	}
}


/**
 * GensKeySequenceButton::keyPressEvent(): Key press event.
 * @param e QKeyEvent.
 */
void GensKeySequenceButton::keyReleaseEvent(QKeyEvent *e)
{
	if (e->key() == -1)
	{
		// ignore garbage, see keyPressEvent()
		return;
	}
	
	if (!d->isRecording)
		return QPushButton::keyReleaseEvent(e);
	
	e->accept();
	
	uint newModifiers = e->modifiers() & (Qt::SHIFT | Qt::CTRL | Qt::ALT | Qt::META);
	
	//if a modifier that belongs to the shortcut was released...
	if ((newModifiers & d->modifierKeys) < d->modifierKeys)
	{
		d->modifierKeys = newModifiers;
		d->controlModifierlessTimout();
		d->updateShortcutDisplay();
	}
}


/**
 * GensKeySequenceButton::focusOutEvent(): Focus out event.
 * @param e QFocusEvent.
 */
void GensKeySequenceButton::focusOutEvent(QFocusEvent *e)
{
	// Event parameter is unused.
	((void)e);
	
	// Key sequence recording should be cancelled if the
	// GensKeySequenceButton loses focus.
	if (d->isRecording)
		d->cancelRecording();
}

}
