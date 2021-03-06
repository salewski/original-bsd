/*-
 * Copyright (c) 1988, 1993
 *	The Regents of the University of California.  All rights reserved.
 *
 * %sccs.include.redist.c%
 *
 *	@(#)disp_asc.h	8.1 (Berkeley) 06/06/93
 */

/*
 * Define the translate tables used to go between 3270 display code
 * and ascii
 */

extern unsigned char
	disp_asc[256],		/* Goes between display code and ascii */
	asc_disp[256];		/* Goes between ascii and display code */
