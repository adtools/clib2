/*
 * $Id: stdio_findvacantiobentry.c,v 1.1.1.1 2004-07-26 16:31:30 obarthel Exp $
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

#ifndef _STDIO_HEADERS_H
#include "stdio_headers.h"
#endif /* _STDIO_HEADERS_H */

/****************************************************************************/

#ifndef NDEBUG

/****************************************************************************/

BOOL
__is_valid_iob(struct iob * iob)
{
	BOOL result = FALSE;

	if(iob != NULL && FLAG_IS_SET(iob->iob_Flags,IOBF_INTERNAL))
	{
		/* This is used by vsprintf(), etc. */
		result = TRUE;
	}
	else
	{
		if(__iob != NULL && __num_iob > 0 && 0 <= iob->iob_SlotNumber && iob->iob_SlotNumber < __num_iob && __iob[iob->iob_SlotNumber] == iob)
			result = TRUE;
	}

	return(result);
}

/****************************************************************************/

#endif /* NDEBUG */

/****************************************************************************/

int
__find_vacant_iob_entry(void)
{
	int result = -1;
	int i;

	assert( __iob != NULL || __num_iob == 0 );

	for(i = 0 ; i < __num_iob ; i++)
	{
		if(FLAG_IS_CLEAR(__iob[i]->iob_Flags,IOBF_IN_USE))
		{
			result = i;
			break;
		}
	}

	return(result);
}
