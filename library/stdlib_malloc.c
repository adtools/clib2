/*
 * $Id: stdlib_malloc.c,v 1.20 2008-09-30 14:09:00 obarthel Exp $
 *
 * :ts=4
 *
 * Portable ISO 'C' (1994) runtime library for the Amiga computer
 * Copyright (c) 2002-2015 by Olaf Barthel <obarthel (at) gmx.net>
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

/*#define DEBUG*/

#ifndef _STDLIB_HEADERS_H
#include "stdlib_headers.h"
#endif /* _STDLIB_HEADERS_H */

/****************************************************************************/

#ifndef _STDLIB_MEMORY_H
#include "stdlib_memory.h"
#endif /* _STDLIB_MEMORY_H */

/****************************************************************************/

#ifndef _STDLIB_CONSTRUCTOR_H
#include "stdlib_constructor.h"
#endif /* _STDLIB_CONSTRUCTOR_H */

/****************************************************************************/

#undef malloc
#undef __malloc

/****************************************************************************/

unsigned long NOCOMMON __maximum_memory_allocated;
unsigned long NOCOMMON __current_memory_allocated;

unsigned long NOCOMMON __maximum_num_memory_chunks_allocated;
unsigned long NOCOMMON __current_num_memory_chunks_allocated;

/****************************************************************************/

#if defined(__MEM_DEBUG) && defined(__USE_MEM_TREES)
struct MemoryTree NOCOMMON __memory_tree;
#endif /* __MEM_DEBUG && __USE_MEM_TREES */

/****************************************************************************/

APTR			NOCOMMON __memory_pool;
struct MinList	NOCOMMON __memory_list;

/****************************************************************************/

/* Check if the sum of two unsigned 32-bit integers will be larger than what
 * an unsigned 32-bit integer can hold and return the overflow. This
 * algorithm comes from Henry S. Warren's book "Hacker's delight".
 */
int
__addition_overflows(ULONG x, ULONG y)
{
	ULONG z;

	assert( sizeof(x) == 4 );
	assert( sizeof(y) == 4 );

	z = (x & y) | ((x | y) & ~(x + y));

	return ((LONG)z) < 0;
}

/****************************************************************************/

void *
__allocate_memory(
	size_t			size,
	BOOL			never_free,
	const char *	debug_file_name UNUSED,
	int				debug_line_number UNUSED)
{
	struct MemoryNode * mn UNUSED;
	size_t allocation_size;
	void * result = NULL;
	size_t original_size UNUSED;

	#if defined(UNIX_PATH_SEMANTICS)
	{
		original_size = size;

		/* The libunix.a flavour of malloc() accepts zero-length
		   memory allocations and quietly turns these into
		   pointer-sized allocations. */
		if (size == 0)
			size = sizeof(BYTE *);
	}
	#endif /* UNIX_PATH_SEMANTICS */

	__memory_lock();

	/* Zero length allocations are by default rejected. */
	if (size == 0)
	{
		__set_errno(EINVAL);
		goto out;
	}

	if (__free_memory_threshold > 0 && AvailMem(MEMF_ANY|MEMF_LARGEST) < __free_memory_threshold)
	{
		SHOWMSG("not enough free memory available to safely proceed with allocation");
		goto out;
	}

	#ifdef __MEM_DEBUG
	{
		assert( MALLOC_HEAD_SIZE > 0 && (MALLOC_HEAD_SIZE % 4) == 0 );
		assert( MALLOC_TAIL_SIZE > 0 && (MALLOC_TAIL_SIZE % 4) == 0 );
		assert( (sizeof(*mn) % 4) == 0 );

		if (__addition_overflows(sizeof(*mn) + MALLOC_HEAD_SIZE + MALLOC_TAIL_SIZE, size))
		{
			SHOWMSG("integer overflow")

			__set_errno(ENOMEM);
			goto out;
		}

		allocation_size = sizeof(*mn) + MALLOC_HEAD_SIZE + size + MALLOC_TAIL_SIZE;
	}
	#else
	{
		if (__addition_overflows(sizeof(*mn), size))
		{
			SHOWMSG("integer overflow");

			__set_errno(ENOMEM);
			goto out;
		}

		allocation_size = sizeof(*mn) + size;
	}
	#endif /* __MEM_DEBUG */

	/* Round up allocation to a multiple of 8 bytes. */
	if ((allocation_size % MEM_BLOCKSIZE) > 0)
	{
		size_t padding;

		padding = MEM_BLOCKSIZE - (allocation_size % MEM_BLOCKSIZE);

		if (__addition_overflows(padding, allocation_size))
		{
			SHOWMSG("integer overflow");

			__set_errno(ENOMEM);
			goto out;
		}

		allocation_size += padding;
	}

	#if defined(__USE_SLAB_ALLOCATOR)
	{
		/* Are we using the slab allocator? */
		if (__slab_data.sd_InUse)
		{
			mn = __slab_allocate(allocation_size);
		}
		/* Are we using the memory pool? */
		else if (__memory_pool != NULL)
		{
			PROFILE_OFF();
			mn = AllocPooled(__memory_pool, allocation_size);
			PROFILE_ON();
		}
		/* Then we'll have to do it the hard way... */
		else
		{
			PROFILE_OFF();
			mn = AllocMem(allocation_size, MEMF_ANY);
			PROFILE_ON();
		}
	}
	#else
	{
		if (__memory_pool != NULL)
		{
			PROFILE_OFF();
			mn = AllocPooled(__memory_pool, allocation_size);
			PROFILE_ON();
		}
		else
		{
			PROFILE_OFF();
			mn = AllocMem(allocation_size, MEMF_ANY);
			PROFILE_ON();
		}
	}
	#endif /* __USE_SLAB_ALLOCATOR */

	if (mn == NULL)
	{
		SHOWMSG("not enough memory");
		goto out;
	}

	mn->mn_AllocationSize = allocation_size;
	mn->mn_Flags = never_free ? MNF_NEVER_FREE : 0;

	__current_memory_allocated += allocation_size;
	if (__maximum_memory_allocated < __current_memory_allocated)
		__maximum_memory_allocated = __current_memory_allocated;

	__current_num_memory_chunks_allocated++;
	if (__maximum_num_memory_chunks_allocated < __current_num_memory_chunks_allocated)
		__maximum_num_memory_chunks_allocated = __current_num_memory_chunks_allocated;

	#ifdef __MEM_DEBUG
	{
		BYTE * head = (BYTE *)&mn[1];
		BYTE * body = &head[MALLOC_HEAD_SIZE];
		BYTE * tail = &body[size];

		mn->mn_AlreadyFree		= FALSE;
		mn->mn_Allocation		= body;
		mn->mn_OriginalSize		= size;
		mn->mn_File				= (char *)debug_file_name;
		mn->mn_Line				= debug_line_number;
		mn->mn_FreeFile			= NULL;
		mn->mn_FreeLine			= 0;

		memset(head, MALLOC_HEAD_FILL, MALLOC_HEAD_SIZE);
		memset(body, MALLOC_NEW_FILL, size);
		memset(tail, MALLOC_TAIL_FILL, MALLOC_TAIL_SIZE);

		#ifdef __MEM_DEBUG_LOG
		{
			kprintf("[%s] + %10ld 0x%08lx [", __program_name, size, body);

			kprintf("allocated at %s:%ld]\n", debug_file_name, debug_line_number);
		}
		#endif /* __MEM_DEBUG_LOG */

		AddTail((struct List *)&__memory_list,(struct Node *)mn);

		#ifdef __USE_MEM_TREES
		{
			__red_black_tree_insert(&__memory_tree, mn);
		}
		#endif /* __USE_MEM_TREES */

		result = mn->mn_Allocation;
	}
	#else
	{
		#if defined(__USE_SLAB_ALLOCATOR)
		{
			/* If we are using neither the slab allocator nor
			 * the memory pool, then the allocation will have
			 * to be freed later, the hard way.
			 */
			if (__slab_data.sd_InUse == FALSE && __memory_pool == NULL)
				AddTail((struct List *)&__memory_list, (struct Node *)mn);
		}
		#else
		{
			if (__memory_pool == NULL)
				AddTail((struct List *)&__memory_list, (struct Node *)mn);
		}
		#endif /* __USE_SLAB_ALLOCATOR */

		result = &mn[1];
	}
	#endif /* __MEM_DEBUG */

	#if defined(UNIX_PATH_SEMANTICS)
	{
		/* Set the zero length allocation contents to NULL. */
		if (original_size == 0 && size >= sizeof(char *))
			*(char **)result = NULL;
	}
	#endif /* UNIX_PATH_SEMANTICS */

	assert( (((ULONG)result) & MEM_BLOCKMASK) == 0 );

 out:

	#ifdef __MEM_DEBUG_LOG
	{
		if (result == NULL)
		{
			kprintf("[%s] + %10ld 0x%08lx [", __program_name, size, NULL);

			kprintf("FAILED: allocated at %s:%ld]\n", debug_file_name, debug_line_number);
		}
	}
	#endif /* __MEM_DEBUG_LOG */

	__memory_unlock();

	return result;
}

/****************************************************************************/

__static void *
__malloc(size_t size, const char * file, int line)
{
	void * result = NULL;

	__memory_lock();

	/* Try to get rid of now unused memory. */
	if (__alloca_cleanup != NULL)
		(*__alloca_cleanup)(file, line);

	__memory_unlock();

	#ifdef __MEM_DEBUG
	{
		/*if ((rand() % 16) == 0)
			__check_memory_allocations(file,line);*/
	}
	#endif /* __MEM_DEBUG */

	/* Allocate memory which can be put through realloc() and free(). */
	result = __allocate_memory(size, FALSE, file, line);

	return result;
}

/****************************************************************************/

void *
malloc(size_t size)
{
	void * result;

	result = __malloc(size, NULL, 0);

	return(result);
}

/****************************************************************************/

#if defined(__THREAD_SAFE)

/****************************************************************************/

static struct SignalSemaphore * memory_semaphore;

/****************************************************************************/

void
__memory_lock(void)
{
	PROFILE_OFF();

	if (memory_semaphore != NULL)
		ObtainSemaphore(memory_semaphore);

	PROFILE_ON();
}

/****************************************************************************/

void
__memory_unlock(void)
{
	PROFILE_OFF();

	if (memory_semaphore != NULL)
		ReleaseSemaphore(memory_semaphore);

	PROFILE_ON();
}

/****************************************************************************/

#endif /* __THREAD_SAFE */

/****************************************************************************/

STDLIB_DESTRUCTOR(stdlib_memory_exit)
{
	ENTER();

	/* Make sure that freeing any memory does not also
	 * trigger the alloca cleanup operations. Otherwise,
	 * the the data structures used by alloca() to track
	 * the scope in which allocated memory remains
	 * valid and should not be freed just yet may be
	 * freed, corrupting them.
	 */
	__alloca_cleanup = NULL;

	#ifdef __MEM_DEBUG
	{
		kprintf("[%s] %ld bytes still allocated upon exit, maximum of %ld bytes allocated at a time.\n",
			__program_name, __current_memory_allocated, __maximum_memory_allocated);

		kprintf("[%s] %ld chunks of memory still allocated upon exit, maximum of %ld chunks allocated at a time.\n",
			__program_name, __current_num_memory_chunks_allocated, __maximum_num_memory_chunks_allocated);

		__check_memory_allocations(__FILE__, __LINE__);

		/* Make sure that those memory nodes which were
		 * intended not to be freed will get freed this
		 * time around.
		 */
		__never_free = FALSE;

		#if defined(__USE_MEM_TREES)
		{
			/* This must remain empty. */
			__initialize_red_black_tree(&__memory_tree);
		}
		#endif /* __USE_MEM_TREES */
	}
	#endif /* __MEM_DEBUG */

	#if defined(__USE_SLAB_ALLOCATOR)
	{
		/* Is the slab memory allocator enabled? */
		if (__slab_data.sd_InUse)
		{
			/* Just release the memory. */
			__slab_exit();
		}
		/* Is the memory pool in use? */
		else if (__memory_pool != NULL)
		{
			/* Just release the memory. */
			DeletePool(__memory_pool);
			__memory_pool = NULL;
		}
		/* Do we have to release every single allocation? */
		else if (__memory_list.mlh_Head != NULL)
		{
			#ifdef __MEM_DEBUG
			{
				while (NOT IsMinListEmpty(&__memory_list))
					__free_memory_node((struct MemoryNode *)__memory_list.mlh_Head, __FILE__, __LINE__);
			}
			#else
			{
				while (NOT IsMinListEmpty(&__memory_list))
					__free_memory_node((struct MemoryNode *)__memory_list.mlh_Head, NULL, 0);
			}
			#endif /* __MEM_DEBUG */
		}
	}
	#else
	{
		if (__memory_pool != NULL)
		{
			DeletePool(__memory_pool);
			__memory_pool = NULL;
		}
		else if (__memory_list.mlh_Head != NULL)
		{
			#ifdef __MEM_DEBUG
			{
				while (NOT IsMinListEmpty(&__memory_list))
					__free_memory_node((struct MemoryNode *)__memory_list.mlh_Head, __FILE__, __LINE__);
			}
			#else
			{
				while (NOT IsMinListEmpty(&__memory_list))
					__free_memory_node((struct MemoryNode *)__memory_list.mlh_Head, NULL, 0);
			}
			#endif /* __MEM_DEBUG */
		}
	}
	#endif /* __USE_SLAB_ALLOCATOR */

	/* The list of memory allocations must remain empty. */
	NewList((struct List *)&__memory_list);

	#if defined(__THREAD_SAFE)
	{
		__delete_semaphore(memory_semaphore);
		memory_semaphore = NULL;
	}
	#endif /* __THREAD_SAFE */

	LEAVE();
}

/****************************************************************************/

STDLIB_CONSTRUCTOR(stdlib_memory_init)
{
	BOOL success = FALSE;

	ENTER();

	NewList((struct List *)&__memory_list);

	#if defined(__USE_MEM_TREES) && defined(__MEM_DEBUG)
	{
		__initialize_red_black_tree(&__memory_tree);
	}
	#endif /* __USE_MEM_TREES && __MEM_DEBUG */

	#if defined(__THREAD_SAFE)
	{
		memory_semaphore = __create_semaphore();
		if (memory_semaphore == NULL)
			goto out;
	}
	#endif /* __THREAD_SAFE */

	#if defined(__USE_SLAB_ALLOCATOR)
	{
		/* ZZZ this is just for the purpose of testing */
		#if DEBUG
		{
			TEXT slab_size_var[20];

			if (GetVar("SLAB_SIZE", slab_size_var, sizeof(slab_size_var), 0) > 0)
			{
				LONG value;

				if (StrToLong(slab_size_var, &value) > 0 && value > 0)
					__slab_max_size = (size_t)value;
			}
		}
		#endif

		/* Enable the slab memory allocator? */
		if (__slab_max_size > 0)
		{
			__slab_init(__slab_max_size);
		}
		else
		{
			#if defined(__amigaos4__)
			{
				__memory_pool = CreatePool(MEMF_PRIVATE, (ULONG)__default_pool_size, (ULONG)__default_puddle_size);
				if (__memory_pool == NULL)
					goto out;
			}
			#else
			{
				/* There is no support for memory pools in the operating system
				 * prior to Kickstart 3.0 (V39).
				 */
				if (((struct Library *)SysBase)->lib_Version >= 39)
				{
					__memory_pool = CreatePool(MEMF_ANY, (ULONG)__default_pool_size, (ULONG)__default_puddle_size);
					if (__memory_pool == NULL)
						goto out;
				}
			}
			#endif /* __amigaos4__ */
		}
	}
	#else
	{
		#if defined(__amigaos4__)
		{
			__memory_pool = CreatePool(MEMF_PRIVATE, (ULONG)__default_pool_size, (ULONG)__default_puddle_size);
			if (__memory_pool == NULL)
				goto out;
		}
		#else
		{
			/* There is no support for memory pools in the operating system
			 * prior to Kickstart 3.0 (V39).
			 */
			if (((struct Library *)SysBase)->lib_Version >= 39)
			{
				__memory_pool = CreatePool(MEMF_ANY, (ULONG)__default_pool_size, (ULONG)__default_puddle_size);
				if (__memory_pool == NULL)
					goto out;
			}
		}
		#endif /* __amigaos4__ */
	}
	#endif /* __USE_SLAB_ALLOCATOR) */

	success = TRUE;

 out:

	SHOWVALUE(success);
	LEAVE();

	if (success)
		CONSTRUCTOR_SUCCEED();
	else
		CONSTRUCTOR_FAIL();
}
