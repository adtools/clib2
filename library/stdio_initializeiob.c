/*
 * $Id: stdio_initializeiob.c,v 1.1.1.1 2004-07-26 16:31:36 obarthel Exp $
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

#ifndef _STDIO_HEADERS_H
#include "stdio_headers.h"
#endif /* _STDIO_HEADERS_H */

/****************************************************************************/

void
__initialize_iob(
	struct iob *	iob,
	HOOKFUNC		hook_function,
	STRPTR			custom_buffer,
	STRPTR			buffer,
	int				buffer_size,
	int				file_descriptor,
	int				slot_number,
	ULONG			flags)
{
	assert( iob != NULL && hook_function != NULL );

	memset(iob,0,sizeof(*iob));

	iob->iob_CustomBuffer	= custom_buffer;
	iob->iob_Buffer			= buffer;
	iob->iob_BufferSize		= buffer_size;
	iob->iob_Descriptor		= file_descriptor;
	iob->iob_SlotNumber		= slot_number;
	iob->iob_Flags			= flags;
	iob->iob_Hook			= &iob->iob_DefaultHook;

	#if defined(__amigaos4__)
	{
		iob->iob_Hook->h_Entry		= (HOOKFUNC)hook_function;
	}
	#else
	{
		iob->iob_Hook->h_Entry		= (HOOKFUNC)HookEntry;
		iob->iob_Hook->h_SubEntry	= (HOOKFUNC)hook_function;
	}
	#endif /* __amigaos4__ */
}
