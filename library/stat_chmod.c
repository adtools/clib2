/*
 * $Id: stat_chmod.c,v 1.2 2004-08-07 09:15:32 obarthel Exp $
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

#ifndef _STAT_HEADERS_H
#include "stat_headers.h"
#endif /* _STAT_HEADERS_H */

/****************************************************************************/

/* The following is not part of the ISO 'C' (1994) standard. */

/****************************************************************************/

int
chmod(const char * path_name, mode_t mode)
{
	#if defined(UNIX_PATH_SEMANTICS)
	struct name_translation_info path_name_nti;
	#endif /* UNIX_PATH_SEMANTICS */
	ULONG protection;
	LONG status;
	int result = -1;

	ENTER();

	SHOWSTRING(path_name);
	SHOWVALUE(mode);

	assert( path_name != NULL );

	#if defined(CHECK_FOR_NULL_POINTERS)
	{
		if(path_name == NULL)
		{
			SHOWMSG("invalid path parameter");	

			errno = EFAULT;
			goto out;
		}
	}
	#endif /* CHECK_FOR_NULL_POINTERS */

	if(__check_abort_enabled)
		__check_abort();

	#if defined(UNIX_PATH_SEMANTICS)
	{
		if(__unix_path_semantics)
		{
			if(__translate_unix_to_amiga_path_name(&path_name,&path_name_nti) != 0)
				goto out;

			if(path_name_nti.is_root)
			{
				errno = EACCES;
				goto out;
			}
		}
	}
	#endif /* UNIX_PATH_SEMANTICS */

	protection = 0;

	if(FLAG_IS_SET(mode,S_IRUSR))
		SET_FLAG(protection,FIBF_READ);

	if(FLAG_IS_SET(mode,S_IWUSR))
	{
		SET_FLAG(protection,FIBF_WRITE);
		SET_FLAG(protection,FIBF_DELETE);
	}

	if(FLAG_IS_SET(mode,S_IXUSR))
		SET_FLAG(protection,FIBF_EXECUTE);

	if(FLAG_IS_SET(mode,S_IRGRP))
		SET_FLAG(protection,FIBF_GRP_READ);

	if(FLAG_IS_SET(mode,S_IWGRP))
	{
		SET_FLAG(protection,FIBF_GRP_WRITE);
		SET_FLAG(protection,FIBF_GRP_DELETE);
	}

	if(FLAG_IS_SET(mode,S_IXGRP))
		SET_FLAG(protection,FIBF_GRP_EXECUTE);

	if(FLAG_IS_SET(mode,S_IROTH))
		SET_FLAG(protection,FIBF_OTR_READ);

	if(FLAG_IS_SET(mode,S_IWOTH))
	{
		SET_FLAG(protection,FIBF_OTR_WRITE);
		SET_FLAG(protection,FIBF_OTR_DELETE);
	}

	if(FLAG_IS_SET(mode,S_IXOTH))
		SET_FLAG(protection,FIBF_OTR_EXECUTE);

	SHOWSTRING(path_name);
	SHOWVALUE(protection);

	PROFILE_OFF();
	status = SetProtection((STRPTR)path_name,(LONG)(protection ^ (FIBF_READ|FIBF_WRITE|FIBF_EXECUTE|FIBF_DELETE)));
	PROFILE_ON();

	if(status == DOSFALSE)
	{
		__translate_io_error_to_errno(IoErr(),&errno);
		goto out;
	}

	result = 0;

 out:

	RETURN(result);
	return(result);
}
