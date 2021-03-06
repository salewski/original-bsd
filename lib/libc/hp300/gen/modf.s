/*-
 * Copyright (c) 1990, 1993
 *	The Regents of the University of California.  All rights reserved.
 *
 * This code is derived from software contributed to Berkeley by
 * the Systems Programming Group of the University of Utah Computer
 * Science Department.
 *
 * %sccs.include.redist.c%
 */

#if defined(LIBC_SCCS) && !defined(lint)
	.asciz "@(#)modf.s	8.1 (Berkeley) 06/04/93"
#endif /* LIBC_SCCS and not lint */

#include "DEFS.h"

/*
 * double modf(val, iptr)
 * returns: xxx and n (in *iptr) where val == n.xxx
 */
ENTRY(modf)
	fmoved	sp@(4),fp0
	movel	sp@(12),a0
	fintrzx	fp0,fp1
	fmoved	fp1,a0@
	fsubx	fp1,fp0
	fmoved	fp0,sp@-
	movel	sp@+,d0
	movel	sp@+,d1
	rts
