/*
 * $Id: dirent_opendir.c,v 1.11 2006-01-08 12:04:22 obarthel Exp $
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

#ifndef _DIRENT_HEADERS_H
#include "dirent_headers.h"
#endif /* _DIRENT_HEADERS_H */

/****************************************************************************/

#ifndef _STDLIB_MEMORY_H
#include "stdlib_memory.h"
#endif /* _STDLIB_MEMORY_H */

/****************************************************************************/

/* The following is not part of the ISO 'C' (1994) standard. */

/****************************************************************************/

#if defined(UNIX_PATH_SEMANTICS)

/****************************************************************************/

STATIC struct Node *
find_by_name(struct List * list,const char * name)
{
	struct Node * result = NULL;
	struct Node * node;

	for(node = list->lh_Head ; node->ln_Succ != NULL ; node = node->ln_Succ)
	{
		if(strcasecmp(node->ln_Name,name) == 0)
		{
			result = node;
			break;
		}
	}

	return(result);
}

/****************************************************************************/

#endif /* UNIX_PATH_SEMANTICS */

/****************************************************************************/

DIR *
opendir(const char * path_name)
{
	#if defined(UNIX_PATH_SEMANTICS)
	struct name_translation_info path_name_nti;
	#endif /* UNIX_PATH_SEMANTICS */
	struct DirectoryHandle * dh = NULL;
	DIR * result = NULL;

	ENTER();

	SHOWSTRING(path_name);

	assert( path_name != NULL );

	if(__check_abort_enabled)
		__check_abort();

	#if defined(CHECK_FOR_NULL_POINTERS)
	{
		if(path_name == NULL)
		{
			SHOWMSG("invalid parameter");

			__set_errno(EFAULT);
			goto out;
		}
	}
	#endif /* CHECK_FOR_NULL_POINTERS */

	dh = malloc(sizeof(*dh));
	if(dh == NULL)
	{
		SHOWMSG("memory allocation failed");
		goto out;
	}

	memset(dh,0,sizeof(*dh));

	#if defined(UNIX_PATH_SEMANTICS)
	{
		struct Node * node;

		NewList((struct List *)&dh->dh_VolumeList);

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

			SHOWSTRING(path_name);

			if(path_name_nti.is_root)
			{
				struct DosList * dol;
				UBYTE * name;

				SHOWMSG("collecting volume names");

				dh->dh_ScanVolumeList = TRUE;

				PROFILE_OFF();
				dol = LockDosList(LDF_VOLUMES|LDF_READ);
				PROFILE_ON();

				while((dol = NextDosEntry(dol,LDF_VOLUMES|LDF_READ)) != NULL)
				{
					name = BADDR(dol->dol_Name);
					if(name != NULL && name[0] > 0)
					{
						size_t len;

						len = name[0];

						node = malloc(sizeof(*node) + len + 2);
						if(node == NULL)
						{
							UnLockDosList(LDF_VOLUMES|LDF_READ);

							__set_errno(ENOMEM);
							goto out;
						}

						node->ln_Name = (char *)(node + 1);

						memmove(node->ln_Name,&name[1],len);
						node->ln_Name[len++]	= ':';
						node->ln_Name[len]		= '\0';

						/* Check if the name is already on the list. Mind you,
						   this is not the most sophisticated algorithm but then
						   the number of volumes should be small. */
						if(find_by_name((struct List *)&dh->dh_VolumeList,node->ln_Name) != NULL)
						{
							free(node);
							continue;
						}

						D(("adding '%s'",node->ln_Name));

						AddTail((struct List *)&dh->dh_VolumeList,node);
					}
				}

				UnLockDosList(LDF_VOLUMES|LDF_READ);

				/* Bail out if we cannot present anything. */
				if(IsMinListEmpty(&dh->dh_VolumeList))
				{
					__set_errno(ENOMEM);
					goto out;
				}
			}
		}
	}
	#endif /* UNIX_PATH_SEMANTICS */

	if(NOT dh->dh_ScanVolumeList)
	{
		LONG status;

		SHOWMSG("we are supposed to scan a directory");
		SHOWSTRING(path_name);

		PROFILE_OFF();
		dh->dh_DirLock = Lock((STRPTR)path_name,SHARED_LOCK);
		PROFILE_ON();

		if(dh->dh_DirLock == ZERO)
		{
			SHOWMSG("couldn't get a lock on it");

			__set_errno(__translate_access_io_error_to_errno(IoErr()));
			goto out;
		}

		assert( (((ULONG)&dh->dh_FileInfo) & 3) == 0 );

		PROFILE_OFF();
		status = Examine(dh->dh_DirLock,&dh->dh_FileInfo);
		PROFILE_ON();

		if(status == DOSFALSE)
		{
			SHOWMSG("couldn't examine it");

			__set_errno(__translate_io_error_to_errno(IoErr()));
			goto out;
		}

		if(dh->dh_FileInfo.fib_DirEntryType < 0)
		{
			SHOWMSG("this isn't a directory");

			__set_errno(ENOTDIR);
			goto out;
		}
	}

	SHOWMSG("OK, done");

	assert( __directory_list.mlh_Head != NULL );

	__dirent_lock();

	AddTail((struct List *)&__directory_list,(struct Node *)dh);

	__dirent_unlock();

	result = (DIR *)dh;
	dh = NULL;

 out:

	if(dh != NULL)
	{
		SHOWMSG("ouch. cleaning up");

		#if defined(UNIX_PATH_SEMANTICS)
		{
			struct Node * node;

			while((node = RemHead((struct List *)&dh->dh_VolumeList)) != NULL)
				free(node);
		}
		#endif /* UNIX_PATH_SEMANTICS */

		PROFILE_OFF();
		UnLock(dh->dh_DirLock);
		PROFILE_ON();

		free(dh);
	}

	RETURN(result);
	return(result);
}
