/*
 * $Id: termios_tcsetattr.c,v 1.3 2005-10-11 09:28:29 obarthel Exp $
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

#ifndef	_TERMIOS_HEADERS_H
#include "termios_headers.h"
#endif /* _TERMIOS_HEADERS_H */

/****************************************************************************/

static int
set_console_termios(struct fd *fd,struct termios *new_tios)
{
	struct termios *old_tios;
	int result = ERROR;
	BPTR file;

	/* TODO: Check for some "impossible" combinations here? */

	old_tios = __get_termios(fd);
	if(old_tios == NULL)
		goto out;

	if(old_tios->type != TIOST_CONSOLE)
		goto out;

	file = fd->fd_DefaultFile;

	#if defined(__THREAD_SAFE)
	{
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
	}
	#endif /* __THREAD_SAFE */

	if(file == ZERO)
		goto out;

	if(FLAG_IS_CLEAR(new_tios->c_lflag,ICANON))
	{
		SetMode(file,DOSTRUE); /* Set Raw = Non-Canonical mode. */

		SET_FLAG(fd->fd_Flags,FDF_NON_BLOCKING);
	}
	else
	{
		SetMode(file,DOSFALSE); /* Set Cooked = Canonical mode. */

		CLEAR_FLAG(fd->fd_Flags,FDF_NON_BLOCKING);
	}

	/* Most of the processing (except raw/cooked mode switch) is handled in the hook. */
	memcpy(old_tios,new_tios,offsetof(struct termios,type));

	result = OK;

 out:

	return(result);
}

/****************************************************************************/

int
tcsetattr(int file_descriptor,int how,struct termios *tios)
{
	int result = ERROR;
	struct fd *fd = NULL;
	struct termios new_tios;
	int type;

	__stdio_lock();

	if(tios == NULL)
	{
		__set_errno(EFAULT);
		goto out;
	}

	fd = __get_file_descriptor(file_descriptor);
	if(fd == NULL)
	{
		SHOWMSG("tcsetattr() was not called with a file descriptor.\n");

		__set_errno(EBADF);
		goto out;
	}

	__fd_lock(fd);

	/* The following is in case the termios structure was manually constructed. (it should have been zero:ed in that case)  */
	if(tios->type == TIOST_INVALID)
	{
		if(tcgetattr(file_descriptor,&new_tios) != OK)
			goto out; /* Pass errno from tcgetattr() */

		type = new_tios.type;

		memcpy(&new_tios,tios,sizeof(struct termios));

		new_tios.c_ispeed = B9600;
		new_tios.c_ospeed = B9600;

		new_tios.type = type;

		tios = &new_tios;
	}

	if(tios->type == TIOST_SERIAL)
	{
		__set_errno(ENXIO);	/* Unimplemented (for now). */
		goto out;
	}
	else if (tios->type == TIOST_CONSOLE)
	{
		if(how == TCSADRAIN)
		{
			if(tcdrain(file_descriptor) != OK)
				goto out;
		}
		else if (how == TCSAFLUSH) 
		{
			if(tcflush(file_descriptor,TCIOFLUSH) != OK)
				goto out;
		}

		if(set_console_termios(fd,tios) != OK)
		{
			__set_errno(EIO);
			goto out;
		}
	}
	else
	{
		__set_errno(ENOTTY);
		goto out;
	}

	result = OK;

 out:

	__fd_unlock(fd);

	__stdio_unlock();

	return(result);
}
