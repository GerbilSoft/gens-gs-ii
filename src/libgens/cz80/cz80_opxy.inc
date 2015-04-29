/********************************************************************************/
/*                                                                              */
/* CZ80 XY opcode include source file                                           */
/* C Z80 emulator version 0.91                                                  */
/* Copyright 2004-2005 Stephane Dallongeville                                   */
/*                                                                              */
/********************************************************************************/

// NOTE: The main jump table already subtracts 4 cycles from the counter
// before executing FD/DD-prefixed instructions. Consequently, the cycle
// timings listed below do not include these 4 cycles.

#if CZ80_USE_JUMPTABLE
    DO_JMPTBL_OPXY(Opcode);
#else
switch (Opcode)
{
#endif

    OPXY(0x00): // NOP

    // 8 BITS LOAD

    OPXY(0x40): // LD   B,B
    OPXY(0x49): // LD   C,C
    OPXY(0x52): // LD   D,D
    OPXY(0x5b): // LD   E,E
    OPXY(0x64): // LD   H,H
    OPXY(0x6d): // LD   L,L
    OPXY(0x7f): // LD   A,A
        RET(4);

#define OPXY_LD_R_R(Opcode) OPJ_LD_R_R(OPXY, Opcode)

    OPXY_LD_R_R(0x41) // LD   B,C
    OPXY_LD_R_R(0x42) // LD   B,D
    OPXY_LD_R_R(0x43) // LD   B,E
    OPXY_LD_R_R(0x47) // LD   B,A

    OPXY_LD_R_R(0x48) // LD   C,B
    OPXY_LD_R_R(0x4a) // LD   C,D
    OPXY_LD_R_R(0x4b) // LD   C,E
    OPXY_LD_R_R(0x4f) // LD   C,A

    OPXY_LD_R_R(0x50) // LD   D,B
    OPXY_LD_R_R(0x51) // LD   D,C
    OPXY_LD_R_R(0x53) // LD   D,E
    OPXY_LD_R_R(0x57) // LD   D,A

    OPXY_LD_R_R(0x58) // LD   E,B
    OPXY_LD_R_R(0x59) // LD   E,C
    OPXY_LD_R_R(0x5a) // LD   E,D
    OPXY_LD_R_R(0x5f) // LD   E,A

    OPXY_LD_R_R(0x78) // LD   A,B
    OPXY_LD_R_R(0x79) // LD   A,C
    OPXY_LD_R_R(0x7a) // LD   A,D
    OPXY_LD_R_R(0x7b) // LD   A,E

#define OPXY_LD_R_XY(Opcode, HL) \
    OPXY(Opcode): \
        zR8((Opcode >> 3) & 7) = data->B.HL; \
	RET(4);

    // TODO: Add byte indexes to data and extract the index from Opcode?
    OPXY_LD_R_XY(0x44, H) // LD   B,HX
    OPXY_LD_R_XY(0x4c, H) // LD   C,HX
    OPXY_LD_R_XY(0x54, H) // LD   D,HX
    OPXY_LD_R_XY(0x5c, H) // LD   E,HX
    OPXY_LD_R_XY(0x7c, H) // LD   A,HX

    OPXY_LD_R_XY(0x45, L) // LD   B,LX
    OPXY_LD_R_XY(0x4d, L) // LD   C,LX
    OPXY_LD_R_XY(0x55, L) // LD   D,LX
    OPXY_LD_R_XY(0x5d, L) // LD   E,LX
    OPXY_LD_R_XY(0x7d, L) // LD   A,LX

#define OPXY_LD_XY_R(Opcode, HL) \
    OPXY(Opcode): \
        data->B.HL = zR8(Opcode & 7); \
	RET(4);

    OPXY_LD_XY_R(0x60, H) // LD   HX,B
    OPXY_LD_XY_R(0x61, H) // LD   HX,C
    OPXY_LD_XY_R(0x62, H) // LD   HX,D
    OPXY_LD_XY_R(0x63, H) // LD   HX,E
    OPXY_LD_XY_R(0x67, H) // LD   HX,A

    OPXY_LD_XY_R(0x68, L) // LD   LX,B
    OPXY_LD_XY_R(0x69, L) // LD   LX,C
    OPXY_LD_XY_R(0x6a, L) // LD   LX,D
    OPXY_LD_XY_R(0x6b, L) // LD   LX,E
    OPXY_LD_XY_R(0x6f, L) // LD   LX,A

    OPXY(0x65): // LD   HX,LX
        data->B.H = data->B.L;
        RET(4);

    OPXY(0x6c): // LD   LX,HX
        data->B.L = data->B.H;
        RET(4);

#define OPXY_LD_R_n(Opcode) OPJ_LD_R_n(OPXY, Opcode)

    OPXY_LD_R_n(0x06) // LD   B,#imm
    OPXY_LD_R_n(0x0e) // LD   C,#imm
    OPXY_LD_R_n(0x16) // LD   D,#imm
    OPXY_LD_R_n(0x1e) // LD   E,#imm
    OPXY_LD_R_n(0x3e) // LD   A,#imm

    OPXY(0x26): // LD   HX,#imm
        data->B.H = FETCH_BYTE();
        RET(7);

    OPXY(0x2e): // LD   LX,#imm
        data->B.L = FETCH_BYTE();
        RET(7);

#define OPXY_LD_A_mRR(Opcode, RR) OPJ_LD_A_mRR(OPXY, Opcode, RR)

    OPXY_LD_A_mRR(0x0a, BC) // LD   A,(BC)
    OPXY_LD_A_mRR(0x1a, DE) // LD   A,(DE)

    OPXY(0x3a): {  // LD   A,(nn)
        uint16_t adr;
        PRE_IO();
        FETCH_WORD(adr);
        READ_BYTE(adr, zA);
        POST_IO();
        RET(13);
    }

#define OPXY_LD_mRR_A(Opcode, RR) OPJ_LD_mRR_A(OPXY, Opcode, RR)

    OPXY_LD_mRR_A(0x02, BC) // LD   (BC),A
    OPXY_LD_mRR_A(0x12, DE) // LD   (DE),A

    OPXY(0x32): {  // LD   (nn),A
        uint16_t adr;
        PRE_IO();
        FETCH_WORD(adr);
        WRITE_BYTE(adr, zA);
        POST_IO();
        RET(13);
    }

    OPXY(0x46): // LD   B,(Ix+d)
    OPXY(0x56): // LD   D,(Ix+d)
    OPXY(0x5e): // LD   E,(Ix+d)
    OPXY(0x66): // LD   H,(Ix+d)
    OPXY(0x6e): // LD   L,(Ix+d)
    OPXY(0x4e): // LD   C,(Ix+d)
    OPXY(0x7e): // LD   A,(Ix+d)
        PRE_IO();
	CALC_EA_XY_D();
        READ_BYTE(zWZ, zR8((Opcode >> 3) & 7));
        POST_IO();
        RET(15);

    OPXY(0x70): // LD   (Ix+d),B
    OPXY(0x71): // LD   (Ix+d),C
    OPXY(0x72): // LD   (Ix+d),D
    OPXY(0x73): // LD   (Ix+d),E
    OPXY(0x74): // LD   (Ix+d),H
    OPXY(0x75): // LD   (Ix+d),L
    OPXY(0x77): // LD   (Ix+d),A
        PRE_IO();
        CALC_EA_XY_D();
        WRITE_BYTE(zWZ, zR8(Opcode & 7));
        POST_IO();
        RET(15);

    OPXY(0x36): // LD   (Ix+d),#imm
        PRE_IO();
        CALC_EA_XY_D();
        WRITE_BYTE(zWZ, FETCH_BYTE());
        POST_IO();
        RET(15);

    // 16 BITS LOAD

#define OPXY_LD_RR_nn(Opcode) OPJ_LD_RR_nn(OPXY, Opcode)

    OPXY_LD_RR_nn(0x01)   // LD   BC,nn
    OPXY_LD_RR_nn(0x11)   // LD   DE,nn

    OPXY(0x21): // LD   IX,nn
        FETCH_WORD(data->W);
        RET(10);

    OPXY(0x31): // LD   SP,nn
        FETCH_WORD(zSP);
        RET(10);

    OPXY(0x2a):   // LD   IX,(nn)
        PRE_IO();
        FETCH_WORD(zWZ);
        READ_WORD_LE(zWZ, data->W);
	zWZ++;
        POST_IO();
        RET(16);

    OPXY(0x22):   // LD   (nn),IX
        PRE_IO();
        FETCH_WORD(zWZ);
        WRITE_WORD_LE(zWZ, data->W);
	zWZ++;
        POST_IO();
        RET(16);

    OPXY(0xf9): // LD   SP,IX
        zSP = data->W;
        RET(6);

    // PUSH / POP

#define OPXY_POP_AF(Opcode) OPJ_POP_AF(OPXY, Opcode)

    OPXY_POP_AF(0xf1) // POP  AF

#define OPXY_POP_RR(Opcode, RR) OPJ_POP_RR(OPXY, Opcode, RR)

    OPXY_POP_RR(0xc1, BC) // POP  BC
    OPXY_POP_RR(0xd1, DE) // POP  DE

#define OPXY_POP_XY(Opcode) \
    OPXY(Opcode): \
        PRE_IO(); \
        POP_16(data->W); \
        POST_IO(); \
        RET(10);

    OPXY_POP_XY(0xe1) // POP  IX

#define OPXY_PUSH_AF(Opcode) OPJ_PUSH_AF(OPXY, Opcode)

    OPXY_PUSH_AF(0xf5) // PUSH AF

#define OPXY_PUSH_RR(Opcode, RR) OPJ_PUSH_RR(OPXY, Opcode, RR)

    OPXY_PUSH_RR(0xc5, BC) // PUSH BC
    OPXY_PUSH_RR(0xd5, DE) // PUSH DE

#define OPXY_PUSH_XY(Opcode) \
    OPXY(Opcode): \
        PRE_IO(); \
        PUSH_16(data->W); \
        POST_IO(); \
        RET(11);

    OPXY_PUSH_XY(0xe5) // PUSH IX

    // EXCHANGE

    OPXY(0x08): // EX   AF,AF'
        goto OP_EX_AF_AF2;

    OPXY(0xeb): // EX   DE,HL
        goto OP_EX_DE_HL;

    OPXY(0xd9): // EXX
        goto OP_EXX;

    OPXY(0xe3): // EX   (SP),IX
        goto OP_EX_xx_mSP;


    // 8 BITS ARITHMETIC
    // INC

    OPXY(0x04): // INC  B
    OPXY(0x0c): // INC  C
    OPXY(0x14): // INC  D
    OPXY(0x1c): // INC  E
    OPXY(0x3c): // INC  A
        goto OP_INC_R;

    OPXY(0x24): // INC  HX
        data->B.H++;
        zF = (zF & CZ80_CF) | SZXYHV_inc[data->B.H];
        RET(4);

    OPXY(0x2c): // INC  LX
        data->B.L++;
        zF = (zF & CZ80_CF) | SZXYHV_inc[data->B.L];
        RET(4);

    OPXY(0x34): // INC  (IX+o)
        goto OP_INC_mIx;

    // DEC

    OPXY(0x05): // DEC  B
    OPXY(0x0d): // DEC  C
    OPXY(0x15): // DEC  D
    OPXY(0x1d): // DEC  E
    OPXY(0x3d): // DEC  A
        goto OP_DEC_R;

    OPXY(0x25): // DEC  HX
        data->B.H--;
        zF = (zF & CZ80_CF) | SZXYHV_dec[data->B.H];
        RET(4);

    OPXY(0x2d): // DEC  LX
        data->B.L--;
        zF = (zF & CZ80_CF) | SZXYHV_dec[data->B.L];
        RET(4);

    OPXY(0x35): // DEC  (IX+o)
        goto OP_DEC_mIx;

    // ADD

#define OPXY_ADD_A_R(Opcode) OPJ_ADD_A_R(OPXY, Opcode)

    OPXY_ADD_A_R(0x80) // ADD  A,B
    OPXY_ADD_A_R(0x81) // ADD  A,C
    OPXY_ADD_A_R(0x82) // ADD  A,D
    OPXY_ADD_A_R(0x83) // ADD  A,E
    OPXY_ADD_A_R(0x87) // ADD  A,A

#define OPXY_ADD_A_XY(Opcode, HL) \
    OPXY(Opcode): { \
        uint8_t val = data->B.HL; \
        OP_ADD_INT(val, 4); \
    }

    OPXY_ADD_A_XY(0x84, H) // ADD  A,HX
    OPXY_ADD_A_XY(0x85, L) // ADD  A,LX

#define OPXY_ADD_A_n(Opcode) OPJ_ADD_A_n(OPXY, Opcode)

    OPXY_ADD_A_n(0xc6) // ADD  A,n

    OPXY(0x86): { // ADD  A,(Ix+d)
        uint8_t val;
        PRE_IO();
	CALC_EA_XY_D();
        READ_BYTE(zWZ, val);
        POST_IO();
        OP_ADD_INT(val, 15);
    }

    // ADC

#define OPXY_ADC_A_R(Opcode) OPJ_ADC_A_R(OPXY, Opcode)

    OPXY_ADC_A_R(0x88) // ADC  A,B
    OPXY_ADC_A_R(0x89) // ADC  A,C
    OPXY_ADC_A_R(0x8a) // ADC  A,D
    OPXY_ADC_A_R(0x8b) // ADC  A,E
    OPXY_ADC_A_R(0x8f) // ADC  A,A

#define OPXY_ADC_A_XY(Opcode, HL) \
    OPXY(Opcode): { \
        uint8_t val = data->B.HL; \
        OP_ADC_INT(val, 4); \
    }

    OPXY_ADC_A_XY(0x8c, H) // ADC  A,HX
    OPXY_ADC_A_XY(0x8d, L) // ADC  A,LX

#define OPXY_ADC_A_n(Opcode) OPJ_ADC_A_n(OPXY, Opcode)

    OPXY_ADC_A_n(0xce) // ADC  A,n

    OPXY(0x8e): { // ADC  A,(Ix+d)
        uint8_t val;
        PRE_IO();
	CALC_EA_XY_D();
        READ_BYTE(zWZ, val);
        POST_IO();
        OP_ADC_INT(val, 15);
    }

    // SUB

#define OPXY_SUB_A_R(Opcode) OPJ_SUB_A_R(OPXY, Opcode)

    OPXY_SUB_A_R(0x90) // SUB  A,B
    OPXY_SUB_A_R(0x91) // SUB  A,C
    OPXY_SUB_A_R(0x92) // SUB  A,D
    OPXY_SUB_A_R(0x93) // SUB  A,E
    OPXY_SUB_A_R(0x97) // SUB  A,A

#define OPXY_SUB_A_XY(Opcode, HL) \
    OPXY(Opcode): { \
        uint8_t val = data->B.HL; \
        OP_SUB_INT(val, 4); \
    }

    OPXY_SUB_A_XY(0x94, H) // SUB  A,HX
    OPXY_SUB_A_XY(0x95, L) // SUB  A,LX

#define OPXY_SUB_A_n(Opcode) OPJ_SUB_A_n(OPXY, Opcode)

    OPXY_SUB_A_n(0xd6) // SUB  A,n

    OPXY(0x96): { // SUB  A,(Ix+d)
        uint8_t val;
        PRE_IO();
	CALC_EA_XY_D();
        READ_BYTE(zWZ, val);
        POST_IO();
        OP_SUB_INT(val, 15);
    }

    // SBC

#define OPXY_SBC_A_R(Opcode) OPJ_SBC_A_R(OPXY, Opcode)

    OPXY_SBC_A_R(0x98) // SBC  A,B
    OPXY_SBC_A_R(0x99) // SBC  A,C
    OPXY_SBC_A_R(0x9a) // SBC  A,D
    OPXY_SBC_A_R(0x9b) // SBC  A,E
    OPXY_SBC_A_R(0x9f) // SBC  A,A

#define OPXY_SBC_A_XY(Opcode, HL) \
    OPXY(Opcode): { \
        uint8_t val = data->B.HL; \
        OP_SBC_INT(val, 4); \
    }

    OPXY_SBC_A_XY(0x9c, H) // SBC  A,HX
    OPXY_SBC_A_XY(0x9d, L) // SBC  A,LX

#define OPXY_SBC_A_n(Opcode) OPJ_SBC_A_n(OPXY, Opcode)

    OPXY_SBC_A_n(0xde) // SBC  A,n

    OPXY(0x9e): { // SBC  A,(Ix+d)
        uint8_t val;
        PRE_IO();
	CALC_EA_XY_D();
        READ_BYTE(zWZ, val);
        POST_IO();
        OP_SBC_INT(val, 15);
    }

    // CP

#define OPXY_CP_A_R(Opcode) OPJ_CP_A_R(OPXY, Opcode)

    OPXY_CP_A_R(0xb8) // CP   A,B
    OPXY_CP_A_R(0xb9) // CP   A,C
    OPXY_CP_A_R(0xba) // CP   A,D
    OPXY_CP_A_R(0xbb) // CP   A,E
    OPXY_CP_A_R(0xbf) // CP   A,A

#define OPXY_CP_A_XY(Opcode, HL) \
    OPXY(Opcode): { \
        uint8_t val = data->B.HL; \
        OP_CP_INT(val, 4); \
    }

    OPXY_CP_A_XY(0xbc, H) // CP   A,HX
    OPXY_CP_A_XY(0xbd, L) // CP   A,LX

#define OPXY_CP_A_n(Opcode) OPJ_CP_A_n(OPXY, Opcode)

    OPXY_CP_A_n(0xfe) // CP   A,n

    OPXY(0xbe): { // CP  A,(Ix+d)
        uint8_t val;
        PRE_IO();
	CALC_EA_XY_D();
        READ_BYTE(zWZ, val);
        POST_IO();
        OP_CP_INT(val, 15);
    }

    // AND

#define OPXY_AND_A_R(Opcode) OPJ_AND_A_R(OPXY, Opcode)

    OPXY_AND_A_R(0xa0) // AND  A,B
    OPXY_AND_A_R(0xa1) // AND  A,C
    OPXY_AND_A_R(0xa2) // AND  A,D
    OPXY_AND_A_R(0xa3) // AND  A,E

#define OPXY_AND_A_A(Opcode) OPJ_AND_A_A(OPXY, Opcode)

    OPXY_AND_A_A(0xa7) // AND  A,A

#define OPXY_AND_A_XY(Opcode, HL) \
    OPXY(Opcode): { \
        uint8_t val = data->B.HL; \
        OP_AND_INT(val, 4); \
    }

    OPXY_AND_A_XY(0xa4, H) // AND  A,HX
    OPXY_AND_A_XY(0xa5, L) // AND  A,LX

#define OPXY_AND_A_n(Opcode) OPJ_AND_A_n(OPXY, Opcode)

    OPXY_AND_A_n(0xe6) // AND  A,n

    OPXY(0xa6): { // AND  (Ix+d)
        uint8_t val;

        PRE_IO();
	CALC_EA_XY_D();
        READ_BYTE(zWZ, val);
        POST_IO();
        OP_AND_INT(val, 15);
    }

    // XOR

#define OPXY_XOR_A_R(Opcode) OPJ_XOR_A_R(OPXY, Opcode)

    OPXY_XOR_A_R(0xa8) // XOR  A,B
    OPXY_XOR_A_R(0xa9) // XOR  A,C
    OPXY_XOR_A_R(0xaa) // XOR  A,D
    OPXY_XOR_A_R(0xab) // XOR  A,E

#define OPXY_XOR_A_A(Opcode) OPJ_XOR_A_A(OPXY, Opcode)

    OPXY_XOR_A_A(0xaf) // XOR  A,A

#define OPXY_XOR_A_XY(Opcode, HL) \
    OPXY(Opcode): { \
        uint8_t val = data->B.HL; \
        OP_XOR_INT(val, 4); \
    }

    OPXY_XOR_A_XY(0xac, H) // XOR  A,HX
    OPXY_XOR_A_XY(0xad, L) // XOR  A,LX

#define OPXY_XOR_A_n(Opcode) OPJ_XOR_A_n(OPXY, Opcode)

    OPXY_XOR_A_n(0xee) // XOR  A,n

    OPXY(0xae): { // XOR  (Ix+d)
        uint8_t val;

        PRE_IO();
	CALC_EA_XY_D();
        READ_BYTE(zWZ, val);
        POST_IO();
        OP_XOR_INT(val, 15);
    }

    // OR

#define OPXY_OR_A_R(Opcode) OPJ_OR_A_R(OPXY, Opcode)

    OPXY_OR_A_R(0xb0) // OR  A,B
    OPXY_OR_A_R(0xb1) // OR  A,C
    OPXY_OR_A_R(0xb2) // OR  A,D
    OPXY_OR_A_R(0xb3) // OR  A,E

#define OPXY_OR_A_A(Opcode) OPJ_OR_A_A(OPXY, Opcode)

    OPXY_OR_A_A(0xb7) // OR  A,A

#define OPXY_OR_A_XY(Opcode, HL) \
    OPXY(Opcode): { \
        uint8_t val = data->B.HL; \
        OP_OR_INT(val, 4); \
    }

    OPXY_OR_A_XY(0xb4, H) // OR  A,HX
    OPXY_OR_A_XY(0xb5, L) // OR  A,LX

#define OPXY_OR_A_n(Opcode) OPJ_OR_A_n(OPXY, Opcode)

    OPXY_OR_A_n(0xf6) // OR  A,n

    OPXY(0xb6): { // OR  (Ix+d)
        uint8_t val;

        PRE_IO();
	CALC_EA_XY_D();
        READ_BYTE(zWZ, val);
        POST_IO();
        OP_OR_INT(val, 15);
    }

    // MISC ARITHMETIC & CPU CONTROL

    OPXY(0x27): // DAA
        goto OP_DAA;

    OPXY(0x2f): // CPL
        goto OP_CPL;

    OPXY(0x37): // SCF
        goto OP_SCF;

    OPXY(0x3f): // CCF
        goto OP_CCF;

    OPXY(0x76): // HALT
        goto OP_HALT;

    OPXY(0xf3): // DI
        goto OP_DI;

    OPXY(0xfb): // EI
        goto OP_EI;


    // 16 BITS ARITHMETIC

    OPXY(0x03): // INC  BC
        zBC++;
        RET(6);

    OPXY(0x13): // INC  DE
        zDE++;
        RET(6);

    OPXY(0x23): // INC  Ix
        data->W++;
        RET(6);

    OPXY(0x33): // INC  SP
        zSP++;
        RET(6);

    OPXY(0x0b): // DEC  BC
        zBC--;
        RET(6);

    OPXY(0x1b): // DEC  DE
        zDE--;
        RET(6);

    OPXY(0x2b): // DEC  Ix
        data->W--;
        RET(6);

    OPXY(0x3b): // DEC  SP
        zSP--;
        RET(6);

    // ADD16

#define OPXY_ADD_RR_RR(Opcode, RRsrc) OPJ_ADD_RR_RR(OPXY, Opcode, RRsrc)

    OPXY_ADD_RR_RR(0x39, zSP)       // ADD  IX,SP
    OPXY_ADD_RR_RR(0x29, data->W)   // ADD  IX,IX (TODO: Optimization?)
    OPXY_ADD_RR_RR(0x09, zBC)       // ADD  IX,BC
    OPXY_ADD_RR_RR(0x19, zDE)       // ADD  IX,DE

    // ROTATE

    OPXY(0x07): // RLCA
        goto OP_RLCA;

    OPXY(0x0f): // RRCA
        goto OP_RRCA;

    OPXY(0x17): // RLA
        goto OP_RLA;

    OPXY(0x1f): // RRA
        goto OP_RRA;

    // JUMP

#define OPXY_JP_NCOND(Opcode, Cond) OPJ_JP_NCOND(OPXY, Opcode, Cond)
#define OPXY_JP_COND( Opcode, Cond) OPJ_JP_COND( OPXY, Opcode, Cond)

    OPXY_JP_NCOND(0xd2, CZ80_CF)   // JP   NC,nn
    OPXY_JP_COND( 0xda, CZ80_CF)   // JP   C,nn
    OPXY_JP_NCOND(0xe2, CZ80_VF)   // JP   PO,nn
    OPXY_JP_COND( 0xea, CZ80_VF)   // JP   PE,nn
    OPXY_JP_NCOND(0xf2, CZ80_SF)   // JP   P,nn
    OPXY_JP_COND( 0xfa, CZ80_SF)   // JP   M,nn
    OPXY_JP_NCOND(0xc2, CZ80_ZF)   // JP   NZ,nn
    OPXY_JP_COND( 0xca, CZ80_ZF)   // JP   Z,nn

#define OPXY_JP(Opcode) OPJ_JP(OPXY, Opcode)

    OPXY_JP(0xc3) // JP   nn

#define OPXY_JP_mHL(Opcode) OPJ_JP_mHL(OPXY, Opcode)

    OPXY_JP_mHL(0xe9) // JP   (IX)

#define OPXY_JR_NCOND(Opcode, Cond) OPJ_JR_NCOND(OPXY, Opcode, Cond)
#define OPXY_JR_COND( Opcode, Cond) OPJ_JR_COND( OPXY, Opcode, Cond)

    OPXY_JR_NCOND(0x30, CZ80_CF)   // JR   NC,n
    OPXY_JR_COND( 0x38, CZ80_CF)   // JR   C,n
    OPXY_JR_NCOND(0x20, CZ80_ZF)   // JR   NZ,n
    OPXY_JR_COND( 0x28, CZ80_ZF)   // JR   Z,n

#define OPXY_DJNZ(Opcode) OPJ_DJNZ(OPXY, Opcode)

    OPXY_DJNZ(0x10)   // DJNZ n

#define OPXY_JR(Opcode) OPJ_JR(OPXY, Opcode)

    OPXY_JR(0x18)   // JR   n

    // CALL & RETURN

#define OPXY_CALL_NCOND(Opcode, Cond) OPJ_CALL_NCOND(OPXY, Opcode, Cond)
#define OPXY_CALL_COND( Opcode, Cond) OPJ_CALL_COND( OPXY, Opcode, Cond)

    OPXY_CALL_NCOND(0xd4, CZ80_CF)   // CALL NC,nn
    OPXY_CALL_COND( 0xdc, CZ80_CF)   // CALL C,nn
    OPXY_CALL_NCOND(0xe4, CZ80_VF)   // CALL PO,nn
    OPXY_CALL_COND( 0xec, CZ80_VF)   // CALL PE,nn
    OPXY_CALL_NCOND(0xf4, CZ80_SF)   // CALL P,nn
    OPXY_CALL_COND( 0xfc, CZ80_SF)   // CALL M,nn
    OPXY_CALL_NCOND(0xc4, CZ80_ZF)   // CALL NZ,nn
    OPXY_CALL_COND( 0xcc, CZ80_ZF)   // CALL Z,nn

#define OPXY_CALL(Opcode) OPJ_CALL(OPXY, Opcode)

    OPXY_CALL(0xcd) // CALL   nn

#define OPXY_RET_NCOND(Opcode, Cond) OPJ_RET_NCOND(OPXY, Opcode, Cond)
#define OPXY_RET_COND( Opcode, Cond) OPJ_RET_COND( OPXY, Opcode, Cond)

    OPXY_RET_NCOND(0xd0, CZ80_CF)   // RET  NC
    OPXY_RET_COND( 0xd8, CZ80_CF)   // RET  C
    OPXY_RET_NCOND(0xe0, CZ80_VF)   // RET  PO
    OPXY_RET_COND( 0xe8, CZ80_VF)   // RET  PE
    OPXY_RET_NCOND(0xf0, CZ80_SF)   // RET  P
    OPXY_RET_COND( 0xf8, CZ80_SF)   // RET  M
    OPXY_RET_NCOND(0xc0, CZ80_ZF)   // RET  NZ
    OPXY_RET_COND( 0xc8, CZ80_ZF)   // RET  Z

#define OPXY_RET(Opcode) OPJ_RET(OPXY, Opcode)

    OPXY_RET(0xc9)   // RET

#define OPXY_RST(Opcode) OPJ_RST(OPXY, Opcode)

    OPXY_RST(0xc7)   // RST  0
    OPXY_RST(0xcf)   // RST  1
    OPXY_RST(0xd7)   // RST  2
    OPXY_RST(0xdf)   // RST  3
    OPXY_RST(0xe7)   // RST  4
    OPXY_RST(0xef)   // RST  5
    OPXY_RST(0xf7)   // RST  6
    OPXY_RST(0xff)   // RST  7

    // INPUT & OUTPUT

    OPXY(0xd3): // OUT  (n),A
        goto OP_OUT_mN_A;

    OPXY(0xdb): // IN   A,(n)
        goto OP_IN_A_mN;


    // PREFIXE

    OPXY(0xcb): // XYCB PREFIXE
    {
        uint8_t src;
        uint8_t res;

        CALC_EA_XY_D();
        Opcode = FETCH_BYTE();
        #include "cz80_opxycb.inc"
    }

    OPXY(0xed): // ED PREFIXE
        goto ED_PREFIXE;

    OPXY(0xdd): // DD PREFIXE (IX)
        goto DD_PREFIXE;

    OPXY(0xfd): // FD PREFIXE (IY)
        goto FD_PREFIXE;

#if CZ80_USE_JUMPTABLE
#else
}
#endif
