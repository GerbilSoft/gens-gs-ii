/***************************************************************************
 * libgens: Gens Emulation Library.                                        *
 * IoTeamplayer.hpp: Sega Teamplayer device.                               *
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

#include "IoTeamplayer.hpp"

// C includes.
#include <string.h>

namespace LibGens
{

IoTeamplayer::IoTeamplayer()
{
	// Reset the counter.
	m_counter = DT_INIT;
	
	// Clear controller data.
	memset(&m_ctrlData, 0xFF, sizeof(m_ctrlData));
	
	// Initialize controller types.
	// TODO: User settings.
	m_ctrlType[0] = PT_6BTN;
	m_ctrlType[1] = PT_NONE;
	m_ctrlType[2] = PT_NONE;
	m_ctrlType[3] = PT_NONE;
	
	// Rebuild the controller dat index table.
	rebuildCtrlIndexTable();
}

IoTeamplayer::IoTeamplayer(const IoBase *other)
	: IoBase(other)
{
	// TODO: Copy multitap information.
	
	// Reset the counter.
	m_counter = DT_INIT;
	
	// Clear controller data.
	memset(&m_ctrlData, 0x0F, sizeof(m_ctrlData));
	
	// Initialize controller types.
	// TODO: User settings.
	m_ctrlType[0] = PT_6BTN;
	m_ctrlType[1] = PT_NONE;
	m_ctrlType[2] = PT_NONE;
	m_ctrlType[3] = PT_NONE;
	
	// Rebuild the controller data index table.
	rebuildCtrlIndexTable();
}

/**
 * reset(): Reset function.
 * Called when the system is reset.
 */
void IoTeamplayer::reset()
{
	IoBase::reset();
	
	// Reset the counter.
	m_counter = DT_INIT;
	
	// Clear controller data.
	memset(&m_ctrlData, 0x0F, sizeof(m_ctrlData));
	
	// Initialize controller types.
	// TODO: User settings.
	m_ctrlType[0] = PT_6BTN;
	m_ctrlType[1] = PT_NONE;
	m_ctrlType[2] = PT_NONE;
	m_ctrlType[3] = PT_NONE;
	
	// Rebuild the controller data index table.
	rebuildCtrlIndexTable();
}


/**
 * writeCtrl(): Set the I/O tristate value.
 * TODO: Combine with writeData().
 * @param ctrl I/O tristate value.
 */
void IoTeamplayer::writeCtrl(uint8_t ctrl)
{
	// Check if either TH or TR has changed.
	uint8_t lastDataTris = applyTristate(m_lastData);
	m_ctrl = ctrl;
	updateSelectLine();
	uint8_t dataTris = applyTristate(m_lastData);
	
	if ((lastDataTris ^ dataTris) & (IOPIN_TH | IOPIN_TR))
	{
		// Check if TH is high.
		if (isSelect())
		{
			// TH high. Reset the counter.
			m_counter = DT_INIT;
		}
		else
		{
			// Increment the counter.
			m_counter++;
		}
	}
}


/**
 * writeData(): Write data to the controller.
 * * TODO: Combine with writeCtrl().
 * @param data Data to the controller.
 */
void IoTeamplayer::writeData(uint8_t data)
{
	// Check if either TH or TR has changed.
	uint8_t lastDataTris = applyTristate(m_lastData);
	m_lastData = data;
	updateSelectLine();
	uint8_t dataTris = applyTristate(data);
	
	if ((lastDataTris ^ dataTris) & (IOPIN_TH | IOPIN_TR))
	{
		// Check if TH is high.
		if (isSelect())
		{
			// TH high. Reset the counter.
			m_counter = DT_INIT;
		}
		else
		{
			// Increment the counter.
			m_counter++;
		}
	}
}


/**
 * readData(): Read data from the controller.
 * @return Data from the controller.
 */
uint8_t IoTeamplayer::readData(void)
{
	uint8_t ret;
	
	// Check the controller data index table.
	if (m_counter >= DT_MAX)
		return applyTristate(0xFF);	// TODO
	
	switch (m_counter)
	{
		case DT_INIT:
			// Initial state.
			ret = 0x73;
			break;
		
		case DT_START:
			// Start request.
			ret = 0x3F;
			break;
		
		case DT_ACK1:
		case DT_ACK2:
			// Acknowledgement request.
			// TH=0, TR=0/1 -> RLDU = 0000
			ret = 0x00;
			break;
		
		case DT_PADTYPE_A:
		case DT_PADTYPE_B:
		case DT_PADTYPE_C:
		case DT_PADTYPE_D:
			// Controller type.
			ret = m_ctrlType[m_counter - DT_PADTYPE_A];
			break;
		
		default:
			// Check the controller data index table.
			int adj_counter = (m_counter - DT_PADA_RLDU);
			if ((adj_counter > (sizeof(m_ctrlIndex)/sizeof(m_ctrlIndex[0]))) ||
			    (m_ctrlIndex[adj_counter] >= DT_MAX))
			{
				// Invalid counter state.
				// TODO: What value should be returned?
				ret = 0xFF;
				break;
			}
			
			// Controller data.
			ret = (m_ctrlData[m_ctrlIndex[adj_counter] - DT_PADA_RLDU] & 0xF);
			break;
	}
	
	// TL should match TR.
	// (from Genesis Plus GX)
	if (m_lastData & IOPIN_TR)
		ret |= IOPIN_TL;
	else
		ret &= ~IOPIN_TL;
	
	return applyTristate(ret);
}


/**
 * update(): I/O device update function.
 */
void IoTeamplayer::update(void)
{
	// TODO: Allow customizable keymaps.
	// TODO: Support more than just Controller A.
	
	// Controller A: D-pad
	m_ctrlData[0] = 0;
	m_ctrlData[0] |= DevManager::IsKeyPressed(KEYV_RIGHT);	// Right
	m_ctrlData[0] <<= 1;
	m_ctrlData[0] |= DevManager::IsKeyPressed(KEYV_LEFT);	// Left
	m_ctrlData[0] <<= 1;
	m_ctrlData[0] |= DevManager::IsKeyPressed(KEYV_DOWN);	// Down
	m_ctrlData[0] <<= 1;
	m_ctrlData[0] |= DevManager::IsKeyPressed(KEYV_UP);	// Up
	m_ctrlData[0] = ~m_ctrlData[0];
	
	// Controller A: Start, A, C, B
	m_ctrlData[1] = 0;
	m_ctrlData[1] |= DevManager::IsKeyPressed(KEYV_RETURN);	// Start
	m_ctrlData[1] <<= 1;
	m_ctrlData[1] |= DevManager::IsKeyPressed(KEYV_a);	// A
	m_ctrlData[1] <<= 1;
	m_ctrlData[1] |= DevManager::IsKeyPressed(KEYV_d);	// C
	m_ctrlData[1] <<= 1;
	m_ctrlData[1] |= DevManager::IsKeyPressed(KEYV_s);	// B
	m_ctrlData[1] = ~m_ctrlData[1];
	
	// Controller A: Mode, X, Y, Z
	m_ctrlData[2] = 0;
	m_ctrlData[2] |= DevManager::IsKeyPressed(KEYV_RSHIFT);	// Mode
	m_ctrlData[2] <<= 1;
	m_ctrlData[2] |= DevManager::IsKeyPressed(KEYV_q);	// X
	m_ctrlData[2] <<= 1;
	m_ctrlData[2] |= DevManager::IsKeyPressed(KEYV_w);	// Y
	m_ctrlData[2] <<= 1;
	m_ctrlData[2] |= DevManager::IsKeyPressed(KEYV_e);	// Z
	m_ctrlData[2] = ~m_ctrlData[2];
}


/**
 * rebuildCtrlIndexTable(): Rebuild the controller data index table.
 */
void IoTeamplayer::rebuildCtrlIndexTable()
{
	unsigned int i = 0;
	for (unsigned int pad = 0; pad < 4; pad++)
	{
		int dtBase = (DT_PADA_RLDU + (pad * 3));
		
		switch (m_ctrlType[pad])
		{
			case PT_NONE:
			default:
				break;
			
			case PT_3BTN:
				m_ctrlIndex[i++] = (DataType)(dtBase + 0);
				m_ctrlIndex[i++] = (DataType)(dtBase + 1);
				break;
			
			case PT_6BTN:
				m_ctrlIndex[i++] = (DataType)(dtBase + 0);
				m_ctrlIndex[i++] = (DataType)(dtBase + 1);
				m_ctrlIndex[i++] = (DataType)(dtBase + 2);
				break;
		}
	}
	
	// Set the rest of the controller data indexes to DT_MAX.
	for (unsigned int x = i; x < (sizeof(m_ctrlIndex)/sizeof(m_ctrlIndex[0])); x++)
		m_ctrlIndex[x] = DT_MAX;
}


/** Controller Configuration. **/


/**
 * nextLogicalButton(): Get the next logical button.
 * @return Next logical button, or -1 if we're at the end.
 */
int IoTeamplayer::nextLogicalButton(int button) const
{
	// TODO: Update for Teamplayer!
	switch (button)
	{
		case BTNI_UP:		return BTNI_DOWN;
		case BTNI_DOWN:		return BTNI_LEFT;
		case BTNI_LEFT:		return BTNI_RIGHT;
		case BTNI_RIGHT:	return BTNI_START;
		case BTNI_START:	return BTNI_A;
		case BTNI_A:		return BTNI_B;
		case BTNI_B:		return BTNI_C;
		case BTNI_C:		return BTNI_MODE;
		case BTNI_MODE:		return BTNI_X;
		case BTNI_X:		return BTNI_Y;
		case BTNI_Y:		return BTNI_Z;
		case BTNI_Z:
		default:
			return -1;
	}
}


/**
 * buttonName(): Get the name for a given button index.
 * @param button Button index.
 * @return Button name, or NULL if the button index is invalid.
 */
const char *IoTeamplayer::buttonName(int button) const
{
	// TODO: Update for Teamplayer!
	static const char *btnNames[] =
	{
		"Up", "Down", "Left", "Right",
		"B", "C", "A", "Start",
		"Z", "Y", "X", "Mode"
	};
	
	if (button >= BTNI_UP && button <= BTNI_MODE)
		return btnNames[button];
	return NULL;
}

}
