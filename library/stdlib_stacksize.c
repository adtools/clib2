/*
 * $Id: stdlib_stacksize.c,v 1.3 2005-02-25 10:14:21 obarthel Exp $
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

#ifndef _STDLIB_GCC_HELP_H
#include "stdlib_gcc_help.h"
#endif /* _STDLIB_GCC_HELP_H */

/****************************************************************************/

/* Quick "hack" to help GCC compile itself */
#if defined(GCC_HELP_SUPPORT)

/******************************************************************************/

#include "stdlib_headers.h"

/******************************************************************************/

STATIC unsigned int
get_default_stack_size(void)
{
	unsigned int result = 0;
	char buffer[40];

	if(GetVar("GCC_STACK_SIZE",buffer,sizeof(buffer)-1,0) > 0)
	{
		LONG n;

		if((StrToLong(buffer,&n) > 0) && (n > 0))
			result = (unsigned int)n;
	}

	return(result);
}

/******************************************************************************/

unsigned int (* __get_default_stack_size)(void) = get_default_stack_size;
char * __stack_size_string = "$STACK: 200000";
unsigned int __stack_size = 200000;

#else

/******************************************************************************/

unsigned int __stack_size;

/******************************************************************************/

#endif /* GCC_HELP_SUPPORT */
