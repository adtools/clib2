/*
 * $Id: stdlib_malloc.c,v 1.4 2004-12-26 10:28:56 obarthel Exp $
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

#undef malloc
#undef __malloc

/****************************************************************************/

#ifdef __MEM_DEBUG

unsigned long __maximum_memory_allocated;
unsigned long __current_memory_allocated;
unsigned long __maximum_num_memory_chunks_allocated;
unsigned long __current_num_memory_chunks_allocated;

#if defined(__USE_MEM_TREES)
struct MemoryTree __memory_tree;
#endif /* __USE_MEM_TREES */

#endif /* __MEM_DEBUG */

/****************************************************************************/

APTR			__memory_pool;
struct MinList	__memory_list;

/****************************************************************************/

size_t
__get_allocation_size(size_t size)
{
	#ifndef __MEM_DEBUG
	{
		size_t total_allocation_size;

		total_allocation_size = sizeof(struct MemoryNode) + size;

		/* Round up the allocation size to the physical allocation granularity. */
		size += ((total_allocation_size + MEM_BLOCKMASK) & ~((ULONG)MEM_BLOCKMASK)) - total_allocation_size;
	}
	#endif /* __MEM_DEBUG */

	return(size);
}

/****************************************************************************/

void *
__allocate_memory(size_t size,BOOL never_free,const char * UNUSED file,int UNUSED line)
{
	struct MemoryNode * mn;
	size_t allocation_size;
	void * result = NULL;

	assert( size > 0 );

	if(__free_memory_threshold > 0 && AvailMem(MEMF_ANY|MEMF_LARGEST) < __free_memory_threshold)
	{
		SHOWMSG("not enough free memory available to safely proceed with allocation");

		errno = ENOMEM;
		goto out;
	}

	#ifdef __MEM_DEBUG
	{
		assert( MALLOC_HEAD_SIZE > 0 && (MALLOC_HEAD_SIZE % 4) == 0 );
		assert( MALLOC_TAIL_SIZE > 0 && (MALLOC_TAIL_SIZE % 4) == 0 );
		assert( (sizeof(*mn) % 4) == 0 );

		allocation_size = sizeof(*mn) + MALLOC_HEAD_SIZE + size + MALLOC_TAIL_SIZE;
	}
	#else
	{
		/* Round up the allocation size to the physical allocation granularity. */
		size = __get_allocation_size(size);

		allocation_size = sizeof(*mn) + size;
	}
	#endif /* __MEM_DEBUG */

	if(__memory_pool != NULL)
		mn = AllocPooled(__memory_pool,allocation_size);
	else
		mn = AllocMem(allocation_size,MEMF_ANY);

	if(mn == NULL)
	{
		SHOWMSG("not enough memory");

		errno = ENOMEM;
		goto out;
	}

	mn->mn_Size			= size;
	mn->mn_NeverFree	= never_free;

	AddTail((struct List *)&__memory_list,(struct Node *)mn);

	#ifdef __MEM_DEBUG
	{
		char * head = (char *)(mn + 1);
		char * body = head + MALLOC_HEAD_SIZE;
		char * tail = body + size;

		mn->mn_AlreadyFree		= FALSE;
		mn->mn_Allocation		= body;
		mn->mn_AllocationSize	= allocation_size;
		mn->mn_File				= (char *)file;
		mn->mn_Line				= line;
		mn->mn_FreeFile			= NULL;
		mn->mn_FreeLine			= 0;

		memset(head,MALLOC_HEAD_FILL,MALLOC_HEAD_SIZE);
		memset(body,MALLOC_NEW_FILL,size);
		memset(tail,MALLOC_TAIL_FILL,MALLOC_TAIL_SIZE);

		__current_memory_allocated += size;
		if(__maximum_memory_allocated < __current_memory_allocated)
			__maximum_memory_allocated = __current_memory_allocated;

		__current_num_memory_chunks_allocated++;
		if(__maximum_num_memory_chunks_allocated < __current_num_memory_chunks_allocated)
			__maximum_num_memory_chunks_allocated = __current_num_memory_chunks_allocated;

		#ifdef __MEM_DEBUG_LOG
		{
			kprintf("[%s] + %10ld 0x%08lx [",__program_name,size,body);

			kprintf("allocated at %s:%ld]\n",file,line);
		}
		#endif /* __MEM_DEBUG_LOG */

		#ifdef __USE_MEM_TREES
		{
			__red_black_tree_insert(&__memory_tree,mn);
		}
		#endif /* __USE_MEM_TREES */

		result = mn->mn_Allocation;
	}
	#else
	{
		result = &mn[1];
	}
	#endif /* __MEM_DEBUG */

	assert( (((ULONG)result) & 3) == 0 );

 out:

	#ifdef __MEM_DEBUG_LOG
	{
		if(result == NULL)
		{
			kprintf("[%s] + %10ld 0x%08lx [",__program_name,size,NULL);

			kprintf("FAILED: allocated at %s:%ld]\n",file,line);
		}
	}
	#endif /* __MEM_DEBUG_LOG */

	return(result);
}

/****************************************************************************/

__static void *
__malloc(size_t size,const char * file,int line)
{
	void * result = NULL;

	/* Try to get rid of now unused memory. */
	/*__alloca_cleanup(file,line);*/

	#ifdef __MEM_DEBUG
	{
		/*if((rand() % 16) == 0)
			__check_memory_allocations(file,line);*/
	}
	#endif /* __MEM_DEBUG */

	if(size == 0)
	{
		#ifdef __MEM_DEBUG
		{
			kprintf("[%s] ",__program_name);

			if(file != NULL)
				kprintf("%s:%ld:",file,line);

			kprintf("malloc(0) called.\n");
		}
		#endif /* __MEM_DEBUG */

		goto out;
	}

	assert( (int)size > 0 );

	/* Allocate memory which can be put through realloc() and free(). */
	result = __allocate_memory(size,FALSE,file,line);

 out:

	return(result);
}

/****************************************************************************/

void *
malloc(size_t size)
{
	void * result;

	result = __malloc(size,NULL,0);

	return(result);
}

/****************************************************************************/

void
__memory_init(void)
{
	ENTER();

	#if defined(__USE_MEM_TREES) && defined(__MEM_DEBUG)
	{
		__initialize_red_black_tree(&__memory_tree);
	}
	#endif /* __USE_MEM_TREES && __MEM_DEBUG */

	NewList((struct List *)&__memory_list);

	if(((struct Library *)SysBase)->lib_Version >= 39)
		__memory_pool = CreatePool(MEMF_ANY,(ULONG)__default_pool_size,(ULONG)__default_puddle_size);

	LEAVE();
}
