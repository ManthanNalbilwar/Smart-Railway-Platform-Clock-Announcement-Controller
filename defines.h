#ifndef DEFINES_H
#define DEFINES_H

#include "types.h"

/* ---------------- BIT OPERATIONS ---------------- */

#define SETBIT(WORD, BIT) \
    ((WORD) |= (1UL << (BIT)))

#define CLRBIT(WORD, BIT) \
    ((WORD) &= ~(1UL << (BIT)))

#define READBIT(WORD, BIT) \
    (((WORD) >> (BIT)) & 1UL)

#define WRITEBIT(WORD, BIT, BITLEVEL) \
    ((WORD) = ((WORD) & ~(1UL << (BIT))) | \
              (((BITLEVEL) & 1UL) << (BIT)))

/* ---------------- BYTE OPERATIONS ---------------- */

#define WRITEBYTE(WORD, STARTBIT, BYTE) \
    ((WORD) = ((WORD) & ~(0xFFUL << (STARTBIT))) | \
              (((u32)(BYTE) & 0xFFUL) << (STARTBIT)))

#define READBYTE(WORD, STARTBIT) \
    (((WORD) >> (STARTBIT)) & 0xFFUL)

/* ---------------- NIBBLE OPERATIONS ---------------- */

#define WRITENIBBLE(WORD, BITPOS, VALUE) \
    ((WORD) = ((WORD) & ~(0x0FUL << (BITPOS))) | \
              (((u32)(VALUE) & 0x0FUL) << (BITPOS)))

#define READNIBBLE(WORD, BITPOS) \
    (((WORD) >> (BITPOS)) & 0x0FUL)

/* ---------------- BIT COPY ---------------- */
/*
 * Copy source bit SBIT into destination bit DBIT.
 */
#define READWRITEBIT(WORD, SBIT, DBIT) \
    ((WORD) = ((WORD) & ~(1UL << (DBIT))) | \
              ((((WORD) >> (SBIT)) & 1UL) << (DBIT)))

#endif
