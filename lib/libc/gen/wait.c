/*
 * Copyright (c) 1988 The Regents of the University of California.
 * All rights reserved.
 *
 * %sccs.include.redist.c%
 */

#if defined(LIBC_SCCS) && !defined(lint)
static char sccsid[] = "@(#)wait.c	5.4 (Berkeley) 02/23/91";
#endif /* LIBC_SCCS and not lint */

#include <sys/types.h>
#include <sys/time.h>
#include <sys/wait.h>
#include <sys/resource.h>

pid_t
wait(istat)
	int *istat;
{
	return (wait4(WAIT_ANY, istat, 0, (struct rusage *)0));
}
