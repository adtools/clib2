/*
 * $Id: stdlib_assertion_failure.c,v 1.7 2005-02-21 16:09:44 obarthel Exp $
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

#ifndef _STDLIB_HEADERS_H
#include "stdlib_headers.h"
#endif /* _STDLIB_HEADERS_H */

#ifndef _STDIO_HEADERS_H
#include "stdio_headers.h"
#endif /* _STDIO_HEADERS_H */

/****************************************************************************/

#include <proto/exec.h>

#define __NOLIBBASE__
#define __NOGLOBALIFACE__
#include <proto/intuition.h>
#include <proto/dos.h>

/****************************************************************************/

void
__assertion_failure(
	const char *	file_name,
	int				line_number,
	const char *	expression)
{
	static int been_here_before;

	/* Don't drop into a recursion. */
	if(been_here_before++ == 0)
	{
		if(__no_standard_io || __WBenchMsg != NULL)
		{
			#if defined(__amigaos4__)
			struct IntuitionIFace *	IIntuition = NULL;
			#endif /* __amigaos4__ */

			struct Library * IntuitionBase;

			IntuitionBase = OpenLibrary("intuition.library",37);

			#if defined(__amigaos4__)
			{
				if(IntuitionBase != NULL)
				{
					IIntuition = (struct IntuitionIFace *)GetInterface(IntuitionBase, "main", 1, 0);
					if(IIntuition == NULL)
					{
						CloseLibrary(IntuitionBase);
						IntuitionBase = NULL;
					}
				}
			}
			#endif /* __amigaos4__ */

			if(IntuitionBase != NULL)
			{
				struct EasyStruct es;

				memset(&es,0,sizeof(es));

				es.es_StructSize	= sizeof(es);
				es.es_Title			= (STRPTR)__program_name;
				es.es_TextFormat	= (STRPTR)"Assertion of expression\n\"%s\"\nfailed in file \"%s\", line %ld.";
				es.es_GadgetFormat	= (STRPTR)"Sorry";

				EasyRequest(NULL,&es,NULL,
					expression,file_name,line_number);

				#if defined(__amigaos4__)
				{
					DropInterface((struct Interface *)IIntuition);
				}
				#endif /* __amigaos4__ */

				CloseLibrary(IntuitionBase);
			}
		}
		else
		{
			if(__program_name != NULL)
				fprintf(stderr,"[%s] ",__program_name);

			fprintf(stderr,
				"%s:%d: failed assertion \"%s\".\n",
					file_name,
					line_number,
					expression);
		}

		abort();
	}

	been_here_before--;
}
