/*
 * Copyright (c) 1983, 1993
 *	The Regents of the University of California.  All rights reserved.
 *
 * %sccs.include.redist.c%
 */

#if defined(LIBC_SCCS) && !defined(lint)
	.asciz "@(#)htonl.s	8.1 (Berkeley) 06/04/93"
#endif /* LIBC_SCCS and not lint */

/* netorder = htonl(hostorder) */

#include "DEFS.h"

ENTRY(htonl, 0)
	rotl	$-8,4(ap),r0
	insv	r0,$16,$8,r0
	movb	7(ap),r0
	ret
