/*
 * $Id: unistd_fileno.c,v 1.5 2005-02-27 18:09:11 obarthel Exp $
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

#ifndef _STDLIB_NULL_POINTER_CHECK_H
#include "stdlib_null_pointer_check.h"
#endif /* _STDLIB_NULL_POINTER_CHECK_H */

/****************************************************************************/

#ifndef _UNISTD_HEADERS_H
#include "unistd_headers.h"
#endif /* _UNISTD_HEADERS_H */

/****************************************************************************/

/* The following is not part of the ISO 'C' (1994) standard. */

/****************************************************************************/

int
fileno(FILE * file)
{
	struct iob * iob = (struct iob *)file;
	int result = -1;

	ENTER();

	SHOWPOINTER(file);

	assert( file != NULL );

	if(__check_abort_enabled)
		__check_abort();

	flockfile(file);

	#if defined(CHECK_FOR_NULL_POINTERS)
	{
		if(file == NULL)
		{
			SHOWMSG("invalid file parameter");

			__set_errno(EFAULT);
			goto out;
		}
	}
	#endif /* CHECK_FOR_NULL_POINTERS */

	assert( __is_valid_iob(iob) );
	assert( FLAG_IS_SET(iob->iob_Flags,IOBF_IN_USE) );

	if(FLAG_IS_CLEAR(iob->iob_Flags,IOBF_IN_USE))
	{
		__set_errno(EBADF);
		goto out;
	}

	assert( iob->iob_Descriptor >= 0 && iob->iob_Descriptor < __num_fd );
	assert( __fd[iob->iob_Descriptor] != NULL );
	assert( FLAG_IS_SET(__fd[iob->iob_Descriptor]->fd_Flags,FDF_IN_USE) );

	result = iob->iob_Descriptor;

 out:

	funlockfile(file);

	RETURN(result);
	return(result);
}
