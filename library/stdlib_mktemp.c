/*
 * $Id: stdlib_mktemp.c,v 1.3 2004-11-10 17:45:40 obarthel Exp $
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
	#endif /* UNIX_PATH_SEMANTICS */
	char * test_name;
	struct Process * this_process;
	APTR old_window_pointer;
	char * result = NULL;
	size_t template_offset;
	size_t template_len;
	size_t name_len;
	size_t offset;
	time_t now;
	ULONG pseudo_random_number;
	BPTR lock;
	size_t i;

	ENTER();

	SHOWSTRING(name_template);

	assert(name_template != NULL);

	this_process = (struct Process *)FindTask(NULL);

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

	SHOWSTRING(name_template);

	/* So, how long is that name template? */
	name_len = strlen(name_template);
	if(name_len == 0)
	{
		SHOWMSG("invalid name template");

		errno = EINVAL;
		goto out;
	}

	/* Find out how many trailing 'X' characters there are in
	   the template. There should be at least 6. We also want
	   to know where to find the first 'X' and how many of the
	   'X' characters there are. */
	template_offset	= 0;
	template_len	= 0;

	for(i = 0 ; i < name_len ; i++)
	{
		assert( name_len >= (i + 1) );

		offset = name_len - (i + 1);

		if(name_template[offset] != 'X')
			break;

		template_offset = offset;
		template_len++;
	}

	SHOWVALUE(template_offset);
	SHOWVALUE(template_len);

	if(template_len == 0)
	{
		SHOWMSG("invalid name template");

		errno = EINVAL;
		goto out;
	}

	/* Generate a pseudo-random number from the current time and
	   the address of the current process. */
	time(&now);

	pseudo_random_number = (ULONG)now + (ULONG)this_process;

	/* Fill the template 'X' characters with letters made up by
	   converting the pseudo-random number. */
	for(i = 0 ; i < template_len ; i++)
	{
		name_template[template_offset + i] = 'A' + (pseudo_random_number % 26);

		/* One more letter taken; if we run out of letters,
		   cook up another pseudo-random number. */
		pseudo_random_number = (pseudo_random_number / 26);
		if(pseudo_random_number == 0)
		{
			time(&now);

			pseudo_random_number = (ULONG)now;
		}
	}

	SHOWSTRING(name_template);

	old_window_pointer = this_process->pr_WindowPtr;

	/* Now check if the name we picked is unique. If not, make another name. */
	while(TRUE)
	{
		if(__check_abort_enabled)
			__check_abort();

		D(("checking '%s'",name_template));

		test_name = name_template;

		/* If necessary, quickly translate the semantics of the file name
		   we cooked up above. */
		#if defined(UNIX_PATH_SEMANTICS)
		{
			if(__unix_path_semantics)
			{
				if(__translate_unix_to_amiga_path_name((char const **)&test_name,&name_template_nti) != 0)
					goto out;

				if(name_template_nti.is_root)
				{
					errno = EACCES;
					goto out;
				}
			}
		}
		#endif /* UNIX_PATH_SEMANTICS */

		/* Turn off DOS error requesters. */
		this_process->pr_WindowPtr = (APTR)-1;

		/* Does this object exist already? */
		PROFILE_OFF();
		lock = Lock(test_name,SHARED_LOCK);
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
		   over with 'A' and move on to the next position. */
		for(i = 0 ; i < template_len ; i++)
		{
			name_template[template_offset + i]++;
			if(name_template[template_offset + i] <= 'Z')
				break;

			name_template[template_offset + i] = 'A';
		}
	}

	SHOWSTRING(name_template);

 out:

	RETURN(result);
	return(result);
}
