/*-
 * Copyright (c) 1980, 1993
 *	The Regents of the University of California.  All rights reserved.
 *
 * %sccs.include.redist.c%
 *
 *	@(#)config.h	8.1 (Berkeley) 06/06/93
 */

    /*
     *	external declarations of things from 
     *		CONFIG.c
     *
     */

    /*
     *	the version of translator
     */
extern char	*version;

    /*
     *	the location of the error strings
     *	and the length of the path to it
     *	(in case of execution of pc0 as a.out)
     */
extern char	*err_file;
extern int	err_pathlen;

    /*
     *	the location of the short explanation
     *	and the length of the path to it
     *	the null at the end is so pix can change it to pi'x' from pi.
     */
extern char	*how_file;
extern int	how_pathlen;
extern char	*px_header;
extern char	*pi_comp;
extern char	*px_intrp;
extern char	*px_debug;
