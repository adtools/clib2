/*
 * $Id: select.h,v 1.2 2005-11-06 14:37:49 obarthel Exp $
 *
 * :ts=4
 *
 * Portable ISO 'C' (1994) runtime library for the Amiga computer
 * Copyright (c) 2002-2005 by Olaf Barthel <olsen@sourcery.han.de>
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 *   - Redistributions of source code must retain the above copyright
 *     notice, this list of conditions and the following disclaimer.
 *
 *   - Neither the name of Olaf Barthel nor the names of contributors
 *     may be used to endorse or promote products derived from this
 *     software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef _SYS_SELECT_H
#define _SYS_SELECT_H

/****************************************************************************/

/* The following is not part of the ISO 'C' (1994) standard. */

/****************************************************************************/

#ifndef _STRING_H
#include <string.h>
#endif /* _STRING_H */

#ifndef _SYS_TIME_H
#include <sys/time.h>
#endif /* _SYS_TIME_H */

/****************************************************************************/

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/****************************************************************************/

/*
 * select() uses bit masks of file descriptors in longs. These macros
 * manipulate such bit fields.
 *
 * FD_SETSIZE may be defined by the user, but the default here should
 * be enough for most uses.
 */
#ifndef	FD_SETSIZE
#define	FD_SETSIZE 256
#endif

typedef	struct fd_set
{
	unsigned long bits[(FD_SETSIZE + 31) / 32];
} fd_set;

#define	FD_SET(n,p)		((void)(((unsigned long)n) < FD_SETSIZE ? (p)->bits[((unsigned long)n) >> 5] |=  (1UL << (((unsigned long)n) & 31)) : 0))
#define	FD_CLR(n,p)		((void)(((unsigned long)n) < FD_SETSIZE ? (p)->bits[((unsigned long)n) >> 5] &= ~(1UL << (((unsigned long)n) & 31)) : 0))
#define	FD_ISSET(n,p)	(((unsigned long)n) < FD_SETSIZE && ((p)->bits[((unsigned long)n) >> 5] & (1UL << (((unsigned long)n) & 31))) != 0)
#define	FD_COPY(f,t)	((void)memmove(t,f,sizeof(*(f))))
#define	FD_ZERO(p)		((void)memset(p,0,sizeof(*(p))))

/****************************************************************************/

/*
 * The following prototypes may clash with the bsdsocket.library or
 * usergroup.library API definitions.
 */

#ifndef __NO_NET_API

extern int select(int nfds, fd_set *readfds,fd_set *writefds, fd_set *errorfds,struct timeval *timeout);

#endif /* __NO_NET_API */

/****************************************************************************/

#ifdef __cplusplus
}
#endif /* __cplusplus */

/****************************************************************************/

#endif /* _SYS_SELECT_H */
