/*
 * $Id: stdlib_alloca.c,v 1.1.1.1 2004-07-26 16:31:50 obarthel Exp $
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

#ifndef _STDLIB_MEM_DEBUG_H
#include "stdlib_mem_debug.h"
#endif /* _STDLIB_MEM_DEBUG_H */

/****************************************************************************/

#ifndef _STDLIB_HEADERS_H
#include "stdlib_headers.h"
#endif /* _STDLIB_HEADERS_H */

/****************************************************************************/

#undef alloca

extern void * alloca(size_t size);

/****************************************************************************/

struct MemoryContextNode
{
	struct MinNode	mcn_MinNode;
	ULONG			mcn_StackPointer;
	char *			mcn_Memory;
};

/****************************************************************************/

extern unsigned long __get_sp(void);

/****************************************************************************/

void
__alloca_cleanup(const char * file,int line)
{
	ULONG stack_pointer = __get_sp();
	struct MemoryContextNode * mcn_next;
	struct MemoryContextNode * mcn;

	/* ZZZ this could be done in a much smarter fashion by paying attention
	 * to the fact that the 'mcn_StackPointer' members are sorted...
	 */
	for(mcn = (struct MemoryContextNode *)__alloca_memory_list.mlh_Head ;
	    mcn->mcn_MinNode.mln_Succ != NULL ;
	    mcn = mcn_next)
	{
		mcn_next = (struct MemoryContextNode *)mcn->mcn_MinNode.mln_Succ;

		if(mcn->mcn_StackPointer < stack_pointer)
		{
			Remove((struct Node *)mcn);

			__force_free(mcn->mcn_Memory,file,line);
			__force_free(mcn,file,line);
		}
	}
}

/****************************************************************************/

void *
__alloca(size_t size,const char * file,int line)
{
	ULONG stack_pointer = __get_sp();
	struct MemoryContextNode * mcn;
	void * result = NULL;

	__alloca_cleanup(file,line);

	assert( (int)size >= 0 );

	if(size == 0)
	{
		#ifdef __MEM_DEBUG
		{
			kprintf("[%s] ",__program_name);

			if(file != NULL)
				kprintf("%s:%ld:",file,line);

			kprintf("alloca(0) called.\n");
		}
		#endif /* __MEM_DEBUG */

		goto out;
	}

	mcn = __allocate_memory(sizeof(*mcn),FALSE,file,line);
	if(mcn == NULL)
	{
		SHOWMSG("not enough memory");
		goto out;
	}

	/* Allocate memory which cannot be run through realloc()
	 * or free().
	 */
	mcn->mcn_Memory = __allocate_memory(size,TRUE,file,line);
	if(mcn->mcn_Memory == NULL)
	{
		SHOWMSG("not enough memory");

		__force_free(mcn,file,line);
		goto out;
	}

	mcn->mcn_StackPointer = stack_pointer;

	AddTail((struct List *)&__alloca_memory_list,(struct Node *)mcn);

	result = mcn->mcn_Memory;

 out:

	return(result);
}

/****************************************************************************/

void *
alloca(size_t size)
{
	void * result;

	result = __alloca(size,NULL,0);

	return(result);
}
