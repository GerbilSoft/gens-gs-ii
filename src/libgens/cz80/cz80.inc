/********************************************************************************/
/*                                                                              */
/* CZ80 macro file                                                              */
/* C Z80 emulator version 0.91                                                  */
/* Copyright 2004-2005 Stephane Dallongeville                                   */
/*                                                                              */
/********************************************************************************/

#if CZ80_USE_JUMPTABLE
#define _SSOP(A,B) A##B
#define OP(A) _SSOP(OP,A)
#define OPCB(A) _SSOP(OPCB,A)
#define OPED(A) _SSOP(OPED,A)
#define OPXY(A) _SSOP(OPXY,A)
#define OPXYCB(A) _SSOP(OPXYCB,A)
#else
#define OP(A) case A
#define OPCB(A) case A
#define OPED(A) case A
#define OPXY(A) case A
#define OPXYCB(A) case A
#endif

/**
 * Get the byte at the program counter.
 */
#define GET_BYTE() \
	(((uint8_t *)CPU->BasePC)[PC])

/**
 * Get the signed byte at the program counter.
 */
#define GET_BYTE_S() \
	(((int8_t *)CPU->BasePC)[PC])

/**
 * Get the word at the program counter.
 */
#define GET_WORD() \
	(((uint8_t *)CPU->BasePC)[PC] |				\
	 (((uint8_t *)CPU->BasePC)[((PC + 1) & 0xffff)] << 8))

/**
 * Get the byte at the program counter,
 * then increment the program counter.
 */
#define FETCH_BYTE() \
	(((uint8_t *)CPU->BasePC)[PC++])

/**
 * Get the signed byte at the program counter,
 * then increment the program counter.
 */
#define FETCH_BYTE_S() \
	(((int8_t *)CPU->BasePC)[PC++])

/**
 * Get the word at the program counter,
 * then increment the program counter.
 * TODO: Return the value directly instead of requiring a parameter?
 */
#define FETCH_WORD(A) do { \
    A = GET_WORD(); \
    PC += 2; \
} while (0)

/**
 * Calculate the effective address when using (XY+d) addressing.
 * This updates the WZ register.
 */
#define CALC_EA_XY_D() do { \
    zWZ = data->W + FETCH_BYTE_S(); \
} while (0)

#if CZ80_SIZE_OPT
    #define RET(A) do {         \
        CCnt -= A;              \
        goto Cz80_Exec_Check;   \
    } while (0)
#else
    #define RET(A) do {         \
        if ((CCnt -= A) <= 0) goto Cz80_Exec_End;  \
        goto Cz80_Exec; \
    } while (0)
#endif

#define SET_PC(A) do { \
    CPU->BasePC = (uintptr_t) CPU->Fetch[(A) >> CZ80_FETCH_SFT];  \
    PC = ((A) & 0xffff); \
} while (0)

#define PRE_IO() do { \
    CPU->CycleIO = CCnt; \
} while (0)

#define POST_IO() do { \
    CCnt = CPU->CycleIO; \
} while (0)

#define READ_BYTE(A, D) do { \
    D = CPU->Read_Byte(CPU->ctx, (A)); \
} while (0)

#if CZ80_USE_WORD_HANDLER
#define READ_WORD(A, D) do { \
    D = CPU->Read_Word(CPU->ctx, (A)); \
} while (0)
#define READ_WORD_LE(A, D) READ_WORD(A, D)
#elif CZ80_LITTLE_ENDIAN
#define READ_WORD(A, D) do { \
    D = CPU->Read_Byte(CPU->ctx, (A)) | (CPU->Read_Byte(CPU->ctx, ((A) + 1)) << 8); \
} while (0)
#define READ_WORD_LE(A, D) READ_WORD(A, D)
#else
#define READ_WORD(A, D) do { \
    D = (CPU->Read_Byte(CPU->ctx, (A)) << 8) | \
         CPU->Read_Byte(CPU->ctx, ((A) + 1)); \
} while (0)
#define READ_WORD_LE(A, D) do { \
    D = CPU->Read_Byte(CPU->ctx, (A)) | (CPU->Read_Byte(CPU->ctx, ((A) + 1)) << 8); \
} while (0)
#endif

#define READSX_BYTE(A, D) do { \
    D = CPU->Read_Byte(CPU->ctx, (A)); \
} while (0)

#define WRITE_BYTE(A, D) do { \
    CPU->Write_Byte(CPU->ctx, (A), (D)); \
} while (0)

#if CZ80_USE_WORD_HANDLER
#define WRITE_WORD(A, D) do { \
    CPU->Write_Word(CPU->ctx, (A), (D)); \
} while (0)
#define WRITE_WORD_LE(A, D) WRITE_WORD(A, D);
#elif CZ80_LITTLE_ENDIAN
#define WRITE_WORD(A, D) do { \
    CPU->Write_Byte(CPU->ctx, (A), (uint8_t)((D) & 0xFF)); \
    CPU->Write_Byte(CPU->ctx, ((A) + 1), (uint8_t)((D) >> 8)); \
} while (0)
#define WRITE_WORD_LE(A, D) WRITE_WORD(A, D);
#else
#define WRITE_WORD(A, D) do { \
    CPU->Write_Byte(CPU->ctx, (A), (uint8_t)((D) >> 8)); \
    CPU->Write_Byte(CPU->ctx, ((A) + 1), (uint8_t)((D) & 0xFF)); \
} while (0)
#define WRITE_WORD_LE(A, D) do { \
    CPU->Write_Byte(CPU->ctx, (A), (uint8_t)((D) & 0xFF))); \
    CPU->Write_Byte(CPU->ctx, ((A) + 1), (uint8_t)((D) >> 8)); \
} while (0)
#endif

#define PUSH_16(A) do { \
    uint16_t sp; \
    zSP -= 2; \
    sp = zSP; \
    WRITE_WORD_LE(sp, A); \
} while (0)

#define POP_16(A) do { \
    uint16_t sp; \
    sp = zSP; \
    READ_WORD_LE(sp, A); \
    zSP = sp + 2; \
} while (0)

#define IN(A, D) do { \
    D = CPU->IN_Port(CPU->ctx, (A)); \
} while (0)

#define OUT(A, D) do { \
    CPU->OUT_Port(CPU->ctx, (A), (D)); \
} while (0)

#define CHECK_INT() do { \
    if (CPU->Status & (zIFF1 | CZ80_HAS_NMI))               \
    {                                                       \
        if (CPU->Status & CZ80_HAS_NMI)                     \
        {                                                   \
            /* NMI */                                       \
            CPU->Status &= ~(CZ80_HALTED | CZ80_HAS_NMI);   \
            zIFF1 = 0;                                      \
            zWZ = 0x66;                                     \
        }                                                   \
        else                                                \
        {                                                   \
            /* MI */                                        \
            CPU->Status &= ~(CZ80_HALTED | CZ80_HAS_INT);   \
            zIFF= 0;                                        \
                                                            \
            if (zIM == 1) zWZ = 0x38;                       \
            else                                            \
            {                                               \
                uint16_t adr;                               \
                                                            \
                Opcode = CPU->Interrupt_Ack(CPU->ctx, CPU->IntVect) & 0xFF; \
                if (zIM == 0) goto Cz80_Exec_IM0;           \
                                                            \
                adr = Opcode | (zI << 8);                   \
                READ_WORD(adr, zWZ);                        \
                CCnt -= 8;                                  \
            }                                               \
        }                                                   \
                                                            \
        {                                                   \
            uint16_t src = PC;                              \
                                                            \
            PUSH_16(src);                                   \
            SET_PC(zWZ);                                    \
            CCnt -= 11;                                     \
        }                                                   \
    }                                                       \
} while (0)
