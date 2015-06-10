/*
 * $Id: termios_tcsendbreak.c,v 1.3 2006-01-08 12:04:27 obarthel Exp $
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

#ifndef	_TERMIOS_HEADERS_H
#include "termios_headers.h"
#endif /* _TERMIOS_HEADERS_H */

/****************************************************************************/

/*
 * tcsendbreak() has a rather odd time specification.
 * If duration is 0, it will send a BREAK for 0.25-0.5 seconds
 * and if duration is non-zero...
 * 	Linux asserts BREAK for N*duration where N is somewhere between 0.25 and 0.5 seconds.
 * 	Solaris calls tcdrain() instead. (According to the manual)
 * Let us try the Solaris way and see how it goes.
 *
 * If the file descriptor does not referr to a serial port, no action is needed.
 */

int
tcsendbreak(int file_descriptor,int duration)
{
	int result = ERROR;
	struct fd *fd;
	struct termios *tios;

	ENTER();

	SHOWVALUE(file_descriptor);
	SHOWVALUE(duration);

	if(__check_abort_enabled)
		__check_abort();

	__stdio_lock();

	fd = __get_file_descriptor(file_descriptor);
	if(fd == NULL || FLAG_IS_CLEAR(fd->fd_Flags,FDF_TERMIOS))
	{
		__set_errno(EBADF);
		goto out;
	}

	__fd_lock(fd);

	assert( fd->fd_Aux != NULL );

	tios = fd->fd_Aux;

	switch(tios->type)
	{
		case TIOST_CONSOLE:

			result = OK;
			break;

		case TIOST_SERIAL:

			if(duration == 0)
			{
				/* TODO */
				result = OK;
			}
			else
			{
				result = tcdrain(file_descriptor);
			}

			break;

		default:

			SHOWMSG("Invalid tios type in tcsendbreak.");
			goto out;
	}

 out:

	__fd_unlock(fd);

	__stdio_unlock();

	RETURN(result);
	return(result);
}
