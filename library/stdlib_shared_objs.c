/*
 * $Id: stdlib_shared_objs.c,v 1.1 2010-08-21 11:37:03 obarthel Exp $
 *
 * :ts=4
 *
 * Portable ISO 'C' (1994) runtime library for the Amiga computer
 * Copyright (c) 2002-2010 by Olaf Barthel <olsen (at) sourcery.han.de>
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

#if defined(__amigaos4__)

/****************************************************************************/

#ifndef _STDLIB_HEADERS_H
#include "stdlib_headers.h"
#endif /* _STDLIB_HEADERS_H */

/****************************************************************************/

#include <libraries/elf.h>
#include <proto/elf.h>

/****************************************************************************/

/* These are used to initialize the shared objects linked to this binary,
   and for the dlopen(), dlclose() and dlsym() functions. */
struct Library *	__ElfBase;
struct ElfIFace *	__IElf;

/* This is used with the dlopen(), dlclose() and dlsym() functions. */
Elf32_Handle __dl_elf_handle;

/****************************************************************************/

/* This is used to initialize the shared objects only. */
static Elf32_Handle elf_handle;

/****************************************************************************/

void shared_obj_init(void);
void shared_obj_exit(void);

/****************************************************************************/

static VOID close_elf_library(void)
{
	if(__IElf != NULL)
	{
		DropInterface((struct Interface *)__IElf);
		__IElf = NULL;
	}
		
	if(__ElfBase != NULL)
	{
		CloseLibrary(__ElfBase);
		__ElfBase = NULL;
	}
}

/****************************************************************************/

static BOOL open_elf_library(void)
{
	BOOL success = FALSE;

	/* We need elf.library V52.2 or higher. */
	__ElfBase = OpenLibrary("elf.library",0);
	if(__ElfBase == NULL || (__ElfBase->lib_Version < 52) || (__ElfBase->lib_Version == 52 && __ElfBase->lib_Revision < 2))
		goto out;

	__IElf = (struct ElfIFace *)GetInterface(__ElfBase,"main",1,NULL);
	if(__IElf == NULL)
		goto out;

	success = TRUE;

 out:

	return(success);
}

/****************************************************************************/

void shared_obj_exit(void)
{
	struct ElfIFace * IElf = __IElf;

	#ifndef __THREAD_SAFE
	{
		/* Release this program's Elf handle, if we grabbed it below. */
		if(__dl_elf_handle != NULL)
		{
			CloseElfTags(__dl_elf_handle,
				CET_ReClose,TRUE,
			TAG_DONE);
			
			__dl_elf_handle = NULL;
		}
	}
	#endif /* __THREAD_SAFE */
	
	/* If we got what we wanted, trigger the destructors,
	   etc. in the shared objects linked to this binary. */
	if(elf_handle != NULL)
	{
		InitSHLibs(elf_handle,FALSE);
		elf_handle = NULL;
	}

	close_elf_library();
}

/****************************************************************************/

void shared_obj_init(void)
{
	if(open_elf_library())
	{
		struct ElfIFace * IElf = __IElf;
		BPTR segment_list;

		/* Try to find the Elf handle associated with this
		   program's segment list. */
		segment_list = GetProcSegList(NULL,GPSLF_CLI | GPSLF_SEG);
		if(segment_list != ZERO)
		{
			if(GetSegListInfoTags(segment_list,
				GSLI_ElfHandle,&elf_handle,
			TAG_DONE) == 1)
			{
				if(elf_handle != NULL)
				{
					/* Trigger the constructors, etc. in the shared objects
					   linked to this binary. */
					InitSHLibs(elf_handle,TRUE);
				}
			}
		}
		
		/* Next: try to grab the Elf handle associated with the currently
		   running process. This is not thread-safe! */
		#ifndef __THREAD_SAFE
		{
			segment_list = GetProcSegList(NULL,GPSLF_RUN);
			if(segment_list != ZERO)
			{
				Elf32_Handle handle = NULL;

				if(GetSegListInfoTags(segment_list,
					GSLI_ElfHandle,&handle,
				TAG_DONE) == 1)
				{
					if(handle != NULL)
					{
						__dl_elf_handle = OpenElfTags(
							OET_ElfHandle,handle,
						TAG_DONE);
					}
				}
			}
		}
		#endif /* __THREAD_SAFE */
	}
}

/****************************************************************************/

#endif /*__amigaos4__ */
