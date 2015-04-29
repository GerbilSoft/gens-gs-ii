/********************************************************************************/
/*                                                                              */
/* CZ80 CB opcode include source file                                           */
/* C Z80 emulator version 0.91                                                  */
/* Copyright 2004-2005 Stephane Dallongeville                                   */
/*                                                                              */
/********************************************************************************/

// NOTE: The main jump table does NOT subtract 4 cycles from the counter
// before executing CB-prefixed instructions. Consequently, all instructions
// here will have a +4 to compensate for the CB opcode.

#if CZ80_USE_JUMPTABLE
    DO_JMPTBL_OPCB(Opcode);
#else
switch (Opcode)
{
#endif

    // ROTATE

    // NOTE: These opcodes have slightly different flag semantics
    // compared to the no-prefix rotate instructions, so they
    // can't be combined.

#define OPJ_RLC_R(JmpType, Opcode) \
    JmpType(Opcode): { \
        uint8_t src = zR8(Opcode & 7); \
        zR8(Opcode & 7) = (src << 1) | (src >> 7); \
        zF = SZXYP[zR8(Opcode & 7)] | (src >> 7); \
        RET(4 + 4); \
    }
#define OPCB_RLC_R(Opcode) OPJ_RLC_R(OPCB, Opcode)

    OPCB_RLC_R(0x00) // RLC  B
    OPCB_RLC_R(0x01) // RLC  C
    OPCB_RLC_R(0x02) // RLC  D
    OPCB_RLC_R(0x03) // RLC  E
    OPCB_RLC_R(0x04) // RLC  H
    OPCB_RLC_R(0x05) // RLC  L
    OPCB_RLC_R(0x07) // RLC  A

    OPCB(0x06): // RLC  (HL)
    {
        uint16_t adr;
        uint8_t src;
        uint8_t res;

        PRE_IO();
        adr = zHL;
        READ_BYTE(adr, src);
        res = ((src << 1) | (src >> 7)) & 0xFF;
        zF = SZXYP[res] | (src >> 7);
        WRITE_BYTE(adr, res);
        POST_IO();
        RET(11 + 4);
    }

#define OPJ_RRC_R(JmpType, Opcode) \
    JmpType(Opcode): { \
        uint8_t src = zR8(Opcode & 7); \
        zR8(Opcode & 7) = (src >> 1) | (src << 7); \
        zF = SZXYP[zR8(Opcode & 7)] | (src & CZ80_CF); \
        RET(4 + 4); \
    }
#define OPCB_RRC_R(Opcode) OPJ_RRC_R(OPCB, Opcode)

    OPCB_RRC_R(0x08) // RRC  B
    OPCB_RRC_R(0x09) // RRC  C
    OPCB_RRC_R(0x0a) // RRC  D
    OPCB_RRC_R(0x0b) // RRC  E
    OPCB_RRC_R(0x0c) // RRC  H
    OPCB_RRC_R(0x0d) // RRC  L
    OPCB_RRC_R(0x0f) // RRC  A

    OPCB(0x0e): // RRC  (HL)
    {
        uint16_t adr;
        uint8_t src;
        uint8_t res;

        PRE_IO();
        adr = zHL;
        READ_BYTE(adr, src);
        res = ((src >> 1) | (src << 7)) & 0xFF;
        zF = SZXYP[res] | (src & CZ80_CF);
        WRITE_BYTE(adr, res);
        POST_IO();
        RET(11 + 4);
    }

#define OPJ_RL_R(JmpType, Opcode) \
    JmpType(Opcode): { \
        uint8_t src = zR8(Opcode & 7); \
        zR8(Opcode & 7) = (src << 1) | (zF & CZ80_CF); \
        zF = SZXYP[zR8(Opcode & 7)] | (src >> 7); \
        RET(4 + 4); \
    }
#define OPCB_RL_R(Opcode) OPJ_RL_R(OPCB, Opcode)

    OPCB_RL_R(0x10) // RL   B
    OPCB_RL_R(0x11) // RL   C
    OPCB_RL_R(0x12) // RL   D
    OPCB_RL_R(0x13) // RL   E
    OPCB_RL_R(0x14) // RL   H
    OPCB_RL_R(0x15) // RL   L
    OPCB_RL_R(0x17) // RL   A

    OPCB(0x16): // RL   (HL)
    {
        uint16_t adr;
        uint8_t src;
        uint8_t res;

        PRE_IO();
        adr = zHL;
        READ_BYTE(adr, src);
        res = ((src << 1) | (zF & CZ80_CF)) & 0xFF;
        zF = SZXYP[res] | (src >> 7);
        WRITE_BYTE(adr, res);
        POST_IO();
        RET(11 + 4);
    }

#define OPJ_RR_R(JmpType, Opcode) \
    JmpType(Opcode): { \
        uint8_t src = zR8(Opcode & 7); \
        zR8(Opcode & 7) = (src >> 1) | (zF << 7); \
        zF = SZXYP[zR8(Opcode & 7)] | (src & CZ80_CF); \
        RET(4 + 4); \
    }
#define OPCB_RR_R(Opcode) OPJ_RR_R(OPCB, Opcode)

    OPCB_RR_R(0x18) // RR   B
    OPCB_RR_R(0x19) // RR   C
    OPCB_RR_R(0x1a) // RR   D
    OPCB_RR_R(0x1b) // RR   E
    OPCB_RR_R(0x1c) // RR   H
    OPCB_RR_R(0x1d) // RR   L
    OPCB_RR_R(0x1f) // RR   A

    OPCB(0x1e): // RR   (HL)
    {
        uint16_t adr;
        uint8_t src;
        uint8_t res;

        PRE_IO();
        adr = zHL;
        READ_BYTE(adr, src);
        res = ((src >> 1) | (zF << 7)) & 0xFF;
        zF = SZXYP[res] | (src & CZ80_CF);
        WRITE_BYTE(adr, res);
        POST_IO();
        RET(11 + 4);
    }

    // SHIFT

#define OPJ_SLA_R(JmpType, Opcode) \
    JmpType(Opcode): { \
        uint8_t src = zR8(Opcode & 7); \
        zR8(Opcode & 7) = src << 1; \
        zF = SZXYP[zR8(Opcode & 7)] | (src >> 7); \
        RET(4 + 4); \
    }
#define OPCB_SLA_R(Opcode) OPJ_SLA_R(OPCB, Opcode)

    OPCB_SLA_R(0x20) // SLA  B
    OPCB_SLA_R(0x21) // SLA  C
    OPCB_SLA_R(0x22) // SLA  D
    OPCB_SLA_R(0x23) // SLA  E
    OPCB_SLA_R(0x24) // SLA  H
    OPCB_SLA_R(0x25) // SLA  L
    OPCB_SLA_R(0x27) // SLA  A

    OPCB(0x26): // SLA  (HL)
    {
        uint16_t adr;
        uint8_t src;
        uint8_t res;

        PRE_IO();
        adr = zHL;
        READ_BYTE(adr, src);
        res = (src << 1) & 0xFF;
        zF = SZXYP[res] | (src >> 7);
        WRITE_BYTE(adr, res);
        POST_IO();
        RET(11 + 4);
    }

#define OPJ_SRA_R(JmpType, Opcode) \
    JmpType(Opcode): { \
        int8_t src = zR8(Opcode & 7); \
        zR8(Opcode & 7) = src >> 1; \
        zF = SZXYP[zR8(Opcode & 7)] | (src & CZ80_CF); \
        RET(4 + 4); \
    }
#define OPCB_SRA_R(Opcode) OPJ_SRA_R(OPCB, Opcode)

    OPCB_SRA_R(0x28) // SRA  B
    OPCB_SRA_R(0x29) // SRA  C
    OPCB_SRA_R(0x2a) // SRA  D
    OPCB_SRA_R(0x2b) // SRA  E
    OPCB_SRA_R(0x2c) // SRA  H
    OPCB_SRA_R(0x2d) // SRA  L
    OPCB_SRA_R(0x2f) // SRA  A

    OPCB(0x2e): // SRA  (HL)
    {
        uint16_t adr;
        uint8_t src;
        uint8_t res;

        PRE_IO();
        adr = zHL;
        READ_BYTE(adr, src);
        res = (uint8_t)(((int8_t)(src)) >> 1);
        zF = SZXYP[res] | (src & CZ80_CF);
        WRITE_BYTE(adr, res);
        POST_IO();
        RET(11 + 4);
    }

#define OPJ_SLL_R(JmpType, Opcode) \
    JmpType(Opcode): { \
        uint8_t src = zR8(Opcode & 7); \
        zR8(Opcode & 7) = (src << 1) | 1; \
        zF = SZXYP[zR8(Opcode & 7)] | (src >> 7); \
        RET(4 + 4); \
    }
#define OPCB_SLL_R(Opcode) OPJ_SLL_R(OPCB, Opcode)

    OPCB_SLL_R(0x30) // SLL  B
    OPCB_SLL_R(0x31) // SLL  C
    OPCB_SLL_R(0x32) // SLL  D
    OPCB_SLL_R(0x33) // SLL  E
    OPCB_SLL_R(0x34) // SLL  H
    OPCB_SLL_R(0x35) // SLL  L
    OPCB_SLL_R(0x37) // SLL  A

    OPCB(0x36): // SLL  (HL)
    {
        uint16_t adr;
        uint8_t src;
        uint8_t res;

        PRE_IO();
        adr = zHL;
        READ_BYTE(adr, src);
        res = ((src << 1) | 1) & 0xFF;
        zF = SZXYP[res] | (src >> 7);
        WRITE_BYTE(adr, res);
        POST_IO();
        RET(11 + 4);
    }

#define OPJ_SRL_R(JmpType, Opcode) \
    JmpType(Opcode): { \
        uint8_t src = zR8(Opcode & 7); \
        zR8(Opcode & 7) = src >> 1; \
        zF = SZXYP[zR8(Opcode & 7)] | (src & CZ80_CF); \
        RET(4 + 4); \
    }
#define OPCB_SRL_R(Opcode) OPJ_SRL_R(OPCB, Opcode)

    OPCB_SRL_R(0x38) // SRL  B
    OPCB_SRL_R(0x39) // SRL  C
    OPCB_SRL_R(0x3a) // SRL  D
    OPCB_SRL_R(0x3b) // SRL  E
    OPCB_SRL_R(0x3c) // SRL  H
    OPCB_SRL_R(0x3d) // SRL  L
    OPCB_SRL_R(0x3f) // SRL  A

    OPCB(0x3e): // SRL  (HL)
    {
        uint16_t adr;
        uint8_t src;
        uint8_t res;

        PRE_IO();
        adr = zHL;
        READ_BYTE(adr, src);
        res = src >> 1;
        zF = SZXYP[res] | (src & CZ80_CF);
        WRITE_BYTE(adr, res);
        POST_IO();
        RET(11 + 4);
    }

    // BIT

#define OPJ_BIT_N_R(JmpType, Opcode) \
    JmpType(Opcode): { \
        uint8_t src = zR8(Opcode & 7); \
        uint8_t bitm = 1 << ((Opcode >> 3) & 7); \
        /* TODO: Remove XF/YF flags from SZXY_BIT[]. */ \
        zF = (zF & CZ80_CF) | CZ80_HF | \
            (SZXY_BIT[src & bitm] & ~(CZ80_YF| CZ80_XF)) | \
            (src & (CZ80_YF| CZ80_XF)); \
        RET(4 + 4); \
    }
#define OPCB_BIT_N_R(Opcode) OPJ_BIT_N_R(OPCB, Opcode)

    OPCB_BIT_N_R(0x40) // BIT  0,B
    OPCB_BIT_N_R(0x41) // BIT  0,C
    OPCB_BIT_N_R(0x42) // BIT  0,D
    OPCB_BIT_N_R(0x43) // BIT  0,E
    OPCB_BIT_N_R(0x44) // BIT  0,H
    OPCB_BIT_N_R(0x45) // BIT  0,L
    OPCB_BIT_N_R(0x47) // BIT  0,A
    
    OPCB_BIT_N_R(0x48) // BIT  1,B
    OPCB_BIT_N_R(0x49) // BIT  1,C
    OPCB_BIT_N_R(0x4a) // BIT  1,D
    OPCB_BIT_N_R(0x4b) // BIT  1,E
    OPCB_BIT_N_R(0x4c) // BIT  1,H
    OPCB_BIT_N_R(0x4d) // BIT  1,L
    OPCB_BIT_N_R(0x4f) // BIT  1,A
    
    OPCB_BIT_N_R(0x50) // BIT  2,B
    OPCB_BIT_N_R(0x51) // BIT  2,C
    OPCB_BIT_N_R(0x52) // BIT  2,D
    OPCB_BIT_N_R(0x53) // BIT  2,E
    OPCB_BIT_N_R(0x54) // BIT  2,H
    OPCB_BIT_N_R(0x55) // BIT  2,L
    OPCB_BIT_N_R(0x57) // BIT  2,A
    
    OPCB_BIT_N_R(0x58) // BIT  3,B
    OPCB_BIT_N_R(0x59) // BIT  3,C
    OPCB_BIT_N_R(0x5a) // BIT  3,D
    OPCB_BIT_N_R(0x5b) // BIT  3,E
    OPCB_BIT_N_R(0x5c) // BIT  3,H
    OPCB_BIT_N_R(0x5d) // BIT  3,L
    OPCB_BIT_N_R(0x5f) // BIT  3,A
    
    OPCB_BIT_N_R(0x60) // BIT  4,B
    OPCB_BIT_N_R(0x61) // BIT  4,C
    OPCB_BIT_N_R(0x62) // BIT  4,D
    OPCB_BIT_N_R(0x63) // BIT  4,E
    OPCB_BIT_N_R(0x64) // BIT  4,H
    OPCB_BIT_N_R(0x65) // BIT  4,L
    OPCB_BIT_N_R(0x67) // BIT  4,A
    
    OPCB_BIT_N_R(0x68) // BIT  5,B
    OPCB_BIT_N_R(0x69) // BIT  5,C
    OPCB_BIT_N_R(0x6a) // BIT  5,D
    OPCB_BIT_N_R(0x6b) // BIT  5,E
    OPCB_BIT_N_R(0x6c) // BIT  5,H
    OPCB_BIT_N_R(0x6d) // BIT  5,L
    OPCB_BIT_N_R(0x6f) // BIT  5,A
    
    OPCB_BIT_N_R(0x70) // BIT  6,B
    OPCB_BIT_N_R(0x71) // BIT  6,C
    OPCB_BIT_N_R(0x72) // BIT  6,D
    OPCB_BIT_N_R(0x73) // BIT  6,E
    OPCB_BIT_N_R(0x74) // BIT  6,H
    OPCB_BIT_N_R(0x75) // BIT  6,L
    OPCB_BIT_N_R(0x77) // BIT  6,A
    
    OPCB_BIT_N_R(0x78) // BIT  7,B
    OPCB_BIT_N_R(0x79) // BIT  7,C
    OPCB_BIT_N_R(0x7a) // BIT  7,D
    OPCB_BIT_N_R(0x7b) // BIT  7,E
    OPCB_BIT_N_R(0x7c) // BIT  7,H
    OPCB_BIT_N_R(0x7d) // BIT  7,L
    OPCB_BIT_N_R(0x7f) // BIT  7,A

#define OPJ_BIT_N_mHL(JmpType, Opcode) \
    JmpType(Opcode): { \
        uint8_t src; \
        uint8_t bitm = 1 << ((Opcode >> 3) & 7); \
        PRE_IO(); \
        READ_BYTE(zHL, src); \
        /* TODO: Remove XF/YF flags from SZXY_BIT[]. */ \
        zF = (zF & CZ80_CF) | CZ80_HF |                         /* C/H flag */ \
             (SZXY_BIT[src & bitm] & ~(CZ80_YF | CZ80_XF)) |    /* Z/V/N flag */ \
             ((zWZ >> 8) & (CZ80_YF | CZ80_XF));                /* X/Y flag */ \
        POST_IO(); \
        RET(8 + 4); \
    }
#define OPCB_BIT_N_mHL(Opcode) OPJ_BIT_N_mHL(OPCB, Opcode)

    OPCB_BIT_N_mHL(0x46) // BIT  0,(HL)
    OPCB_BIT_N_mHL(0x4e) // BIT  1,(HL)
    OPCB_BIT_N_mHL(0x56) // BIT  2,(HL)
    OPCB_BIT_N_mHL(0x5e) // BIT  3,(HL)
    OPCB_BIT_N_mHL(0x66) // BIT  4,(HL)
    OPCB_BIT_N_mHL(0x6e) // BIT  5,(HL)
    OPCB_BIT_N_mHL(0x76) // BIT  6,(HL)
    OPCB_BIT_N_mHL(0x7e) // BIT  7,(HL)

    // RES

#define OPJ_RES_N_R(JmpType, Opcode) \
    JmpType(Opcode): \
        zR8(Opcode & 7) &= ~(1 << ((Opcode >> 3) & 7)); \
        RET(4 + 4);
#define OPCB_RES_N_R(Opcode) OPJ_RES_N_R(OPCB, Opcode)

    OPCB_RES_N_R(0x80) // RES  0,B
    OPCB_RES_N_R(0x81) // RES  0,C
    OPCB_RES_N_R(0x82) // RES  0,D
    OPCB_RES_N_R(0x83) // RES  0,E
    OPCB_RES_N_R(0x84) // RES  0,H
    OPCB_RES_N_R(0x85) // RES  0,L
    OPCB_RES_N_R(0x87) // RES  0,A

    OPCB_RES_N_R(0x88) // RES  1,B
    OPCB_RES_N_R(0x89) // RES  1,C
    OPCB_RES_N_R(0x8a) // RES  1,D
    OPCB_RES_N_R(0x8b) // RES  1,E
    OPCB_RES_N_R(0x8c) // RES  1,H
    OPCB_RES_N_R(0x8d) // RES  1,L
    OPCB_RES_N_R(0x8f) // RES  1,A

    OPCB_RES_N_R(0x90) // RES  2,B
    OPCB_RES_N_R(0x91) // RES  2,C
    OPCB_RES_N_R(0x92) // RES  2,D
    OPCB_RES_N_R(0x93) // RES  2,E
    OPCB_RES_N_R(0x94) // RES  2,H
    OPCB_RES_N_R(0x95) // RES  2,L
    OPCB_RES_N_R(0x97) // RES  2,A

    OPCB_RES_N_R(0x98) // RES  3,B
    OPCB_RES_N_R(0x99) // RES  3,C
    OPCB_RES_N_R(0x9a) // RES  3,D
    OPCB_RES_N_R(0x9b) // RES  3,E
    OPCB_RES_N_R(0x9c) // RES  3,H
    OPCB_RES_N_R(0x9d) // RES  3,L
    OPCB_RES_N_R(0x9f) // RES  3,A

    OPCB_RES_N_R(0xa0) // RES  4,B
    OPCB_RES_N_R(0xa1) // RES  4,C
    OPCB_RES_N_R(0xa2) // RES  4,D
    OPCB_RES_N_R(0xa3) // RES  4,E
    OPCB_RES_N_R(0xa4) // RES  4,H
    OPCB_RES_N_R(0xa5) // RES  4,L
    OPCB_RES_N_R(0xa7) // RES  4,A

    OPCB_RES_N_R(0xa8) // RES  5,B
    OPCB_RES_N_R(0xa9) // RES  5,C
    OPCB_RES_N_R(0xaa) // RES  5,D
    OPCB_RES_N_R(0xab) // RES  5,E
    OPCB_RES_N_R(0xac) // RES  5,H
    OPCB_RES_N_R(0xad) // RES  5,L
    OPCB_RES_N_R(0xaf) // RES  5,A

    OPCB_RES_N_R(0xb0) // RES  6,B
    OPCB_RES_N_R(0xb1) // RES  6,C
    OPCB_RES_N_R(0xb2) // RES  6,D
    OPCB_RES_N_R(0xb3) // RES  6,E
    OPCB_RES_N_R(0xb4) // RES  6,H
    OPCB_RES_N_R(0xb5) // RES  6,L
    OPCB_RES_N_R(0xb7) // RES  6,A

    OPCB_RES_N_R(0xb8) // RES  7,B
    OPCB_RES_N_R(0xb9) // RES  7,C
    OPCB_RES_N_R(0xba) // RES  7,D
    OPCB_RES_N_R(0xbb) // RES  7,E
    OPCB_RES_N_R(0xbc) // RES  7,H
    OPCB_RES_N_R(0xbd) // RES  7,L
    OPCB_RES_N_R(0xbf) // RES  7,A

#define OPJ_RES_N_mHL(JmpType, Opcode) \
    JmpType(Opcode): { \
        uint16_t adr = zHL; \
        uint8_t res; \
        uint8_t bitm = (uint8_t)~(1 << ((Opcode >> 3) & 7)); \
        PRE_IO(); \
        READ_BYTE(adr, res); \
        res &= bitm; \
        WRITE_BYTE(adr, res); \
        POST_IO(); \
        RET(11 + 4); \
    }
#define OPCB_RES_N_mHL(Opcode) OPJ_RES_N_mHL(OPCB, Opcode)

    OPCB_RES_N_mHL(0x86) // RES  0,(HL)
    OPCB_RES_N_mHL(0x8e) // RES  1,(HL)
    OPCB_RES_N_mHL(0x96) // RES  2,(HL)
    OPCB_RES_N_mHL(0x9e) // RES  3,(HL)
    OPCB_RES_N_mHL(0xa6) // RES  4,(HL)
    OPCB_RES_N_mHL(0xae) // RES  5,(HL)
    OPCB_RES_N_mHL(0xb6) // RES  6,(HL)
    OPCB_RES_N_mHL(0xbe) // RES  7,(HL)

    // SET

#define OPJ_SET_N_R(JmpType, Opcode) \
    JmpType(Opcode): \
        zR8(Opcode & 7) |= (1 << ((Opcode >> 3) & 7)); \
        RET(4 + 4);
#define OPCB_SET_N_R(Opcode) OPJ_SET_N_R(OPCB, Opcode)

    OPCB_SET_N_R(0xc0) // SET  0,B
    OPCB_SET_N_R(0xc1) // SET  0,C
    OPCB_SET_N_R(0xc2) // SET  0,D
    OPCB_SET_N_R(0xc3) // SET  0,E
    OPCB_SET_N_R(0xc4) // SET  0,H
    OPCB_SET_N_R(0xc5) // SET  0,L
    OPCB_SET_N_R(0xc7) // SET  0,A

    OPCB_SET_N_R(0xc8) // SET  1,B
    OPCB_SET_N_R(0xc9) // SET  1,C
    OPCB_SET_N_R(0xca) // SET  1,D
    OPCB_SET_N_R(0xcb) // SET  1,E
    OPCB_SET_N_R(0xcc) // SET  1,H
    OPCB_SET_N_R(0xcd) // SET  1,L
    OPCB_SET_N_R(0xcf) // SET  1,A

    OPCB_SET_N_R(0xd0) // SET  2,B
    OPCB_SET_N_R(0xd1) // SET  2,C
    OPCB_SET_N_R(0xd2) // SET  2,D
    OPCB_SET_N_R(0xd3) // SET  2,E
    OPCB_SET_N_R(0xd4) // SET  2,H
    OPCB_SET_N_R(0xd5) // SET  2,L
    OPCB_SET_N_R(0xd7) // SET  2,A

    OPCB_SET_N_R(0xd8) // SET  3,B
    OPCB_SET_N_R(0xd9) // SET  3,C
    OPCB_SET_N_R(0xda) // SET  3,D
    OPCB_SET_N_R(0xdb) // SET  3,E
    OPCB_SET_N_R(0xdc) // SET  3,H
    OPCB_SET_N_R(0xdd) // SET  3,L
    OPCB_SET_N_R(0xdf) // SET  3,A

    OPCB_SET_N_R(0xe0) // SET  4,B
    OPCB_SET_N_R(0xe1) // SET  4,C
    OPCB_SET_N_R(0xe2) // SET  4,D
    OPCB_SET_N_R(0xe3) // SET  4,E
    OPCB_SET_N_R(0xe4) // SET  4,H
    OPCB_SET_N_R(0xe5) // SET  4,L
    OPCB_SET_N_R(0xe7) // SET  4,A

    OPCB_SET_N_R(0xe8) // SET  5,B
    OPCB_SET_N_R(0xe9) // SET  5,C
    OPCB_SET_N_R(0xea) // SET  5,D
    OPCB_SET_N_R(0xeb) // SET  5,E
    OPCB_SET_N_R(0xec) // SET  5,H
    OPCB_SET_N_R(0xed) // SET  5,L
    OPCB_SET_N_R(0xef) // SET  5,A

    OPCB_SET_N_R(0xf0) // SET  6,B
    OPCB_SET_N_R(0xf1) // SET  6,C
    OPCB_SET_N_R(0xf2) // SET  6,D
    OPCB_SET_N_R(0xf3) // SET  6,E
    OPCB_SET_N_R(0xf4) // SET  6,H
    OPCB_SET_N_R(0xf5) // SET  6,L
    OPCB_SET_N_R(0xf7) // SET  6,A

    OPCB_SET_N_R(0xf8) // SET  7,B
    OPCB_SET_N_R(0xf9) // SET  7,C
    OPCB_SET_N_R(0xfa) // SET  7,D
    OPCB_SET_N_R(0xfb) // SET  7,E
    OPCB_SET_N_R(0xfc) // SET  7,H
    OPCB_SET_N_R(0xfd) // SET  7,L
    OPCB_SET_N_R(0xff) // SET  7,A

#define OPJ_SET_N_mHL(JmpType, Opcode) \
    JmpType(Opcode): { \
        uint16_t adr = zHL; \
        uint8_t res; \
        uint8_t bitm = 1 << ((Opcode >> 3) & 7); \
        PRE_IO(); \
        READ_BYTE(adr, res); \
        res |= bitm; \
        WRITE_BYTE(adr, res); \
        POST_IO(); \
        RET(11 + 4); \
    }
#define OPCB_SET_N_mHL(Opcode) OPJ_SET_N_mHL(OPCB, Opcode)

    OPCB_SET_N_mHL(0xc6) // SET  0,(HL)
    OPCB_SET_N_mHL(0xce) // SET  1,(HL)
    OPCB_SET_N_mHL(0xd6) // SET  2,(HL)
    OPCB_SET_N_mHL(0xde) // SET  3,(HL)
    OPCB_SET_N_mHL(0xe6) // SET  4,(HL)
    OPCB_SET_N_mHL(0xee) // SET  5,(HL)
    OPCB_SET_N_mHL(0xf6) // SET  6,(HL)
    OPCB_SET_N_mHL(0xfe) // SET  7,(HL)
    
#if CZ80_USE_JUMPTABLE
#else
}
#endif
