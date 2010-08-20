/*
 * $Id: stdlib_shared_libs.c,v 1.1 2010-08-20 15:33:36 obarthel Exp $
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

/* The following is not part of the ISO 'C' (1994) standard. */

/****************************************************************************/

#ifndef _STDLIB_HEADERS_H
#include "stdlib_headers.h"
#endif /* _STDLIB_HEADERS_H */

/****************************************************************************/

#ifndef _STDLIB_CONSTRUCTOR_H
#include "stdlib_constructor.h"
#endif /* _STDLIB_CONSTRUCTOR_H */

/****************************************************************************/

#include <libraries/elf.h>

#include <proto/elf.h>
#include <proto/dos.h>

/****************************************************************************/

static VOID shared_libs_init_exit(BOOL init_or_exit)
{
	struct Library * ElfBase;
	struct Library * DOSBase = NULL;
	struct ElfIFace * IElf = NULL;
	struct DOSIFace * IDOS = NULL;
	BPTR segment_list;

	ENTER();

	/* We need elf.library V52.2 or higher. */
	ElfBase = OpenLibrary("elf.library",0);
	if(ElfBase == NULL || (ElfBase->lib_Version < 52) || (ElfBase->lib_Version == 52 && ElfBase->lib_Revision < 2))
		goto out;
	
	IElf = (struct ElfIFace *)GetInterface(ElfBase,"main",1,NULL);
	if(IElf == NULL)
		goto out;
		
	DOSBase = OpenLibrary("dos.library",0);
	if(DOSBase == NULL)
		goto out;
		
	IDOS = (struct DOSIFace *)GetInterface(DOSBase,"main",1,NULL);
	if(IDOS == NULL)
		goto out;
	
	/* Try to find the Elf handle associated with this
	   program's segment list. */
	segment_list = GetProcSegList(NULL,GPSLF_CLI | GPSLF_SEG);
	if(segment_list != ZERO)
	{
		Elf32_Handle elf_handle = NULL;	

		if(GetSegListInfoTags(segment_list,
			GSLI_ElfHandle,&elf_handle,
		TAG_DONE) == 1)
		{
			/* Initialize the shared object system. Note that
			   we have no way of finding out whether this actually
			   worked... */
			if(elf_handle != NULL)
				InitSHLibs(elf_handle,init_or_exit);
		}
	}

 out:

	if(IDOS != NULL)
		DropInterface((struct Interface *)IDOS);
		
	if(DOSBase != NULL)
		CloseLibrary(DOSBase);
		
	if(IElf != NULL)
		DropInterface((struct Interface *)IElf);
		
	if(ElfBase != NULL)
		CloseLibrary(ElfBase);
		
	LEAVE();
}

/****************************************************************************/

CLIB_DESTRUCTOR(shared_libs_exit)
{
	ENTER();

	shared_libs_init_exit(FALSE);

	LEAVE();
}

/****************************************************************************/

CLIB_CONSTRUCTOR(shared_libs_init)
{
	ENTER();

	shared_libs_init_exit(TRUE);

	LEAVE();

	CONSTRUCTOR_SUCCEED();
}

/****************************************************************************/

#endif /* __amigaos4__ */
