/*
 * $Id: fcntl_lseek.c,v 1.9 2005-04-24 09:53:11 obarthel Exp $
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

#ifndef _FCNTL_HEADERS_H
#include "fcntl_headers.h"
#endif /* _FCNTL_HEADERS_H */

/****************************************************************************/

/* The following is not part of the ISO 'C' (1994) standard. */

/****************************************************************************/

off_t
lseek(int file_descriptor, off_t offset, int mode)
{
	struct file_action_message fam;
	off_t result = SEEK_ERROR;
	off_t position;
	struct fd * fd;

	ENTER();

	SHOWVALUE(file_descriptor);
	SHOWVALUE(offset);
	SHOWVALUE(mode);

	assert( file_descriptor >= 0 && file_descriptor < __num_fd );
	assert( __fd[file_descriptor] != NULL );
	assert( FLAG_IS_SET(__fd[file_descriptor]->fd_Flags,FDF_IN_USE) );

	if(__check_abort_enabled)
		__check_abort();

	fd = __get_file_descriptor(file_descriptor);
	if(fd == NULL)
	{
		__set_errno(EBADF);
		goto out;
	}

	if(mode < SEEK_SET || mode > SEEK_END)
	{
		SHOWMSG("seek mode is invalid");

		__set_errno(EINVAL);
		goto out;
	}

	fam.fam_Action	= file_action_seek;
	fam.fam_Offset	= offset;
	fam.fam_Mode	= mode;

	assert( fd->fd_Action != NULL );

	/* Note that a return value of -1 (= SEEK_ERROR) may be a
	   valid file position in files larger than 2 GBytes. Just
	   to be sure, we therefore also check the secondary error
	   to verify that what could be a file position is really
	   an error indication. */
	position = (*fd->fd_Action)(fd,&fam);
	if(position == SEEK_ERROR && fam.fam_Error != OK)
	{
		__set_errno(fam.fam_Error);
		goto out;
	}

	/* If this is a valid file position, clear 'errno' so that
	   it cannot be mistaken for an error. */
	if(position < 0)
		__set_errno(OK);

	result = position;

 out:

	RETURN(result);
	return(result);
}
