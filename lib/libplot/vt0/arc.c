/*-
 * Copyright (c) 1983, 1993
 *	The Regents of the University of California.  All rights reserved.
 *
 * %sccs.include.proprietary.c%
 */

#ifndef lint
static char sccsid[] = "@(#)arc.c	8.1 (Berkeley) 06/04/93";
#endif /* not lint */

extern vti;
arc(xi,yi,x0,y0,x1,y1){
	char c;
	c = 6;
	write(vti,&c,1);
	write(vti,&xi,12);
}
