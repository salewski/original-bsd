.\" Copyright (c) 1983 Regents of the University of California.
.\" All rights reserved.  The Berkeley software License Agreement
.\" specifies the terms and conditions for redistribution.
.\"
.\"	@(#)5.t	6.1 (Berkeley) 05/26/86
.\"
.nr H2 1
.ds RH "Memory management
.NH
\s+2Memory management\s0
.PP
A single mechanism is used for data storage: memory buffers, or
\fImbuf\fP's.  An mbuf is a structure of the form:
.DS
._f
struct mbuf {
	struct	mbuf *m_next;		/* next buffer in chain */
	u_long	m_off;			/* offset of data */
	short	m_len;			/* amount of data in this mbuf */
	short	m_type;			/* mbuf type (accounting) */
	u_char	m_dat[MLEN];		/* data storage */
	struct	mbuf *m_act;		/* link in higher-level mbuf list */
};
.DE
The \fIm_next\fP field is used to chain mbufs together on linked
lists, while the \fIm_act\fP field allows lists of mbufs to be
accumulated.  By convention, the mbufs common to a single object
(for example, a packet) are chained together with the \fIm_next\fP
field, while groups of objects are linked via the \fIm_act\fP
field (possibly when in a queue).
.PP
Each mbuf has a small data area for storing information, \fIm_dat\fP.
The \fIm_len\fP field indicates the amount of data, while the \fIm_off\fP
field is an offset to the beginning of the data from the base of the
mbuf.  Thus, for example, the macro \fImtod\fP, which converts a pointer
to an mbuf to a pointer to the data stored in the mbuf, has the form
.DS
._d
#define	mtod(x,t)	((t)((int)(x) + (x)->m_off))
.DE
(note the \fIt\fP parameter, a C type cast, is used to cast
the resultant pointer for proper assignment).
.PP
In addition to storing data directly in the mbuf's data area, data
of page size may be also be stored in a separate area of memory.
The mbuf utility routines maintain
a pool of pages for this purpose and manipulate a private page map
for such pages.  The virtual addresses of
these data pages precede those of mbufs, so when pages of data are
separated from an mbuf, the mbuf data offset is a negative value.
An array of reference counts on pages is also maintained
so that copies of pages may be made without core to core
copying  (copies are created simply by duplicating the relevant
page table entries in the data page map and incrementing the associated
reference counts for the pages).  Separate data pages are currently
used only
when copying data from a user process into the kernel,
and when bringing data in at the hardware level.  Routines which
manipulate mbufs are not normally aware if data is stored directly in 
the mbuf data array, or if it is kept in separate pages.
.PP
The following utility routines are available for manipulating mbuf
chains:
.IP "m = m_copy(m0, off, len);"
.br
The \fIm_copy\fP routine create a copy of all, or part, of a
list of the mbufs in \fIm0\fP.  \fILen\fP bytes of data, starting 
\fIoff\fP bytes from the front of the chain, are copied. 
Where possible, reference counts on pages are used instead
of core to core copies.  The original mbuf chain must have at
least \fIoff\fP + \fIlen\fP bytes of data.  If \fIlen\fP is
specified as M_COPYALL, all the data present, offset
as before, is copied.  
.IP "m_cat(m, n);"
.br
The mbuf chain, \fIn\fP, is appended to the end of \fIm\fP.
Where possible, compaction is performed.
.IP "m_adj(m, diff);"
.br
The mbuf chain, \fIm\fP is adjusted in size by \fIdiff\fP
bytes.  If \fIdiff\fP is non-negative, \fIdiff\fP bytes
are shaved off the front of the mbuf chain.  If \fIdiff\fP
is negative, the alteration is performed from back to front.
No space is reclaimed in this operation, alterations are
accomplished by changing the \fIm_len\fP and \fIm_off\fP
fields of mbufs.
.IP "m = m_pullup(m0, size);"
.br
After a successful call to \fIm_pullup\fP, the mbuf at
the head of the returned list, \fIm\fP, is guaranteed
to have at least \fIsize\fP
bytes of data in contiguous memory (allowing access via
a pointer, obtained using the \fImtod\fP macro).
If the original data was less than \fIsize\fP bytes long,
\fIlen\fP was greater than the size of an mbuf data
area (112 bytes), or required resources were unavailable,
\fIm\fP is 0 and the original mbuf chain is deallocated.
.IP
This routine is particularly useful when verifying packet
header lengths on reception.  For example, if a packet is
received and only 8 of the necessary 16 bytes required
for a valid packet header are present at the head of the list
of mbufs representing the packet, the remaining 8 bytes
may be ``pulled up'' with a single \fIm_pullup\fP call.
If the call fails the invalid packet will have been discarded.
.PP
By insuring mbufs always reside on 128 byte boundaries
it is possible to always locate the mbuf associated with a data
area by masking off the low bits of the virtual address.
This allows modules to store data structures in mbufs and
pass them around without concern for locating the original
mbuf when it comes time to free the structure.
The \fIdtom\fP macro is used to convert a pointer into an mbuf's
data area to a pointer to the mbuf,
.DS
#define	dtom(x)	((struct mbuf *)((int)x & ~(MSIZE-1)))
.DE
.PP
Mbufs are used for dynamically allocated data structures such as
sockets, as well as memory allocated for packets.  Statistics are
maintained on mbuf usage and can be viewed by users using the
\fInetstat\fP(1) program.
.ds RH "Internal layering
.bp
