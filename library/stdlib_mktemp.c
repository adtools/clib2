/*
 * $Id: stdlib_mktemp.c,v 1.2 2004-08-07 09:15:32 obarthel Exp $
 *
 * :ts=4
 *
 * Portable ISO 'C' (1994) runtime library for the Amiga computer
 * Copyright (c) 2002-2004 by Olaf Barthel <olsen@sourcery.han.de>
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

#ifndef _STDLIB_NULL_POINTER_CHECK_H
#include "stdlib_null_pointer_check.h"
#endif /* _STDLIB_NULL_POINTER_CHECK_H */

/****************************************************************************/

#ifndef _STDLIB_HEADERS_H
#include "stdlib_headers.h"
#endif /* _STDLIB_HEADERS_H */

/****************************************************************************/

#include <time.h>
#include <dos.h>

/****************************************************************************/

/* The following is not part of the ISO 'C' (1994) standard. */

/****************************************************************************/

char *
mktemp(char * name_template)
{
	#if defined(UNIX_PATH_SEMANTICS)
	struct name_translation_info name_template_nti;
	char * original_name_template = NULL;
	#endif /* UNIX_PATH_SEMANTICS */
	struct Process * this_process;
	APTR old_window_pointer;
	char * result = NULL;
	int template_offset;
	int template_len;
	time_t now;
	BPTR lock;
	int i;

	ENTER();

	SHOWSTRING(name_template);

	assert(name_template != NULL);

	#if defined(CHECK_FOR_NULL_POINTERS)
	{
		if(name_template == NULL)
		{
			SHOWMSG("invalid name template");

			errno = EFAULT;
			goto out;
		}
	}
	#endif /* CHECK_FOR_NULL_POINTERS */

	if(__check_abort_enabled)
		__check_abort();

	#if defined(UNIX_PATH_SEMANTICS)
	{
		if(__unix_path_semantics)
		{
			original_name_template = name_template;

			if(__translate_unix_to_amiga_path_name((char const **)&name_template,&name_template_nti) != 0)
				goto out;

			if(name_template_nti.is_root)
			{
				errno = EACCES;
				goto out;
			}
		}
	}
	#endif /* UNIX_PATH_SEMANTICS */

	SHOWSTRING(name_template);

	template_offset	= -1;
	template_len	= 0;

	for(i = strlen(name_template)-1 ; i >= 0 ; i--)
	{
		if(name_template[i] == 'X')
		{
			template_offset = i;
			template_len++;
		}
		else
		{
			break;
		}
	}

	if(template_offset == -1)
	{
		SHOWMSG("invalid name template");

		errno = EINVAL;
		goto out;
	}

	this_process = (struct Process *)FindTask(NULL);

	time(&now);

	now += (time_t)this_process;

	for(i = 0 ; i < template_len ; i++)
	{
		name_template[template_offset + i] = 'A' + (now % 26);

		now = (now / 26);
		if(now == 0)
			time(&now);
	}

	old_window_pointer = this_process->pr_WindowPtr;

	while(TRUE)
	{
		if(__check_abort_enabled)
			__check_abort();

		D(("checking '%s'",name_template));

		/* Turn off DOS error requesters. */
		this_process->pr_WindowPtr = (APTR)-1;

		/* Does this object exist already? */
		PROFILE_OFF();
		lock = Lock(name_template,SHARED_LOCK);
		PROFILE_ON();

		/* Restore DOS requesters. */
		this_process->pr_WindowPtr = old_window_pointer;

		if(lock == ZERO)
		{
			/* If the object does not exist yet then we
			 * are finished.
			 */
			if(IoErr() == ERROR_OBJECT_NOT_FOUND)
			{
				result = name_template;
				break;
			}

			/* Looks like a serious error. */
			__translate_io_error_to_errno(IoErr(),&errno);
			goto out;
		}

		/* OK, so it exists already. Start over... */

		PROFILE_OFF();
		UnLock(lock);
		PROFILE_ON();

		/* Change one letter; if that 'overflows', start
		 * over with 'A' and move on to the next position.
		 */
		for(i = 0 ; i < template_len ; i++)
		{
			name_template[template_offset + i]++;
			if(name_template[template_offset + i] <= 'Z')
				break;

			name_template[template_offset + i] = 'A';
		}
	}

	SHOWSTRING(name_template);

	#if defined(UNIX_PATH_SEMANTICS)
	{
		if(__unix_path_semantics)
		{
			if(__translate_amiga_to_unix_path_name((char const **)&name_template,&name_template_nti) != 0)
				goto out;

			strcpy(original_name_template,name_template);
		}
	}
	#endif /* UNIX_PATH_SEMANTICS */

	SHOWSTRING(name_template);

 out:

	RETURN(result);
	return(result);
}
