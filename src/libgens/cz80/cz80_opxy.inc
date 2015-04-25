/********************************************************************************/
/*                                                                              */
/* CZ80 XY opcode include source file                                           */
/* C Z80 emulator version 0.91                                                  */
/* Copyright 2004-2005 Stephane Dallongeville                                   */
/*                                                                              */
/********************************************************************************/

#if CZ80_USE_JUMPTABLE
    goto *JumpTableXY[Opcode];
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
        goto OP_NOP;

#define OPXY_LD_R_R_FN(Opcode) OPJ_LD_R_R_FN(OPXY, Opcode)

    OPXY_LD_R_R_FN(0x41) // LD   B,C
    OPXY_LD_R_R_FN(0x42) // LD   B,D
    OPXY_LD_R_R_FN(0x43) // LD   B,E
    OPXY_LD_R_R_FN(0x47) // LD   B,A

    OPXY_LD_R_R_FN(0x48) // LD   C,B
    OPXY_LD_R_R_FN(0x4a) // LD   C,D
    OPXY_LD_R_R_FN(0x4b) // LD   C,E
    OPXY_LD_R_R_FN(0x4f) // LD   C,A

    OPXY_LD_R_R_FN(0x50) // LD   D,B
    OPXY_LD_R_R_FN(0x51) // LD   D,C
    OPXY_LD_R_R_FN(0x53) // LD   D,E
    OPXY_LD_R_R_FN(0x57) // LD   D,A

    OPXY_LD_R_R_FN(0x58) // LD   E,B
    OPXY_LD_R_R_FN(0x59) // LD   E,C
    OPXY_LD_R_R_FN(0x5a) // LD   E,D
    OPXY_LD_R_R_FN(0x5f) // LD   E,A

    OPXY_LD_R_R_FN(0x78) // LD   A,B
    OPXY_LD_R_R_FN(0x79) // LD   A,C
    OPXY_LD_R_R_FN(0x7a) // LD   A,D
    OPXY_LD_R_R_FN(0x7b) // LD   A,E

#define OPXY_LD_R_XY_FN(Opcode, HL) \
    OPXY(Opcode): \
        zR8((Opcode >> 3) & 7) = data->B.HL; \
	RET(4)

    // TODO: Add byte indexes to data and extract the index from Opcode?
    OPXY_LD_R_XY_FN(0x44, H) // LD   B,HX
    OPXY_LD_R_XY_FN(0x4c, H) // LD   C,HX
    OPXY_LD_R_XY_FN(0x54, H) // LD   D,HX
    OPXY_LD_R_XY_FN(0x5c, H) // LD   E,HX
    OPXY_LD_R_XY_FN(0x7c, H) // LD   A,HX

    OPXY_LD_R_XY_FN(0x45, L) // LD   B,LX
    OPXY_LD_R_XY_FN(0x4d, L) // LD   C,LX
    OPXY_LD_R_XY_FN(0x55, L) // LD   D,LX
    OPXY_LD_R_XY_FN(0x5d, L) // LD   E,LX
    OPXY_LD_R_XY_FN(0x7d, L) // LD   A,LX

#define OPXY_LD_XY_R_FN(Opcode, HL) \
    OPXY(Opcode): \
        data->B.HL = zR8(Opcode & 7); \
	RET(4)

    OPXY_LD_XY_R_FN(0x60, H) // LD   HX,B
    OPXY_LD_XY_R_FN(0x61, H) // LD   HX,C
    OPXY_LD_XY_R_FN(0x62, H) // LD   HX,D
    OPXY_LD_XY_R_FN(0x63, H) // LD   HX,E
    OPXY_LD_XY_R_FN(0x67, H) // LD   HX,A

    OPXY_LD_XY_R_FN(0x68, L) // LD   LX,B
    OPXY_LD_XY_R_FN(0x69, L) // LD   LX,C
    OPXY_LD_XY_R_FN(0x6a, L) // LD   LX,D
    OPXY_LD_XY_R_FN(0x6b, L) // LD   LX,E
    OPXY_LD_XY_R_FN(0x6f, L) // LD   LX,A

    OPXY(0x65): // LD   HX,LX
        data->B.H = data->B.L;
        RET(4)

    OPXY(0x6c): // LD   LX,HX
        data->B.L = data->B.H;
        RET(4)

#define OPXY_LD_R_n_FN(Opcode) OPJ_LD_R_n_FN(OPXY, Opcode)

    OPXY_LD_R_n_FN(0x06) // LD   B,#imm
    OPXY_LD_R_n_FN(0x0e) // LD   C,#imm
    OPXY_LD_R_n_FN(0x16) // LD   D,#imm
    OPXY_LD_R_n_FN(0x1e) // LD   E,#imm
    OPXY_LD_R_n_FN(0x3e) // LD   A,#imm

    OPXY(0x26): // LD   HX,#imm
        data->B.H = FETCH_BYTE;
        RET(7)

    OPXY(0x2e): // LD   LX,#imm
        data->B.L = FETCH_BYTE;
        RET(7)

#define OPXY_LD_A_mRR_FN(Opcode, RR) OPJ_LD_A_mRR_FN(OPXY, Opcode, RR)

    OPXY_LD_A_mRR_FN(0x0a, BC) // LD   A,(BC)
    OPXY_LD_A_mRR_FN(0x1a, DE) // LD   A,(DE)

    OPXY(0x3a): {  // LD   A,(nn)
        uint16_t adr;
        PRE_IO
        FETCH_WORD(adr)
        READ_BYTE(adr, zA)
        POST_IO
        RET(13)
    }

#define OPXY_LD_mRR_A_FN(Opcode, RR) OPJ_LD_mRR_A_FN(OPXY, Opcode, RR)

    OPXY_LD_mRR_A_FN(0x02, BC) // LD   (BC),A
    OPXY_LD_mRR_A_FN(0x12, DE) // LD   (DE),A

    OPXY(0x32): {  // LD   (nn),A
        uint16_t adr;
        PRE_IO
        FETCH_WORD(adr)
        WRITE_BYTE(adr, zA)
        POST_IO
        RET(13)
    }

    OPXY(0x46): // LD   B,(IX+d)
    OPXY(0x56): // LD   D,(IX+d)
    OPXY(0x5e): // LD   E,(IX+d)
    OPXY(0x66): // LD   H,(IX+d)
    OPXY(0x6e): // LD   L,(IX+d)
    OPXY(0x4e): // LD   C,(IX+d)
    OPXY(0x7e): // LD   A,(IX+d)
        PRE_IO
	CALC_EA_XY_D();
        READ_BYTE(zWZ, zR8((Opcode >> 3) & 7))
        POST_IO
        RET(15)

    OPXY(0x70): // LD   (IX+d),B
    OPXY(0x71): // LD   (IX+d),C
    OPXY(0x72): // LD   (IX+d),D
    OPXY(0x73): // LD   (IX+d),E
    OPXY(0x74): // LD   (IX+d),H
    OPXY(0x75): // LD   (IX+d),L
    OPXY(0x77): // LD   (IX+d),A
        PRE_IO
        CALC_EA_XY_D();
        WRITE_BYTE(zWZ, zR8(Opcode & 7))
        POST_IO
        RET(15)

    OPXY(0x36): // LD   (IX+d),#imm
        PRE_IO
        CALC_EA_XY_D();
        WRITE_BYTE(zWZ, FETCH_BYTE)
        POST_IO
        RET(15)

    // 16 BITS LOAD

#define OPXY_LD_RR_nn_FN(Opcode) OPJ_LD_RR_nn_FN(OPXY, Opcode)

    OPXY_LD_RR_nn_FN(0x01)   // LD   BC,nn
    OPXY_LD_RR_nn_FN(0x11)   // LD   DE,nn

    OPXY(0x21): // LD   IX,nn
        FETCH_WORD(data->W)
        RET(10)

    OPXY(0x31): // LD   SP,nn
        FETCH_WORD(zSP)
        RET(10)

    OPXY(0x2a):   // LD   IX,(nn)
        PRE_IO
        FETCH_WORD(zWZ)
        READ_WORD_LE(zWZ, data->W)
	zWZ++;
        POST_IO
        RET(16)

    OPXY(0x22):   // LD   (nn),IX
        PRE_IO
        FETCH_WORD(zWZ)
        WRITE_WORD_LE(zWZ, data->W)
	zWZ++;
        POST_IO
        RET(16)

    OPXY(0xf9): // LD   SP,IX
        zSP = data->W;
        RET(6)

    // PUSH / POP

#define OPXY_POP_AF(Opcode) OPJ_POP_AF(OPXY, Opcode)

    OPXY_POP_AF(0xf1) // POP  AF

#define OPXY_POP_RR(Opcode, RR) OPJ_POP_RR(OPXY, Opcode, RR)

    OPXY_POP_RR(0xc1, BC) // POP  BC
    OPXY_POP_RR(0xd1, DE) // POP  DE

#define OPXY_POP_XY(Opcode) \
    OPXY(Opcode): \
        PRE_IO \
        POP_16(data->W) \
        POST_IO \
        RET(10)

    OPXY_POP_XY(0xe1) // POP  IX

#define OPXY_PUSH_AF(Opcode) OPJ_PUSH_AF(OPXY, Opcode)

    OPXY_PUSH_AF(0xf5) // PUSH AF

#define OPXY_PUSH_RR(Opcode, RR) OPJ_PUSH_RR(OPXY, Opcode, RR)

    OPXY_PUSH_RR(0xc5, BC) // PUSH BC
    OPXY_PUSH_RR(0xd5, DE) // PUSH DE

#define OPXY_PUSH_XY(Opcode) \
    OPXY(Opcode): \
        PRE_IO \
        PUSH_16(data->W); \
        POST_IO \
        RET(11)

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
        RET(4)

    OPXY(0x2c): // INC  LX
        data->B.L++;
        zF = (zF & CZ80_CF) | SZXYHV_inc[data->B.L];
        RET(4)

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
        RET(4)

    OPXY(0x2d): // DEC  LX
        data->B.L--;
        zF = (zF & CZ80_CF) | SZXYHV_dec[data->B.L];
        RET(4)

    OPXY(0x35): // DEC  (IX+o)
        goto OP_DEC_mIx;


    // ADD

    OPXY(0x80): // ADD  A,B
    OPXY(0x81): // ADD  A,C
    OPXY(0x82): // ADD  A,D
    OPXY(0x83): // ADD  A,E
    OPXY(0x87): // ADD  A,A
        goto OP_ADD_R;

    OPXY(0x84): // ADD  A,HX
        goto OP_ADD_IxH;

    OPXY(0x85): // ADD  A,LX
        goto OP_ADD_IxL;

    OPXY(0xc6): // ADD  A,n
        goto OP_ADD_imm;

    OPXY(0x86): // ADD  A,(IX+o)
        goto OP_ADD_mIx;

    // ADC

    OPXY(0x88): // ADC  A,B
    OPXY(0x89): // ADC  A,C
    OPXY(0x8a): // ADC  A,D
    OPXY(0x8b): // ADC  A,E
    OPXY(0x8f): // ADC  A,A
        goto OP_ADC_R;

    OPXY(0xce): // ADC  A,n
        goto OP_ADC_imm;

    OPXY(0x8c): // ADC  A,HX
        goto OP_ADC_IxH;

    OPXY(0x8d): // ADC  A,LX
        goto OP_ADC_IxL;

    OPXY(0x8e): // ADC  A,(IX+o)
        goto OP_ADC_mIx;

    // SUB

    OPXY(0x90): // SUB  B
    OPXY(0x91): // SUB  C
    OPXY(0x92): // SUB  D
    OPXY(0x93): // SUB  E
    OPXY(0x97): // SUB  A
        goto OP_SUB_R;

    OPXY(0x94): // SUB  HX
        goto OP_SUB_IxH;

    OPXY(0x95): // SUB  LX
        goto OP_SUB_IxL;

    OPXY(0xd6): // SUB  A,n
        goto OP_SUB_imm;

    OPXY(0x96): // SUB  (IX+o)
        goto OP_SUB_mIx;

    // SBC

    OPXY(0x98): // SBC  A,B
    OPXY(0x99): // SBC  A,C
    OPXY(0x9a): // SBC  A,D
    OPXY(0x9b): // SBC  A,E
    OPXY(0x9f): // SBC  A,A
        goto OP_SBC_R;

    OPXY(0x9c): // SBC  A,HX
        goto OP_SBC_IxH;

    OPXY(0x9d): // SBC  A,LX
        goto OP_SBC_IxL;

    OPXY(0xde): // SBC  A,n
        goto OP_SBC_imm;

    OPXY(0x9e): // SBC  A,(IX+o)
        goto OP_SBC_mIx;

    // CP

    OPXY(0xb8): // CP   B
    OPXY(0xb9): // CP   C
    OPXY(0xba): // CP   D
    OPXY(0xbb): // CP   E
    OPXY(0xbf): // CP   A
        goto OP_CP_R;

    OPXY(0xbc): // CP   HX
        goto OP_CP_IxH;

    OPXY(0xbd): // CP   LX
        goto OP_CP_IxL;

    OPXY(0xfe): // CP   n
        goto OP_CP_imm;

    OPXY(0xbe): // CP   (IX+o)
        goto OP_CP_mIx;

    // AND

    OPXY(0xa0): // AND  B
    OPXY(0xa1): // AND  C
    OPXY(0xa2): // AND  D
    OPXY(0xa3): // AND  E
        goto OP_AND_R;

    OPXY(0xa7): // AND  A
        goto OP_AND_A;

    OPXY(0xa4): // AND  HX
        goto OP_AND_IxH;

    OPXY(0xa5): // AND  LX
        goto OP_AND_IxL;

    OPXY(0xe6): // AND  A,n
        goto OP_AND_imm;

    OPXY(0xa6): // AND  (IX+o)
    {
        uint8_t val;

        PRE_IO
        CALC_EA_XY_D();
        READ_BYTE(zWZ, val)
        POST_IO
        zA = zA & val;
        zF = SZXYP[zA] | CZ80_HF;
        RET(15)
    }

    // XOR

    OPXY(0xa8): // XOR  B
    OPXY(0xa9): // XOR  C
    OPXY(0xaa): // XOR  D
    OPXY(0xab): // XOR  E
        goto OP_XOR_R;

    OPXY(0xaf): // XOR  A
        goto OP_XOR_A;

    OPXY(0xac): // XOR  HX
        goto OP_XOR_IxH;

    OPXY(0xad): // XOR  LX
        goto OP_XOR_IxL;

    OPXY(0xee): // XOR  A,n
        goto OP_XOR_imm;

    OPXY(0xae): // XOR  (IX+o)
    {
        uint8_t val;

        PRE_IO
        CALC_EA_XY_D();
        READ_BYTE(zWZ, val)
        POST_IO
        zA = zA ^ val;
        zF = SZXYP[zA];
        RET(15)
    }

    // OR

    OPXY(0xb0): // OR   B
    OPXY(0xb1): // OR   C
    OPXY(0xb2): // OR   D
    OPXY(0xb3): // OR   E
        goto OP_OR_R;

    OPXY(0xb7): // OR   A
        goto OP_OR_A;

    OPXY(0xb4): // OR   HX
        goto OP_OR_IxH;

    OPXY(0xb5): // OR   LX
        goto OP_OR_IxL;

    OPXY(0xf6): // OR   A,n
        goto OP_OR_imm;

    OPXY(0xb6): // OR   (IX+o)
    {
        uint8_t val;

        PRE_IO
        CALC_EA_XY_D();
        READ_BYTE(zWZ, val)
        POST_IO
        zA = zA | val;
        zF = SZXYP[zA];
        RET(15)
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
        goto OP_INC_BC;

    OPXY(0x13): // INC  DE
        goto OP_INC_DE;

    OPXY(0x23): // INC  IX
        goto OP_INC_xx;

    OPXY(0x33): // INC  SP
        goto OP_INC_SP;

    OPXY(0x0b): // DEC  BC
        goto OP_DEC_BC;

    OPXY(0x1b): // DEC  DE
        goto OP_DEC_DE;

    OPXY(0x2b): // DEC  IX
        goto OP_DEC_xx;

    OPXY(0x3b): // DEC  SP
        goto OP_DEC_SP;

    // ADD16

    OPXY(0x09): // ADD  IX,BC
        goto OP_ADD16_xx_BC;

    OPXY(0x19): // ADD  IX,DE
        goto OP_ADD16_xx_DE;

    OPXY(0x29): // ADD  IX,IX
        goto OP_ADD16_xx_xx;

    OPXY(0x39): // ADD  IX,SP
        goto OP_ADD16_xx_SP;


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

    OPXY(0xc3): // JP   nn
        goto OP_JP;

    OPXY(0xe9): // JP   (IX)
        goto OP_JP_xx;


    OPXY(0xd2): // JP   NC,nn
        goto OP_JP_NC;

    OPXY(0xda): // JP   C,nn
        goto OP_JP_C;

    OPXY(0xe2): // JP   PO,nn
        goto OP_JP_PO;

    OPXY(0xea): // JP   PE,nn
        goto OP_JP_PE;

    OPXY(0xf2): // JP   P,nn
        goto OP_JP_P;

    OPXY(0xfa): // JP   M,nn
        goto OP_JP_M;

    OPXY(0xca): // JP   Z,nn
        goto OP_JP_Z;

    OPXY(0xc2): // JP   NZ,nn
        goto OP_JP_NZ;


    OPXY(0x18): // JR   n
        goto OP_JR;


    OPXY(0x38): // JR   C,n
        goto OP_JR_C;

    OPXY(0x30): // JR   NC,n
        goto OP_JR_NC;

    OPXY(0x28): // JR   Z,n
        goto OP_JR_Z;

    OPXY(0x20): // JR   NZ,n
        goto OP_JR_NZ;


    OPXY(0x10): // DJNZ n
        goto OP_DJNZ;


    // CALL & RETURN

    OPXY(0xcd): // CALL nn
        goto OP_CALL;


    OPXY(0xd4): // CALL NC,nn
        goto OP_CALL_NC;

    OPXY(0xdc): // CALL C,nn
        goto OP_CALL_C;

    OPXY(0xe4): // CALL PO,nn
        goto OP_CALL_PO;

    OPXY(0xec): // CALL PE,nn
        goto OP_CALL_PE;

    OPXY(0xf4): // CALL P,nn
        goto OP_CALL_P;

    OPXY(0xfc): // CALL M,nn
        goto OP_CALL_M;

    OPXY(0xcc): // CALL Z,nn
        goto OP_CALL_Z;

    OPXY(0xc4): // CALL NZ,nn
        goto OP_CALL_NZ;


    OPXY(0xc9): // RET
        goto OP_RET;


    OPXY(0xd0): // RET  NC
        goto OP_RET_NC;

    OPXY(0xd8): // RET  C
        goto OP_RET_C;

    OPXY(0xe0): // RET  PO
        goto OP_RET_PO;

    OPXY(0xe8): // RET  PE
        goto OP_RET_PE;

    OPXY(0xf0): // RET  P
        goto OP_RET_P;

    OPXY(0xf8): // RET  M
        goto OP_RET_M;

    OPXY(0xc0): // RET  NZ
        goto OP_RET_NZ;

    OPXY(0xc8): // RET  Z
        goto OP_RET_Z;


    OPXY(0xc7): // RST  0
    OPXY(0xcf): // RST  1
    OPXY(0xd7): // RST  2
    OPXY(0xdf): // RST  3
    OPXY(0xe7): // RST  4
    OPXY(0xef): // RST  5
    OPXY(0xf7): // RST  6
    OPXY(0xff): // RST  7
        goto OP_RST;


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
        Opcode = FETCH_BYTE;
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
