/*
 * $Id: stdlib_showerror.c,v 1.7 2005-01-09 09:40:32 obarthel Exp $
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

#include <exec/libraries.h>

#include <intuition/intuition.h>

#include <dos/dos.h>

#include <workbench/startup.h>

/****************************************************************************/

#include <proto/exec.h>

#define __NOLIBBASE__
#define __NOGLOBALIFACE__
#include <proto/intuition.h>
#include <proto/dos.h>

/****************************************************************************/

#include <string.h>

/****************************************************************************/

#include "macros.h"

/****************************************************************************/

#ifndef _STDLIB_HEADERS_H
#include "stdlib_headers.h"
#endif /* _STDLIB_HEADERS_H */

#ifndef _STDIO_HEADERS_H
#include "stdio_headers.h"
#endif /* _STDIO_HEADERS_H */

/****************************************************************************/

void
__show_error(const char * message)
{
	#if defined(__amigaos4__)
	struct IntuitionIFace *	IIntuition	= NULL;
	struct DOSIFace *		IDOS		= NULL;
	#endif /* __amigaos4__ */

	struct Library * IntuitionBase;
	struct Library * DOSBase;

	PROFILE_OFF();

	DOSBase			= OpenLibrary("dos.library",0);
	IntuitionBase	= OpenLibrary("intuition.library",0);

	if(DOSBase == NULL || IntuitionBase == NULL)
		goto out;

	#if defined(__amigaos4__)
	{
		IDOS = (struct DOSIFace *)GetInterface(DOSBase, "main", 1, 0);
		if (IDOS == NULL)
			goto out;

		IIntuition = (struct IntuitionIFace *)GetInterface(IntuitionBase, "main", 1, 0);
		if (IIntuition == NULL)
			goto out;
	}
	#endif /* __amigaos4__ */

	/* If we can't hope to print the error message, show a requester instead. */
	if(__no_standard_io || __WBenchMsg != NULL)
	{
		if(IntuitionBase->lib_Version >= 37)
		{
			UBYTE program_name[256];
			struct EasyStruct es;
			STRPTR title_string;

			if(__WBenchMsg != NULL)
			{
				title_string = (STRPTR)FilePart(__WBenchMsg->sm_ArgList[0].wa_Name);
			}
			else
			{
				if(GetProgramName(program_name,sizeof(program_name)))
					title_string = FilePart((STRPTR)program_name);
				else
					title_string = (STRPTR)"Error";
			}

			memset(&es,0,sizeof(es));

			es.es_StructSize	= sizeof(es);
			es.es_Title			= title_string;
			es.es_TextFormat	= (STRPTR)message;
			es.es_GadgetFormat	= (STRPTR)"Sorry";

			EasyRequestArgs(NULL,&es,NULL,NULL);
		}
		else
		{
			/* The following does not make great sense on OS4. */
			#if NOT defined(__amigaos4__)
			{
				static struct TextAttr default_font	= { (STRPTR)"topaz.font",8,FS_NORMAL,FPF_ROMFONT|FPF_DESIGNED };
				static struct IntuiText sorry_text	= {0,1,JAM1,6,3,(struct TextAttr *)NULL,(STRPTR)"Sorry",(struct IntuiText *)NULL};
				static struct IntuiText body_text	= {0,1,JAM1,5,3,(struct TextAttr *)NULL,(STRPTR)NULL,(struct IntuiText *)NULL};

				sorry_text.ITextFont	= &default_font;
				body_text.ITextFont		= &default_font;

				body_text.IText = (STRPTR)message;

				AutoRequest(NULL,&body_text,NULL,&sorry_text,0,0,37 + 8 * strlen(message),46);
			}
			#endif /* __amigaos4__ */
		}
	}
	else
	{
		BPTR output;

		#if defined(__amigaos4__)
		{
			/* Try to print the error message on the default error output stream. */
			output = ErrorOutput();
			if(output == ZERO)
				output = Output();
		}
		#else
		{
			struct Process * this_process = (struct Process *)FindTask(NULL);

			if(this_process->pr_CES != ZERO)
				output = this_process->pr_CES;
			else
				output = Output();
		}
		#endif /* __amigaos4__ */

		if(output != ZERO)
		{
			Write(output,(STRPTR)message,(LONG)strlen(message));
			Write(output,"\n",1);
		}
	}

 out:

	#if defined(__amigaos4__)
	{
		if(IIntuition != NULL)
			DropInterface((struct Interface *)IIntuition);

		if(IDOS != NULL)
			DropInterface((struct Interface *)IDOS);
	}
	#endif /* __amigaos4__ */

	if (IntuitionBase != NULL)
		CloseLibrary(IntuitionBase);

	if (DOSBase != NULL)
		CloseLibrary(DOSBase);

	PROFILE_ON();
}
