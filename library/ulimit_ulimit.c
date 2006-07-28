/*
 * $Id: ulimit_ulimit.c,v 1.2 2006-07-28 14:37:27 obarthel Exp $
 *
 * :ts=4
 *
 * Portable ISO 'C' (1994) runtime library for the Amiga computer
 * Copyright (c) 2002-2006 by Olaf Barthel <olsen (at) sourcery.han.de>
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

#include <ulimit.h>

/****************************************************************************/

#ifndef _STDLIB_HEADERS_H
#include "stdlib_headers.h"
#endif /* _STDLIB_HEADERS_H */

/****************************************************************************/

/* The following is not part of the ISO 'C' (1994) standard. */

/****************************************************************************/

long
ulimit(int cmd,long newlim)
{
	long ret = -1;

	switch(cmd)
	{
		case UL_GETFSIZE:

			/* Maximum number of 512-byte blocks in a file. Largefile aware programs should not use ulimit() anyway. */
			ret = (0x7fffffffL >> 9) - 1L; /* Max Filesize/512 - 1 */
			break;

		case UL_GMEMLIM:	/* Which flags are appropriate for AvailMem()? */

			#if defined(__amigaos4__)
			{
				ret = AvailMem(MEMF_TOTAL|MEMF_VIRTUAL);
			}
			#else
			{
				ret = AvailMem(MEMF_ANY|MEMF_LARGEST);	/* Too conservative? */
			}
			#endif

			break;

		case UL_GDESLIM:	/* No limit, so we just return a reasonably large value. */

			ret = 1024;
			break;

		case UL_SETFSIZE:	/* Not supported */

			__set_errno(EPERM);
			goto out;

		default:

			__set_errno(EINVAL);
			goto out;
	}

 out:

	return(ret);
}
