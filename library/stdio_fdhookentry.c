/*
 * $Id: stdio_fdhookentry.c,v 1.11 2005-02-18 18:53:16 obarthel Exp $
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

void
__fd_hook_entry(
	struct Hook *				UNUSED	unused_hook,
	struct fd *							fd,
	struct file_hook_message *			message)
{
	char * buffer = NULL;
	int result = -1;
	int error = OK;

	ENTER();

	assert( message != NULL && fd != NULL );
	assert( __is_valid_fd(fd) );

	switch(message->action)
	{
		case file_hook_action_read:

			SHOWMSG("file_hook_action_read");

			if(fd->fd_DefaultFile == ZERO)
			{
				SHOWMSG("file is closed");

				error = EBADF;
				break;
			}

			assert( message->data != NULL );
			assert( message->size > 0 );

			D(("read %ld bytes from position %ld to 0x%08lx",message->size,Seek(fd->fd_DefaultFile,0,OFFSET_CURRENT),message->data));

			PROFILE_OFF();

			result = Read(fd->fd_DefaultFile,message->data,message->size);

			PROFILE_ON();

			if(result < 0)
			{
				D(("read failed ioerr=%ld",IoErr()));

				error = __translate_io_error_to_errno(IoErr());
				break;
			}

			if(FLAG_IS_SET(fd->fd_Flags,FDF_CACHE_POSITION))
				fd->fd_Position += result;

			break;

		case file_hook_action_write:

			SHOWMSG("file_hook_action_write");

			if(fd->fd_DefaultFile == ZERO)
			{
				SHOWMSG("file is closed");

				error = EBADF;
				break;
			}

			assert( message->data != NULL );
			assert( message->size > 0 );

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

			D(("write %ld bytes to position %ld from 0x%08lx",message->size,Seek(fd->fd_DefaultFile,0,OFFSET_CURRENT),message->data));

			PROFILE_OFF();

			result = Write(fd->fd_DefaultFile,message->data,message->size);

			PROFILE_ON();

			if(result < 0)
			{
				D(("write failed ioerr=%ld",IoErr()));

				error = __translate_io_error_to_errno(IoErr());
				break;
			}

			if(FLAG_IS_SET(fd->fd_Flags,FDF_CACHE_POSITION))
				fd->fd_Position += result;

			break;

		case file_hook_action_close:

			SHOWMSG("file_hook_action_close");

			if(fd->fd_DefaultFile != ZERO)
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
					error = __translate_io_error_to_errno(IoErr());

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

				result = 0;
			}
			else
			{
				SHOWMSG("file is closed");

				error = EBADF;
			}

			break;

		case file_hook_action_set_blocking:

			SHOWMSG("file_hook_action_set_blocking");

			PROFILE_OFF();

			if(FLAG_IS_SET(fd->fd_Flags,FDF_IS_INTERACTIVE))
			{
				LONG mode;

				SHOWMSG("changing the mode");

				if(message->arg != 0)
					mode = DOSFALSE;	/* buffered mode */
				else
					mode = DOSTRUE;		/* single character mode */

				if(CANNOT SetMode(fd->fd_DefaultFile,mode))
				{
					error = EBADF;
					break;
				}

				result = 0;
			}
			else
			{
				SHOWMSG("can't do anything here");

				error = EBADF;
			}

			PROFILE_ON();

			break;

		case file_hook_action_examine:

			SHOWMSG("file_hook_action_examine");

			if(fd->fd_DefaultFile != ZERO)
			{
				struct FileHandle * fh;

				if(CANNOT __safe_examine_file_handle(fd->fd_DefaultFile,message->file_info))
				{
					SHOWMSG("couldn't examine the file");

					error = __translate_io_error_to_errno(IoErr());
					break;
				}

				fh = BADDR(fd->fd_DefaultFile);

				message->file_system = fh->fh_Type;

				result = 0;
			}
			else
			{
				SHOWMSG("file is already closed");

				error = EBADF;
			}

			break;

		default:

			SHOWVALUE(message->action);

			error = EBADF;
			break;
	}

	if(buffer != NULL)
		free(buffer);

	SHOWVALUE(result);

	message->result	= result;
	message->error	= error;

	LEAVE();
}
