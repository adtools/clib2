/*
 * $Id: stdio_fclose.c,v 1.7 2005-02-28 10:07:30 obarthel Exp $
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

#ifndef _STDLIB_MEM_DEBUG_H
#include "stdlib_mem_debug.h"
#endif /* _STDLIB_MEM_DEBUG_H */

/****************************************************************************/

#ifndef _STDIO_HEADERS_H
#include "stdio_headers.h"
#endif /* _STDIO_HEADERS_H */

/****************************************************************************/

int
fclose(FILE *stream)
{
	struct iob * file = (struct iob *)stream;
	struct file_action_message fam;
	int result = 0;

	ENTER();

	SHOWPOINTER(stream);

	assert( stream != NULL );

	assert( file->iob_Lock == NULL || file->iob_Lock->ss_Owner == NULL );

	if(__check_abort_enabled)
		__check_abort();

	#if defined(CHECK_FOR_NULL_POINTERS)
	{
		if(stream == NULL)
		{
			SHOWMSG("invalid stream parameter");

			result = EOF;

			__set_errno(EFAULT);
			goto out;
		}
	}
	#endif /* CHECK_FOR_NULL_POINTERS */

	assert( __is_valid_iob(file) );
	assert( FLAG_IS_SET(file->iob_Flags,IOBF_IN_USE) );
	assert( file->iob_BufferSize > 0 );

	if(FLAG_IS_CLEAR(file->iob_Flags,IOBF_IN_USE))
	{
		SHOWMSG("this file is not even in use");

		result = EOF;

		__set_errno(EBADF);
		goto out;
	}

	/* Push back any buffered data to the stream. */
	if(__iob_write_buffer_is_valid(file) && __flush_iob_write_buffer(file) < 0)
		result = EOF;

	/* Make sure that the stream is closed. */
	SHOWMSG("calling the action function");

	fam.fam_Action = file_action_close;

	assert( file->iob_Action != NULL );

	if((*file->iob_Action)(file,&fam) < 0 && result != EOF)
	{
		result = EOF;

		__set_errno(fam.fam_Error);
	}

	/* Now that the file is closed and we are in fact
	 * dealing with a temporary file, delete that file.
	 */
	if(FLAG_IS_SET(file->iob_Flags,IOBF_TEMP))
	{
		BPTR old_current_dir;

		SHOWMSG("this is a temporary file; deleting it");

		assert( file->iob_TempFileName != NULL );

		PROFILE_OFF();

		old_current_dir = CurrentDir(file->iob_TempFileLock);
		DeleteFile(file->iob_TempFileName);
		CurrentDir(old_current_dir);

		UnLock(file->iob_TempFileLock);

		PROFILE_ON();

		free(file->iob_TempFileName);
	}

	/* Get rid of any custom file buffer allocated. */
	if(file->iob_CustomBuffer != NULL)
		free(file->iob_CustomBuffer);

	/* Free the lock semaphore now. */
	FreeVec(file->iob_Lock);

	memset(file,0,sizeof(*file));

 out:

	RETURN(result);
	return(result);
}
