/*
 * $Id: stdlib_constructor_begin.c,v 1.9 2005-03-20 12:14:09 obarthel Exp $
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

/****************************************************************************/

#if defined(__SASC)

/****************************************************************************/

extern int	(* __far __ctors[])(void);
extern void	(* __far __dtors[])(void);

/****************************************************************************/

/* With SAS/C this function is placed in front of the first constructor
   table entry. It will invoke all following constructors and
   finally all the destructors. We don't use this approach here. */
void
__construct(void)
{
}

/****************************************************************************/

void
_init(void)
{
	void * volatile p = &__ctors;

	/* The address is NULL if no constructor functions are to be used. */
	if(p != NULL)
	{
		int num_constructors;
		int i;

		num_constructors = 0;

		while(__ctors[num_constructors] != NULL)
			num_constructors++;

		for(i = 0 ; i < num_constructors ; i++)
		{
			if((*__ctors[num_constructors - (i + 1)])() != 0)
				exit(RETURN_FAIL);
		}
	}
}

/****************************************************************************/

void
_fini(void)
{
	void * volatile p = &__dtors;

	/* The address is NULL if no destructor functions are to be used. */
	if(p != NULL)
	{
		static int i;

		int num_destructors;

		num_destructors = 0;

		while(__dtors[num_destructors] != NULL)
			num_destructors++;

		while(i < num_destructors)
		{
			/* Increment this before jumping in, so that the next
			   invocation will always pick up the destructor following
			   the one we will invoke rigt now. */
			i++;

			(*__dtors[num_destructors - i])();
		}
	}
}

/****************************************************************************/

#endif /* __SASC */

/****************************************************************************/

#if defined(__GNUC__) && !defined(__amigaos4__)

/****************************************************************************/

/* The GCC 68k port does not sort constructor/destructor functions. We
   have to sort them here all on our own before they can be used. */

/****************************************************************************/

typedef void (*func_ptr)(void);

/****************************************************************************/

struct private_function
{
	func_ptr	function;
	int			priority;
};

/****************************************************************************/

extern func_ptr __INIT_LIST__[];
extern func_ptr __EXIT_LIST__[];

/****************************************************************************/

extern func_ptr __CTOR_LIST__[];
extern func_ptr __DTOR_LIST__[];

/****************************************************************************/

/* Sort the private function table in ascending order by priority. This is
   a simple bubblesort algorithm which assumes that there will be at least
   two entries in the table worth sorting. */
static void
sort_private_functions(struct private_function * base, size_t count)
{
	struct private_function * a;
	struct private_function * b;
	size_t i,j;

	i = count - 2;

	do
	{
		a = base;

		for(j = 0 ; j <= i ; j++)
		{
			b = a + 1;

			if(a->priority > b->priority)
			{
				struct private_function t;

				t		= (*a);
				(*a)	= (*b);
				(*b)	= t;
			}

			a = b;
		}
	}
	while(i-- > 0);
}

/****************************************************************************/

/* Sort all the init and exit functions (private library constructors), then
   invoke the init functions in descending order. */
static void
call_init_functions(void)
{
	LONG num_init_functions = (LONG)(__INIT_LIST__[0]) / 2;
	LONG num_exit_functions = (LONG)(__EXIT_LIST__[0]) / 2;

	ENTER();

	SHOWVALUE(num_init_functions);

	if(num_init_functions > 1)
		sort_private_functions((struct private_function *)&__INIT_LIST__[1],num_init_functions);

	SHOWVALUE(num_exit_functions);

	if(num_exit_functions > 1)
		sort_private_functions((struct private_function *)&__EXIT_LIST__[1],num_exit_functions);

	if(num_init_functions > 0)
	{
		struct private_function * t = (struct private_function *)&__INIT_LIST__[1];
		LONG i,j;

		for(j = 0 ; j < num_init_functions ; j++)
		{
			i = num_init_functions - (j + 1);

			D(("calling init function #%ld, 0x%08lx",i,t[i].function));

			(*t[i].function)();
		}
	}

	LEAVE();
}

/****************************************************************************/

/* Call all exit functions in ascending order; this assumes that the function
   table was prepared by call_init_functions() above. */
static void
call_exit_functions(void)
{
	LONG num_exit_functions = (LONG)(__EXIT_LIST__[0]) / 2;

	ENTER();

	if(num_exit_functions > 0)
	{
		STATIC LONG j = 0;

		struct private_function * t = (struct private_function *)&__EXIT_LIST__[1];
		LONG i;

		while(j++ < num_exit_functions)
		{
			i = j - 1;

			D(("calling exit function #%ld, 0x%08lx",i,t[i].function));

			(*t[i].function)();
		}
	}

	LEAVE();
}

/****************************************************************************/

static void
call_constructors(void)
{
	ULONG num_ctors = (ULONG)__CTOR_LIST__[0];
	ULONG i;

	ENTER();

	D(("there are %ld constructors to be called",num_ctors));

	/* Call all constructors in reverse order */
	for(i = 0 ; i < num_ctors ; i++)
	{
		D(("calling constructor #%ld, 0x%08lx",i,__CTOR_LIST__[1+i]));

		__CTOR_LIST__[num_ctors - i]();
	}

	LEAVE();
}

/****************************************************************************/

static void
call_destructors(void)
{
	ULONG num_dtors = (ULONG)__DTOR_LIST__[0];
	static ULONG i;

	ENTER();

	D(("there are %ld destructors to be called",num_dtors));

	/* Call all destructors in forward order */
	while(i++ < num_dtors)
	{
		D(("calling destructor #%ld, 0x%08lx",i,__DTOR_LIST__[i]));

		__DTOR_LIST__[i]();
	}

	SHOWMSG("all done.");

	LEAVE();
}

/****************************************************************************/

void
_init(void)
{
	ENTER();

	call_init_functions();
	call_constructors();

	LEAVE();
}

/****************************************************************************/

void
_fini(void)
{
	ENTER();

	call_destructors();
	call_exit_functions();

	LEAVE();
}

/****************************************************************************/

#endif /* __GNUC__ && !__amigaos4__ */
