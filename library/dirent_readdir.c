/*
 * $Id: dirent_readdir.c,v 1.10 2006-09-25 14:51:15 obarthel Exp $
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

#ifndef _DIRENT_HEADERS_H
#include "dirent_headers.h"
#endif /* _DIRENT_HEADERS_H */

/****************************************************************************/

/* The following is not part of the ISO 'C' (1994) standard. */

/****************************************************************************/

struct dirent *
readdir(DIR * directory_pointer)
{
	struct dirent * result = NULL;
	struct DirectoryHandle * dh;
	BPTR parent_directory = ZERO;

	ENTER();

	SHOWPOINTER(directory_pointer);

	if(__check_abort_enabled)
		__check_abort();

	PROFILE_OFF();

	if(directory_pointer == NULL)
	{
		SHOWMSG("ouch. invalid parameter");

		__set_errno(EBADF);
		goto out;
	}

	dh = (struct DirectoryHandle *)directory_pointer;

	#if defined(UNIX_PATH_SEMANTICS)
	{
		if(__unix_path_semantics && dh->dh_ScanVolumeList)
		{
			SHOWMSG("we are scanning the volume list");

			if(dh->dh_Position == 0)
			{
				SHOWMSG("returning the .");

				dh->dh_Position++;

				dh->dh_DirectoryEntry.d_ino = 0;
				strcpy(dh->dh_DirectoryEntry.d_name,".");

				result = &dh->dh_DirectoryEntry;
			}
			else
			{
				D_S(struct FileInfoBlock,fib);
				D_S(struct bcpl_name,bcpl_name);
				char * name = (char *)bcpl_name->name;
				BPTR dir_lock;

				assert( (((ULONG)name) & 3) == 0 );

				if(dh->dh_VolumeNode == NULL && NOT IsMinListEmpty(&dh->dh_VolumeList))
					dh->dh_VolumeNode = (struct Node *)dh->dh_VolumeList.mlh_Head;

				strcpy(name,"\1:"); /* BSTR for ":" */

				while(result == NULL && dh->dh_VolumeNode != NULL && dh->dh_VolumeNode->ln_Succ != NULL)
				{
					if(IsFileSystem(dh->dh_VolumeNode->ln_Name))
					{
						struct DevProc * dvp;

						dvp = GetDeviceProc(dh->dh_VolumeNode->ln_Name,NULL);
						if(dvp != NULL)
						{
							dir_lock = DoPkt(dvp->dvp_Port,ACTION_LOCATE_OBJECT,ZERO,MKBADDR(name),SHARED_LOCK,	0,0);
							if(dir_lock != ZERO)
							{
								if(Examine(dir_lock,fib))
								{
									assert( sizeof(dh->dh_DirectoryEntry.d_name) >= sizeof(fib->fib_FileName) );

									strcpy(dh->dh_DirectoryEntry.d_name,fib->fib_FileName);

									dh->dh_DirectoryEntry.d_ino = fib->fib_DiskKey;

									result = &dh->dh_DirectoryEntry;
								}

								UnLock(dir_lock);
							}

							FreeDeviceProc(dvp);
						}
					}

					dh->dh_VolumeNode = dh->dh_VolumeNode->ln_Succ;
				}
			}
		}
	}
	#endif /* UNIX_PATH_SEMANTICS */

	if(NOT dh->dh_ScanVolumeList)
	{
		#if defined(UNIX_PATH_SEMANTICS)
		{
			if(__unix_path_semantics)
			{
				if(dh->dh_Position == 0)
				{
					SHOWMSG("returning .");

					dh->dh_Position++;

					dh->dh_DirectoryEntry.d_ino = dh->dh_FileInfo.fib_DiskKey;

					strcpy(dh->dh_DirectoryEntry.d_name,".");

					result = &dh->dh_DirectoryEntry;
				}
				else if (dh->dh_Position == 1)
				{
					D_S(struct FileInfoBlock,fib);

					dh->dh_Position++;

					parent_directory = ParentDir(dh->dh_DirLock);
					if(parent_directory != ZERO)
					{
						if(CANNOT Examine(parent_directory,fib))
						{
							__set_errno(__translate_io_error_to_errno(IoErr()));
							goto out;
						}
					}
					else
					{
						/* This is the virtual root directory's key. */
						fib->fib_DiskKey = 0;
					}

					SHOWMSG("returning ..");

					dh->dh_DirectoryEntry.d_ino = fib->fib_DiskKey;

					strcpy(dh->dh_DirectoryEntry.d_name,"..");

					result = &dh->dh_DirectoryEntry;
				}
			}
		}
		#endif /* UNIX_PATH_SEMANTICS */

		if(result == NULL)
		{
			assert( (((ULONG)&dh->dh_FileInfo) & 3) == 0 );

			if(ExNext(dh->dh_DirLock,&dh->dh_FileInfo))
			{
				dh->dh_DirectoryEntry.d_ino = dh->dh_FileInfo.fib_DiskKey;

				assert( sizeof(dh->dh_DirectoryEntry.d_name) >= sizeof(dh->dh_FileInfo.fib_FileName) );

				strcpy(dh->dh_DirectoryEntry.d_name,dh->dh_FileInfo.fib_FileName);

				result = &dh->dh_DirectoryEntry;
			}
			else
			{
				if(IoErr() != ERROR_NO_MORE_ENTRIES)
				{
					SHOWMSG("error scanning directory");

					__set_errno(__translate_io_error_to_errno(IoErr()));
					goto out;
				}

				SHOWMSG("that was the end of the line");
			}
		}
	}

	if(result != NULL)
		D(("next entry to return is '%s'",result->d_name));
	else
		SHOWMSG("end of directory reached");

 out:

	UnLock(parent_directory);

	PROFILE_ON();

	RETURN(result);
	return(result);
}
