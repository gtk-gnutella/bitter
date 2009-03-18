/* (PD) 2001 The Bitzi Corporation
 * Please see file COPYING or http://bitzi.com/publicdomain 
 * for more info.
 *
 * tigertree.c - Implementation of the TigerTree algorithm
 *
 * NOTE: The TigerTree hash value cannot be calculated using a
 * constant amount of memory; rather, the memory required grows
 * with the size of input. (Roughly, one more interim value must
 * be remembered for each doubling of the input size.) The
 * default TT_CONTEXT struct size reserves enough memory for
 * input up to 2^64 in length
 *
 * Requires the tiger() function as defined in the reference
 * implementation provided by the creators of the Tiger
 * algorithm. See
 *
 *    http://www.cs.technion.ac.il/~biham/Reports/Tiger/
 *
 * $Id$
 *
 */

#include "tigertree.h"

/* Initialize the tigertree context */
void
tt_init(TT_CONTEXT *ctx)
{
  ctx->count = 0;
  ctx->leaf[0] = 0; /* flag for leaf  calculation -- never changed */
  ctx->node[0] = 1; /* flag for inner node calculation -- never changed */
  ctx->block = ctx->leaf + 1 ; /* working area for blocks */
  ctx->index = 0;   /* partial block pointer/block length */
  ctx->top = ctx->nodes;
}

static void
tt_compose(TT_CONTEXT *ctx)
{
  char *node = ctx->top - TTH_NODESIZE;

  memmove(&ctx->node[1], node, TTH_NODESIZE); /* copy to scratch area */
  /* combine two nodes */
  tiger(ctx->node, TTH_NODESIZE + 1, ctx->top);
  memmove(node,ctx->top,TIGERSIZE);           /* move up result */
  ctx->top -= TIGERSIZE;                      /* update top ptr */
}

static void
tt_block(TT_CONTEXT *ctx)
{
  uint64_t b;

  tiger(ctx->leaf, ctx->index + 1, ctx->top);
  ctx->top += TIGERSIZE;
  ++ctx->count;
  b = ctx->count;
  while (0 == (b & 1)) { /* while evenly divisible by 2... */
    tt_compose(ctx);
    b >>= 1;
  }
}

void
tt_update(TT_CONTEXT *ctx, const void *data, size_t len)
{
  const char *buffer = data;

  if (ctx->index) { /* Try to fill partial block */
 	unsigned left = TTH_BLOCKSIZE - ctx->index;
  	if (len < left) {
		memmove(ctx->block + ctx->index, buffer, len);
		ctx->index += len;
		return; /* Finished */
	} else {
		memmove(ctx->block + ctx->index, buffer, left);
		ctx->index = TTH_BLOCKSIZE;
		tt_block(ctx);
		buffer += left;
		len -= left;
	}
  }

  while (len >= TTH_BLOCKSIZE) {
	memmove(ctx->block, buffer, TTH_BLOCKSIZE);
	ctx->index = TTH_BLOCKSIZE;
	tt_block(ctx);
	buffer += TTH_BLOCKSIZE;
	len -= TTH_BLOCKSIZE;
  }
  ctx->index = len;
  if (0 != len) {
	/* Buffer leftovers */
	memmove(ctx->block, buffer, len);
  }
}

/* no need to call this directly; tt_digest calls it for you */
static void
tt_final(TT_CONTEXT *ctx)
{
  /* do last partial block, unless index is 1 (empty leaf) */
  /* AND we're past the first block */
  if (ctx->index > 0 || ctx->top == ctx->nodes) {
    tt_block(ctx);
  }
}

void
tt_digest(TT_CONTEXT *ctx, char hash[TIGERSIZE])
{
  tt_final(ctx);
  while (ctx->top - TIGERSIZE > ctx->nodes) {
    tt_compose(ctx);
  }
  memmove(hash, ctx->nodes, TIGERSIZE);
}

/* vi: set ai et sts=2 sw=2 cindent: */
