/*
 * $Id: stdlib_init_exit.c,v 1.2 2004-12-19 16:42:51 obarthel Exp $
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

void
__stdlib_exit(void)
{
	ENTER();

	#ifdef __MEM_DEBUG
	{
		kprintf("[%s] %ld bytes still allocated upon exit, maximum of %ld bytes allocated at a time.\n",
			__program_name,__current_memory_allocated,__maximum_memory_allocated);

		kprintf("[%s] %ld chunks of memory still allocated upon exit, maximum of %ld chunks allocated at a time.\n",
			__program_name,__current_num_memory_chunks_allocated,__maximum_num_memory_chunks_allocated);

		__check_memory_allocations(__FILE__,__LINE__);
	}
	#endif /* __MEM_DEBUG */

	/* Clean this up, too, just to be safe. */
	NewList((struct List *)&__alloca_memory_list);

	#if defined(__MEM_DEBUG)
	{
		__never_free = FALSE;

		if(__memory_list.mlh_Head != NULL)
		{
			while(NOT IsListEmpty((struct List *)&__memory_list))
			{
				((struct MemoryNode *)__memory_list.mlh_Head)->mn_AlreadyFree = FALSE;

				__free_memory_node((struct MemoryNode *)__memory_list.mlh_Head,__FILE__,__LINE__);
			}
		}
	}
	#endif /* __MEM_DEBUG */

	#if defined(__USE_MEM_TREES) && defined(__MEM_DEBUG)
	{
		__initialize_red_black_tree(&__memory_tree);
	}
	#endif /* __USE_MEM_TREES && __MEM_DEBUG */

	if(__memory_pool != NULL)
	{
		NewList((struct List *)&__memory_list);

		DeletePool(__memory_pool);
		__memory_pool = NULL;
	}
	else
	{
		if(__memory_list.mlh_Head != NULL)
		{
			#ifdef __MEM_DEBUG
			{
				while(NOT IsListEmpty((struct List *)&__memory_list))
					__free_memory_node((struct MemoryNode *)__memory_list.mlh_Head,__FILE__,__LINE__);
			}
			#else
			{
				while(NOT IsListEmpty((struct List *)&__memory_list))
					__free_memory_node((struct MemoryNode *)__memory_list.mlh_Head,NULL,0);
			}
			#endif /* __MEM_DEBUG */
		}
	}

	if(__free_program_name && __program_name != NULL)
	{
		FreeVec(__program_name);
		__program_name = NULL;
	}

	LEAVE();
}

/****************************************************************************/

int
__stdlib_init(void)
{
	ENTER();

	#if defined(__USE_MEM_TREES) && defined(__MEM_DEBUG)
	{
		__initialize_red_black_tree(&__memory_tree);
	}
	#endif /* __USE_MEM_TREES && __MEM_DEBUG */

	NewList((struct List *)&__memory_list);
	NewList((struct List *)&__alloca_memory_list);

	if(((struct Library *)SysBase)->lib_Version >= 39)
		__memory_pool = CreatePool(MEMF_ANY,(ULONG)__default_pool_size,(ULONG)__default_puddle_size);

	RETURN(OK);
	return(OK);
}
