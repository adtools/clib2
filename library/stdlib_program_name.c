/*
 * $Id: stdlib_program_name.c,v 1.3 2008-09-30 14:09:00 obarthel Exp $
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

#ifndef _STDLIB_HEADERS_H
#include "stdlib_headers.h"
#endif /* _STDLIB_HEADERS_H */

/****************************************************************************/

#ifndef _STDLIB_MEMORY_H
#include "stdlib_memory.h"
#endif /* _STDLIB_MEMORY_H */

/****************************************************************************/

#ifndef _STDLIB_CONSTRUCTOR_H
#include "stdlib_constructor.h"
#endif /* _STDLIB_CONSTRUCTOR_H */

/****************************************************************************/

#ifdef __amigaos4__
#define MEMORY_TYPE MEMF_PRIVATE
#else
#define MEMORY_TYPE MEMF_ANY
#endif /* __amigaos4__ */

/****************************************************************************/

static BOOL free_program_name;

/****************************************************************************/

char * NOCOMMON __program_name;

/****************************************************************************/

STDLIB_DESTRUCTOR(stdlib_program_name_exit)
{
	ENTER();

	if(free_program_name && __program_name != NULL)
	{
		FreeVec(__program_name);
		__program_name = NULL;
	}

	LEAVE();
}

/****************************************************************************/

STDLIB_CONSTRUCTOR(stdlib_program_name_init)
{
	BOOL success = FALSE;

	ENTER();

	if(__WBenchMsg == NULL)
	{
		const size_t program_name_size = 256;

		/* Make a copy of the current command name string. */
		__program_name = AllocVec((ULONG)program_name_size,MEMORY_TYPE);
		if(__program_name == NULL)
			goto out;

		free_program_name = TRUE;

		if(CANNOT GetProgramName(__program_name,program_name_size))
			goto out;
	}
	else
	{
		__program_name = (char *)__WBenchMsg->sm_ArgList[0].wa_Name;
	}

	success = TRUE;

 out:

	SHOWVALUE(success);
	LEAVE();

	if(success)
		CONSTRUCTOR_SUCCEED();
	else
		CONSTRUCTOR_FAIL();
}
