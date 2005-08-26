/*
 * $Id: unistd_chown.c,v 1.8 2005-08-26 12:39:33 obarthel Exp $
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
chown(const char * path_name, uid_t owner, gid_t group)
{
	#if defined(UNIX_PATH_SEMANTICS)
	struct name_translation_info path_name_nti;
	#endif /* UNIX_PATH_SEMANTICS */
	struct DevProc * dvp = NULL;
	LONG status;
	int result = ERROR;

	ENTER();

	SHOWSTRING(path_name);
	SHOWVALUE(owner);
	SHOWVALUE(group);

	assert( path_name != NULL );

	if(__check_abort_enabled)
		__check_abort();

	#if defined(CHECK_FOR_NULL_POINTERS)
	{
		if(path_name == NULL)
		{
			SHOWMSG("invalid path name");

			__set_errno(EFAULT);
			goto out;
		}
	}
	#endif /* CHECK_FOR_NULL_POINTERS */

	if(owner > 65535 || group > 65535)
	{
		SHOWMSG("invalid owner or group");

		__set_errno(EINVAL);
		goto out;
	}

	#if defined(UNIX_PATH_SEMANTICS)
	{
		if(__unix_path_semantics)
		{
			if(path_name[0] == '\0')
			{
				SHOWMSG("no name given");

				__set_errno(ENOENT);
				goto out;
			}

			if(__translate_unix_to_amiga_path_name(&path_name,&path_name_nti) != 0)
				goto out;

			if(path_name_nti.is_root)
			{
				__set_errno(EACCES);
				goto out;
			}
		}
	}
	#endif /* UNIX_PATH_SEMANTICS */

	D(("changing owner of '%s'",path_name));

	#if defined(__amigaos4__)
	{
		PROFILE_OFF();
		status = SetOwner((STRPTR)path_name,(LONG)((((ULONG)owner) << 16) | group));
		PROFILE_ON();
	}
	#else
	{
		if(((struct Library *)DOSBase)->lib_Version >= 39)
		{
			PROFILE_OFF();
			status = SetOwner((STRPTR)path_name,(LONG)((((ULONG)owner) << 16) | group));
			PROFILE_ON();
		}
		else
		{
			D_S(struct bcpl_name,new_name);
			size_t len;

			len = strlen(path_name);
			if(len >= sizeof(new_name->name))
			{
				__set_errno(ENAMETOOLONG);
				goto out;
			}

			PROFILE_OFF();
			dvp = GetDeviceProc((STRPTR)path_name,NULL);
			PROFILE_ON();

			if(dvp == NULL)
			{
				__set_errno(__translate_io_error_to_errno(IoErr()));
				goto out;
			}

			new_name->name[0] = len;
			memmove(&new_name->name[1],path_name,len);

			PROFILE_OFF();
			status = DoPkt(dvp->dvp_Port,ACTION_SET_OWNER,dvp->dvp_Lock,MKBADDR(new_name),(LONG)((((ULONG)owner) << 16) | group),0,0);
			PROFILE_ON();
		}
	}
	#endif /* __amigaos4__ */

	if(status == DOSFALSE)
	{
		__set_errno(__translate_io_error_to_errno(IoErr()));
		goto out;
	}

	result = OK;

 out:

	PROFILE_OFF();
	FreeDeviceProc(dvp);
	PROFILE_ON();

	RETURN(result);
	return(result);
}
