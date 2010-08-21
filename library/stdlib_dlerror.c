/*
 * $Id: stdlib_dlerror.c,v 1.1 2010-08-21 10:59:34 obarthel Exp $
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

/****************************************************************************/

extern Elf32_Error __elf_error_code;

/****************************************************************************/

const char * dlerror(void)
{
	const char * result;

	switch(__elf_error_code)
	{
		case ELF32_NO_ERROR:

			break;

		case ELF32_OUT_OF_MEMORY:

			result = "out of memory";
			break;

		case ELF32_INVALID_HANDLE:

			result = "invalid handle";
			break;

		case ELF32_NO_MORE_RELOCS:

			result = "no more relocs";
			break;

		case ELF32_SECTION_NOT_LOADED:

			result = "section not loaded";
			break;

		case ELF32_UNKNOWN_RELOC:

			result = "unknown reloc";
			break;

		case ELF32_READ_ERROR:

			result = "read error";
			break;

		case ELF32_INVALID_SDA_BASE:

			result = "invalid SDA base";
			break;

		case ELF32_SYMBOL_NOT_FOUND:

			result = "symbol not found";
			break;

		case ELF32_INVALID_NAME:

			result = "invalid name";
			break;

		case ELF32_REQUIRED_OBJECT_MISSING:

			result = required object missing";
			break;

		default:

			result = "unknown error";
			break;
	}

	__elf_error_code = ELF32_NO_ERROR;

	return(result);
}

/****************************************************************************/

#endif /* __amigaos4__ */
