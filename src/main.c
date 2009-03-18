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

#include "lib/common.h"
#include "lib/tigertree.h"
#include "lib/tiger.h"
#include "lib/base16.h"
#include "lib/base32.h"
#include "lib/compat_sha1.h"
#include "lib/nettools.h"

#define SHA1_BASE32_LEN 32
#define SHA1_BASE16_LEN 40

static const unsigned bitter_major_version = 1, bitter_minor_version = 3;

struct tth {
  char data[24 /* TIGERSIZE */];
};

static const char *
sha1_to_base32(const struct sha1 *hash)
{
  const char *s;
  
  if (hash) {
    static char buf[33];

    s = buf;
    base32_encode(buf, sizeof buf, hash->data, sizeof hash->data);
    buf[ARRAY_LEN(buf) - 1] = '\0';
  } else {
    s = NULL;
  }
  return s;
}

static const char *
sha1_to_base16(const struct sha1 *hash)
{
  const char *s;
  
  if (hash) {
    static char buf[41];

    s = buf;
    base16_encode(buf, sizeof buf, hash->data, sizeof hash->data);
    buf[ARRAY_LEN(buf) - 1] = '\0';
  } else {
    s = NULL;
  }
  return s;
}

static const char *
tth_to_base32(const struct tth *hash)
{
  const char *s;
  
  if (hash) {
    static char buf[40];

    s = buf;
    base32_encode(buf, sizeof buf, hash->data, sizeof hash->data);
    buf[ARRAY_LEN(buf) - 1] = '\0';
  } else {
    s = NULL;
  }
  return s;
}

static void
print_bitprint(FILE *f, const struct sha1 *sha1, const struct tth *tth)
{
  if (f && sha1 && tth) {
    fputs("urn:bitprint:", f);
    fputs(sha1_to_base32(sha1), f);
    fputs(".", f);
    fputs(tth_to_base32(tth), f);
  }
}


static void
print_sha1(FILE *f, const struct sha1 *sha1)
{
  if (f && sha1) {
    fputs("urn:sha1:", f);
    fputs(sha1_to_base32(sha1), f);
  }
}

static void
print_tth(FILE *f, const struct tth *hash)
{
  if (f && hash) {
    fputs("urn:tree:tiger:", f);
    fputs(tth_to_base32(hash), f);
  }
}

struct thex_context {
  off_t filesize;
  unsigned nodes;
};

static int
get_sums(int fd, struct tth *tth, struct sha1 *sha1)
{
  struct compat_sha1 sha1_ctx;
  TT_CONTEXT tt_ctx;
  struct thex_context thex_ctx;
  struct stat sb;

  if (fstat(fd, &sb)) {
    fprintf(stderr, "fstat(): %s\n", compat_strerror(errno));
    return -1;
  }
  thex_ctx.filesize = sb.st_size;

  if (sha1) {
    compat_sha1_init(&sha1_ctx);
  }
  if (tth) {
    tt_init(&tt_ctx);
  }

  for (;;) {
    static uint64_t data[4 * 1024]; /* 32 KiB */
    ssize_t ret;

    ret = read(fd, data, sizeof data);
    if (0 == ret) {
      break;
    } else if ((ssize_t) -1 != ret) {
      if (sha1) {
        compat_sha1_update(&sha1_ctx, data, (size_t) ret);
      }
      if (tth) {
        tt_update(&tt_ctx, data, (size_t) ret);
      }
    } else if (EINTR != errno && EAGAIN != errno) {
      fprintf(stderr, "read(): %s\n", compat_strerror(errno));
      return -1;
    }
  }

  if (sha1) {
    compat_sha1_final(&sha1_ctx, sha1);
  }
  if (tth) {
    tt_digest(&tt_ctx, tth->data);
  }

  return 0;
}


static void
print_filename(FILE *f, const char *filename)
{
  if (filename) {
    fputs(filename, f);
    fputs(": ", f);
  }
}

static void
print_result(FILE *f, const char *filename, bool get_bitprint,
    const struct sha1 *sha1, const struct tth *tth)
{
  if (get_bitprint && tth && sha1) {
    print_filename(f, filename);
    print_bitprint(f, sha1, tth);
    fputs("\n", f);
  } else {
    if (tth) {
      print_filename(f, filename);
      print_tth(f, tth);
      fputs("\n", f);
    }
    if (sha1) {
      print_filename(f, filename);
      print_sha1(f, sha1);
      fputs("\n", f);
    }
  }
}

static void
usage(int status)
{
  fprintf(stderr, "Usage: bitter [-h|-c|-S|-T] [FILE ...]\n");
  fprintf(stderr, "   -h: Show this help.\n");
  fprintf(stderr, "   -v: Show version information.\n");
  fprintf(stderr, "   -S: Calculate the SHA1 only.\n");
  fprintf(stderr, "   -T: Calculate the TTH only.\n");
  fprintf(stderr, "   -q: Do not print the filename.\n");
  fprintf(stderr, "   -c sha1: Convert SHA-1 representation.\n");
  fprintf(stderr, "You may specify multiple filenames or none\n");
  fprintf(stderr, "to read from the standard input.\n\n");
  exit(status);
}

int 
main(int argc, char *argv[])
{
  static struct tth *tth;
  static struct sha1 *sha1;
  static bool get_bitprint = true,
              get_sha1 = false,
              get_tth = false,
              convert = false,
              quiet = false;
  unsigned depth = 9;
  int i, c;

  while (-1 != (c = getopt(argc, argv, "c:d:hvqST"))) {
    switch (c) {
    case 'h':
      usage(EXIT_SUCCESS);
      break;

    case 'S':
    case 'T':
      if (!get_bitprint) {
        fprintf(stderr,
            "Error: "
            "The options -S and -T are mutually exclusive and must not \n"
            "       be specified more than once.\n");
        usage(EXIT_FAILURE);
      }
      get_bitprint = false;
      get_sha1 = 'S' == c;
      get_tth  = 'T' == c;
      break;

    case 'c':
      convert = true;
      {
        const char *s;
        size_t len;

        s = skip_ci_prefix(optarg, "urn:sha1:");
        if (s) {
          len = strlen(s);
          if (SHA1_BASE32_LEN == len) {
            struct sha1 raw;

            base32_decode(cast_to_void_ptr(raw.data), sizeof raw.data, s, len);
            printf("%s\n", sha1_to_base16(&raw));
            exit(EXIT_SUCCESS);
          } else {
            fprintf(stderr, "Error: urn:sha1 has wrong length.\n");
            usage(EXIT_FAILURE);
          }
        } else {
          s = optarg;
          len = strlen(s);
          if (SHA1_BASE16_LEN == len) {
            struct sha1 raw;

            base16_decode(cast_to_void_ptr(raw.data), sizeof raw.data, s, len);
            printf("urn:sha1:%s\n", sha1_to_base32(&raw));
            exit(EXIT_SUCCESS);
          } else {
            fprintf(stderr, "Error: hexadecimal SHA-1 has wrong length.\n");
            usage(EXIT_FAILURE);
          }
        }
      }
      break;

    case 'd':
      if (0 != depth) {
        fprintf(stderr, "Error: -d must not be given more than once.\n");
        usage(EXIT_FAILURE);
      } else {
        char *endptr;
        int error;
        
        depth = parse_uint16(optarg, &endptr, 10, &error);
        if (0 == depth || depth > 32 || error || '\0' != *endptr) {
          fprintf(stderr, "Error: Requested depth is out of range (1..32).\n");
          usage(EXIT_FAILURE);
        }
      }
      break;

    case 'v':
      printf("bitter %u.%u\n", bitter_major_version, bitter_minor_version);
      exit(EXIT_SUCCESS);
      break;

    case 'q':
      quiet = true;
      break;

    default:
      usage(EXIT_FAILURE);
    }
  }
  argc -= optind;
  argv += optind;

  if (get_bitprint) {
    get_sha1 = true;
    get_tth = true;
  }
  if (get_sha1) {
    static struct sha1 sha1_buf;
    sha1 = &sha1_buf;
  }
  if (get_tth) {
    static struct tth tth_buf;
    tth = &tth_buf;
  }

  if (0 == argc) {
    if (0 == get_sums(STDIN_FILENO, tth, sha1)) {
      print_result(stdout, NULL, get_bitprint, sha1, tth);
      exit(EXIT_SUCCESS);
    } else {
      fprintf(stderr, "FAILURE!\n");
      exit(EXIT_FAILURE);
    }
  }

  for (i = 0; i < argc; i++) {
    const char *filename;
    int fd;

    filename = argv[i];
    fd = open(filename, O_RDONLY, 0);
    if (fd < 0) {
      fprintf(stderr, "open(\"%s\"): %s\n", filename, compat_strerror(errno));
      exit(EXIT_FAILURE);
    }

#ifdef POSIX_FADV_SEQUENTIAL
     posix_fadvise(fd, 0, 0, POSIX_FADV_SEQUENTIAL);
#endif  /* POSIX_FADV_SEQUENTIAL */

    if (0 == get_sums(fd, tth, sha1)) {
      print_result(stdout, quiet ? NULL : filename, get_bitprint, sha1, tth);
    }
    close(fd);
    fd = -1;
  }

  return 0;
}

/* vi: set ai et sts=2 sw=2 cindent: */
