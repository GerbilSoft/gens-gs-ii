/********************************************************************************/
/*                                                                              */
/* CZ80 XYCB opcode include source file                                         */
/* C Z80 emulator version 0.91                                                  */
/* Copyright 2004-2005 Stephane Dallongeville                                   */
/*                                                                              */
/********************************************************************************/

#ifndef IS_IN_CZ80
#error Do not compile this file by itself - compile cz80.c instead
#endif

#if CZ80_USE_JUMPTABLE
    DO_JMPTBL_OPXYCB(Opcode);
#else
switch (Opcode)
{
#endif

    // ROTATE and SHIFT

    // TODO: Backport to cz80_opcb.inc?
    // Also maybe combine common macros for ADD, etc.
#define OPXYCB_ROTSHF(Opcode, uOp) \
    OPXYCB(Opcode): \
        PRE_IO(); \
	READ_BYTE(zWZ, src); \
	uOp(); \
        WRITE_BYTE(zWZ, res); \
        POST_IO(); \
        RET(15 + 4);

#if CZ80_EXACT
#define OPXYCB_ROTSHF_R(Opcode, uOp) \
    OPXYCB(Opcode): \
        PRE_IO(); \
	READ_BYTE(zWZ, src); \
	uOp(); \
	zR8(Opcode & 7) = res; \
        WRITE_BYTE(zWZ, res); \
        POST_IO(); \
        RET(15 + 4);
#else
#define OPXYCB_ROTSHF_R(Opcode, uOp) OPXYCB_ROTSHF(Opcode, uOp)
#endif

#define UOP_RLC() do { \
        res = ((src << 1) | (src >> 7)) & 0xFF; \
        zF = SZXYP[res] | (src >> 7); \
    } while (0)

    OPXYCB_ROTSHF_R(0x00, UOP_RLC)   // RLC  (Ix+d), B
    OPXYCB_ROTSHF_R(0x01, UOP_RLC)   // RLC  (Ix+d), C
    OPXYCB_ROTSHF_R(0x02, UOP_RLC)   // RLC  (Ix+d), D
    OPXYCB_ROTSHF_R(0x03, UOP_RLC)   // RLC  (Ix+d), E
    OPXYCB_ROTSHF_R(0x04, UOP_RLC)   // RLC  (Ix+d), H
    OPXYCB_ROTSHF_R(0x05, UOP_RLC)   // RLC  (Ix+d), L
    OPXYCB_ROTSHF_R(0x07, UOP_RLC)   // RLC  (Ix+d), A

    OPXYCB_ROTSHF(0x06, UOP_RLC)     // RLC  (Ix+d)

#define UOP_RRC() do { \
        res = ((src >> 1) | (src << 7)) & 0xFF; \
        zF = SZXYP[res] | (src & CZ80_CF); \
    } while (0)

    OPXYCB_ROTSHF_R(0x08, UOP_RRC)   // RRC  (Ix+d), B
    OPXYCB_ROTSHF_R(0x09, UOP_RRC)   // RRC  (Ix+d), C
    OPXYCB_ROTSHF_R(0x0a, UOP_RRC)   // RRC  (Ix+d), D
    OPXYCB_ROTSHF_R(0x0b, UOP_RRC)   // RRC  (Ix+d), E
    OPXYCB_ROTSHF_R(0x0c, UOP_RRC)   // RRC  (Ix+d), H
    OPXYCB_ROTSHF_R(0x0d, UOP_RRC)   // RRC  (Ix+d), L
    OPXYCB_ROTSHF_R(0x0f, UOP_RRC)   // RRC  (Ix+d), A

    OPXYCB_ROTSHF(0x0e, UOP_RRC)     // RRC  (Ix+d)

#define UOP_RL() do { \
        res = ((src << 1) | (zF & CZ80_CF)) & 0xFF; \
        zF = SZXYP[res] | (src >> 7); \
    } while (0)

    OPXYCB_ROTSHF_R(0x10, UOP_RL)   // RL   (Ix+d), B
    OPXYCB_ROTSHF_R(0x11, UOP_RL)   // RL   (Ix+d), C
    OPXYCB_ROTSHF_R(0x12, UOP_RL)   // RL   (Ix+d), D
    OPXYCB_ROTSHF_R(0x13, UOP_RL)   // RL   (Ix+d), E
    OPXYCB_ROTSHF_R(0x14, UOP_RL)   // RL   (Ix+d), H
    OPXYCB_ROTSHF_R(0x15, UOP_RL)   // RL   (Ix+d), L
    OPXYCB_ROTSHF_R(0x17, UOP_RL)   // RL   (Ix+d), A

    OPXYCB_ROTSHF(0x16, UOP_RL)     // RL   (Ix+d)

#define UOP_RR() do { \
        res = ((src >> 1) | (zF << 7)) & 0xFF; \
        zF = SZXYP[res] | (src & CZ80_CF); \
    } while (0)

    OPXYCB_ROTSHF_R(0x18, UOP_RR)   // RR   (Ix+d), B
    OPXYCB_ROTSHF_R(0x19, UOP_RR)   // RR   (Ix+d), C
    OPXYCB_ROTSHF_R(0x1a, UOP_RR)   // RR   (Ix+d), D
    OPXYCB_ROTSHF_R(0x1b, UOP_RR)   // RR   (Ix+d), E
    OPXYCB_ROTSHF_R(0x1c, UOP_RR)   // RR   (Ix+d), H
    OPXYCB_ROTSHF_R(0x1d, UOP_RR)   // RR   (Ix+d), L
    OPXYCB_ROTSHF_R(0x1f, UOP_RR)   // RR   (Ix+d), A

    OPXYCB_ROTSHF(0x1e, UOP_RR)     // RR   (Ix+d)

#define UOP_SLA() do { \
        res = (src << 1) & 0xFF; \
        zF = SZXYP[res] | (src >> 7); \
    } while (0)

    OPXYCB_ROTSHF_R(0x20, UOP_SLA)   // SLA  (Ix+d), B
    OPXYCB_ROTSHF_R(0x21, UOP_SLA)   // SLA  (Ix+d), C
    OPXYCB_ROTSHF_R(0x22, UOP_SLA)   // SLA  (Ix+d), D
    OPXYCB_ROTSHF_R(0x23, UOP_SLA)   // SLA  (Ix+d), E
    OPXYCB_ROTSHF_R(0x24, UOP_SLA)   // SLA  (Ix+d), H
    OPXYCB_ROTSHF_R(0x25, UOP_SLA)   // SLA  (Ix+d), L
    OPXYCB_ROTSHF_R(0x27, UOP_SLA)   // SLA  (Ix+d), A

    OPXYCB_ROTSHF(0x26, UOP_SLA)     // SLA  (Ix+d)

#define UOP_SRA() do { \
        res = (uint8_t)(((int8_t)(src)) >> 1); \
        zF = SZXYP[res] | (src & CZ80_CF); \
    } while (0)

    OPXYCB_ROTSHF_R(0x28, UOP_SRA)   // SRA  (Ix+d), B
    OPXYCB_ROTSHF_R(0x29, UOP_SRA)   // SRA  (Ix+d), C
    OPXYCB_ROTSHF_R(0x2a, UOP_SRA)   // SRA  (Ix+d), D
    OPXYCB_ROTSHF_R(0x2b, UOP_SRA)   // SRA  (Ix+d), E
    OPXYCB_ROTSHF_R(0x2c, UOP_SRA)   // SRA  (Ix+d), H
    OPXYCB_ROTSHF_R(0x2d, UOP_SRA)   // SRA  (Ix+d), L
    OPXYCB_ROTSHF_R(0x2f, UOP_SRA)   // SRA  (Ix+d), A

    OPXYCB_ROTSHF(0x2e, UOP_SRA)     // SRA  (Ix+d)

#define UOP_SLL() do { \
        res = ((src << 1) | 1) & 0xFF; \
        zF = SZXYP[res] | (src >> 7); \
    } while (0)

    OPXYCB_ROTSHF_R(0x30, UOP_SLL)   // SLL  (Ix+d), B
    OPXYCB_ROTSHF_R(0x31, UOP_SLL)   // SLL  (Ix+d), C
    OPXYCB_ROTSHF_R(0x32, UOP_SLL)   // SLL  (Ix+d), D
    OPXYCB_ROTSHF_R(0x33, UOP_SLL)   // SLL  (Ix+d), E
    OPXYCB_ROTSHF_R(0x34, UOP_SLL)   // SLL  (Ix+d), H
    OPXYCB_ROTSHF_R(0x35, UOP_SLL)   // SLL  (Ix+d), L
    OPXYCB_ROTSHF_R(0x37, UOP_SLL)   // SLL  (Ix+d), A

    OPXYCB_ROTSHF(0x36, UOP_SLL)     // SLL  (Ix+d)

#define UOP_SRL() do { \
        res = src >> 1; \
        zF = SZXYP[res] | (src & CZ80_CF); \
    } while (0)

    OPXYCB_ROTSHF_R(0x38, UOP_SRL)   // SRL  (Ix+d), B
    OPXYCB_ROTSHF_R(0x39, UOP_SRL)   // SRL  (Ix+d), C
    OPXYCB_ROTSHF_R(0x3a, UOP_SRL)   // SRL  (Ix+d), D
    OPXYCB_ROTSHF_R(0x3b, UOP_SRL)   // SRL  (Ix+d), E
    OPXYCB_ROTSHF_R(0x3c, UOP_SRL)   // SRL  (Ix+d), H
    OPXYCB_ROTSHF_R(0x3d, UOP_SRL)   // SRL  (Ix+d), L
    OPXYCB_ROTSHF_R(0x3f, UOP_SRL)   // SRL  (Ix+d), A

    OPXYCB_ROTSHF(0x3e, UOP_SRL)     // SRL  (Ix+d)

    // BIT

    // NOTE: The source register field is ignored.
#define OPXYCB_BIT_N_mXYd(Opcode, Op1, Op2, Op3, Op4, Op5, Op6, Op7) \
    OPXYCB(Opcode): \
    OPXYCB(Op1): \
    OPXYCB(Op2): \
    OPXYCB(Op3): \
    OPXYCB(Op4): \
    OPXYCB(Op5): \
    OPXYCB(Op6): \
    OPXYCB(Op7): { \
        const uint8_t bitm = 1 << ((Opcode >> 3) & 7); \
        PRE_IO(); \
        READ_BYTE(zWZ, src); \
        /* TODO: Remove XF/YF flags from SZXY_BIT[]. */ \
        zF = (zF & CZ80_CF) | CZ80_HF |                         /* C/H flag */ \
             (SZXY_BIT[src & bitm] & ~(CZ80_YF | CZ80_XF)) |    /* Z/V/N flag */ \
             ((zWZ >> 8) & (CZ80_YF | CZ80_XF));                /* X/Y flag */ \
        POST_IO(); \
        RET(12 + 4); \
    }

    OPXYCB_BIT_N_mXYd(0x46, 0x40, 0x41, 0x42, 0x43, 0x44, 0x45, 0x47)   // BIT  0,(Ix+d)
    OPXYCB_BIT_N_mXYd(0x4e, 0x48, 0x49, 0x4a, 0x4b, 0x4c, 0x4d, 0x4f)   // BIT  1,(Ix+d)
    OPXYCB_BIT_N_mXYd(0x56, 0x50, 0x51, 0x52, 0x53, 0x54, 0x55, 0x57)   // BIT  2,(Ix+d)
    OPXYCB_BIT_N_mXYd(0x5e, 0x58, 0x59, 0x5a, 0x5b, 0x5c, 0x5d, 0x5f)   // BIT  3,(Ix+d)
    OPXYCB_BIT_N_mXYd(0x66, 0x60, 0x61, 0x62, 0x63, 0x64, 0x65, 0x67)   // BIT  4,(Ix+d)
    OPXYCB_BIT_N_mXYd(0x6e, 0x68, 0x69, 0x6a, 0x6b, 0x6c, 0x6d, 0x6f)   // BIT  5,(Ix+d)
    OPXYCB_BIT_N_mXYd(0x76, 0x70, 0x71, 0x72, 0x73, 0x74, 0x75, 0x77)   // BIT  6,(Ix+d)
    OPXYCB_BIT_N_mXYd(0x7e, 0x78, 0x79, 0x7a, 0x7b, 0x7c, 0x7d, 0x7f)   // BIT  7,(Ix+d)

    // TODO: src variable isn't necessary here.
    // Verify that it's optimized out...
#define UOP_RES() do { \
        res = src & ~(1 << ((Opcode >> 3) & 7)); \
    } while (0)

    OPXYCB_ROTSHF_R(0x80, UOP_RES)   // RES  0,(Ix+d),B
    OPXYCB_ROTSHF_R(0x81, UOP_RES)   // RES  0,(Ix+d),C
    OPXYCB_ROTSHF_R(0x82, UOP_RES)   // RES  0,(Ix+d),D
    OPXYCB_ROTSHF_R(0x83, UOP_RES)   // RES  0,(Ix+d),E
    OPXYCB_ROTSHF_R(0x84, UOP_RES)   // RES  0,(Ix+d),H
    OPXYCB_ROTSHF_R(0x85, UOP_RES)   // RES  0,(Ix+d),L
    OPXYCB_ROTSHF_R(0x87, UOP_RES)   // RES  0,(Ix+d),A

    OPXYCB_ROTSHF_R(0x88, UOP_RES)   // RES  1,(Ix+d),B
    OPXYCB_ROTSHF_R(0x89, UOP_RES)   // RES  1,(Ix+d),C
    OPXYCB_ROTSHF_R(0x8a, UOP_RES)   // RES  1,(Ix+d),D
    OPXYCB_ROTSHF_R(0x8b, UOP_RES)   // RES  1,(Ix+d),E
    OPXYCB_ROTSHF_R(0x8c, UOP_RES)   // RES  1,(Ix+d),H
    OPXYCB_ROTSHF_R(0x8d, UOP_RES)   // RES  1,(Ix+d),L
    OPXYCB_ROTSHF_R(0x8f, UOP_RES)   // RES  1,(Ix+d),A

    OPXYCB_ROTSHF_R(0x90, UOP_RES)   // RES  2,(Ix+d),B
    OPXYCB_ROTSHF_R(0x91, UOP_RES)   // RES  2,(Ix+d),C
    OPXYCB_ROTSHF_R(0x92, UOP_RES)   // RES  2,(Ix+d),D
    OPXYCB_ROTSHF_R(0x93, UOP_RES)   // RES  2,(Ix+d),E
    OPXYCB_ROTSHF_R(0x94, UOP_RES)   // RES  2,(Ix+d),H
    OPXYCB_ROTSHF_R(0x95, UOP_RES)   // RES  2,(Ix+d),L
    OPXYCB_ROTSHF_R(0x97, UOP_RES)   // RES  2,(Ix+d),A

    OPXYCB_ROTSHF_R(0x98, UOP_RES)   // RES  3,(Ix+d),B
    OPXYCB_ROTSHF_R(0x99, UOP_RES)   // RES  3,(Ix+d),C
    OPXYCB_ROTSHF_R(0x9a, UOP_RES)   // RES  3,(Ix+d),D
    OPXYCB_ROTSHF_R(0x9b, UOP_RES)   // RES  3,(Ix+d),E
    OPXYCB_ROTSHF_R(0x9c, UOP_RES)   // RES  3,(Ix+d),H
    OPXYCB_ROTSHF_R(0x9d, UOP_RES)   // RES  3,(Ix+d),L
    OPXYCB_ROTSHF_R(0x9f, UOP_RES)   // RES  3,(Ix+d),A

    OPXYCB_ROTSHF_R(0xa0, UOP_RES)   // RES  4,(Ix+d),B
    OPXYCB_ROTSHF_R(0xa1, UOP_RES)   // RES  4,(Ix+d),C
    OPXYCB_ROTSHF_R(0xa2, UOP_RES)   // RES  4,(Ix+d),D
    OPXYCB_ROTSHF_R(0xa3, UOP_RES)   // RES  4,(Ix+d),E
    OPXYCB_ROTSHF_R(0xa4, UOP_RES)   // RES  4,(Ix+d),H
    OPXYCB_ROTSHF_R(0xa5, UOP_RES)   // RES  4,(Ix+d),L
    OPXYCB_ROTSHF_R(0xa7, UOP_RES)   // RES  4,(Ix+d),A

    OPXYCB_ROTSHF_R(0xa8, UOP_RES)   // RES  5,(Ix+d),B
    OPXYCB_ROTSHF_R(0xa9, UOP_RES)   // RES  5,(Ix+d),C
    OPXYCB_ROTSHF_R(0xaa, UOP_RES)   // RES  5,(Ix+d),D
    OPXYCB_ROTSHF_R(0xab, UOP_RES)   // RES  5,(Ix+d),E
    OPXYCB_ROTSHF_R(0xac, UOP_RES)   // RES  5,(Ix+d),H
    OPXYCB_ROTSHF_R(0xad, UOP_RES)   // RES  5,(Ix+d),L
    OPXYCB_ROTSHF_R(0xaf, UOP_RES)   // RES  5,(Ix+d),A

    OPXYCB_ROTSHF_R(0xb0, UOP_RES)   // RES  6,(Ix+d),B
    OPXYCB_ROTSHF_R(0xb1, UOP_RES)   // RES  6,(Ix+d),C
    OPXYCB_ROTSHF_R(0xb2, UOP_RES)   // RES  6,(Ix+d),D
    OPXYCB_ROTSHF_R(0xb3, UOP_RES)   // RES  6,(Ix+d),E
    OPXYCB_ROTSHF_R(0xb4, UOP_RES)   // RES  6,(Ix+d),H
    OPXYCB_ROTSHF_R(0xb5, UOP_RES)   // RES  6,(Ix+d),L
    OPXYCB_ROTSHF_R(0xb7, UOP_RES)   // RES  6,(Ix+d),A

    OPXYCB_ROTSHF_R(0xb8, UOP_RES)   // RES  7,(Ix+d),B
    OPXYCB_ROTSHF_R(0xb9, UOP_RES)   // RES  7,(Ix+d),C
    OPXYCB_ROTSHF_R(0xba, UOP_RES)   // RES  7,(Ix+d),D
    OPXYCB_ROTSHF_R(0xbb, UOP_RES)   // RES  7,(Ix+d),E
    OPXYCB_ROTSHF_R(0xbc, UOP_RES)   // RES  7,(Ix+d),H
    OPXYCB_ROTSHF_R(0xbd, UOP_RES)   // RES  7,(Ix+d),L
    OPXYCB_ROTSHF_R(0xbf, UOP_RES)   // RES  7,(Ix+d),A

    OPXYCB_ROTSHF(0x86, UOP_RES)     // RES  0,(Ix+d)
    OPXYCB_ROTSHF(0x8e, UOP_RES)     // RES  1,(Ix+d)
    OPXYCB_ROTSHF(0x96, UOP_RES)     // RES  2,(Ix+d)
    OPXYCB_ROTSHF(0x9e, UOP_RES)     // RES  3,(Ix+d)
    OPXYCB_ROTSHF(0xa6, UOP_RES)     // RES  4,(Ix+d)
    OPXYCB_ROTSHF(0xae, UOP_RES)     // RES  5,(Ix+d)
    OPXYCB_ROTSHF(0xb6, UOP_RES)     // RES  6,(Ix+d)
    OPXYCB_ROTSHF(0xbe, UOP_RES)     // RES  7,(Ix+d)

    // TODO: src variable isn't necessary here.
    // Verify that it's optimized out...
#define UOP_SET() do { \
        res = src | (1 << ((Opcode >> 3) & 7)); \
    } while (0)

    OPXYCB_ROTSHF_R(0xc0, UOP_SET)   // SET  0,(Ix+d),B
    OPXYCB_ROTSHF_R(0xc1, UOP_SET)   // SET  0,(Ix+d),C
    OPXYCB_ROTSHF_R(0xc2, UOP_SET)   // SET  0,(Ix+d),D
    OPXYCB_ROTSHF_R(0xc3, UOP_SET)   // SET  0,(Ix+d),E
    OPXYCB_ROTSHF_R(0xc4, UOP_SET)   // SET  0,(Ix+d),H
    OPXYCB_ROTSHF_R(0xc5, UOP_SET)   // SET  0,(Ix+d),L
    OPXYCB_ROTSHF_R(0xc7, UOP_SET)   // SET  0,(Ix+d),A

    OPXYCB_ROTSHF_R(0xc8, UOP_SET)   // SET  1,(Ix+d),B
    OPXYCB_ROTSHF_R(0xc9, UOP_SET)   // SET  1,(Ix+d),C
    OPXYCB_ROTSHF_R(0xca, UOP_SET)   // SET  1,(Ix+d),D
    OPXYCB_ROTSHF_R(0xcb, UOP_SET)   // SET  1,(Ix+d),E
    OPXYCB_ROTSHF_R(0xcc, UOP_SET)   // SET  1,(Ix+d),H
    OPXYCB_ROTSHF_R(0xcd, UOP_SET)   // SET  1,(Ix+d),L
    OPXYCB_ROTSHF_R(0xcf, UOP_SET)   // SET  1,(Ix+d),A

    OPXYCB_ROTSHF_R(0xd0, UOP_SET)   // SET  2,(Ix+d),B
    OPXYCB_ROTSHF_R(0xd1, UOP_SET)   // SET  2,(Ix+d),C
    OPXYCB_ROTSHF_R(0xd2, UOP_SET)   // SET  2,(Ix+d),D
    OPXYCB_ROTSHF_R(0xd3, UOP_SET)   // SET  2,(Ix+d),E
    OPXYCB_ROTSHF_R(0xd4, UOP_SET)   // SET  2,(Ix+d),H
    OPXYCB_ROTSHF_R(0xd5, UOP_SET)   // SET  2,(Ix+d),L
    OPXYCB_ROTSHF_R(0xd7, UOP_SET)   // SET  2,(Ix+d),A

    OPXYCB_ROTSHF_R(0xd8, UOP_SET)   // SET  3,(Ix+d),B
    OPXYCB_ROTSHF_R(0xd9, UOP_SET)   // SET  3,(Ix+d),C
    OPXYCB_ROTSHF_R(0xda, UOP_SET)   // SET  3,(Ix+d),D
    OPXYCB_ROTSHF_R(0xdb, UOP_SET)   // SET  3,(Ix+d),E
    OPXYCB_ROTSHF_R(0xdc, UOP_SET)   // SET  3,(Ix+d),H
    OPXYCB_ROTSHF_R(0xdd, UOP_SET)   // SET  3,(Ix+d),L
    OPXYCB_ROTSHF_R(0xdf, UOP_SET)   // SET  3,(Ix+d),A

    OPXYCB_ROTSHF_R(0xe0, UOP_SET)   // SET  4,(Ix+d),B
    OPXYCB_ROTSHF_R(0xe1, UOP_SET)   // SET  4,(Ix+d),C
    OPXYCB_ROTSHF_R(0xe2, UOP_SET)   // SET  4,(Ix+d),D
    OPXYCB_ROTSHF_R(0xe3, UOP_SET)   // SET  4,(Ix+d),E
    OPXYCB_ROTSHF_R(0xe4, UOP_SET)   // SET  4,(Ix+d),H
    OPXYCB_ROTSHF_R(0xe5, UOP_SET)   // SET  4,(Ix+d),L
    OPXYCB_ROTSHF_R(0xe7, UOP_SET)   // SET  4,(Ix+d),A

    OPXYCB_ROTSHF_R(0xe8, UOP_SET)   // SET  5,(Ix+d),B
    OPXYCB_ROTSHF_R(0xe9, UOP_SET)   // SET  5,(Ix+d),C
    OPXYCB_ROTSHF_R(0xea, UOP_SET)   // SET  5,(Ix+d),D
    OPXYCB_ROTSHF_R(0xeb, UOP_SET)   // SET  5,(Ix+d),E
    OPXYCB_ROTSHF_R(0xec, UOP_SET)   // SET  5,(Ix+d),H
    OPXYCB_ROTSHF_R(0xed, UOP_SET)   // SET  5,(Ix+d),L
    OPXYCB_ROTSHF_R(0xef, UOP_SET)   // SET  5,(Ix+d),A

    OPXYCB_ROTSHF_R(0xf0, UOP_SET)   // SET  6,(Ix+d),B
    OPXYCB_ROTSHF_R(0xf1, UOP_SET)   // SET  6,(Ix+d),C
    OPXYCB_ROTSHF_R(0xf2, UOP_SET)   // SET  6,(Ix+d),D
    OPXYCB_ROTSHF_R(0xf3, UOP_SET)   // SET  6,(Ix+d),E
    OPXYCB_ROTSHF_R(0xf4, UOP_SET)   // SET  6,(Ix+d),H
    OPXYCB_ROTSHF_R(0xf5, UOP_SET)   // SET  6,(Ix+d),L
    OPXYCB_ROTSHF_R(0xf7, UOP_SET)   // SET  6,(Ix+d),A

    OPXYCB_ROTSHF_R(0xf8, UOP_SET)   // SET  7,(Ix+d),B
    OPXYCB_ROTSHF_R(0xf9, UOP_SET)   // SET  7,(Ix+d),C
    OPXYCB_ROTSHF_R(0xfa, UOP_SET)   // SET  7,(Ix+d),D
    OPXYCB_ROTSHF_R(0xfb, UOP_SET)   // SET  7,(Ix+d),E
    OPXYCB_ROTSHF_R(0xfc, UOP_SET)   // SET  7,(Ix+d),H
    OPXYCB_ROTSHF_R(0xfd, UOP_SET)   // SET  7,(Ix+d),L
    OPXYCB_ROTSHF_R(0xff, UOP_SET)   // SET  7,(Ix+d),A

    OPXYCB_ROTSHF(0xc6, UOP_SET)     // SET  0,(Ix+d)
    OPXYCB_ROTSHF(0xce, UOP_SET)     // SET  1,(Ix+d)
    OPXYCB_ROTSHF(0xd6, UOP_SET)     // SET  2,(Ix+d)
    OPXYCB_ROTSHF(0xde, UOP_SET)     // SET  3,(Ix+d)
    OPXYCB_ROTSHF(0xe6, UOP_SET)     // SET  4,(Ix+d)
    OPXYCB_ROTSHF(0xee, UOP_SET)     // SET  5,(Ix+d)
    OPXYCB_ROTSHF(0xf6, UOP_SET)     // SET  6,(Ix+d)
    OPXYCB_ROTSHF(0xfe, UOP_SET)     // SET  7,(Ix+d)

#if CZ80_USE_JUMPTABLE
#else
}
#endif
