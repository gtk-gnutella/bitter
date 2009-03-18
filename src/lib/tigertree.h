/* (PD) 2001 The Bitzi Corporation
 * Please see file COPYING or http://bitzi.com/publicdomain 
 * for more info.
 *
 * $Id$
 */

#ifndef TIGERTREE_HEADER_FILE
#define TIGERTREE_HEADER_FILE

#include "tiger.h"

/* tiger hash result size, in uint8_ts */
#define TIGERSIZE 24

/* size of each block independently tiger-hashed, not counting leaf 0x00 prefix */
#define TTH_BLOCKSIZE 1024

/* size of input to each non-leaf hash-tree node, not counting node 0x01 prefix */
#define TTH_NODESIZE (TIGERSIZE*2)

/* default size of interim values stack, in TIGERSIZE
 * blocks. If this overflows (as it will for input
 * longer than 2^64 in size), havoc may ensue. */
#define TTH_STACKSIZE (TIGERSIZE*56)

typedef void (*tt_callback)(const char hash[TIGERSIZE], void *udata);

typedef struct tt_context {
  uint64_t count;               /* total blocks processed */
  char leaf[1+TTH_BLOCKSIZE];	/* leaf in progress */
  char *block;           	/* leaf data */
  char node[1+TTH_NODESIZE];	/* node scratch space */
  int index;                    /* index into block */
  char *top;             	/* top (next empty) stack slot */
  char nodes[TTH_STACKSIZE];	/* stack of interim node values */
} TT_CONTEXT;

void tt_init(TT_CONTEXT *ctx);
void tt_update(TT_CONTEXT *ctx, const void *data, size_t len);
void tt_digest(TT_CONTEXT *ctx, char hash[TIGERSIZE]);

#endif /* TIGERTREE_HEADER_FILE */
