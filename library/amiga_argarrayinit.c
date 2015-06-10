/*
 * $Id: amiga_argarrayinit.c,v 1.7 2008-09-30 14:09:00 obarthel Exp $
 *
 * :ts=4
 *
 * Portable ISO 'C' (1994) runtime library for the Amiga computer
 * Copyright (c) 2002-2015 by Olaf Barthel <obarthel (at) gmx.net>
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

#include <libraries/commodities.h>
#include <workbench/workbench.h>
#include <workbench/startup.h>
#include <exec/memory.h>

#include <string.h>

/****************************************************************************/

#include <proto/exec.h>
#include <proto/dos.h>
#include <proto/icon.h>
#include <proto/commodities.h>
#include <clib/alib_protos.h>

/****************************************************************************/

#include "debug.h"

/****************************************************************************/

#ifdef __amigaos4__
#define MEMORY_TYPE MEMF_PRIVATE
#else
#define MEMORY_TYPE MEMF_ANY
#endif /* __amigaos4__ */

/****************************************************************************/

const unsigned char ** CXLIB_argarray;

struct DiskObject * CXLIB_disko;

/****************************************************************************/

STRPTR *
ArgArrayInit(LONG argc, CONST_STRPTR * argv)
{
	STRPTR * result = NULL;

	if(argc != 0) /* run from CLI */
	{
		LONG i;

		if(argc == 1)
			goto out; /* skip command name */

		CXLIB_argarray = (const unsigned char **)AllocVec(sizeof(char *) * argc,MEMORY_TYPE|MEMF_CLEAR);
		if(CXLIB_argarray == NULL)
			goto out;

		for(i = 1 ; i < argc ; i++)
			CXLIB_argarray[i-1] = (unsigned char *)argv[i];

		result = (STRPTR *)CXLIB_argarray;
	}
	else if (IconBase != NULL)
	{
		struct WBStartup * msg;

		/* run from WB */
		msg = (struct WBStartup *)argv;

		CXLIB_disko = GetDiskObject(msg->sm_ArgList[0].wa_Name);
		if(CXLIB_disko == NULL)
			goto out;

		result = (STRPTR *)CXLIB_disko->do_ToolTypes;
	}

 out:

	return(result);
}
