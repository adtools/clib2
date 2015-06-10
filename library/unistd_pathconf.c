/*
 * $Id: unistd_pathconf.c,v 1.1 2006-07-28 14:37:28 obarthel Exp $
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

long
pathconf(const char *path,int name)
{
	struct name_translation_info path_name_nti;
	struct DevProc * dvp = NULL;
	BOOL ignore_port = FALSE;
	long ret = -1;

	ENTER();

	SHOWSTRING(path);
	SHOWVALUE(name);

	#if defined(CHECK_FOR_NULL_POINTERS)
	{
		if(path == NULL)
		{
			SHOWMSG("invalid path name");

			__set_errno(EFAULT);
			goto out;
		}
	}
	#endif /* CHECK_FOR_NULL_POINTERS */

	#if defined(UNIX_PATH_SEMANTICS)
	{
		if(__unix_path_semantics)
		{
			if(path[0] == '\0')
			{
				SHOWMSG("Empty name");

				__set_errno(ENOENT);
				goto out;
			}

			if(__translate_unix_to_amiga_path_name(&path,&path_name_nti) != 0)
				goto out;

			if(path_name_nti.is_root)
			{
				/* Should we disallow / or use OFS as the lowest common denominator? */
				ignore_port = TRUE;
			}
		}
	}
	#endif /* UNIX_PATH_SEMANTICS */

	if(!ignore_port)
	{
		dvp = GetDeviceProc((STRPTR)path,NULL);
		if(dvp == NULL)
		{
			__set_errno(__translate_access_io_error_to_errno(IoErr()));
			goto out;
		}
	}

	ret = __pathconf((dvp != NULL) ? dvp->dvp_Port : NULL,name);

out:

	if(dvp != NULL)
	{
		FreeDeviceProc(dvp);
		dvp = NULL;
	}

	RETURN(ret);
	return(ret);
}
