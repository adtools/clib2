/*
 * $Id: stdio_fdhookentry.c,v 1.16 2005-03-03 14:20:55 obarthel Exp $
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

#ifndef _STDLIB_MEM_DEBUG_H
#include "stdlib_mem_debug.h"
#endif /* _STDLIB_MEM_DEBUG_H */

/****************************************************************************/

#ifndef _STDIO_HEADERS_H
#include "stdio_headers.h"
#endif /* _STDIO_HEADERS_H */

#ifndef _UNISTD_HEADERS_H
#include "unistd_headers.h"
#endif /* _UNISTD_HEADERS_H */

/****************************************************************************/

#include <strings.h>
#include <limits.h>

/****************************************************************************/

int
__fd_hook_entry(
	struct fd *						fd,
	struct file_action_message *	fam)
{
	D_S(struct FileInfoBlock,fib);
	BOOL fib_is_valid = FALSE;
	off_t current_position;
	off_t new_position;
	int new_mode;
	char * buffer = NULL;
	int result = -1;

	ENTER();

	assert( fam != NULL && fd != NULL );
	assert( __is_valid_fd(fd) );

	__fd_lock(fd);

	switch(fam->fam_Action)
	{
		case file_action_read:

			SHOWMSG("file_action_read");

			if(fd->fd_DefaultFile == ZERO)
			{
				SHOWMSG("file is closed");

				fam->fam_Error = EBADF;
				break;
			}

			assert( fam->fam_Data != NULL );
			assert( fam->fam_Size > 0 );

			D(("read %ld bytes from position %ld to 0x%08lx",fam->fam_Size,Seek(fd->fd_DefaultFile,0,OFFSET_CURRENT),fam->fam_Data));

			PROFILE_OFF();

			result = Read(fd->fd_DefaultFile,fam->fam_Data,fam->fam_Size);

			PROFILE_ON();

			if(result < 0)
			{
				D(("read failed ioerr=%ld",IoErr()));

				fam->fam_Error = __translate_io_error_to_errno(IoErr());
				break;
			}

			if(FLAG_IS_SET(fd->fd_Flags,FDF_CACHE_POSITION))
				fd->fd_Position += result;

			break;

		case file_action_write:

			SHOWMSG("file_action_write");

			if(fd->fd_DefaultFile == ZERO)
			{
				SHOWMSG("file is closed");

				fam->fam_Error = EBADF;
				break;
			}

			assert( fam->fam_Data != NULL );
			assert( fam->fam_Size > 0 );

			if(FLAG_IS_SET(fd->fd_Flags,FDF_APPEND))
			{
				SHOWMSG("appending data");

				PROFILE_OFF();

				if(Seek(fd->fd_DefaultFile,0,OFFSET_END) >= 0)
				{
					if(FLAG_IS_SET(fd->fd_Flags,FDF_CACHE_POSITION))
						fd->fd_Position = Seek(fd->fd_DefaultFile,0,OFFSET_CURRENT);
				}

				PROFILE_ON();
			}

			D(("write %ld bytes to position %ld from 0x%08lx",fam->fam_Size,Seek(fd->fd_DefaultFile,0,OFFSET_CURRENT),fam->fam_Data));

			PROFILE_OFF();

			result = Write(fd->fd_DefaultFile,fam->fam_Data,fam->fam_Size);

			PROFILE_ON();

			if(result < 0)
			{
				D(("write failed ioerr=%ld",IoErr()));

				fam->fam_Error = __translate_io_error_to_errno(IoErr());
				break;
			}

			if(FLAG_IS_SET(fd->fd_Flags,FDF_CACHE_POSITION))
				fd->fd_Position += result;

			break;

		case file_action_close:

			SHOWMSG("file_action_close");

			/* If this is an alias, just remove it. */
			if(__fd_is_aliased(fd))
			{
				__remove_fd_alias(fd);
			}
			else
			{
				/* Is this file open in the first place? */
				if(fd->fd_DefaultFile == ZERO)
				{
					SHOWMSG("file is closed");

					fam->fam_Error = EBADF;
					break;
				}

				/* The following is almost guaranteed not to fail. */
				result = 0;

				/* Are we disallowed to close this file? */
				if(FLAG_IS_SET(fd->fd_Flags,FDF_NO_CLOSE))
				{
					/* OK, so we cannot close it. But we might be obliged to
					   reset a console into buffered mode. */
					if(FLAG_IS_SET(fd->fd_Flags,FDF_NON_BLOCKING) && FLAG_IS_SET(fd->fd_Flags,FDF_IS_INTERACTIVE))
						SetMode(fd->fd_DefaultFile,DOSFALSE);
				}
				else
				{
					BOOL name_and_path_valid = FALSE;
					D_S(struct FileInfoBlock,fib);
					BPTR parent_dir;

					memset(fib,0,sizeof(*fib));

					/* Call a cleanup function, such as the one which
					 * releases locked records.
					 */
					if(fd->fd_Cleanup != NULL)
						(*fd->fd_Cleanup)(fd);

					PROFILE_OFF();

					parent_dir = __safe_parent_of_file_handle(fd->fd_DefaultFile);
					if(parent_dir != ZERO)
					{
						if(__safe_examine_file_handle(fd->fd_DefaultFile,fib))
							name_and_path_valid = TRUE;
					}

					if(CANNOT Close(fd->fd_DefaultFile))
					{
						fam->fam_Error = __translate_io_error_to_errno(IoErr());

						result = -1;
					}

					PROFILE_ON();

					fd->fd_DefaultFile = ZERO;

					#if defined(UNIX_PATH_SEMANTICS)
					{
						DECLARE_UTILITYBASE();

						assert( UtilityBase != NULL );

						/* Now that we have closed this file, know where it is and what its
						 * name would be, check if we tried to unlink it earlier. If we did,
						 * we'll try to finish the job here and now.
						 */
						if(name_and_path_valid)
						{
							struct UnlinkNode * node;
							struct UnlinkNode * uln_next;
							struct UnlinkNode * uln;
							BOOL file_deleted = FALSE;

							assert( __unlink_list.mlh_Head != NULL );

							/* Check all files to be unlinked when this program exits. */
							for(uln = (struct UnlinkNode *)__unlink_list.mlh_Head ;
							    (uln_next = (struct UnlinkNode *)uln->uln_MinNode.mln_Succ) != NULL ;
							    uln = uln_next)
							{
								node = NULL;

								/* If the file name matches, check if the path matches, too. */
								if(Stricmp(FilePart(uln->uln_Name),fib->fib_FileName) == SAME)
								{
									BPTR old_dir;
									BPTR node_lock;
									BPTR path_lock = ZERO;

									PROFILE_OFF();

									/* Try to get a lock on the file first, then move on to
									 * the directory it is stored in.
									 */
									old_dir = CurrentDir(uln->uln_Lock);

									node_lock = Lock(uln->uln_Name,SHARED_LOCK);
									if(node_lock != ZERO)
									{
										path_lock = ParentDir(node_lock);

										UnLock(node_lock);
									}

									CurrentDir(old_dir);

									/* If we found the file's parent directory, check if it matches
									 * the parent directory of the file we just closed.
									 */
									if(path_lock != ZERO)
									{
										if(SameLock(path_lock,parent_dir) == LOCK_SAME)
											node = uln;

										UnLock(path_lock);
									}

									PROFILE_ON();
								}

								/* If we found that this file was set up for deletion,
								 * delete it here and now.
								 */
								if(node != NULL)
								{
									if(NOT file_deleted)
									{
										BPTR old_dir;

										PROFILE_OFF();

										old_dir = CurrentDir(parent_dir);

										if(DeleteFile(fib->fib_FileName))
										{
											file_deleted		= TRUE;
											name_and_path_valid	= FALSE;
										}

										CurrentDir(old_dir);

										PROFILE_ON();
									}

									if(file_deleted)
									{
										Remove((struct Node *)node);
										free(node);
									}
								}
							}
						}
					}
					#endif /* UNIX_PATH_SEMANTICS */

					if(FLAG_IS_SET(fd->fd_Flags,FDF_CREATED) && name_and_path_valid)
					{
						ULONG flags;
						BPTR old_dir;

						PROFILE_OFF();

						old_dir = CurrentDir(parent_dir);

						flags = fib->fib_Protection ^ (FIBF_READ|FIBF_WRITE|FIBF_EXECUTE|FIBF_DELETE);

						CLEAR_FLAG(flags,FIBF_EXECUTE);
						CLEAR_FLAG(flags,FIBF_OTR_EXECUTE);
						CLEAR_FLAG(flags,FIBF_GRP_EXECUTE);

						SetProtection(fib->fib_FileName,(LONG)(flags ^ (FIBF_READ|FIBF_WRITE|FIBF_EXECUTE|FIBF_DELETE)));

						CurrentDir(old_dir);

						PROFILE_ON();
					}

					PROFILE_OFF();
					UnLock(parent_dir);
					PROFILE_ON();
				}
			}

			__fd_unlock(fd);

			/* Free the lock semaphore now. */
			__delete_semaphore(fd->fd_Lock);

			/* And that's the last for this file descriptor. */
			memset(fd,0,sizeof(*fd));

			break;

		case file_action_seek:

			SHOWMSG("file_action_seek");

			if(fam->fam_Mode == SEEK_CUR)
				new_mode = OFFSET_CURRENT;
			else if (fam->fam_Mode == SEEK_SET)
				new_mode = OFFSET_BEGINNING;
			else
				new_mode = OFFSET_END;

			D(("seek to offset %ld, new_mode %ld; current position = %ld",fam->fam_Offset,new_mode,Seek(fd->fd_DefaultFile,0,OFFSET_CURRENT)));

			if(FLAG_IS_SET(fd->fd_Flags,FDF_CACHE_POSITION))
			{
				current_position = fd->fd_Position;
			}
			else
			{
				PROFILE_OFF();
				current_position = Seek(fd->fd_DefaultFile,0,OFFSET_CURRENT);
				PROFILE_ON();

				if(current_position < 0)
				{
					fam->fam_Error = EBADF;
					break;
				}
			}

			new_position = current_position;

			switch(new_mode)
			{
				case OFFSET_CURRENT:

					new_position += fam->fam_Offset;
					break;

				case OFFSET_BEGINNING:

					new_position = fam->fam_Offset;
					break;

				case OFFSET_END:

					if(__safe_examine_file_handle(fd->fd_DefaultFile,fib))
					{
						new_position = fib->fib_Size + fam->fam_Offset;

						fib_is_valid = TRUE;
					}

					break;
			}

			if(new_position != current_position)
			{
				LONG position;

				PROFILE_OFF();
				position = Seek(fd->fd_DefaultFile,fam->fam_Offset,new_mode);
				PROFILE_ON();

				if(position < 0)
				{
					D(("seek failed, fam->fam_Mode=%ld (%ld), offset=%ld, ioerr=%ld",new_mode,fam->fam_Mode,fam->fam_Offset,IoErr()));

					fam->fam_Error = __translate_io_error_to_errno(IoErr());

					#if defined(UNIX_PATH_SEMANTICS)
					{
						if(NOT fib_is_valid && CANNOT __safe_examine_file_handle(fd->fd_DefaultFile,fib))
						{
							fam->fam_Error = __translate_io_error_to_errno(IoErr());
							break;
						}

						if(new_position <= fib->fib_Size)
						{
							fam->fam_Error = __translate_io_error_to_errno(IoErr());
							break;
						}

						if(__grow_file_size(fd,new_position - fib->fib_Size) < 0)
						{
							fam->fam_Error = __translate_io_error_to_errno(IoErr());
							break;
						}
					}
					#else
					{
						break;
					}
					#endif /* UNIX_PATH_SEMANTICS */
				}

				if(FLAG_IS_SET(fd->fd_Flags,FDF_CACHE_POSITION))
					fd->fd_Position = new_position;
			}

			result = new_position;

			break;

		case file_action_set_blocking:

			SHOWMSG("file_action_set_blocking");

			PROFILE_OFF();

			if(FLAG_IS_SET(fd->fd_Flags,FDF_IS_INTERACTIVE))
			{
				LONG mode;

				SHOWMSG("changing the mode");

				if(fam->fam_Arg != 0)
					mode = DOSFALSE;	/* buffered mode */
				else
					mode = DOSTRUE;		/* single character mode */

				if(CANNOT SetMode(fd->fd_DefaultFile,mode))
				{
					fam->fam_Error = __translate_io_error_to_errno(IoErr());
					break;
				}

				result = 0;
			}
			else
			{
				SHOWMSG("can't do anything here");

				fam->fam_Error = EBADF;
			}

			PROFILE_ON();

			break;

		case file_action_examine:

			SHOWMSG("file_action_examine");

			if(fd->fd_DefaultFile != ZERO)
			{
				struct FileHandle * fh;

				if(CANNOT __safe_examine_file_handle(fd->fd_DefaultFile,fam->fam_FileInfo))
				{
					SHOWMSG("couldn't examine the file");

					fam->fam_Error = __translate_io_error_to_errno(IoErr());
					break;
				}

				fh = BADDR(fd->fd_DefaultFile);

				fam->fam_FileSystem = fh->fh_Type;

				result = 0;
			}
			else
			{
				SHOWMSG("file is already closed");

				fam->fam_Error = EBADF;
			}

			break;

		default:

			SHOWVALUE(fam->fam_Action);

			fam->fam_Error = EBADF;
			break;
	}

	__fd_unlock(fd);

	if(buffer != NULL)
		free(buffer);

	SHOWVALUE(result);

	RETURN(result);
	return(result);
}
