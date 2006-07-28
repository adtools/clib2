/*
 * $Id: resource_getrlimit.c,v 1.1 2006-07-28 13:36:16 obarthel Exp $
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

#include <sys/resource.h>

/****************************************************************************/

#ifndef _STDLIB_HEADERS_H
#include "stdlib_headers.h"
#endif /* _STDLIB_HEADERS_H */

/****************************************************************************/

int
getrlimit(int resource,struct rlimit *rlp)
{
	struct Task *self;
	int ret = -1;
	rlim_t l;

	if(rlp == NULL)
	{
		__set_errno(EFAULT);
		goto out;
	}

	switch(resource)
	{
		case RLIM_VMEM:

			rlp->rlim_cur = RLIM_INFINITY;
			rlp->rlim_max = RLIM_INFINITY;
			break;

		case RLIM_CORE:	/* Coredumps are not supported. */

			rlp->rlim_cur = 0;
			rlp->rlim_max = 0;
			break;

		case RLIM_CPU:

			rlp->rlim_cur = RLIM_INFINITY;
			rlp->rlim_max = RLIM_INFINITY;
			break;

		case RLIM_DATA:

			#if defined(__amigaos4__)
			{
		 		l = AvailMem(MEMF_TOTAL|MEMF_VIRTUAL);
		 	}
		 	#else
			{
		 		l = AvailMem(MEMF_TOTAL);
		 	}
		 	#endif /* __amigaos4__ */

			rlp->rlim_cur = l;
			rlp->rlim_max = l;
			break;

		case RLIM_FSIZE:

			rlp->rlim_cur = RLIM_INFINITY;	/* Use RLIM_INFINITY in case we have a 64-bit fs. pathconf() can be more precise. */
			rlp->rlim_max = RLIM_INFINITY;
			break;

		case RLIM_NOFILE:

			rlp->rlim_cur = RLIM_INFINITY;
			rlp->rlim_max = RLIM_INFINITY;
			break;

		case RLIM_STACK:	/* Return current stacksize. */

			self = FindTask(NULL);

			l = (char *)self->tc_SPUpper - (char *)self->tc_SPLower;

			rlp->rlim_cur = l;
			rlp->rlim_max = RLIM_INFINITY;

			break;

		default:

			__set_errno(EINVAL);
			goto out;
	}

	ret = 0;

 out:

	return(ret);
}
