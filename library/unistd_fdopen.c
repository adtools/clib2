/*
 * $Id: unistd_fdopen.c,v 1.2 2004-08-07 09:15:32 obarthel Exp $
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

FILE *
fdopen(int file_descriptor, const char * type)
{
	FILE * result = NULL;
	int slot_number;

	ENTER();

	SHOWVALUE(file_descriptor);
	SHOWSTRING(type);

	assert(type != NULL);

	#if defined(CHECK_FOR_NULL_POINTERS)
	{
		if(type == NULL)
		{
			SHOWMSG("invalid type parameter");

			errno = EFAULT;
			goto out;
		}
	}
	#endif /* CHECK_FOR_NULL_POINTERS */

	if(__check_abort_enabled)
		__check_abort();

	slot_number = __find_vacant_iob_entry();
	if(slot_number < 0)
	{
		if(__grow_iob_table() < 0)
		{
			SHOWMSG("not enough memory for a file buffer slot");
			goto out;
		}

		slot_number = __find_vacant_iob_entry();
		assert( slot_number >= 0 );
	}

	assert( file_descriptor >= 0 && file_descriptor < __num_fd );
	assert( __fd[file_descriptor] != NULL );
	assert( FLAG_IS_SET(__fd[file_descriptor]->fd_Flags,FDF_IN_USE) );

	if(__open_iob(NULL, type, file_descriptor, slot_number) < 0)
	{
		SHOWMSG("couldn't open the file for the file descriptor");
		goto out;
	}

	result = (FILE *)__iob[slot_number];

 out:

	RETURN(result);
	return(result);
}
