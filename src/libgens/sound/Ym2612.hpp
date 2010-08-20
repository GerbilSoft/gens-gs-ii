/***************************************************************************
 * libgens: Gens Emulation Library.                                        *
 * Ym2612.hpp: Yamaha YM2612 FM synthesis chip emulator.                   *
 *                                                                         *
 * Copyright (c) 1999-2002 by Stéphane Dallongeville                       *
 * Copyright (c) 2003-2004 by Stéphane Akhoun                              *
 * Copyright (c) 2008-2010 by David Korth                                  *
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

#ifndef __LIBGENS_SOUND_YM2612_HPP__
#define __LIBGENS_SOUND_YM2612_HPP__

#include <stdint.h>

// Change it if you need to do long update
#define	MAX_UPDATE_LENGTH   2000

// Gens always uses 16 bits sound (in 32 bits buffer) and do the convertion later if needed.
#define OUTPUT_BITS         16

/** Various YM2612 definitions needed for table size. **/
/** TODO: Rework them to be more C++-esque? **/

// SIN_LBITS <= 16
// LFO_HBITS <= 16
// (SIN_LBITS + SIN_HBITS) <= 26
// (ENV_LBITS + ENV_HBITS) <= 28
// (LFO_LBITS + LFO_HBITS) <= 28

#define SIN_HBITS      12	// Sinus phase counter int part
#define SIN_LBITS      (26 - SIN_HBITS)	// Sinus phase counter float part (best setting)

#if (SIN_LBITS > 16)
#define SIN_LBITS      16	// Can't be greater than 16 bits
#endif

#define ENV_HBITS      12	// Env phase counter int part
#define ENV_LBITS      (28 - ENV_HBITS)	// Env phase counter float part (best setting)

#define LFO_HBITS      10	// LFO phase counter int part
#define LFO_LBITS      (28 - LFO_HBITS)	// LFO phase counter float part (best setting)

#define SIN_LENGTH     (1 << SIN_HBITS)
#define ENV_LENGTH     (1 << ENV_HBITS)
#define LFO_LENGTH     (1 << LFO_HBITS)

#define TL_LENGTH      (ENV_LENGTH * 3)	// Env + TL scaling + LFO

#define SIN_MASK       (SIN_LENGTH - 1)
#define ENV_MASK       (ENV_LENGTH - 1)
#define LFO_MASK       (LFO_LENGTH - 1)

#define ENV_STEP       (96.0 / ENV_LENGTH)	// ENV_MAX = 96 dB

#define ENV_ATTACK     ((ENV_LENGTH * 0) << ENV_LBITS)
#define ENV_DECAY      ((ENV_LENGTH * 1) << ENV_LBITS)
#define ENV_END        ((ENV_LENGTH * 2) << ENV_LBITS)

#define MAX_OUT_BITS   (SIN_HBITS + SIN_LBITS + 2)	// Modulation = -4 <--> +4
#define MAX_OUT        ((1 << MAX_OUT_BITS) - 1)

//Just for tests stuff...
//
//#define COEF_MOD       0.5
//#define MAX_OUT        ((int) (((1 << MAX_OUT_BITS) - 1) * COEF_MOD))

#define OUT_BITS       (OUTPUT_BITS - 2)
#define OUT_SHIFT      (MAX_OUT_BITS - OUT_BITS)
#define LIMIT_CH_OUT   ((int) (((1 << OUT_BITS) * 1.5) - 1))

#define PG_CUT_OFF     ((int) (78.0 / ENV_STEP))
#define ENV_CUT_OFF    ((int) (68.0 / ENV_STEP))

#define AR_RATE        399128
#define DR_RATE        5514396

//#define AR_RATE        426136
//#define DR_RATE        (AR_RATE * 12)

#define LFO_FMS_LBITS  9	// FIXED (LFO_FMS_BASE gives somethink as 1)
#define LFO_FMS_BASE   ((int) (0.05946309436 * 0.0338 * (double) (1 << LFO_FMS_LBITS)))

namespace LibGens
{

class Ym2612
{
	public:
		Ym2612();
		Ym2612(int clock, int rate);
		
		int reinit(int clock, int rate);
		void reset(void);
		
		uint8_t read(void) const;
		int write(unsigned int address, uint8_t data);
		void update(int32_t *bufL, int32_t *bufR, int length);
		
		// Properties.
		// TODO: Read-only for now.
		bool enabled(void) const { return m_enabled; }
		bool dacEnabled(void) const { return m_dacEnabled; }
		bool improved(void) const { return m_improved; }
		
		/* GSX savestate functions. */
		int saveState(uint8_t state[0x200]);
		int restoreState(const uint8_t state[0x200]);
		
		/** Gens-specific code. **/
		void updateDacAndTimers(int32_t *bufL, int32_t *bufR, int length);
		void specialUpdate(void);
		int getReg(int regID);
		
		// YM write length.
		inline void addWriteLen(int len) { m_writeLen += len; }
		inline void clearWriteLen(void) { m_writeLen = 0; }
		
		// Reset buffer pointers.
		void resetBufferPtrs(void);
	
	protected:
		enum ADSR
		{
			ATTACK	= 0,
			DECAY	= 1,
			SUSTAIN	= 2,
			RELEASE	= 3,
			
			ADSR_MAX
		};
		
		enum SlotID
		{
			S0	= 0,
			S1	= 2,	// Stupid typo of the YM2612
			S2	= 1,
			S3	= 3,
			
			SlotID_MAX
		};
		
		struct Ym2612_Slot
		{
			unsigned int *DT; // paramètre detune
			int MUL;	// paramètre "multiple de fréquence"
			int TL;		// Total Level = volume lorsque l'enveloppe est au plus haut
			int TLL;	// Total Level ajusted
			int SLL;	// Sustin Level (ajusted) = volume où l'enveloppe termine sa première phase de régression
			int KSR_S;	// Key Scale Rate Shift = facteur de prise en compte du KSL dans la variations de l'enveloppe
			int KSR;	// Key Scale Rate = cette valeur est calculée par rapport à la fréquence actuelle, elle va influer
						// sur les différents paramètres de l'enveloppe comme l'attaque, le decay ...  comme dans la réalité !
			int SEG;	// Type enveloppe SSG
			unsigned int *AR; // Attack Rate (table pointeur) = Taux d'attaque (AR[KSR])
			unsigned int *DR; // Decay Rate (table pointeur) = Taux pour la régression (DR[KSR])
			unsigned int *SR; // Sustin Rate (table pointeur) = Taux pour le maintien (SR[KSR])
			unsigned int *RR; // Release Rate (table pointeur) = Taux pour le relâchement (RR[KSR])
			int Fcnt;	// Frequency Count = compteur-fréquence pour déterminer l'amplitude actuelle (SIN[Finc >> 16])
			int Finc;	// frequency step = pas d'incrémentation du compteur-fréquence
						// plus le pas est grand, plus la fréquence est aïgu (ou haute)
			int Ecurp;	// Envelope current phase = cette variable permet de savoir dans quelle phase
						// de l'enveloppe on se trouve, par exemple phase d'attaque ou phase de maintenue ...
						// en fonction de la valeur de cette variable, on va appeler une fonction permettant
						// de mettre à jour l'enveloppe courante.
			int Ecnt;	// Envelope counter = le compteur-enveloppe permet de savoir où l'on se trouve dans l'enveloppe
			int Einc;	// Envelope step courant
			int Ecmp;	// Envelope counter limite pour la prochaine phase
			int EincA;	// Envelope step for Attack = pas d'incrémentation du compteur durant la phase d'attaque
						// cette valeur est égal à AR[KSR]
			int EincD;	// Envelope step for Decay = pas d'incrémentation du compteur durant la phase de regression
						// cette valeur est égal à DR[KSR]
			int EincS;	// Envelope step for Sustain = pas d'incrémentation du compteur durant la phase de maintenue
						// cette valeur est égal à SR[KSR]
			int EincR;	// Envelope step for Release = pas d'incrémentation du compteur durant la phase de relâchement
						// cette valeur est égal à RR[KSR]
			int *OUTp;	// pointeur of SLOT output = pointeur permettant de connecter la sortie de ce slot à l'entrée
						// d'un autre ou carrement à la sortie de la voie
			int INd;	// input data of the slot = données en entrée du slot
			int ChgEnM;	// Change envelop mask.
			int AMS;	// AMS depth level of this SLOT = degré de modulation de l'amplitude par le LFO
			int AMSon;	// AMS enable flag = drapeau d'activation de l'AMS
		};
		
		struct Ym2612_Channel
		{
			int S0_OUT[4];	// anciennes sorties slot 0 (pour le feed back)
			int Old_OUTd;	// ancienne sortie de la voie (son brut)
			int OUTd;	// sortie de la voie (son brut)
			int LEFT;	// LEFT enable flag
			int RIGHT;	// RIGHT enable flag
			int ALGO;	// Algorythm = détermine les connections entre les opérateurs
			int FB;		// shift count of self feed back = degré de "Feed-Back" du SLOT 1 (il est son unique entrée)
			int FMS;	// Fréquency Modulation Sensitivity of channel = degré de modulation de la fréquence sur la voie par le LFO
			int AMS;	// Amplitude Modulation Sensitivity of channel = degré de modulation de l'amplitude sur la voie par le LFO
			int FNUM[4];	// hauteur fréquence de la voie (+ 3 pour le mode spécial)
			int FOCT[4];	// octave de la voie (+ 3 pour le mode spécial)
			int KC[4];	// Key Code = valeur fonction de la fréquence (voir KSR pour les slots, KSR = KC >> KSR_S)
			Ym2612_Slot _SLOT[4];	// four slot.operators = les 4 slots de la voie
			int FFlag;	// Frequency step recalculation flag
		};
		
		struct Ym2612_Data
		{
			int Clock;		// Horloge YM2612
			int Rate;		// Sample Rate (11025/22050/44100)
			int TimerBase;		// TimerBase calculation
			int status;		// YM2612 Status (timer overflow)
			int OPNAadr;		// addresse pour l'écriture dans l'OPN A (propre à l'émulateur)
			int OPNBadr;		// addresse pour l'écriture dans l'OPN B (propre à l'émulateur)
			int LFOcnt;		// LFO counter = compteur-fréquence pour le LFO
			int LFOinc;		// LFO step counter = pas d'incrémentation du compteur-fréquence du LFO
						// plus le pas est grand, plus la fréquence est grande
						
			int TimerA;		// timerA limit = valeur jusqu'à laquelle le timer A doit compter
			int TimerAL;
			int TimerAcnt;		// timerA counter = valeur courante du Timer A
			int TimerB;		// timerB limit = valeur jusqu'à laquelle le timer B doit compter
			int TimerBL;
			int TimerBcnt;		// timerB counter = valeur courante du Timer B
			int Mode;		// Mode actuel des voie 3 et 6 (normal / spécial)
			int DAC;		// DAC enabled flag
			int DACdata;		// DAC data
			
			int dummy;		// MSVC++ enforces 8-byte alignment on doubles. This forces said alignment on gcc.
			double Frequence;	// Fréquence de base, se calcul par rapport à l'horlage et au sample rate
			
			unsigned int Inter_Cnt;		// Interpolation Counter
			unsigned int Inter_Step;	// Interpolation Step
			Ym2612_Channel CHANNEL[6];	// Les 6 voies du YM2612
			
			int REG[2][0x100];	// Sauvegardes des valeurs de tout les registres, c'est facultatif
						// cela nous rend le débuggage plus facile
		};
		
		Ym2612_Data m_data;
		
		// Static tables.
		static bool ms_Init;
		static int *SIN_TAB[SIN_LENGTH];			// SINUS TABLE (pointer on TL TABLE)
		static int TL_TAB[TL_LENGTH * 2];			// TOTAL LEVEL TABLE (plus and minus)
		static unsigned int ENV_TAB[2 * ENV_LENGTH * 8];	// ENV CURVE TABLE (attack & decay)
		//static unsigned int ATTACK_TO_DECAY[ENV_LENGTH];	// Conversion from attack to decay phase
		static unsigned int DECAY_TO_ATTACK[ENV_LENGTH];	// Conversion from decay to attack phase
		
		// Member tables.
		unsigned int FINC_TAB[2048];		// Frequency step table
		
		// Rate tables.
		// All of these are Member variables, except for NULL_RATE.
		// (NULL_RATE consists of all zeroes.)
		unsigned int AR_TAB[128];		// Attack rate table.
		unsigned int DR_TAB[96];		// Decay rate table.
		unsigned int DT_TAB[8][32];		// Detune table.
		unsigned int SL_TAB[16];		// Sustain level table.
		static unsigned int NULL_RATE[32];	// Table for NULL rate. (STATIC)
		
		// LFO tables. (Static class variables)
		static int LFO_ENV_TAB[LFO_LENGTH];		// LFO AMS TABLE (adjusted for 11.8 dB)
		static int LFO_FREQ_TAB[LFO_LENGTH];		// LFO FMS TABLE
		
		// LFO temporary tables. (Member variables)
		int LFO_ENV_UP[MAX_UPDATE_LENGTH];	// Temporary calculated LFO AMS (adjusted for 11.8 dB)
		int LFO_FREQ_UP[MAX_UPDATE_LENGTH];	// Temporary calculated LFO FMS
		
		// NOTE: INTER_TAB isn't used...
#if 0
		int INTER_TAB[MAX_UPDATE_LENGTH];	// Interpolation table
#endif
		
		// LFO step table. (static class variable)
		static int LFO_INC_TAB[8];		// LFO step table.
		
		// Envelope function declarations.
		static void Env_Attack_Next(Ym2612_Slot *SL);
		static void Env_Decay_Next(Ym2612_Slot *SL);
		static void Env_Substain_Next(Ym2612_Slot *SL);
		static void Env_Release_Next(Ym2612_Slot *SL);
		static void Env_NULL_Next(Ym2612_Slot *SL);
		
		typedef void (*Env_Event)(Ym2612_Slot *SL);
		static const Env_Event ENV_NEXT_EVENT[8];
		
		// Default detune table.
		// FD == F number
		static const uint8_t DT_DEF_TAB[4][32];
		
		static const uint8_t FKEY_TAB[16];
		static const uint8_t LFO_AMS_TAB[4];
		static const unsigned int LFO_FMS_TAB[8];
		
		// Interpolation calculation.
		int int_cnt;
		
		/** Functions for calculating parameters. **/
		static inline void CALC_FINC_SL(Ym2612_Slot *SL, int finc, int kc);
		inline void CALC_FINC_CH(Ym2612_Channel *CH);
		
		/** Functions for setting values. **/
		static inline void KEY_ON(Ym2612_Channel *CH, int nsl);
		static inline void KEY_OFF(Ym2612_Channel *CH, int nsl);
		
		inline void CSM_Key_Control(void);
		
		int SLOT_SET(int address, uint8_t data);
		int CHANNEL_SET(int address, uint8_t data);
		int YM_SET(int address, uint8_t data);
		
		/** Update Channel templates. **/
		template<int algo>
		inline void T_Update_Chan(Ym2612_Channel *CH, int32_t *bufL, int32_t *bufR, int length);
		
		template<int algo>
		inline void T_Update_Chan_LFO(Ym2612_Channel *CH, int32_t *bufL, int32_t *bufR, int length);
		
		template<int algo>
		inline void T_Update_Chan_Int(Ym2612_Channel *CH, int32_t *bufL, int32_t *bufR, int length);
		
		template<int algo>
		inline void T_Update_Chan_LFO_Int(Ym2612_Channel *CH, int32_t *bufL, int32_t *bufR, int length);
		
		void Update_Chan(int algo_type, Ym2612_Channel *CH, int32_t *bufL, int32_t *bufR, int length);
		
		// PSG write length. (for audio output)
		int m_writeLen;
		bool m_enabled;		// YM2612 Enabled
		bool m_dacEnabled;	// DAC Enabled
		bool m_improved;	// YM2612 Improved
		
		// YM buffer pointers.
		// TODO: Figure out how to get rid of these!
		int32_t *m_bufPtrL;
		int32_t *m_bufPtrR;
};

/* Gens */

#if 0
/* GSX v7 savestate functionality. */
struct _gsx_v7_ym2612;
int YM2612_Save_Full(struct _gsx_v7_ym2612 *save);
int YM2612_Restore_Full(struct _gsx_v7_ym2612 *save); 
#endif

/* end */

}

#endif /* __LIBGENS_SOUND_YM2612_HPP__ */
