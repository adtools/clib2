/*
 * $Id: stdio_translateioerror.c,v 1.4 2006-01-08 12:04:25 obarthel Exp $
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

#ifndef _STDIO_HEADERS_H
#include "stdio_headers.h"
#endif /* _STDIO_HEADERS_H */

/****************************************************************************/

int
__translate_io_error_to_errno(LONG io_err)
{
	static const struct { LONG io_err; LONG errno; } map_table[] =
	{
		{ ERROR_NO_FREE_STORE,			ENOMEM },
		{ ERROR_TASK_TABLE_FULL,		ENOMEM },
		{ ERROR_BAD_TEMPLATE,			EINVAL },
		{ ERROR_BAD_NUMBER,				EINVAL },
		{ ERROR_REQUIRED_ARG_MISSING,	EINVAL },
		{ ERROR_KEY_NEEDS_ARG,			EINVAL },
		{ ERROR_TOO_MANY_ARGS,			EINVAL },
		{ ERROR_UNMATCHED_QUOTES,		EINVAL },
		{ ERROR_LINE_TOO_LONG,			ENAMETOOLONG },
		{ ERROR_FILE_NOT_OBJECT,		ENOEXEC },
		{ ERROR_OBJECT_IN_USE,			EBUSY },
		{ ERROR_OBJECT_EXISTS,			EEXIST },
		{ ERROR_DIR_NOT_FOUND,			ENOENT },
		{ ERROR_OBJECT_NOT_FOUND,		ENOENT },
		{ ERROR_BAD_STREAM_NAME,		EINVAL },
		{ ERROR_OBJECT_TOO_LARGE,		EFBIG },
		{ ERROR_ACTION_NOT_KNOWN,		ENOSYS },
		{ ERROR_INVALID_COMPONENT_NAME,	EINVAL },
		{ ERROR_INVALID_LOCK,			EBADF },
		{ ERROR_OBJECT_WRONG_TYPE,		EFTYPE },
		{ ERROR_DISK_NOT_VALIDATED,		EROFS },
		{ ERROR_DISK_WRITE_PROTECTED,	EROFS },
		{ ERROR_RENAME_ACROSS_DEVICES,	EXDEV },
		{ ERROR_DIRECTORY_NOT_EMPTY,	ENOTEMPTY },
		{ ERROR_TOO_MANY_LEVELS,		ENAMETOOLONG },
		{ ERROR_DEVICE_NOT_MOUNTED,		ENXIO },
		{ ERROR_COMMENT_TOO_BIG,		ENAMETOOLONG },
		{ ERROR_DISK_FULL,				ENOSPC },
		{ ERROR_DELETE_PROTECTED,		EACCES },
		{ ERROR_WRITE_PROTECTED,		EACCES },
		{ ERROR_READ_PROTECTED,			EACCES },
		{ ERROR_NOT_A_DOS_DISK,			EFTYPE },
		{ ERROR_NO_DISK,				EACCES },
		{ ERROR_IS_SOFT_LINK,			EFTYPE },
		{ ERROR_BAD_HUNK,				ENOEXEC },
		{ ERROR_NOT_IMPLEMENTED,		ENOSYS },
		{ ERROR_LOCK_COLLISION,			EACCES },
		{ ERROR_BREAK,					EINTR },
		{ ERROR_NOT_EXECUTABLE,			ENOEXEC }
	};

	unsigned int i;
	int result;

	result = EIO;

	for(i = 0 ; i < NUM_ENTRIES(map_table) ; i++)
	{
		if(map_table[i].io_err == io_err)
		{
			result = map_table[i].errno;
			break;
		}
	}

	return(result);
}

/****************************************************************************/

/* Same as above, except that we translate ERROR_OBJECT_WRONG_TYPE
   into ENOTDIR by default. */
int
__translate_access_io_error_to_errno(LONG io_err)
{
	int result;

	if(io_err == ERROR_OBJECT_WRONG_TYPE)
		result = ENOTDIR;
	else
		result = __translate_io_error_to_errno(io_err);

	return(result);
}
