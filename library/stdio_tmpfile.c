/*
 * $Id: stdio_tmpfile.c,v 1.1.1.1 2004-07-26 16:31:42 obarthel Exp $
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

#ifndef _STDLIB_MEM_DEBUG_H
#include "stdlib_mem_debug.h"
#endif /* _STDLIB_MEM_DEBUG_H */

/****************************************************************************/

#ifndef _STDIO_HEADERS_H
#include "stdio_headers.h"
#endif /* _STDIO_HEADERS_H */

/****************************************************************************/

FILE *
tmpfile(void)
{
	BPTR temp_file_lock = ZERO;
	char * temp_file_name;
	FILE * result = NULL;
	struct iob * file;

	ENTER();

	if(__check_abort_enabled)
		__check_abort();

	temp_file_name = malloc(L_tmpnam);
	if(temp_file_name == NULL)
	{
		SHOWMSG("not enough memory for temp file name");
		goto out;
	}

	/* Get a temp_file_lock on the current directory; it will be needed
	 * later when the temporary file is deleted.
	 */

	PROFILE_OFF();
	temp_file_lock = Lock("",SHARED_LOCK);
	PROFILE_ON();

	if(temp_file_lock == ZERO)
	{
		SHOWMSG("couldn't get a temp_file_lock on the current directory");

		__translate_io_error_to_errno(IoErr(),&errno);
		goto out;
	}

	/* Find a new temporary file name. */
	tmpnam(temp_file_name);

	SHOWSTRING(temp_file_name);

	/* Open that file; we're going to need it in a minute. */
	file = (struct iob *)fopen(temp_file_name,"w+");
	if(file == NULL)
	{
		SHOWMSG("file didn't open");
		goto out;
	}

	/* Remember this for later when the temporary file is
	 * to be deleted.
	 */
	SET_FLAG(file->iob_Flags,IOBF_TEMP);

	file->iob_TempFileName = temp_file_name;
	temp_file_name = NULL;

	file->iob_TempFileLock = temp_file_lock;
	temp_file_lock = ZERO;

	result = (FILE *)file;

 out:

	PROFILE_OFF();
	UnLock(temp_file_lock);
	PROFILE_ON();

	if(temp_file_name != NULL)
		free(temp_file_name);

	RETURN(result);
	return(result);
}
