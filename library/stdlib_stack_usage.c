/*
 * $Id: stdlib_stack_usage.c,v 1.8 2010-10-20 13:50:17 obarthel Exp $
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

#ifndef NDEBUG

/****************************************************************************/

#include "stdlib_headers.h"

/****************************************************************************/

static struct StackSwapStruct stack_swap_struct;

/****************************************************************************/

#define STACK_FILL_COOKIE 0xA1

/****************************************************************************/

void
__stack_usage_init(struct StackSwapStruct * stk)
{
	if(stk != NULL)
	{
		size_t stack_size = ((ULONG)stk->stk_Upper - (ULONG)stk->stk_Lower);

		memset(stk->stk_Lower,STACK_FILL_COOKIE,stack_size);

		stack_swap_struct = (*stk);
	}
}

/****************************************************************************/

void
__stack_usage_exit(void)
{
	if(stack_swap_struct.stk_Lower != NULL && stack_swap_struct.stk_Upper != 0)
	{
		const UBYTE * m = (const UBYTE *)stack_swap_struct.stk_Lower;
		size_t stack_size = ((ULONG)stack_swap_struct.stk_Upper - (ULONG)stack_swap_struct.stk_Lower);
		size_t total,i;

		total = 0;

		/* Figure out how much of the stack was used by checking
		   if the fill pattern was overwritten. */
		for(i = 0 ; i < stack_size ; i++)
		{
			/* Strangely, the first long word is always trashed,
			   even if the stack doesn't grow down this far... */
			if(i > sizeof(LONG) && m[i] != STACK_FILL_COOKIE)
				break;

			total++;				
		}

		kprintf("[%s] total amount of stack space used = %ld bytes\n",
			__program_name,stack_size - total);

		stack_swap_struct.stk_Lower = NULL;
		stack_swap_struct.stk_Upper = 0;
	}
}

/****************************************************************************/

#endif /* NDEBUG */
