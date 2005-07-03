/*
 * $Id: stat_umask.c,v 1.6 2005-07-03 10:36:47 obarthel Exp $
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

mode_t NOCOMMON __current_umask = S_IWGRP | S_IWOTH;

/****************************************************************************/

#if defined(USERGROUP_SUPPORT)

/****************************************************************************/

#ifndef _USERGROUP_HEADERS_H
#include "usergroup_headers.h"
#endif /* _USERGROUP_HEADERS_H */

/****************************************************************************/

mode_t
umask(mode_t new_mask)
{
	mode_t result;

	ENTER();

	SHOWVALUE(new_mask);

	assert(__UserGroupBase != NULL);

	PROFILE_OFF();
	result = __getumask();
	PROFILE_ON();

	__current_umask = new_mask & (S_IRWXU | S_IRWXG | S_IRWXO);

	__umask(__current_umask);

	RETURN(result);
	return(result);
}

/****************************************************************************/

#else

/****************************************************************************/

mode_t
umask(mode_t new_mask)
{
	mode_t result;

	ENTER();

	SHOWVALUE(new_mask);

	result = __current_umask;

	__current_umask = new_mask & (S_IRWXU | S_IRWXG | S_IRWXO);

	RETURN(result);
	return(result);
}

/****************************************************************************/

#endif /* USERGROUP_SUPPORT */
