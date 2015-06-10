/*
 * $Id: stdio_tmpnam.c,v 1.6 2006-01-08 12:04:25 obarthel Exp $
 *
 * :ts=4
 *
 * Portable ISO 'C' (1994) runtime library for the Amiga computer
 * Copyright (c) 2002-2015 by Olaf Barthel <obarthel (at) gmx.net>
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

#ifndef _STDLIB_HEADERS_H
#include "stdlib_headers.h"
#endif /* _STDLIB_HEADERS_H */

/****************************************************************************/

char *
tmpnam(char *buf)
{
	static char local_buffer[L_tmpnam];
	static unsigned long counter;

	APTR old_window_pointer;
	unsigned long c;
	char * result = NULL; /* ZZZ compiler claims that this assignment is unnecessary. */
	BPTR lock;
	int i;

	ENTER();

	if(__check_abort_enabled)
		__check_abort();

	/* If no user-supplied buffer is available, use the local one. */
	if(buf == NULL)
		buf = local_buffer;

	while(TRUE)
	{
		if(__check_abort_enabled)
			__check_abort();

		c = counter;

		counter = (counter + 1) % TMP_MAX;

		/* Build another temporary file name, which begins with the
		   letters 'tmp' followed by an octal number. */
		strcpy(buf,"tmp");

		/* There's room for L_tmpnam - 4 digits, which for
		   L_tmpnam == 10 leaves room for 6 * 3 bits. */
		for(i = 3 ; i < L_tmpnam-1 ; i++)
		{
			buf[i] = '0' + (c % 8);
			c = (c / 8);
		}

		buf[i] = '\0';

		D(("checking if '%s' exists",buf));

		/* Turn off DOS error requesters. */
		old_window_pointer = __set_process_window((APTR)-1);

		/* Does this object exist already? */
		PROFILE_OFF();
		lock = Lock(buf,SHARED_LOCK);
		PROFILE_ON();

		/* Restore DOS requesters. */
		__set_process_window(old_window_pointer);

		if(lock == ZERO)
		{
			/* If the object does not exist yet then we
			   are finished. */
			if(IoErr() == ERROR_OBJECT_NOT_FOUND)
				result = buf;
			else
				__set_errno(__translate_io_error_to_errno(IoErr()));

			break;
		}

		/* OK, so it exists already. Start over... */

		PROFILE_OFF();
		UnLock(lock);
		PROFILE_ON();
	}

	SHOWSTRING(result);

	RETURN(result);
	return(result);
}
