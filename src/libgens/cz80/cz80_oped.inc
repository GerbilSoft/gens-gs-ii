/********************************************************************************/
/*                                                                              */
/* CZ80 ED opcode include source file                                           */
/* C Z80 emulator version 0.91                                                  */
/* Copyright 2004-2005 Stephane Dallongeville                                   */
/*                                                                              */
/********************************************************************************/

#if CZ80_USE_JUMPTABLE
    goto *JumpTableED[Opcode];
#else
switch (Opcode)
{
#endif

    // ILLEGAL
    
    OPED(0x00):
    OPED(0x01):
    OPED(0x02):
    OPED(0x03):
    OPED(0x04):
    OPED(0x05):
    OPED(0x06):
    OPED(0x07):
    OPED(0x08):
    OPED(0x09):
    OPED(0x0a):
    OPED(0x0b):
    OPED(0x0c):
    OPED(0x0d):
    OPED(0x0e):
    OPED(0x0f):
    OPED(0x10):
    OPED(0x11):
    OPED(0x12):
    OPED(0x13):
    OPED(0x14):
    OPED(0x15):
    OPED(0x16):
    OPED(0x17):
    OPED(0x18):
    OPED(0x19):
    OPED(0x1a):
    OPED(0x1b):
    OPED(0x1c):
    OPED(0x1d):
    OPED(0x1e):
    OPED(0x1f):
    OPED(0x20):
    OPED(0x21):
    OPED(0x22):
    OPED(0x23):
    OPED(0x24):
    OPED(0x25):
    OPED(0x26):
    OPED(0x27):
    OPED(0x28):
    OPED(0x29):
    OPED(0x2a):
    OPED(0x2b):
    OPED(0x2c):
    OPED(0x2d):
    OPED(0x2e):
    OPED(0x2f):
    OPED(0x30):
    OPED(0x31):
    OPED(0x32):
    OPED(0x33):
    OPED(0x34):
    OPED(0x35):
    OPED(0x36):
    OPED(0x37):
    OPED(0x38):
    OPED(0x39):
    OPED(0x3a):
    OPED(0x3b):
    OPED(0x3c):
    OPED(0x3d):
    OPED(0x3e):
    OPED(0x3f):
    OPED(0xbc):
    OPED(0xbd):
    OPED(0xbe):
    OPED(0xbf):
    OPED(0xc0):
    OPED(0xc1):
    OPED(0xc2):
    OPED(0xc3):
    OPED(0xc4):
    OPED(0xc5):
    OPED(0xc6):
    OPED(0xc7):
    OPED(0xc8):
    OPED(0xc9):
    OPED(0xca):
    OPED(0xcb):
    OPED(0xcc):
    OPED(0xcd):
    OPED(0xce):
    OPED(0xcf):
    OPED(0xd0):
    OPED(0xd1):
    OPED(0xd2):
    OPED(0xd3):
    OPED(0xd4):
    OPED(0xd5):
    OPED(0xd6):
    OPED(0xd7):
    OPED(0xd8):
    OPED(0xd9):
    OPED(0xda):
    OPED(0xdb):
    OPED(0xdc):
    OPED(0xdd):
    OPED(0xde):
    OPED(0xdf):
    OPED(0xe0):
    OPED(0xe1):
    OPED(0xe2):
    OPED(0xe3):
    OPED(0xe4):
    OPED(0xe5):
    OPED(0xe6):
    OPED(0xe7):
    OPED(0xe8):
    OPED(0xe9):
    OPED(0xea):
    OPED(0xeb):
    OPED(0xec):
    OPED(0xed):
    OPED(0xee):
    OPED(0xef):
    OPED(0xf0):
    OPED(0xf1):
    OPED(0xf2):
    OPED(0xf3):
    OPED(0xf4):
    OPED(0xf5):
    OPED(0xf6):
    OPED(0xf7):
    OPED(0xf8):
    OPED(0xf9):
    OPED(0xfa):
    OPED(0xfb):
    OPED(0xfc):
    OPED(0xfd):
    OPED(0xfe):
    OPED(0xff):
    OPED(0x77):
    OPED(0x7f):
    OPED(0x80):
    OPED(0x81):
    OPED(0x82):
    OPED(0x83):
    OPED(0x84):
    OPED(0x85):
    OPED(0x86):
    OPED(0x87):
    OPED(0x88):
    OPED(0x89):
    OPED(0x8a):
    OPED(0x8b):
    OPED(0x8c):
    OPED(0x8d):
    OPED(0x8e):
    OPED(0x8f):
    OPED(0x90):
    OPED(0x91):
    OPED(0x92):
    OPED(0x93):
    OPED(0x94):
    OPED(0x95):
    OPED(0x96):
    OPED(0x97):
    OPED(0x98):
    OPED(0x99):
    OPED(0x9a):
    OPED(0x9b):
    OPED(0x9c):
    OPED(0x9d):
    OPED(0x9e):
    OPED(0x9f):
    OPED(0xa4):
    OPED(0xa5):
    OPED(0xa6):
    OPED(0xa7):
    OPED(0xac):
    OPED(0xad):
    OPED(0xae):
    OPED(0xaf):
    OPED(0xb4):
    OPED(0xb5):
    OPED(0xb6):
    OPED(0xb7):
        RET(4);

#define OPED_LD_mNN_RR(Opcode, RR) OPJ_LD_mNN_RR(OPED, Opcode, RR)

    OPED_LD_mNN_RR(0x43, BC) // LD   (nn),BC
    OPED_LD_mNN_RR(0x53, DE) // LD   (nn),DE
    OPED_LD_mNN_RR(0x63, HL) // LD   (nn),HL
    OPED_LD_mNN_RR(0x73, SP) // LD   (nn),SP

#define OPED_LD_RR_mNN(Opcode, RR) OPJ_LD_RR_mNN(OPED, Opcode, RR)

    OPED_LD_RR_mNN(0x4b, BC) // LD   BC,(nn)
    OPED_LD_RR_mNN(0x5b, DE) // LD   DE,(nn)
    OPED_LD_RR_mNN(0x6b, HL) // LD   HL,(nn)
    OPED_LD_RR_mNN(0x7b, SP) // LD   SP,(nn)


    OPED(0x47): // LD   I,A
        zI = zA;
        RET(5);

    OPED(0x4f): // LD   R,A
        zR = zA - ((CPU->CycleToDo - CCnt) / 4);
        zR2 = zA & 0x80;
        RET(5);

    OPED(0x57): // LD   A,I
    {
        uint8_t F;

        zA = zI;
        F = zF & CZ80_CF;
        F |= zA & (CZ80_SF | CZ80_YF | CZ80_XF);
        F |= zIFF2;
        if (!zA) F |= CZ80_ZF;
        zF = F;
        RET(5);
    }

    OPED(0x5f): // LD   A,R
    {
        uint8_t F;

        zA = zR2 + ((zR + ((CPU->CycleToDo - CCnt) / 4)) & 0x7F);
        F = zF & CZ80_CF;
        F |= zA & (CZ80_SF | CZ80_YF | CZ80_XF);
        F |= zIFF2;
        if (!zA) F |= CZ80_ZF;
        zF = F;
        RET(5);
    }

    OPED(0x5c): // NEG
    OPED(0x54): // NEG
    OPED(0x4c): // NEG
    OPED(0x44): // NEG
    OPED(0x64): // NEG
    OPED(0x6c): // NEG
    OPED(0x74): // NEG
    OPED(0x7c): // NEG
    {
        uint16_t val;
        uint16_t res;

        val = zA;
        res = 0 - val;
        zF = SZXY[res & 0xFF] |                     // S/Z/X/Y flag
            ((res ^ val) & CZ80_HF) |               // H flag
            (((val & res) & 0x80) >> 5) |           // V flag
            ((res >> 8) & CZ80_CF) | CZ80_NF;       // C/N flag
        zA = res;
        RET(4);
    }


    OPED(0x67): // RRD  (HL)
    {
        uint16_t adr;
        uint8_t src;

        PRE_IO();
        adr = zHL;
        zWZ = adr + 1;
        READ_BYTE(adr, src);
        WRITE_BYTE(adr, (src >> 4) | (zA << 4));
        zA = (zA & 0xF0) | (src & 0x0F);
        zF = SZXYP[zA] | (zF & CZ80_CF);
        POST_IO();
        RET(14);
    }

    OPED(0x6f): // RLD  (HL)
    {
        uint16_t adr;
        uint8_t src;

        PRE_IO();
        adr = zHL;
        zWZ = adr + 1;
        READ_BYTE(adr, src);
        WRITE_BYTE(adr, (src << 4) | (zA & 0x0F));
        zA = (zA & 0xF0) | (src >> 4);
        zF = SZXYP[zA] | (zF & CZ80_CF);
        POST_IO();
        RET(14);
    }

#define OPJ_ADC_HL_RR(JmpType, Opcode, RR) \
    JmpType(Opcode): { \
        uint16_t src = z##RR; \
        uint32_t res = zHL + src + (zF & CZ80_CF); \
        zWZ = zHL + 1; \
        zF = (((src ^ zHL ^ res) >> 8) & CZ80_HF) |                     /* H flag */ \
            (((src ^ zHL ^ 0x8000) & (src ^ res) & 0x8000) >> 13) |     /* V flag */ \
            ((res >> 8) & (CZ80_SF | CZ80_XF | CZ80_YF)) |              /* S/X/Y flag */ \
            ((res >> 16) & CZ80_CF) |                                   /* C flag */ \
            ((res & 0xFFFF) ? 0 : CZ80_ZF);                             /* Z flag */ \
        zHL = res; \
        RET(11); \
    }

#define OPED_ADC_HL_RR(Opcode, RR) OPJ_ADC_HL_RR(OPED, Opcode, RR)

    OPED_ADC_HL_RR(0x7a, SP) // ADC  HL,SP
    OPED_ADC_HL_RR(0x4a, BC) // ADC  HL,BC
    OPED_ADC_HL_RR(0x5a, DE) // ADC  HL,DE
    OPED_ADC_HL_RR(0x6a, HL) // ADC  HL,HL
        
#if CZ80_DEBUG
#define OPJ_SBC_HL_RR(JmpType, Opcode, RR) \
    JmpType(Opcode): { \
        uint16_t src = z##RR; \
        uint32_t res = zHL - (src + (zF & CZ80_CF)); \
        zWZ = data->W + 1; \
        zF = (((src ^ zHL ^ res) >> 8) & CZ80_HF) | CZ80_NF |           /* H/N flag */ \
            (((src ^ zHL) & (zHL ^ res) & 0x8000) >> 13) |              /* V flag */ \
            ((res >> 8) & CZ80_SF) |                                    /* S flag */ \
            ((res >> 16) & CZ80_CF) |                                   /* C flag */ \
            ((res & 0xFFFF) ? 0 : CZ80_ZF);                             /* Z flag */ \
        data->W = (uint16_t)res; \
        RET(11); \
    }
#else
#define OPJ_SBC_HL_RR(JmpType, Opcode, RR) \
    JmpType(Opcode): { \
        uint16_t src = z##RR; \
        uint32_t res = zHL - (src + (zF & CZ80_CF)); \
        zWZ = data->W + 1; \
        zF = (((src ^ zHL ^ res) >> 8) & CZ80_HF) | CZ80_NF |           /* H/N flag */ \
            (((src ^ zHL) & (zHL ^ res) & 0x8000) >> 13) |              /* V flag */ \
            ((res >> 8) & (CZ80_SF | CZ80_XF | CZ80_YF)) |              /* S/X/Y flag */ \
            ((res >> 16) & CZ80_CF) |                                   /* C flag */ \
            ((res & 0xFFFF) ? 0 : CZ80_ZF);                             /* Z flag */ \
        data->W = (uint16_t)res; \
        RET(11); \
    }
#endif

#define OPED_SBC_HL_RR(Opcode, RR) OPJ_SBC_HL_RR(OPED, Opcode, RR)

    OPED_SBC_HL_RR(0x72, SP) // SBC  HL,SP
    OPED_SBC_HL_RR(0x42, BC) // SBC  HL,BC
    OPED_SBC_HL_RR(0x52, DE) // SBC  HL,DE
    OPED_SBC_HL_RR(0x62, HL) // SBC  HL,HL

#define OPJ_IN_R_mBC(JmpType, Opcode) \
    JmpType(Opcode): { \
        uint8_t res; \
        IN(zBC, res); \
        zWZ = zBC + 1; \
        zR8((Opcode >> 3) & 7) = res; \
        zF = (zF & CZ80_CF) | SZXYP[res]; \
        RET(8); \
    }

#define OPED_IN_R_mBC(Opcode) OPJ_IN_R_mBC(OPED, Opcode)

    OPED_IN_R_mBC(0x40) // IN   B,(C)
    OPED_IN_R_mBC(0x48) // IN   C,(C)
    OPED_IN_R_mBC(0x50) // IN   D,(C)
    OPED_IN_R_mBC(0x58) // IN   E,(C)
    OPED_IN_R_mBC(0x60) // IN   H,(C)
    OPED_IN_R_mBC(0x68) // IN   L,(C)
    OPED_IN_R_mBC(0x78) // IN   A,(C)

    OPED(0x70): { // IN   0,(C)
        uint8_t res;
        IN(zBC, res);
        zWZ = zBC + 1;
        zF = (zF & CZ80_CF) | SZXYP[res];
        RET(8);
    }

#define OPJ_OUT_mBC_R(JmpType, Opcode, RRsrc) \
    JmpType(Opcode): { \
        uint8_t src = RRsrc; \
        OUT(zBC, src); \
        zWZ = zBC + 1; \
        RET(8); \
    }

#define OPED_OUT_mBC_R(Opcode, RRsrc) OPJ_OUT_mBC_R(OPED, Opcode, RRsrc)

    OPED_OUT_mBC_R(0x71,  0) // OUT  (C),0
    OPED_OUT_mBC_R(0x41, zB) // OUT  (C),B
    OPED_OUT_mBC_R(0x49, zC) // OUT  (C),C
    OPED_OUT_mBC_R(0x51, zD) // OUT  (C),D
    OPED_OUT_mBC_R(0x59, zE) // OUT  (C),E
    OPED_OUT_mBC_R(0x61, zH) // OUT  (C),H
    OPED_OUT_mBC_R(0x69, zL) // OUT  (C),L
    OPED_OUT_mBC_R(0x79, zA) // OUT  (C),A

    OPED(0x4d): // RETI
    OPED(0x5d): // RETI
    OPED(0x6d): // RETI
    OPED(0x7d): // RETI
        if (CPU->RetI) CPU->RetI(CPU->ctx);

    OPED(0x45): // RETN;
    OPED(0x55): // RETN;
    OPED(0x65): // RETN;
    OPED(0x75): // RETN;
        PRE_IO();
        POP_16(zWZ);
        SET_PC(zWZ);
        POST_IO();
        zIFF1 = zIFF2;
        CCnt -= 10;
        // we need to test for interrupt
        goto Cz80_Exec_End;

    OPED(0x46): // IM   0
    OPED(0x4e): // IM   0
    OPED(0x66): // IM   0
    OPED(0x6e): // IM   0
        zIM = 0;
        RET(4);

    OPED(0x76): // IM   1
    OPED(0x56): // IM   1
        zIM = 1;
        RET(4);

    OPED(0x5e): // IM   2
    OPED(0x7e): // IM   2
        zIM = 2;
        RET(4);


    {
        uint8_t val;
        uint8_t F;

    OPED(0xa8): // LDD
        PRE_IO();
        READ_BYTE(zHL--, val);
        WRITE_BYTE(zDE--, val);
        goto OP_LDX;

    OPED(0xa0): // LDI
        PRE_IO();
        READ_BYTE(zHL++, val);
        WRITE_BYTE(zDE++, val);

OP_LDX:
#if CZ80_EXACT
        val += zA;
        F = (zF & (CZ80_SF | CZ80_ZF | CZ80_CF)) |
            (val & CZ80_XF) | ((val << 4) & CZ80_YF);
#else
        F = zF & (CZ80_SF | CZ80_ZF | CZ80_YF | CZ80_XF | CZ80_CF);
#endif
        if (--zBC) F |= CZ80_PF;
        zF = F;
        POST_IO();
        RET(12);
    }

    {
        uint8_t val;
        uint8_t F;

    OPED(0xb8): // LDDR
        do
        {
            PRE_IO();
            READ_BYTE(zHL--, val);
            WRITE_BYTE(zDE--, val);
            POST_IO();
            zBC--;
            if (zBC) {
                // WZ is not changed if BC == 1
                // *before* the decrement.
                zWZ = zPC + 1;
            }
            CCnt -= 21;
        } while ((zBC) && (CCnt > -4));
        goto OP_LDXR;

    OPED(0xb0): // LDIR
        do
        {
            PRE_IO();
            READ_BYTE(zHL++, val);
            WRITE_BYTE(zDE++, val);
            POST_IO();
            zBC--;
            if (zBC) {
                // WZ is not changed if BC == 1
                // *before* the decrement.
                zWZ = zPC + 1;
            }
            CCnt -= 21;
        } while ((zBC) && (CCnt > -4));

OP_LDXR:
#if CZ80_EXACT
        val += zA;
        F = (zF & (CZ80_SF | CZ80_ZF | CZ80_CF)) |
            (val & CZ80_XF) | ((val << 4) & CZ80_YF);
#else
        F = zF & (CZ80_SF | CZ80_ZF | CZ80_YF | CZ80_XF | CZ80_CF);
#endif

        if (zBC)
        {
            // instruction not yet completed...
            // we will continu it at next CZ80_Exec
            zF = F | CZ80_PF;
            zPC -= 2;
            CCnt += 4;
            goto Cz80_Exec_End;
        }

        // instruction completed...
        zF = F;
        RET(-(5 + 4));
    }


    {
        uint8_t val;
        uint8_t res;
        uint8_t F;

    OPED(0xa9): // CPD
        PRE_IO();
        zWZ--;
        READ_BYTE(zHL--, val);
        goto OP_CPX;

    OPED(0xa1): // CPI
        PRE_IO();
        zWZ++;
        READ_BYTE(zHL++, val);

OP_CPX:
        res = zA - val;
#if CZ80_EXACT
        F = (zF & CZ80_CF) | (SZXY[res] & ~(CZ80_YF | CZ80_XF)) |
            ((zA ^ val ^ res) & CZ80_HF) | CZ80_NF;
        if (F & CZ80_HF) res--;
        F |= (res & CZ80_XF) | ((res >> 4) & CZ80_YF);
#else
        F = (zF & CZ80_CF) | SZXY[res] |
            ((zA ^ val ^ res) & CZ80_HF) | CZ80_NF;
#endif
        if (--zBC) F |= CZ80_PF;
        zF = F;
        POST_IO();
        RET(12);
    }

    {
        uint8_t val;
        uint8_t res;
        uint8_t F;

    OPED(0xb9): // CPDR
        do
        {
            PRE_IO();
            READ_BYTE(zHL--, val);
            res = zA - val;
            POST_IO();
            zBC--;
            if (zBC) {
                // WZ is not changed if BC == 1
                // *before* the decrement.
                zWZ = zPC + 1;
            }
            CCnt -= 21;
        } while ((zBC) && (res) && (CCnt > -4));
        goto OP_CPXR;

    OPED(0xb1): // CPIR
        do
        {
            PRE_IO();
            READ_BYTE(zHL++, val);
            res = zA - val;
            POST_IO();
            zBC--;
            if (zBC) {
                // WZ is not changed if BC == 1
                // *before* the decrement.
                zWZ = zPC + 1;
            }
            CCnt -= 21;
        } while ((zBC) && (res) && (CCnt > -4));

OP_CPXR:
#if CZ80_EXACT
        F = (zF & CZ80_CF) | (SZXY[res] & ~(CZ80_YF | CZ80_XF)) |
            ((zA ^ val ^ res) & CZ80_HF) | CZ80_NF;
        if (F & CZ80_HF) res--;
        F |= (res & CZ80_XF) | ((res >> 4) & CZ80_YF);
#else
        F = (zF & CZ80_CF) | SZXY[res] |
            ((zA ^ val ^ res) & CZ80_HF) | CZ80_NF;
#endif

        if (zBC)
        {
            // instruction not yet completed...
            // we will continu it at next CZ80_Exec
            zF = F | CZ80_PF;
            zPC -= 2;
            CCnt += 4;
            goto Cz80_Exec_End;
        }

        // instruction completed...
        zF = F;
        RET(-(3 + 4));
    }


    {
        uint8_t val;
#if CZ80_EXACT
        uint8_t F;
#endif

    OPED(0xaa): // IND
        PRE_IO();
        zWZ = zBC - 1;
        IN(zBC, val);
        WRITE_BYTE(zHL--, val);
#if CZ80_EXACT
        if ((((zC - 1) & 0xFF) + val) & 0x100)
        {
            F = CZ80_HF | CZ80_CF;
            goto OP_INX;
        }
        F = 0;
#endif
        goto OP_INX;

    OPED(0xa2): // INI
        PRE_IO();
        zWZ = zBC + 1;
        IN(zBC, val);
        WRITE_BYTE(zHL++, val);
#if CZ80_EXACT
        if ((((zC + 1) & 0xFF) + val) & 0x100)
        {
            F = CZ80_HF | CZ80_CF;
            goto OP_INX;
        }
        F = 0;
#endif

OP_INX:
#if CZ80_EXACT
        // P FLAG isn't correct here !
        zF = F | (SZXY[--zB] + ((val >> 6) & CZ80_NF) + (val & CZ80_PF));
#else
        zF = SZXY[--zB] + ((val >> 6) & CZ80_NF);
#endif
        POST_IO();
        RET(12);
    }

    {
        uint8_t val;
#if CZ80_EXACT
        uint8_t F;
#endif

    OPED(0xba): // INDR
        do
        {
            PRE_IO();
            IN(zBC, val);
            WRITE_BYTE(zHL--, val);
            POST_IO();
            zB--;
            CCnt -= 21;
        } while ((zB) && (CCnt > -4));
#if CZ80_EXACT
        if ((((zC - 1) & 0xFF) + val) & 0x100)
        {
            F = CZ80_HF | CZ80_CF;
            goto OP_INXR;
        }
        F = 0;
#endif
        goto OP_INXR;

    OPED(0xb2): // INIR
        do
        {
            PRE_IO();
            IN(zBC, val);
            WRITE_BYTE(zHL++, val);
            POST_IO();
            zB--;
            CCnt -= 21;
        } while ((zB) && (CCnt > -4));
#if CZ80_EXACT
        if ((((zC + 1) & 0xFF) + val) & 0x100)
        {
            F = CZ80_HF | CZ80_CF;
            goto OP_INXR;
        }
        F = 0;
#endif

OP_INXR:
#if CZ80_EXACT
        // P FLAG isn't correct here !
        zF = F | (SZXY[zB] + ((val >> 6) & CZ80_NF) + (val & CZ80_PF));
#else
        zF = SZXY[zB] + ((val >> 6) & CZ80_NF);
#endif

        if (zB)
        {
            // instruction not yet completed...
            // we will continu it at next CZ80_Exec
            zPC -= 2;
            CCnt += 4;
            goto Cz80_Exec_End;
        }

        // instruction completed...
        RET(-(5 + 4));
    }


    {
        uint8_t val;
#if CZ80_EXACT
        uint8_t F;
#endif

    OPED(0xab): // OUTD
        PRE_IO();
        zWZ = zBC - 1;
        READ_BYTE(zHL--, val);
        OUT(zBC, val);
        goto OP_OUTX;

    OPED(0xa3): // OUTI
        PRE_IO();
        zWZ = zBC + 1;
        READ_BYTE(zHL++, val);
        OUT(zBC, val);

OP_OUTX:
#if CZ80_EXACT
        // P FLAG isn't correct here !
        F = SZXY[--zB] + ((val >> 6) & CZ80_NF) + (val & CZ80_PF);
        if ((val + zL) & 0x100) F |= CZ80_HF | CZ80_CF;
        zF = F;
#else
        zF = SZXY[--zB] + ((val >> 6) & CZ80_NF);
#endif
        POST_IO();
        RET(12);
    }


    {
        uint8_t val;
#if CZ80_EXACT
        uint8_t F;
#endif

    OPED(0xbb): // OUTDR
        do
        {
            PRE_IO();
            READ_BYTE(zHL--, val);
            OUT(zBC, val);
            POST_IO();
            zB--;
            CCnt -= 21;
        } while ((zB) && (CCnt > -4));
        goto OP_OUTXR;

    OPED(0xb3): // OUTIR
        do
        {
            PRE_IO();
            READ_BYTE(zHL++, val);
            OUT(zBC, val);
            POST_IO();
            zB--;
            CCnt -= 21;
        } while ((zB) && (CCnt > -4));

OP_OUTXR:
#if CZ80_EXACT
        // P FLAG isn't correct here !
        F = SZXY[zB] + ((val >> 6) & CZ80_NF) + (val & CZ80_PF);
        if ((val + zL) & 0x100) F |= CZ80_HF | CZ80_CF;
        zF = F;
#else
        zF = SZXY[zB] + ((val >> 6) & CZ80_NF);
#endif

        if (zB)
        {
            // instruction not yet completed...
            // we will continu it at next CZ80_Exec
            zPC -= 2;
            CCnt += 4;
            goto Cz80_Exec;
        }

        // instruction not yet completed...
        RET(-(5 + 4));
    }

#if CZ80_USE_JUMPTABLE
#else
}
#endif
