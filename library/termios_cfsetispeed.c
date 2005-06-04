/*
 * $Id: termios_cfsetispeed.c,v 1.1 2005-06-04 10:46:21 obarthel Exp $
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

int
cfsetispeed(struct termios *tios,speed_t ispeed)
{
	int result = ERROR;

	ENTER();

	SHOWPOINTER(tios);
	SHOWVALUE(ispeed);

	if(tios == NULL)
	{
		__set_errno(EFAULT);
		goto out;
	}

	switch(ispeed)
	{
		case B0:
		case B50:
		case B75:
		case B110:
		case B134:
		case B150:
		case B200:
		case B300:
		case B600:
		case B1200:
		case B1800:
		case B2400:
		case B4800:
		case B9600:
		case B19200:
		case B31250:
		case B38400:
		case B57600:
		case B115200:
		case B230400:
		case B460800:
		case B576000:
		case B1152000:

			tios->c_ispeed = ispeed;
			break;

		default:

			__set_errno(EINVAL);
			goto out;
	}

	result = OK;

 out:

	RETURN(result);
	return(result);
}
