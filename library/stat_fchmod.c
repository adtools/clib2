/*
 * $Id: stat_fchmod.c,v 1.4 2005-02-03 16:56:15 obarthel Exp $
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

#ifndef _STAT_HEADERS_H
#include "stat_headers.h"
#endif /* _STAT_HEADERS_H */

/****************************************************************************/

/* The following is not part of the ISO 'C' (1994) standard. */

/****************************************************************************/

int
fchmod(int file_descriptor, mode_t mode)
{
	DECLARE_UTILITYBASE();
	struct file_hook_message message;
	ULONG protection;
	int result = -1;
	struct fd * fd;

	ENTER();

	SHOWVALUE(file_descriptor);
	SHOWVALUE(mode);

	assert( UtilityBase != NULL );

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

	protection = 0;

	if(FLAG_IS_SET(mode,S_IRUSR))
		SET_FLAG(protection,FIBF_READ);

	if(FLAG_IS_SET(mode,S_IWUSR))
	{
		SET_FLAG(protection,FIBF_WRITE);
		SET_FLAG(protection,FIBF_DELETE);
	}

	if(FLAG_IS_SET(mode,S_IXUSR))
		SET_FLAG(protection,FIBF_EXECUTE);

	if(FLAG_IS_SET(mode,S_IRGRP))
		SET_FLAG(protection,FIBF_GRP_READ);

	if(FLAG_IS_SET(mode,S_IWGRP))
	{
		SET_FLAG(protection,FIBF_GRP_WRITE);
		SET_FLAG(protection,FIBF_GRP_DELETE);
	}

	if(FLAG_IS_SET(mode,S_IXGRP))
		SET_FLAG(protection,FIBF_GRP_EXECUTE);

	if(FLAG_IS_SET(mode,S_IROTH))
		SET_FLAG(protection,FIBF_OTR_READ);

	if(FLAG_IS_SET(mode,S_IWOTH))
	{
		SET_FLAG(protection,FIBF_OTR_WRITE);
		SET_FLAG(protection,FIBF_OTR_DELETE);
	}

	if(FLAG_IS_SET(mode,S_IXOTH))
		SET_FLAG(protection,FIBF_OTR_EXECUTE);

	SHOWMSG("calling the hook");

	message.action		= file_hook_action_change_attributes;
	message.attributes	= protection ^ (FIBF_READ|FIBF_WRITE|FIBF_EXECUTE|FIBF_DELETE);

	assert( fd->fd_Hook != NULL );

	CallHookPkt(fd->fd_Hook,fd,&message);

	result = message.result;

	__set_errno(message.error);

 out:

	RETURN(result);
	return(result);
}
