/*
 * $Id: stdio_vasprintf.c,v 1.1.1.1 2004-07-26 16:31:43 obarthel Exp $
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

#ifndef _STDIO_HEADERS_H
#include "stdio_headers.h"
#endif /* _STDIO_HEADERS_H */

/****************************************************************************/

#include "stdlib_protos.h"

/****************************************************************************/

/* The following is not part of the ISO 'C' standard. */

/****************************************************************************/

#undef vasprintf

/****************************************************************************/

int
__vasprintf(const char *file,int line,char **ret,const char *format,va_list arg)
{
	struct iob string_iob;
	int result = EOF;
	char local_buffer[32];

	ENTER();

	SHOWPOINTER(ret);
	SHOWSTRING(format);

	assert( ret != NULL && format != NULL && arg != NULL );

	#if defined(CHECK_FOR_NULL_POINTERS)
	{
		if(ret == NULL || format == NULL || format == arg)
		{
			SHOWMSG("invalid parameters");

			errno = EFAULT;
			goto out;
		}
	}
	#endif /* CHECK_FOR_NULL_POINTERS */

	if(__check_abort_enabled)
		__check_abort();

	(*ret) = NULL;

	__initialize_iob(&string_iob,(HOOKFUNC)__vasprintf_hook_entry,
		NULL,
		local_buffer,sizeof(local_buffer),
		-1,
		-1,
		IOBF_IN_USE | IOBF_WRITE | IOBF_BUFFER_MODE_NONE | IOBF_INTERNAL);

	string_iob.iob_String		= NULL;
	string_iob.iob_StringSize	= 0;
	string_iob.iob_File			= (char *)file;
	string_iob.iob_Line			= line;

	result = vfprintf((FILE *)&string_iob,format,arg);
	if(result < 0)
	{
		SHOWMSG("ouch. that didn't work");

		if(string_iob.iob_String != NULL)
			__free(string_iob.iob_String,string_iob.iob_File,string_iob.iob_Line);

		goto out;
	}

	SHOWSTRING(string_iob.iob_String);

	(*ret) = string_iob.iob_String;

 out:

	RETURN(result);
	return(result);
}

/****************************************************************************/

int
vasprintf(char **ret,const char *format,va_list arg)
{
	int result;

	result = __vasprintf(NULL,0,ret,format,arg);

	return(result);
}
