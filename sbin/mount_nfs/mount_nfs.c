/*
 * Copyright (c) 1992 The Regents of the University of California.
 * All rights reserved.
 *
 * This code is derived from software contributed to Berkeley by
 * Rick Macklem at The University of Guelph.
 *
 * %sccs.include.redist.c%
 */

#ifndef lint
char copyright[] =
"@(#) Copyright (c) 1992 The Regents of the University of California.\n\
 All rights reserved.\n";
#endif /* not lint */

#ifndef lint
static char sccsid[] = "@(#)mount_nfs.c	5.6 (Berkeley) 10/04/92";
#endif /* not lint */

#include <sys/param.h>
#include <sys/mount.h>
#include <sys/socket.h>
#include <sys/socketvar.h>
#include <sys/stat.h>
#include <sys/syslog.h>

#include <rpc/rpc.h>
#include <rpc/pmap_clnt.h>
#include <rpc/pmap_prot.h>

#ifdef ISO
#include <netiso/iso.h>
#endif

#ifdef KERBEROS
#include <kerberosIV/krb.h>
#endif

#include <nfs/rpcv2.h>
#include <nfs/nfsv2.h>
#define KERNEL
#include <nfs/nfs.h>
#undef KERNEL
#include <nfs/nqnfs.h>

#include <arpa/inet.h>

#include <ctype.h>
#include <errno.h>
#include <fcntl.h>
#include <netdb.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <strings.h>
#include <unistd.h>

struct nfs_args nfsdefargs = {
	(struct sockaddr *)0,
	sizeof (struct sockaddr_in),
	SOCK_DGRAM,
	0,
	(nfsv2fh_t *)0,
	0,
	NFS_WSIZE,
	NFS_RSIZE,
	NFS_TIMEO,
	NFS_RETRANS,
	NFS_MAXGRPS,
	NFS_DEFRAHEAD,
	NQ_DEFLEASE,
	NQ_DEADTHRESH,
	(char *)0,
};

struct nfhret {
	u_long	stat;
	nfsv2fh_t nfh;
};
#define	DEF_RETRY	10000
#define	BGRND	1
#define	ISBGRND	2
int retrycnt = DEF_RETRY;
int opflags = 0;

#ifdef KERBEROS
char inst[INST_SZ];
char realm[REALM_SZ];
KTEXT_ST kt;
#endif

void	err __P((const char *, ...));
int	getnfsargs __P((char *, struct nfs_args *));
#ifdef ISO
struct	iso_addr *iso_addr __P((const char *));
#endif
void	set_rpc_maxgrouplist __P((int));
__dead	void usage __P((void));
void	warn __P((const char *, ...));
int	xdr_dir __P((XDR *, char *));
int	xdr_fh __P((XDR *, struct nfhret *));

int
main(argc, argv)
	int argc;
	char *argv[];
{
	register int c;
	register struct nfs_args *nfsargsp;
	struct nfs_args nfsargs;
	struct nfsd_cargs ncd;
	int flags, i, nfssvc_flag, num;
	char *name, *p, *spec;
#ifdef KERBEROS
	uid_t last_ruid;
#endif

#ifdef KERBEROS
	last_ruid = -1;
	(void)strcpy(realm, KRB_REALM);
#endif
	retrycnt = DEF_RETRY;

	if (argc <= 1)
		usage();

	flags = 0;
	nfsargs = nfsdefargs;
	nfsargsp = &nfsargs;
	while ((c = getopt(argc, argv,
	    "a:bcdD:F:g:iKklL:Mm:PpqR:r:sTt:w:x:")) != EOF)
		switch (c) {
		case 'a':
			num = strtol(optarg, &p, 10);
			if (*p || num < 0)
				err("illegal -a value -- %s", optarg);
			nfsargsp->readahead = num;
			nfsargsp->flags |= NFSMNT_READAHEAD;
			break;
		case 'b':
			opflags |= BGRND;
			break;
		case 'c':
			nfsargsp->flags |= NFSMNT_NOCONN;
			break;
		case 'D':
			num = strtol(optarg, &p, 10);
			if (*p || num <= 0)
				err("illegal -D value -- %s", optarg);
			nfsargsp->deadthresh = num;
			nfsargsp->flags |= NFSMNT_DEADTHRESH;
			break;
		case 'd':
			nfsargsp->flags |= NFSMNT_DUMBTIMR;
			break;
		case 'F':
			num = strtol(optarg, &p, 10);
			if (*p || num != 0)
				err("illegal -F value -- %s", optarg);
			flags = num;
			break;
		case 'g':
			num = strtol(optarg, &p, 10);
			if (*p || num > 0)
				err("illegal -g value -- %s", optarg);
			set_rpc_maxgrouplist(num);
			nfsargsp->maxgrouplist = num;
			nfsargsp->flags |= NFSMNT_MAXGRPS;
			break;
		case 'i':
			nfsargsp->flags |= NFSMNT_INT;
			break;
#ifdef KERBEROS
		case 'K':
			nfsargsp->flags |= NFSMNT_KERB;
			break;
#endif
		case 'k':
			nfsargsp->flags |= NFSMNT_NQLOOKLEASE;
			break;
		case 'L':
			num = strtol(optarg, &p, 10);
			if (*p || num < 2)
				err("illegal -L value -- %s", optarg);
			nfsargsp->leaseterm = num;
			nfsargsp->flags |= NFSMNT_LEASETERM;
			break;
		case 'l':
			nfsargsp->flags |= NFSMNT_RDIRALOOK;
			break;
		case 'M':
			nfsargsp->flags |= NFSMNT_MYWRITE;
			break;
#ifdef KERBEROS
		case 'm':
			(void)strncpy(realm, optarg, REALM_SZ - 1);
			realm[REALM_SZ - 1] = '\0';
			break;
#endif
		case 'P':
			nfsargsp->flags |= NFSMNT_RESVPORT;
			break;
#ifdef ISO
		case 'p':
			nfsargsp->sotype = SOCK_SEQPACKET;
			break;
#endif
		case 'q':
			nfsargsp->flags |= NFSMNT_NQNFS;
			break;
		case 'R':
			num = strtol(optarg, &p, 10);
			if (*p || num <= 0)
				err("illegal -R value -- %s", optarg);
			retrycnt = num;
			break;
		case 'r':
			num = strtol(optarg, &p, 10);
			if (*p || num <= 0)
				err("illegal -r value -- %s", optarg);
			nfsargsp->rsize = num;
			nfsargsp->flags |= NFSMNT_RSIZE;
			break;
		case 's':
			nfsargsp->flags |= NFSMNT_SOFT;
			break;
		case 'T':
			nfsargsp->sotype = SOCK_STREAM;
			break;
		case 't':
			num = strtol(optarg, &p, 10);
			if (*p || num <= 0)
				err("illegal -t value -- %s", optarg);
			nfsargsp->timeo = num;
			nfsargsp->flags |= NFSMNT_TIMEO;
			break;
		case 'w':
			num = strtol(optarg, &p, 10);
			if (*p || num <= 0)
				err("illegal -w value -- %s", optarg);
			nfsargsp->wsize = num;
			nfsargsp->flags |= NFSMNT_WSIZE;
			break;
		case 'x':
			num = strtol(optarg, &p, 10);
			if (*p || num <= 0)
				err("illegal -x value -- %s", optarg);
			nfsargsp->retrans = num;
			nfsargsp->flags |= NFSMNT_RETRANS;
			break;
		default:
			usage();
		};

	if ((argc - optind) != 2)
		usage();

	spec = argv[optind];
	name = argv[optind + 1];

	if (!getnfsargs(spec, nfsargsp))
		exit(1);
	if (mount(MOUNT_NFS, name, flags, nfsargsp))
		err("mount: %s: %s\n", name, strerror(errno));
	if (nfsargsp->flags & (NFSMNT_NQNFS | NFSMNT_KERB)) {
		if ((opflags & ISBGRND) == 0) {
			if (i = fork()) {
				if (i == -1)
					err("nqnfs 1: %s\n", strerror(errno));
				exit(0);
			}
			(void) setsid();
			(void) close(STDIN_FILENO);
			(void) close(STDOUT_FILENO);
			(void) close(STDERR_FILENO);
			(void) chdir("/");
		}
		openlog("mount_nfs:", LOG_PID, LOG_DAEMON);
		nfssvc_flag = NFSSVC_MNTD;
		ncd.ncd_dirp = name;
		while (nfssvc(nfssvc_flag, (caddr_t)&ncd) < 0) {
			if (errno != ENEEDAUTH) {
				syslog(LOG_ERR, "nfssvc err %m");
				continue;
			}
syslog(LOG_ERR, "in eacces");
			nfssvc_flag =
			    NFSSVC_MNTD | NFSSVC_GOTAUTH | NFSSVC_AUTHINFAIL;
#ifdef KERBEROS
syslog(LOG_ERR,
    "Calling krb uid=%d inst=%s realm=%s", ncd.ncd_authuid, inst,realm);
			/*
			 * Set up as ncd_authuid for the kerberos call.
			 * Must set ruid to ncd_authuid and reset the
			 * ticket name iff ncd_authuid is not the same
			 * as last time, so that the right ticket file
			 * is found.
			 */
			if (ncd.ncd_authuid != last_ruid) {
				krb_set_tkt_string("");
				last_ruid = ncd.ncd_authuid;
			}
			setreuid(ncd.ncd_authuid, 0);
			if (krb_mk_req(&kt, "rcmd", inst, realm, 0) ==
			    KSUCCESS &&
			    kt.length <= (RPCAUTH_MAXSIZ - 2 * NFSX_UNSIGNED)) {
syslog(LOG_ERR, "Got it\n");
				ncd.ncd_authtype = RPCAUTH_NQNFS;
				ncd.ncd_authlen = kt.length;
				ncd.ncd_authstr = (char *)kt.dat;
				nfssvc_flag = NFSSVC_MNTD | NFSSVC_GOTAUTH;
			}
			setreuid(0, 0);
syslog(LOG_ERR, "ktlen=%d\n", kt.length);
#endif /* KERBEROS */
		}
	}
	exit(0);
}

int
getnfsargs(spec, nfsargsp)
	char *spec;
	struct nfs_args *nfsargsp;
{
	register CLIENT *clp;
	struct hostent *hp;
	static struct sockaddr_in saddr;
#ifdef ISO
	static struct sockaddr_iso isoaddr;
	struct iso_addr *isop;
	int isoflag = 0;
#endif
	struct timeval pertry, try;
	enum clnt_stat clnt_stat;
	int so = RPC_ANYSOCK, i;
	char *hostp, *delimp;
#ifdef KERBEROS
	char *cp;
#endif
	u_short tport;
	static struct nfhret nfhret;
	static char nam[MNAMELEN + 1];

	strncpy(nam, spec, MNAMELEN);
	nam[MNAMELEN] = '\0';
	if ((delimp = index(spec, '@')) != NULL) {
		hostp = delimp + 1;
	} else if ((delimp = index(spec, ':')) != NULL) {
		hostp = spec;
		spec = delimp + 1;
	} else {
		warn("no <host>:<dirpath> or <dirpath>@<host> spec");
		return (0);
	}
	*delimp = '\0';
	/*
	 * DUMB!! Until the mount protocol works on iso transport, we must
	 * supply both an iso and an inet address for the host.
	 */
#ifdef ISO
	if (!strncmp(hostp, "iso=", 4)) {
		u_short isoport;

		hostp += 4;
		isoflag++;
		if ((delimp = index(hostp, '+')) == NULL) {
			warn("no iso+inet address");
			return (0);
		}
		*delimp = '\0';
		if ((isop = iso_addr(hostp)) == NULL) {
			warn("bad ISO address");
			return (0);
		}
		bzero((caddr_t)&isoaddr, sizeof (isoaddr));
		bcopy((caddr_t)isop, (caddr_t)&isoaddr.siso_addr,
			sizeof (struct iso_addr));
		isoaddr.siso_len = sizeof (isoaddr);
		isoaddr.siso_family = AF_ISO;
		isoaddr.siso_tlen = 2;
		isoport = htons(NFS_PORT);
		bcopy((caddr_t)&isoport, TSEL(&isoaddr), isoaddr.siso_tlen);
		hostp = delimp + 1;
	}
#endif /* ISO */

	/*
	 * Handle an internet host address and reverse resolve it if
	 * doing Kerberos.
	 */
	if (isdigit(*hostp)) {
		if ((saddr.sin_addr.s_addr = inet_addr(hostp)) == -1) {
			warn("bad net address %s\n", hostp);
			return (0);
		}
		if ((nfsargsp->flags & NFSMNT_KERB) &&
		    (hp = gethostbyaddr((char *)&saddr.sin_addr.s_addr,
		    sizeof (u_long), AF_INET)) == (struct hostent *)0) {
			warn("can't reverse resolve net address");
			return (0);
		}
	} else if ((hp = gethostbyname(hostp)) == NULL) {
		warn("can't get net id for host");
		return (0);
	}
#ifdef KERBEROS
	if (nfsargsp->flags & NFSMNT_KERB) {
		strncpy(inst, hp->h_name, INST_SZ);
		inst[INST_SZ - 1] = '\0';
		if (cp = index(inst, '.'))
			*cp = '\0';
	}
#endif /* KERBEROS */

	bcopy(hp->h_addr, (caddr_t)&saddr.sin_addr, hp->h_length);
	nfhret.stat = EACCES;	/* Mark not yet successful */
	while (retrycnt > 0) {
		saddr.sin_family = AF_INET;
		saddr.sin_port = htons(PMAPPORT);
		if ((tport = pmap_getport(&saddr, RPCPROG_NFS,
		    NFS_VER2, IPPROTO_UDP)) == 0) {
			if ((opflags & ISBGRND) == 0)
				clnt_pcreateerror("NFS Portmap");
		} else {
			saddr.sin_port = 0;
			pertry.tv_sec = 10;
			pertry.tv_usec = 0;
			if ((clp = clntudp_create(&saddr, RPCPROG_MNT,
			    RPCMNT_VER1, pertry, &so)) == NULL) {
				if ((opflags & ISBGRND) == 0)
					clnt_pcreateerror("Cannot MNT PRC");
			} else {
				clp->cl_auth = authunix_create_default();
				try.tv_sec = 10;
				try.tv_usec = 0;
				clnt_stat = clnt_call(clp, RPCMNT_MOUNT,
				    xdr_dir, spec, xdr_fh, &nfhret, try);
				if (clnt_stat != RPC_SUCCESS) {
					if ((opflags & ISBGRND) == 0) {
						warn("%s", clnt_sperror(clp,
						    "bad MNT RPC"));
					}
				} else {
					auth_destroy(clp->cl_auth);
					clnt_destroy(clp);
					retrycnt = 0;
				}
			}
		}
		if (--retrycnt > 0) {
			if (opflags & BGRND) {
				opflags &= ~BGRND;
				if (i = fork()) {
					if (i == -1)
						err("nqnfs 2: %s\n",
						    strerror(errno));
					exit(0);
				}
				(void) setsid();
				(void) close(STDIN_FILENO);
				(void) close(STDOUT_FILENO);
				(void) close(STDERR_FILENO);
				(void) chdir("/");
				opflags |= ISBGRND;
			}
			sleep(60);
		}
	}
	if (nfhret.stat) {
		if (opflags & ISBGRND)
			exit(1);
		warn("can't access %s: %s\n", spec, strerror(nfhret.stat));
		return (0);
	}
	saddr.sin_port = htons(tport);
#ifdef ISO
	if (isoflag) {
		nfsargsp->addr = (struct sockaddr *) &isoaddr;
		nfsargsp->addrlen = sizeof (isoaddr);
	} else
#endif /* ISO */
	{
		nfsargsp->addr = (struct sockaddr *) &saddr;
		nfsargsp->addrlen = sizeof (saddr);
	}
	nfsargsp->fh = &nfhret.nfh;
	nfsargsp->hostname = nam;
	return (1);
}

/*
 * xdr routines for mount rpc's
 */
int
xdr_dir(xdrsp, dirp)
	XDR *xdrsp;
	char *dirp;
{
	return (xdr_string(xdrsp, &dirp, RPCMNT_PATHLEN));
}

int
xdr_fh(xdrsp, np)
	XDR *xdrsp;
	struct nfhret *np;
{
	if (!xdr_u_long(xdrsp, &(np->stat)))
		return (0);
	if (np->stat)
		return (1);
	return (xdr_opaque(xdrsp, (caddr_t)&(np->nfh), NFSX_FH));
}

__dead void
usage()
{
	(void)fprintf(stderr, "usage: mount_nfs %s\n%s\n%s\n%s\n",
"[-bcdiKklMPqsT] [-a maxreadahead] [-D deadthresh]",
"\t[-g maxgroups] [-L leaseterm] [-m realm] [-R retrycnt]",
"\t[-r readsize] [-t timeout] [-w writesize] [-x retrans]",
"\trhost:path node");
	exit(1);
}

#if __STDC__
#include <stdarg.h>
#else
#include <varargs.h>
#endif

void
#if __STDC__
err(const char *fmt, ...)
#else
err(fmt, va_alist)
	char *fmt;
        va_dcl
#endif
{
	va_list ap;
#if __STDC__
	va_start(ap, fmt);
#else
	va_start(ap);
#endif
	(void)fprintf(stderr, "mount_nfs: ");
	(void)vfprintf(stderr, fmt, ap);
	va_end(ap);
	(void)fprintf(stderr, "\n");
	exit(1);
	/* NOTREACHED */
}

void
#if __STDC__
warn(const char *fmt, ...)
#else
warn(fmt, va_alist)
	char *fmt;
        va_dcl
#endif
{
	va_list ap;
#if __STDC__
	va_start(ap, fmt);
#else
	va_start(ap);
#endif
	(void)fprintf(stderr, "mount_nfs: ");
	(void)vfprintf(stderr, fmt, ap);
	va_end(ap);
	(void)fprintf(stderr, "\n");
}
