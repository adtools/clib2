/*
 * $Id: unistd_unlink.c,v 1.3 2005-01-02 09:07:19 obarthel Exp $
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

#ifndef _UNISTD_HEADERS_H
#include "unistd_headers.h"
#endif /* _UNISTD_HEADERS_H */

/****************************************************************************/

/* The following is not part of the ISO 'C' (1994) standard. */

/****************************************************************************/

int
unlink(const char * path_name)
{
	#if defined(UNIX_PATH_SEMANTICS)
	DECLARE_UTILITYBASE();
	struct name_translation_info path_name_nti;
	#endif /* UNIX_PATH_SEMANTICS */
	BPTR current_dir = ZERO;
	int result = -1;
	LONG status;

	ENTER();

	SHOWSTRING(path_name);

	assert( path_name != NULL );

	#if defined(CHECK_FOR_NULL_POINTERS)
	{
		if(path_name == NULL)
		{
			SHOWMSG("invalid path name");

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

	D(("trying to delete '%s'",path_name));

	PROFILE_OFF();
	status = DeleteFile((STRPTR)path_name);
	PROFILE_ON();

	if(status == DOSFALSE)
	{
		#if defined(UNIX_PATH_SEMANTICS)
		{
			struct UnlinkNode * node;
			struct UnlinkNode * uln;
			BOOL found = FALSE;

			assert( UtilityBase != NULL );

			if(IoErr() != ERROR_OBJECT_IN_USE)
			{
				__translate_access_io_error_to_errno(IoErr(),&errno);
				goto out;
			}

			SHOWMSG("will try to delete that file later");

			/* ZZZ we should be locking the path's parent directory
			   and then just remember what the last part of the path
			   pointed to. */
			PROFILE_OFF();
			current_dir = Lock("",SHARED_LOCK);
			PROFILE_ON();

			if(current_dir == ZERO)
			{
				__translate_io_error_to_errno(IoErr(),&errno);
				goto out;
			}

			PROFILE_OFF();

			assert( __unlink_list.mlh_Head != NULL );

			for(node = (struct UnlinkNode *)__unlink_list.mlh_Head ;
			    node->uln_MinNode.mln_Succ != NULL ;
			    node = (struct UnlinkNode *)node->uln_MinNode.mln_Succ)
			{
				if(Stricmp(node->uln_Name,path_name) == SAME && SameLock(node->uln_Lock,current_dir) == LOCK_SAME)
				{
					found = TRUE;
					break;
				}
			}

			PROFILE_ON();

			if(NOT found)
			{
				uln = malloc(sizeof(*uln) + strlen(path_name) + 1);
				if(uln == NULL)
					goto out;

				uln->uln_Lock = current_dir;
				uln->uln_Name = (char *)(uln + 1);

				strcpy(uln->uln_Name,path_name);
				AddTail((struct List *)&__unlink_list,(struct Node *)uln);

				current_dir = ZERO;
			}
		}
		#else
		{
			__translate_io_error_to_errno(IoErr(),&errno);
			goto out;
		}
		#endif /* UNIX_PATH_SEMANTICS */
	}

	result = 0;

 out:

	PROFILE_OFF();
	UnLock(current_dir);
	PROFILE_ON();

	RETURN(result);
	return(result);
}
