/*-
 * Copyright (c) 1979, 1993
 *	The Regents of the University of California.  All rights reserved.
 *
 * %sccs.include.redist.c%
 */

#ifndef lint
static char sccsid[] = "@(#)NIL.c	8.1 (Berkeley) 06/06/93";
#endif /* not lint */

#include "h00vars.h"

char ENIL[] = "Pointer value out of legal range\n";

char *
NIL(ptr)
	char	*ptr;		/* pointer to struct */
{
	if (ptr > _maxptr || ptr < _minptr) {
		ERROR(ENIL, 0);
	}
	return ptr;
}
