/*
 * $Id: info_hes.c,v 5.2 90/06/23 22:19:30 jsp Rel $
 *
 * Copyright (c) 1989 Jan-Simon Pendry
 * Copyright (c) 1989 Imperial College of Science, Technology & Medicine
 * Copyright (c) 1989 The Regents of the University of California.
 * All rights reserved.
 *
 * This code is derived from software contributed to Berkeley by
 * Jan-Simon Pendry at Imperial College, London.
 *
 * %sccs.include.redist.c%
 *
 *	@(#)info_hes.c	5.1 (Berkeley) 06/29/90
 */

/*
 * Get info from Hesiod
 */

#include "am.h"

#ifdef HAS_HESIOD_MAPS
#include <hesiod.h>

#define	HES_PREFIX	"hesiod."
#define	HES_PREFLEN	7

/*
 * No way to probe the server - check the map name begins with "hesiod."
 */
hesiod_init(map)
char *map;
{
	return strncmp(map, HES_PREFIX, HES_PREFLEN) == 0 ? 0 : ENOENT;
}


/*
 * Do a Hesiod nameserver call.
 * Modify time is ignored by Hesiod - XXX
 */
int hesiod_search(m, map, key, pval, tp)
mnt_map *m;
char *map;
char *key;
char **pval;
time_t *tp;
{
	int error;
	char hes_map[MAXPATHLEN];
	char **rvec;
	/*
	 * Make Hesiod name.  Skip past the "hesiod."
	 * at the start of the map name and append
	 * ".automount".  The net effect is that a lookup
	 * of /defaults in hesiod.home will result in a
	 * call to hes_resolve("/defaults", "home.automount");
	 */
	sprintf(hes_map, "%s%s", map + HES_PREFLEN, ".automount");
	/*
	 * Call the resolver
	 */
	rvec = hes_resolve(key, hes_map);
	/*
	 * If a reply was forthcoming then return
	 * it (and free subsequent replies)
	 */
	if (rvec && *rvec) {
		*pval = *rvec;
		while (*++rvec)
			free(*rvec);
		return 0;
	}

	/*
	 * Otherwise reflect the hesiod error into a Un*x error
	 */
	switch (hes_error()) {
	case HES_ER_NOTFOUND:	error = ENOENT; break;
	case HES_ER_CONFIG:	error = EIO; break;
	case HES_ER_NET:	error = ETIMEDOUT; break;
	default:		error = EINVAL; break;
	}
	return error;
}
#endif /* HAS_HESIOD_MAPS */
