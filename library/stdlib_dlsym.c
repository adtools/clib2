/*
 * $Id: stdlib_dlsym.c,v 1.1 2010-08-21 10:59:34 obarthel Exp $
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

#ifndef _STDLIB_HEADERS_H
#include "stdlib_headers.h"
#endif /* _STDLIB_HEADERS_H */

/****************************************************************************/

/* The shared object API is available only on AmigaOS 4.0. */
#if defined(__amigaos4__)

/****************************************************************************/

#include <dlfcn.h>

/****************************************************************************/

#include <libraries/elf.h>
#include <proto/elf.h>

/****************************************************************************/

extern struct ElfIFace *	__IElf;
extern Elf32_Handle			__elf_handle;

/****************************************************************************/

extern Elf32_Error			__elf_error_code;

/****************************************************************************/

void dlsym(void * handle,const char * symbol_name)
{
	void * result = NULL;

	if(__elf_handle != NULL)
	{
		struct ElfIFace * IElf = __IElf;
		APTR symbol_data = NULL;
		Elf32_Error error;

		error = DLSym(__elf_handle,handle,symbol_name,&symbol_data);
		if(error != ELF32_NO_ERROR)
		{
			__elf_error_code = error;
			goto out;
		}

		result = symbol_data;
	}

 out:

	return(result);
}

/****************************************************************************/

#endif /* __amigaos4__ */
