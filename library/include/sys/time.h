/*
 * $Id: time.h,v 1.5 2004-09-09 10:56:24 obarthel Exp $
 *
 * :ts=4
 *
 * Portable ISO 'C' (1994) runtime library for the Amiga computer
 * Copyright (c) 2002-2004 by Olaf Barthel <olsen@sourcery.han.de>
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

#ifndef _SYS_TIME_H
#define _SYS_TIME_H

/****************************************************************************/

/* The following is not part of the ISO 'C' (1994) standard. */

/****************************************************************************/

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/****************************************************************************/

/* This is a tough one. Some code takes a bad turn if the <exec/types.h>
   header file is pulled in, which rules out that the structure definitions
   in <devices/timer.h> are used. We are only interested in the timeval
   structure and try to get by with this definition. Sometimes it works,
   sometimes it doesn't. Not sure if there really is a good solution for
   this problem... */
#ifndef __TIMEVAL_ALREADY_DEFINED

/****************************************************************************/

#ifndef DEVICES_TIMER_H

/****************************************************************************/

/* The V40 header files (OS 3.x) and below will always define the
   'struct timeval' in <devices/timer.h>. But the V50 header
   files and beyond will not define 'struct timeval' if it is
   included from here. We start by checking which header files
   are being used. */
#ifndef EXEC_TYPES_H
#include <exec/types.h>
#endif /* EXEC_TYPES_H */

/****************************************************************************/

/* Which header file version is being used? */
#if INCLUDE_VERSION < 50

/****************************************************************************/

/* This will define the 'struct timeval' */
#include <devices/timer.h>

/****************************************************************************/

#else

/****************************************************************************/

/* We will have to make our own... */
struct timeval
{
	unsigned long tv_secs;
	unsigned long tv_micro;
};

/* Make sure that the 'struct timeval' is not redefined, should
   <devices/timer.h> get included again. */
#define __TIMEVAL_ALREADY_DEFINED

/****************************************************************************/

#endif /* INCLUDE_VERSION */

/****************************************************************************/

#endif /* DEVICES_TIMER_H */

/****************************************************************************/

#endif /* __TIMEVAL_ALREADY_DEFINED */

/****************************************************************************/

#ifdef DEVICES_TIMER_H

/****************************************************************************/

#ifndef tv_sec
#define tv_sec tv_secs
#endif /* tv_sec */

#ifndef tv_usec
#define tv_usec tv_micro
#endif /* tv_usec */

/****************************************************************************/

#endif /* DEVICES_TIMER_H */

/****************************************************************************/

struct timezone
{
	int tz_minuteswest;	/* of Greenwich */
	int tz_dsttime;		/* type of dst correction to apply */
};

/****************************************************************************/

int gettimeofday(struct timeval *tp, struct timezone *tzp);

/****************************************************************************/

#ifdef __cplusplus
}
#endif /* __cplusplus */

/****************************************************************************/

#endif /* _SYS_TIME_H */
