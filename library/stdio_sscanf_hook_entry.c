/*
 * $Id: stdio_sscanf_hook_entry.c,v 1.1.1.1 2004-07-26 16:31:42 obarthel Exp $
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

#ifndef _STDIO_HEADERS_H
#include "stdio_headers.h"
#endif /* _STDIO_HEADERS_H */

/****************************************************************************/

void
__sscanf_hook_entry(
	struct Hook *				UNUSED	unused_hook,
	struct iob *						string_iob,
	struct file_hook_message *			message)
{
	int result = -1;
	int error = OK;
	int num_bytes;

	assert( message != NULL && string_iob != NULL );

	if(message->action != file_hook_action_read)
	{
		error = EBADF;
		goto out;
	}

	assert( string_iob->iob_StringPosition >= 0 );
	assert( string_iob->iob_StringLength >= 0 );

	if(string_iob->iob_StringPosition < string_iob->iob_StringLength)
	{
		int num_bytes_left;

		num_bytes_left = string_iob->iob_StringLength - string_iob->iob_StringPosition;

		num_bytes = message->size;
		if(num_bytes > num_bytes_left)
			num_bytes = num_bytes_left;

		assert( message->data != NULL );
		assert( num_bytes >= 0 );

		memmove(message->data,&string_iob->iob_String[string_iob->iob_StringPosition],(size_t)num_bytes);
		string_iob->iob_StringPosition += num_bytes;
	}
	else
	{
		num_bytes = 0;
	}

	result = num_bytes;

 out:

	message->result	= result;
	message->error	= error;
}
