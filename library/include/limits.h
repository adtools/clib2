/*
 * $Id: limits.h,v 1.3 2004-09-20 17:16:07 obarthel Exp $
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

#ifndef _LIMITS_H
#define _LIMITS_H

/****************************************************************************/

#define CHAR_BIT	8
#define CHAR_MAX	127
#define CHAR_MIN	-128
#define INT_MAX		2147483647L
#define INT_MIN		(-2147483647L - 1)
#define LONG_MAX	2147483647L
#define LONG_MIN	(-2147483647L - 1)
#define SCHAR_MAX	127
#define SCHAR_MIN	-128
#define SHRT_MAX	32767
#define SHRT_MIN	-32768
#define UCHAR_MAX	255
#define UINT_MAX	4294967295UL
#define ULONG_MAX	4294967295UL
#define USHRT_MAX	65535

/****************************************************************************/

/* The following is not part of the ISO 'C' (1994) standard. */

/****************************************************************************/

#define	LLONG_MIN	(-0x7fffffffffffffffLL-1)
#define	LLONG_MAX	0x7fffffffffffffffLL
#define	ULLONG_MAX	0xffffffffffffffffULL

/****************************************************************************/

#define PATH_MAX 1024

/****************************************************************************/

#endif /* _LIMITS_H */
