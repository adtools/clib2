/*
 * $Id: termios_console_fdhookentry.c,v 1.2 2006-01-08 12:04:27 obarthel Exp $
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

#ifndef	_TERMIOS_HEADERS_H
#include "termios_headers.h"
#endif /* _TERMIOS_HEADERS_H */

/****************************************************************************/

/*
 * Hook for termios emulation on a console. This can probably be cleaned up a bit
 * by removing things which will (should) never happen on a console.
 */

#ifndef _STDIO_HEADERS_H
#include "stdio_headers.h"
#endif /* _STDIO_HEADERS_H */

#ifndef _UNISTD_HEADERS_H
#include "unistd_headers.h"
#endif /* _UNISTD_HEADERS_H */

/****************************************************************************/

#ifndef _STDLIB_MEMORY_H
#include "stdlib_memory.h"
#endif /* _STDLIB_MEMORY_H */

/****************************************************************************/

#include <strings.h>
#include <limits.h>

/****************************************************************************/

/*
 * Emulate canonical no-echo mode with a simple line-editor in raw mode.
 */
static int
LineEditor(BPTR file,char *buf,const int buflen,struct termios *tios)
{
	int pos = 0,len = 0;
	unsigned char z;
	int do_edit = 1;
	int shift_mode = 0;

	SetMode(file,DOSTRUE);	/* Set raw mode. */

	while(do_edit && len < buflen)
	{
		if(WaitForChar(file,5000000) != DOSFALSE) /* 5 seconds. */
		{
			if(Read(file,&z,1) == ERROR)
			{
				len = -1;
				break;
			}

			if(z == tios->c_cc[VQUIT])
				break;

			switch(z)
			{
				case '\n':	/* NL */
				case '\r':	/* CR */

					do_edit = 0;

					buf[len++] = '\n';
					continue;

				case 155:	/* CSI */

					shift_mode = 1;
					continue;

				case '\b':	/* Backspace */

					if(pos > 0)
					{
						memmove(&buf[pos-1],&buf[pos],len-pos);
						pos--;
						len--;
					}

					continue;

				case 127:	/* Delete */

					if(pos < len)
					{
						memmove(&buf[pos],&buf[pos+1],len-pos-1);
						len--;
					}

					continue;
			}

			if(shift_mode)
			{
				shift_mode = 0;

				switch(z)
				{
					case 'C':  /* Right arrowkey */

						if(pos < len)
							pos++;

						continue;

					case 'D':	/* Left arrowkey */

						if(pos > 0)
							pos--;

						continue;
				}
			}

			if(pos != len)
				memmove(&buf[pos + 1],&buf[pos],len - pos);

			buf[pos]=z;
			pos++;
			len++;
		}
	}

	if(len >= 0 && len < buflen) /* Does not hurt to null-terminate if we can. */
		buf[len] = '\0';

	SetMode(file,DOSFALSE);	/* Restore mode */
	
	return(len); /* Number of characters read. */
}

/****************************************************************************/

int
__termios_console_hook(
	struct fd *						fd,
	struct file_action_message *	fam)
{
	const unsigned char CR = '\r',NL = '\n';
	struct FileHandle * fh;
	char * buffer = NULL;
	int result = EOF;
	int actual_out;
	BOOL is_aliased;
	BPTR file;
	struct termios *tios;

	ENTER();

	assert( fam != NULL && fd != NULL );
	assert( __is_valid_fd(fd) );
	assert( FLAG_IS_SET(fd->fd_Flags,FDF_TERMIOS) );
	assert( fd->fd_Aux != NULL);

	tios = (struct termios *)fd->fd_Aux;

	/* Careful: file_action_close has to monkey with the file descriptor
	            table and therefore needs to obtain the stdio lock before
	            it locks this particular descriptor entry. */
	if(fam->fam_Action == file_action_close)
		__stdio_lock();

	__fd_lock(fd);

	#if defined(__THREAD_SAFE)
	{
		/* Check if this file should be dynamically bound to one of the
		   three standard I/O streams. */
		if(FLAG_IS_SET(fd->fd_Flags,FDF_STDIO))
		{
			switch(fd->fd_DefaultFile)
			{
				case STDIN_FILENO:

					file = Input();
					break;

				case STDOUT_FILENO:

					file = Output();
					break;

				case STDERR_FILENO:

					#if defined(__amigaos4__)
					{
						file = ErrorOutput();
					}
					#else
					{
						struct Process * this_process = (struct Process *)FindTask(NULL);

						file = this_process->pr_CES;
					}
					#endif /* __amigaos4__ */

					/* The following is rather controversial; if the standard error stream
					   is unavailable, we default to reuse the standard output stream. This
					   is problematic if the standard output stream was redirected and should
					   not be the same as the standard error output stream. */
					if(file == ZERO)
						file = Output();

					break;

				default:

					file = ZERO;
					break;
			}
		}
		else
		{
			file = fd->fd_DefaultFile;
		}
	}
	#else
	{
		file = fd->fd_DefaultFile;
	}
	#endif /* __THREAD_SAFE */

	if(file == ZERO)
	{
		SHOWMSG("file is closed");

		fam->fam_Error = EBADF;
		goto out;
	}

	switch(fam->fam_Action)
	{
		case file_action_read:

			SHOWMSG("file_action_read");

			if(FLAG_IS_CLEAR(tios->c_cflag,CREAD))
			{
				SHOWMSG("Reading is not enabled for this console descriptor.");
				fam->fam_Error = EIO;
				goto out;
			}

			assert( fam->fam_Data != NULL );
			assert( fam->fam_Size > 0 );

			D(("read %ld bytes from position %ld to 0x%08lx",fam->fam_Size,Seek(file,0,OFFSET_CURRENT),fam->fam_Data));

			PROFILE_OFF();

			/* Attempt to fake everything needed in non-canonical mode. */

			if(FLAG_IS_SET(tios->c_lflag,ICANON)) /* Canonical read = same as usual. Unless... */
			{
				if(FLAG_IS_CLEAR(tios->c_lflag,ECHO))	/* No-echo mode needs to be emulated. */
					result = LineEditor(file,fam->fam_Data,fam->fam_Size,tios);
				else
					result = Read(file,fam->fam_Data,fam->fam_Size);
			}
			else if (fam->fam_Size > 0)
			{
				/* Non-canonical reads have timeouts and a minimum number of characters to read. */
				int i = 0;

				result = 0;

				if(tios->c_cc[VMIN]>0)
				{
					i = Read(file,fam->fam_Data,1);	/* Reading the first character is not affected by the timeout unless VMIN==0. */
					if(i == ERROR)
					{
						fam->fam_Error = EIO;
						goto out;
					}

					result = i;

					while((result < tios->c_cc[VMIN]) && (result < fam->fam_Size))
					{
						if(tios->c_cc[VTIME] > 0)
						{
							if(WaitForChar(file,100000 * tios->c_cc[VTIME]) == DOSFALSE)
								break;	/* No more characters available within alloted time. */
						}

						i = Read(file,&fam->fam_Data[result],1);
						if(i <= 0)
							break;	/* Break out of this while loop only. */

						result += i;
					}
				}
				else
				{
					if(WaitForChar(fd->fd_DefaultFile,100000*tios->c_cc[VTIME]))
						result = Read(file,fam->fam_Data,fam->fam_Size);
				}
			}
			else
			{
				result = 0; /* Reading zero characters will always succeed. */
			}

			PROFILE_ON();

			if(result == ERROR)
			{
				D(("read failed ioerr=%ld",IoErr()));

				fam->fam_Error = __translate_io_error_to_errno(IoErr());
				goto out;
			}

			if(result > 0)
			{
				if(tios->c_iflag != 0) /* Input processing enabled. */
				{
					int i,n;
					int num_bytes = result;
					unsigned char byte_in;

					/* XXX The input substitution could possibly be moved to the console handler with an input-map. (?) */
					for(i = n = 0 ; i < num_bytes ; i++)
					{
						byte_in = fam->fam_Data[i];

						if(FLAG_IS_SET(tios->c_iflag,ISTRIP)) /* Strip 8:th bit. Done before any other processing. */
							byte_in &= 0x7f;

						if(FLAG_IS_SET(tios->c_iflag,IGNCR) && byte_in == CR) /* Remove CR */
						{
							result--;
							continue;
						}

						if(FLAG_IS_SET(tios->c_iflag,ICRNL) && byte_in == CR) /* Map CR->NL */
							byte_in = NL;

						if(FLAG_IS_SET(tios->c_iflag,INLCR) && byte_in == NL) /* Map NL->CR */
							byte_in = CR;

						fam->fam_Data[n++] = byte_in;
					}
				}

				if(FLAG_IS_SET(tios->c_lflag,ECHO) && FLAG_IS_CLEAR(tios->c_lflag,ICANON) && FLAG_IS_SET(fd->fd_Flags,FDF_WRITE))
				{
					if(Write(file,fam->fam_Data,result) == ERROR)
					{
						/* "Silently" disable echoing. */
						SHOWMSG("Echo failed and has been disabled.");
						CLEAR_FLAG(tios->c_lflag,ECHO);
					}
				}
			}

			if(FLAG_IS_SET(fd->fd_Flags,FDF_CACHE_POSITION))
				fd->fd_Position += (ULONG)result;

			break;

		case file_action_write:

			SHOWMSG("file_action_write");

			assert( fam->fam_Data != NULL );
			assert( fam->fam_Size > 0 );

			if(FLAG_IS_SET(tios->c_oflag,OPOST)) /* Output processing enabled. */
			{
				unsigned char byte_out;
				int i,n;

				buffer = malloc(2 * fam->fam_Size);
				if(buffer == NULL)
				{
					fam->fam_Error = ENOMEM;
					goto out;
				}

				for(i = n = 0 ; i < fam->fam_Size ; i++)
				{
					byte_out=fam->fam_Data[i];

					if(FLAG_IS_SET(tios->c_oflag,ONLRET) && byte_out == CR)
						continue;

					if(FLAG_IS_SET(tios->c_oflag,OCRNL) && byte_out == CR)
						byte_out = NL;

					if(FLAG_IS_SET(tios->c_oflag,ONOCR) && byte_out == CR)
						byte_out = NL;

					if(FLAG_IS_SET(tios->c_oflag,ONLCR) && byte_out == NL)
					{
						buffer[n++] = CR;
						byte_out = NL;
					}

					buffer[n++] = byte_out;
				}

				actual_out = n;
			}
			else
			{
				buffer		= fam->fam_Data;
				actual_out	= fam->fam_Size;
			}

			/* Note. When output processing is enabled, write() can return _more_ than the data length. */
			D(("write %ld bytes to position %ld from 0x%08lx",actual_out,Seek(file,0,OFFSET_CURRENT),buffer));

			if(actual_out > 0)
			{
				PROFILE_OFF();

				result = Write(file,buffer,actual_out);

				PROFILE_ON();
			}
			else
			{
				result = 0;
			}

			if(buffer == fam->fam_Data)
				buffer = NULL;	/* Must do this to avoid freeing the user data. */

			if(result == ERROR)
			{
				D(("write failed ioerr=%ld",IoErr()));

				fam->fam_Error = __translate_io_error_to_errno(IoErr());
				goto out;
			}

			if(FLAG_IS_SET(fd->fd_Flags,FDF_CACHE_POSITION))
				fd->fd_Position += (ULONG)result;

			break;

		case file_action_close:

			SHOWMSG("file_action_close");

			/* The following is almost guaranteed not to fail. */
			result = OK;

			/* If this is an alias, just remove it. */
			is_aliased = __fd_is_aliased(fd);
			if(is_aliased)
			{
				__remove_fd_alias(fd);
			}
			else if (FLAG_IS_CLEAR(fd->fd_Flags,FDF_STDIO))
			{
				/* Should we reset this file into line buffered mode? */
				if(FLAG_IS_SET(fd->fd_Flags,FDF_NON_BLOCKING) && FLAG_IS_SET(fd->fd_Flags,FDF_IS_INTERACTIVE))
					SetMode(fd->fd_DefaultFile,DOSFALSE);

				/* Are we allowed to close this file? */
				if(FLAG_IS_CLEAR(fd->fd_Flags,FDF_NO_CLOSE))
				{
					/* Call a cleanup function, such as the one which
					 * releases locked records.
					 */
					if(fd->fd_Cleanup != NULL)
						(*fd->fd_Cleanup)(fd);

					PROFILE_OFF();

					if(CANNOT Close(fd->fd_DefaultFile))
					{
						fam->fam_Error = __translate_io_error_to_errno(IoErr());

						result = EOF;
					}

					PROFILE_ON();

					fd->fd_DefaultFile = ZERO;
				}
			}

			__fd_unlock(fd);

			#if defined(__THREAD_SAFE)
			{
				/* Free the lock semaphore now. */
				if(NOT is_aliased)
					__delete_semaphore(fd->fd_Lock);
			}
			#endif /* __THREAD_SAFE */

			/* And that's the last for this file descriptor. */
			memset(fd,0,sizeof(*fd));

			break;

		case file_action_seek:

			SHOWMSG("file_action_seek");

			fam->fam_Error = EINVAL;
			goto out;

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

				if(CANNOT SetMode(file,mode))
				{
					fam->fam_Error = __translate_io_error_to_errno(IoErr());
					goto out;
				}

				/* Update tios to reflect state change. */
				if(mode == DOSTRUE)
					CLEAR_FLAG(tios->c_lflag,ICANON);
				else
					SET_FLAG(tios->c_lflag,ICANON);

				result = OK;
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

			fh = BADDR(file);

			if(CANNOT __safe_examine_file_handle(file,fam->fam_FileInfo))
			{
				LONG error;

				/* So that didn't work. Did the file system simply fail to
				   respond to the request or is something more sinister
				   at work? */
				error = IoErr();
				if(error != ERROR_ACTION_NOT_KNOWN)
				{
					SHOWMSG("couldn't examine the file");

					fam->fam_Error = __translate_io_error_to_errno(error);
					goto out;
				}

				/* OK, let's have another look at this file. Could it be a
				   console stream? */
				if(NOT IsInteractive(file))
				{
					SHOWMSG("whatever it is, we don't know");

					fam->fam_Error = ENOSYS;
					goto out;
				}

				/* Make up some stuff for this stream. */
				memset(fam->fam_FileInfo,0,sizeof(*fam->fam_FileInfo));

				DateStamp(&fam->fam_FileInfo->fib_Date);

				fam->fam_FileInfo->fib_DirEntryType = ST_CONSOLE;
			}

			fam->fam_FileSystem = fh->fh_Type;

			result = OK;

			break;

		default:

			SHOWVALUE(fam->fam_Action);

			fam->fam_Error = EBADF;
			break;
	}

 out:

	__fd_unlock(fd);

	if(fam->fam_Action == file_action_close)
		__stdio_unlock();

	if(buffer != NULL)
		free(buffer);

	SHOWVALUE(result);

	RETURN(result);
	return(result);
}
