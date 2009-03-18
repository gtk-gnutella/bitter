/*
 * Copyright (c) 2006 Christian Biere <christianbiere@gmx.de>
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHORS AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

#include "common.h"
#include "base16.h"

/*
 * See RFC 3548 for details about Base 16 encoding:
 *  http://www.faqs.org/rfcs/rfc3548.html
 */

static const char base16_alphabet[] = "0123456789abcdef";

size_t
base16_encode(char *dst, size_t size, const void *data, size_t len)
{
  const unsigned char *p = data;
  char *q = dst;
  size_t i;

  if (size / 2 < len) {
    len = size * 2;
  }

  for (i = 0; i < len; i++) {
    unsigned char c = p[i] & 0xff;
    *q++ = base16_alphabet[(c >> 4) & 0xf];
    *q++ = base16_alphabet[c & 0xf];
  }

  return q - dst;
}

static inline int
ascii_toupper(int c)
{
  return c >= 97 && c <= 122 ? c - 32 : c;
}

static inline int
ascii_tolower(int c)
{
  return c >= 65 && c <= 90 ? c + 32 : c;
}

static char base16_map[(unsigned char) -1];
static int base16_initialized;

static void
base16_init(void)
{
  size_t i;
    
  base16_initialized = 1;
  for (i = 0; i < ARRAY_LEN(base16_map); i++) {
    const char *x;
      
    x = memchr(base16_alphabet, ascii_tolower(i), sizeof base16_alphabet);
    base16_map[i] = x ? (x - base16_alphabet) : (unsigned char) -1;
  }
}

size_t
base16_decode(char *dst, size_t size, const void *data, size_t len)
{
  const unsigned char *p = data;
  char *q = dst;
  size_t i;

  if (!base16_initialized) {
    base16_init();
  }

  if (size < len / 2) {
    len = size * 2;
  }
  
  for (i = 0; i < len; i++) {
      unsigned char c;

      c = base16_map[p[i]];
      if ((unsigned char) -1 == c) {
        return -1;
      }
      if ((i & 1) == 0) {
        *q = c << 4;
      } else {
        *q++ |= c;
      }
  }

  return q - dst;
}

/* vi: set ai et sts=2 sw=2 cindent: */
