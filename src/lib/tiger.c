/* (PD) 2001 The Bitzi Corporation
 * Please see file COPYING or http://bitzi.com/publicdomain 
 * for more info.
 *
 * Created and released into the public domain by Eli Biham
 *
 * $Id$
 */

#include "tiger.h"

/* The following macro denotes that an optimization    */
/* for 32-bit machines is desired. It is used only for */
/* optimization of time. Otherwise it does nothing.    */
#if (ULONG_MAX == 0xffffffffUL)
#define OPTIMIZE_FOR_32BIT 1
#endif

/* NOTE that this code is NOT FULLY OPTIMIZED for any  */
/* machine. Assembly code might be much faster on some */
/* machines, especially if the code is compiled with   */
/* gcc.                                                */

/* The number of passes of the hash function.          */
/* Three passes are recommended.                       */
/* Use four passes when you need extra security.       */
/* Must be at least three.                             */
#define PASSES 3

#include "tiger_sboxes.h"

#define U64_FROM_2xU32(hi, lo) (((uint64_t) (hi) << 32) | (lo))

#define t1 (tiger_sboxes)
#define t2 (&tiger_sboxes[256])
#define t3 (&tiger_sboxes[256*2])
#define t4 (&tiger_sboxes[256*3])

#define save_abc \
      aa = a; \
      bb = b; \
      cc = c;

#if !defined(OPTIMIZE_FOR_32BIT)
/* This is the official definition of round */
#define round(a,b,c,x,mul) \
      c ^= x; \
      a -= t1[((c)>>(0*8))&0xFF] ^ t2[((c)>>(2*8))&0xFF] ^ \
	   t3[((c)>>(4*8))&0xFF] ^ t4[((c)>>(6*8))&0xFF] ; \
      b += t4[((c)>>(1*8))&0xFF] ^ t3[((c)>>(3*8))&0xFF] ^ \
	   t2[((c)>>(5*8))&0xFF] ^ t1[((c)>>(7*8))&0xFF] ; \
      b *= mul;
#else
/* This code works faster when compiled on 32-bit machines */
/* (but works slower on Alpha) */
#define round(a,b,c,x,mul) \
      c ^= x; \
      a -= t1[(uint8_t)(c)] ^ \
           t2[(uint8_t)(((uint32_t)(c))>>(2*8))] ^ \
	   t3[(uint8_t)((c)>>(4*8))] ^ \
           t4[(uint8_t)(((uint32_t)((c)>>(4*8)))>>(2*8))] ; \
      b += t4[(uint8_t)(((uint32_t)(c))>>(1*8))] ^ \
           t3[(uint8_t)(((uint32_t)(c))>>(3*8))] ^ \
	   t2[(uint8_t)(((uint32_t)((c)>>(4*8)))>>(1*8))] ^ \
           t1[(uint8_t)(((uint32_t)((c)>>(4*8)))>>(3*8))]; \
      b *= mul;
#endif

#define pass(a,b,c,mul) \
      round(a,b,c,x0,mul) \
      round(b,c,a,x1,mul) \
      round(c,a,b,x2,mul) \
      round(a,b,c,x3,mul) \
      round(b,c,a,x4,mul) \
      round(c,a,b,x5,mul) \
      round(a,b,c,x6,mul) \
      round(b,c,a,x7,mul)

#define key_schedule \
      x0 -= x7 ^ U64_FROM_2xU32(0xA5A5A5A5UL, 0xA5A5A5A5UL); \
      x1 ^= x0; \
      x2 += x1; \
      x3 -= x2 ^ ((~x1)<<19); \
      x4 ^= x3; \
      x5 += x4; \
      x6 -= x5 ^ ((~x4)>>23); \
      x7 ^= x6; \
      x0 += x7; \
      x1 -= x0 ^ ((~x7)<<19); \
      x2 ^= x1; \
      x3 += x2; \
      x4 -= x3 ^ ((~x2)>>23); \
      x5 ^= x4; \
      x6 += x5; \
      x7 -= x6 ^ U64_FROM_2xU32(0x01234567UL,  0x89ABCDEFUL);

#define feedforward \
      a ^= aa; \
      b -= bb; \
      c += cc;

#if !defined(OPTIMIZE_FOR_32BIT)
/* The loop is unrolled: works better on Alpha */
#define compress \
      save_abc \
      pass(a,b,c,5) \
      key_schedule \
      pass(c,a,b,7) \
      key_schedule \
      pass(b,c,a,9) \
      for(pass_no=3; pass_no<PASSES; pass_no++) { \
        key_schedule \
	pass(a,b,c,9) \
	tmpa=a; a=c; c=b; b=tmpa;} \
      feedforward
#else
/* loop: works better on PC and Sun (smaller cache?) */
#define compress \
      save_abc \
      for(pass_no=0; pass_no<PASSES; pass_no++) { \
        if(pass_no != 0) {key_schedule} \
	pass(a,b,c,(pass_no==0?5:pass_no==1?7:9)); \
	tmpa=a; a=c; c=b; b=tmpa;} \
      feedforward
#endif

#define tiger_compress_macro(str, state) \
{ \
  uint64_t a, b, c, tmpa; \
  uint64_t aa, bb, cc; \
  uint64_t x0, x1, x2, x3, x4, x5, x6, x7; \
  int pass_no; \
\
  a = state[0]; \
  b = state[1]; \
  c = state[2]; \
\
  x0=str[0]; x1=str[1]; x2=str[2]; x3=str[3]; \
  x4=str[4]; x5=str[5]; x6=str[6]; x7=str[7]; \
\
  compress; \
\
  state[0] = a; \
  state[1] = b; \
  state[2] = c; \
}

/* The compress function is a function. Requires smaller cache?    */
static inline void
tiger_compress(const uint64_t *data, uint64_t state[3])
{
  tiger_compress_macro(data, state);
}

void
tiger(const void *data, uint64_t length, char hash[24])
{
  uint64_t i, j, res[3];
  const uint8_t *data_u8 = data;
  union {
    uint64_t u64[8];
    uint8_t u8[64];
  } temp;

  res[0] = U64_FROM_2xU32(0x01234567UL, 0x89ABCDEFUL);
  res[1] = U64_FROM_2xU32(0xFEDCBA98UL, 0x76543210UL);
  res[2] = U64_FROM_2xU32(0xF096A5B4UL, 0xC3B2E187UL);

#ifdef HAVE_BIG_ENDIAN
  for (i = length; i >= 64; i -= 64) {
    for (j = 0; j < 64; j++) {
      temp.u8[j ^ 7] = data_u8[j];
    }
    tiger_compress(temp.u64, res);
    data_u8 += 64;
  }
#else	/* !HAVE_BIG_ENDIAN */
  if (PTR2UINT(data) & 7) {
    for (i = length; i >= 64; i -= 64) {
      memcpy(temp.u64, data_u8, 64);
      tiger_compress(temp.u64, res);
      data_u8 += 64;
    }
  } else {
    for (i = length; i >= 64; i -= 64) {
      tiger_compress((const void *) data_u8, res);
      data_u8 += 64;
    }
  }
#endif	/* HAVE_BIG_ENDIAN */


#ifdef HAVE_BIG_ENDIAN
  for (j = 0; j < i; j++) {
    temp.u8[j ^ 7] = data_u8[j];
  }

  temp.u8[j ^ 7] = 0x01;
  j++;
  for (; j & 7; j++) {
    temp.u8[j ^ 7] = 0;
  }
#else
  for(j = 0; j < i; j++) {
    temp.u8[j] = data_u8[j];
  }

  temp.u8[j++] = 0x01;
  for (; j & 7; j++) {
    temp.u8[j] = 0;
  }
#endif

  if (j > 56) {
    for (; j < 64; j++) {
      temp.u8[j] = 0;
    }
    tiger_compress(temp.u64, res);
    j = 0;
  }

  for (; j < 56; j++) {
    temp.u8[j] = 0;
  }
  temp.u64[7] = length << 3;
  tiger_compress(temp.u64, res);

  for (i = 0; i < 3; i++) {
    poke_le64(&hash[i * 8], res[i]);
  }
}

/* vi: set ai et sts=2 sw=2 cindent: */
