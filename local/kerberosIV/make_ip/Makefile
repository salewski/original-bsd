#	@(#)Makefile	8.1 (Berkeley) 06/01/93

PROG=	make_ip
SRCS=	make_ip.c misc.c
.PATH:	${.CURDIR}/../make_key_perm
CFLAGS+=-DKERBEROS -DDEBUG -I${.CURDIR}/../include \
	-I${.CURDIR}/../des
NOMAN=	noman

.include <bsd.prog.mk>
