/*
 * $Id: stdlib_atexit.c,v 1.6 2006-01-08 12:04:25 obarthel Exp $
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

#ifndef _STDLIB_NULL_POINTER_CHECK_H
#include "stdlib_null_pointer_check.h"
#endif /* _STDLIB_NULL_POINTER_CHECK_H */

/****************************************************************************/

#ifndef _STDLIB_HEADERS_H
#include "stdlib_headers.h"
#endif /* _STDLIB_HEADERS_H */

/****************************************************************************/

#ifndef _STDLIB_MEMORY_H
#include "stdlib_memory.h"
#endif /* _STDLIB_MEMORY_H */

/****************************************************************************/

struct ExitTrapNode
{
	struct MinNode	etn_MinNode;
	void			(*etn_Function)(void);
};

/****************************************************************************/

/* 32 functions are the minimum required by ISO 'C'. */
static struct ExitTrapNode	exit_node_table[32];
static struct MinList		exit_trap_list;
static size_t				num_exit_nodes_used;
static BOOL					atexit_blocked;

/****************************************************************************/

int
atexit(void (*function)(void))
{
	struct ExitTrapNode * etn;
	int result = ERROR;

	assert( function != NULL );

	#if defined(CHECK_FOR_NULL_POINTERS)
	{
		if(function == NULL)
		{
			__set_errno(EFAULT);
			goto out;
		}
	}
	#endif /* CHECK_FOR_NULL_POINTERS */

	if(atexit_blocked)
	{
		__set_errno(EACCES);
		goto out;
	}

	/* We always have room for 32 exit trap functions. */
	if(num_exit_nodes_used < NUM_ENTRIES(exit_node_table))
	{
		etn = &exit_node_table[num_exit_nodes_used++];
	}
	else
	{
		/* More than 32 exit trap functions are called
		 * for. Let's make room for another one.
		 */
		etn = malloc(sizeof(*etn));
		if(etn == NULL)
		{
			__set_errno(ENOMEM);
			goto out;
		}
	}

	etn->etn_Function = function;

	if(exit_trap_list.mlh_Head == NULL)
		NewList((struct List *)&exit_trap_list);

	AddHead((struct List *)&exit_trap_list,(struct Node *)etn);

	result = OK;

 out:

	return(result);
}

/****************************************************************************/

void
__exit_trap_trigger(void)
{
	struct ExitTrapNode * etn;

	/* Disallow further additions to the atexit() list. */
	atexit_blocked = TRUE;

	/* Remove the trap entries one at a time and process them.
	 * Should one of the routines drop into exit(), processing
	 * will continue with the next entry until eventually all
	 * list items have been taken care of.
	 */
	if(exit_trap_list.mlh_Head != NULL)
	{
		while((etn = (struct ExitTrapNode *)RemHead((struct List *)&exit_trap_list)) != NULL)
			(*etn->etn_Function)();
	}
}
