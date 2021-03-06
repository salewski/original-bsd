/*
 * Copyright (c) 1988, 1993
 *	The Regents of the University of California.  All rights reserved.
 *
 * %sccs.include.redist.c%
 *
 * This code is derived from software contributed to Berkeley by
 * Computer Consoles Inc.
 */

#if defined(LIBC_SCCS) && !defined(lint)
	.asciz "@(#)bzero.s	8.1 (Berkeley) 06/04/93"
#endif /* LIBC_SCCS and not lint */

/* bzero(base, length) */
#include "DEFS.h"

ENTRY(bzero, 0)
	movl	$1f,r0				# r0 = null source string
	movl	4(fp),r1			# r1 = destination address
	movl	8(fp),r2			# r2 = count
	movs3
	ret
1:
	.byte 0
