/*
 * $Id: stdlib_constructor_begin.c,v 1.4 2005-02-25 10:14:21 obarthel Exp $
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

/****************************************************************************/

#include <stdlib.h>
#include <setjmp.h>

/****************************************************************************/

#if defined(__SASC)

/****************************************************************************/

extern int	(* __far __ctors[])(void);
extern void	(* __far __dtors[])(void);

/****************************************************************************/

/* With SAS/C this function is placed in front of the first constructor
 * table entry. It will invoke all following constructors and
 * finally all the destructors. We don't use this approach here.
 */
void
__construct(void)
{
}

/****************************************************************************/

STATIC VOID
_do_ctors(void)
{
	void * volatile p = &__ctors;

	/* The address is NULL if no constructor functions are to be used. */
	if(p != NULL)
	{
		int i;

		for(i = 0 ; __ctors[i] != NULL ; i++)
		{
			if((*__ctors[i])() != 0)
				exit(RETURN_FAIL);
		}
	}
}

/****************************************************************************/

STATIC VOID
_do_dtors(void)
{
	void * volatile p = &__dtors;

	/* The address is NULL if no destructor functions are to be used. */
	if(p != NULL)
	{
		static int i;

		/* If one of the destructors drops into
		 * exit(), processing will continue with
		 * the next following destructor.
		 */
		(void)setjmp(__exit_jmp_buf);

		while(__dtors[i] != NULL)
		{
			/* Increment this before jumping in, so that the next
			 * invocation will always pick up the destructor following
			 * the one we will invoke rigt now.
			 */
			i++;

			(*__dtors[i-1])();
		}
	}
}

/****************************************************************************/

#else

/****************************************************************************/

typedef void (*func_ptr)(void);

/****************************************************************************/

STATIC VOID
_do_ctors(void)
{
	extern func_ptr __CTOR_LIST__[];
	ULONG nptrs = (ULONG)__CTOR_LIST__[0];
	ULONG i;

	ENTER();

	D(("there are %ld constructors to be called",nptrs));

	/* Call all constructors in forward order */
	for(i = 0 ; i < nptrs ; i++)
	{
		D(("calling constructor #%ld, 0x%08lx",i,__CTOR_LIST__[1+i]));

		__CTOR_LIST__[1+i]();
	}

	SHOWMSG("all done.");

	LEAVE();
}

/****************************************************************************/

STATIC VOID
_do_dtors(void)
{
	extern func_ptr __DTOR_LIST__[];
	extern jmp_buf __exit_jmp_buf;
	ULONG nptrs = (ULONG)__DTOR_LIST__[0];
	static ULONG i;

	ENTER();

	D(("there are %ld destructors to be called",nptrs));

	/* If one of the destructors drops into
	 * exit(), processing will continue with
	 * the next following destructor.
	 */
	(void)setjmp(__exit_jmp_buf);

	/* Call all destructors in reverse order */
	while(i++ < nptrs)
	{
		D(("calling destructor #%ld, 0x%08lx",i,__DTOR_LIST__[1+nptrs - i]));

		__DTOR_LIST__[1+nptrs - i]();
	}

	SHOWMSG("all done.");

	LEAVE();
}

/****************************************************************************/

#endif /*__amigaos4__ */

/****************************************************************************/

/* FIXME: Do we need to put these in .init/.fini sections? */

//void _init(void) __attribute__((section(".init")));
//void _fini(void) __attribute__((section(".fini")));

/****************************************************************************/

void
_init(void)
{
	_do_ctors();
}

/****************************************************************************/

void
_fini(void)
{
	_do_dtors();
}
