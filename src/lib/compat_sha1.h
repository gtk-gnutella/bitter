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

#ifndef COMPAT_SHA1_HEADER_FILE
#define COMPAT_SHA1_HEADER_FILE

#include "common.h"
#include "nettools.h"

#if !defined(HAVE_OPENSSL_SHA1) && \
    !defined(HAVE_FREEBSD_SHA1) && \
    !defined(HAVE_NETBSD_SHA1) && \
    !defined(HAVE_BEECRYPT_SHA1)
#error "No SHA-1 calculation function available!"
#undef HAVE_SHA1
#endif

#ifdef HAVE_SHA1

/* OpenSSL and FreeBSD use the same prototypes but different header files. */
#if defined(HAVE_OPENSSL_SHA1) || defined(HAVE_FREEBSD_SHA1)

#if defined(HAVE_OPENSSL_SHA1)
#include <openssl/sha.h>
#else
#include <sha.h>
#endif /* HAVE_OPENSSL_SHA1 */

struct compat_sha1 { SHA_CTX data; };
static inline void
compat_sha1_init(struct compat_sha1 *ctx)
{
  SHA1_Init(&ctx->data);
}

static inline void
compat_sha1_update(struct compat_sha1 *ctx, const void *data, size_t size)
{
  SHA1_Update(&ctx->data, data, size);
}

static inline void
compat_sha1_final(struct compat_sha1 *ctx, struct sha1 *md)
{
  SHA1_Final(md->data, &ctx->data);
}
#endif /* HAVE_OPENSSL_SHA1 || HAVE_FREEBSD_SHA1 */

#ifdef HAVE_NETBSD_SHA1
#include <sha1.h>
struct compat_sha1 { SHA1_CTX data; };

static inline void
compat_sha1_init(struct compat_sha1 *ctx)
{
  SHA1Init(&ctx->data);
}

static inline void
compat_sha1_update(struct compat_sha1 *ctx, const void *data, size_t size)
{
  /* NetBSD uses "u_int" instead of size_t or unsigned long. Thus, use
   * a loop just in case someone really passes a huge chunk. */
  for (;;) {
    u_int n = MIN((u_int) -1, size);
    SHA1Update(&ctx->data, data, n);
    if (size <= n)
      break;
    data = (const char *) data + n;
    size -= n;
  }
}

static inline void
compat_sha1_final(struct compat_sha1 *ctx, struct sha1 *md)
{
  SHA1Final(md->data, &ctx->data);
}
#endif /* HAVE_NETBSD_SHA1 */

#ifdef HAVE_BEECRYPT_SHA1
#include <beecrypt/sha1.h>
struct compat_sha1 { sha1Param data; };

static inline void
compat_sha1_init(struct compat_sha1 *ctx)
{
  sha1Reset(&ctx->data);
}

static inline void
compat_sha1_update(struct compat_sha1 *ctx, const void *data, size_t size)
{
  sha1Update(&ctx->data, data, size);
}

static inline void
compat_sha1_final(struct compat_sha1 *ctx, struct sha1 *md)
{
  sha1Digest(&ctx->data, md->data);
}
#endif /* HAVE_BEECRYPT_SHA1 */

static inline void compat_sha1_init(struct compat_sha1 *ctx);
static inline void compat_sha1_update(struct compat_sha1 *ctx,
    const void *data, size_t size);
static inline void compat_sha1_final(struct compat_sha1 *ctx,
    struct sha1 *md);

#endif /* HAVE_SHA1 */

/* vi: set ai et sts=2 sw=2 cindent: */
#endif /* COMPAT_SHA1_HEADER_FILE */
