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

void *
__allocate_memory(size_t size,BOOL never_free,const char *debug_file_name UNUSED,int debug_line_number UNUSED)
{
	struct MemoryNode * mn;
	size_t allocation_size;
	void * result = NULL;

	#if defined(UNIX_PATH_SEMANTICS)
	size_t original_size;

	{
		original_size = size;

		/* The libunix.a flavour accepts zero length memory allocations
		   and quietly turns them into a pointer sized allocations. */
		if(size == 0)
			size = sizeof(char *);
	}
	#endif /* UNIX_PATH_SEMANTICS */

	__memory_lock();

	/* Zero length allocations are by default rejected. */
	if(size == 0)
	{
		__set_errno(EINVAL);
		goto out;
	}

	if(__free_memory_threshold > 0 && AvailMem(MEMF_ANY|MEMF_LARGEST) < __free_memory_threshold)
	{
		SHOWMSG("not enough free memory available to safely proceed with allocation");
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
		/* Round up allocation to a multiple of 32 bits. */
		if((size & 3) != 0)
			size += 4 - (size & 3);

		allocation_size = sizeof(*mn) + size;
	}
	#endif /* __MEM_DEBUG */

	/* Integer overflow has occured? */
	if(size == 0 || allocation_size < size)
	{
		__set_errno(ENOMEM);
		goto out;
	}

	/* We reuse the MemoryNode.mn_Size field to mark
	 * allocations are not suitable for use with
	 * free() and realloc(). This limits allocation
	 * sizes to a little less than 2 GBytes.
	 */
	if(allocation_size & MN_SIZE_NEVERFREE)
	{
		__set_errno(ENOMEM);
		goto out;
	}

	#if defined(__USE_SLAB_ALLOCATOR)
	{
		/* Are we using the slab allocator? */
		if(__slab_data.sd_InUse)
		{
			mn = __slab_allocate(allocation_size);
		}
		else
		{
			if (__memory_pool != NULL)
			{
				PROFILE_OFF();
				mn = AllocPooled(__memory_pool,allocation_size);
				PROFILE_ON();
			}
			else
			{
				#ifdef __MEM_DEBUG
				{
					PROFILE_OFF();
					mn = AllocMem(allocation_size,MEMF_ANY);
					PROFILE_ON();
				}
				#else
				{
					struct MinNode * mln;

					PROFILE_OFF();
					mln = AllocMem(sizeof(*mln) + allocation_size,MEMF_ANY);
					PROFILE_ON();

					if(mln != NULL)
					{
						AddTail((struct List *)&__memory_list,(struct Node *)mln);

						mn = (struct MemoryNode *)&mln[1];
					}
					else
					{
						mn = NULL;
					}
				}
				#endif /* __MEM_DEBUG */
			}
		}
	}
	#else
	{
		if(__memory_pool != NULL)
		{
			PROFILE_OFF();
			mn = AllocPooled(__memory_pool,allocation_size);
			PROFILE_ON();
		}
		else
		{
			#ifdef __MEM_DEBUG
			{
				PROFILE_OFF();
				mn = AllocMem(allocation_size,MEMF_ANY);
				PROFILE_ON();
			}
			#else
			{
				struct MinNode * mln;

				PROFILE_OFF();
				mln = AllocMem(sizeof(*mln) + allocation_size,MEMF_ANY);
				PROFILE_ON();

				if(mln != NULL)
				{
					AddTail((struct List *)&__memory_list,(struct Node *)mln);

					mn = (struct MemoryNode *)&mln[1];
				}
				else
				{
					mn = NULL;
				}
			}
			#endif /* __MEM_DEBUG */
		}
	}
	#endif /* __USE_SLAB_ALLOCATOR */

	if(mn == NULL)
	{
		SHOWMSG("not enough memory");
		goto out;
	}

	mn->mn_Size = size;

	if(never_free)
		SET_FLAG(mn->mn_Size, MN_SIZE_NEVERFREE);

	__current_memory_allocated += allocation_size;
	if(__maximum_memory_allocated < __current_memory_allocated)
		__maximum_memory_allocated = __current_memory_allocated;

	__current_num_memory_chunks_allocated++;
	if(__maximum_num_memory_chunks_allocated < __current_num_memory_chunks_allocated)
		__maximum_num_memory_chunks_allocated = __current_num_memory_chunks_allocated;

	#ifdef __MEM_DEBUG
	{
		char * head = (char *)(mn + 1);
		char * body = head + MALLOC_HEAD_SIZE;
		char * tail = body + size;

		AddTail((struct List *)&__memory_list,(struct Node *)mn);

		mn->mn_AlreadyFree		= FALSE;
		mn->mn_Allocation		= body;
		mn->mn_AllocationSize	= allocation_size;
		mn->mn_File				= (char *)debug_file_name;
		mn->mn_Line				= debug_line_number;
		mn->mn_FreeFile			= NULL;
		mn->mn_FreeLine			= 0;

		memset(head,MALLOC_HEAD_FILL,MALLOC_HEAD_SIZE);
		memset(body,MALLOC_NEW_FILL,size);
		memset(tail,MALLOC_TAIL_FILL,MALLOC_TAIL_SIZE);

		#ifdef __MEM_DEBUG_LOG
		{
			kprintf("[%s] + %10ld 0x%08lx [",__program_name,size,body);

			kprintf("allocated at %s:%ld]\n",debug_file_name,debug_line_number);
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

	#if defined(UNIX_PATH_SEMANTICS)
	{
		/* Set the zero length allocation contents to NULL. */
		if(original_size == 0)
			*(char **)result = NULL;
	}
	#endif /* UNIX_PATH_SEMANTICS */

	assert( (((ULONG)result) & 3) == 0 );

 out:

	#ifdef __MEM_DEBUG_LOG
	{
		if(result == NULL)
		{
			kprintf("[%s] + %10ld 0x%08lx [",__program_name,size,NULL);

			kprintf("FAILED: allocated at %s:%ld]\n",debug_file_name,debug_line_number);
		}
	}
	#endif /* __MEM_DEBUG_LOG */

	__memory_unlock();

	return(result);
}

/****************************************************************************/

__static void *
__malloc(size_t size,const char * file,int line)
{
	void * result = NULL;

	__memory_lock();

	/* Try to get rid of now unused memory. */
	if(__alloca_cleanup != NULL)
		(*__alloca_cleanup)(file,line);

	__memory_unlock();

	#ifdef __MEM_DEBUG
	{
		/*if((rand() % 16) == 0)
			__check_memory_allocations(file,line);*/
	}
	#endif /* __MEM_DEBUG */

	/* Allocate memory which can be put through realloc() and free(). */
	result = __allocate_memory(size,FALSE,file,line);

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

#if defined(__THREAD_SAFE)

/****************************************************************************/

static struct SignalSemaphore * memory_semaphore;

/****************************************************************************/

void
__memory_lock(void)
{
	PROFILE_OFF();

	if(memory_semaphore != NULL)
		ObtainSemaphore(memory_semaphore);

	PROFILE_ON();
}

/****************************************************************************/

void
__memory_unlock(void)
{
	PROFILE_OFF();

	if(memory_semaphore != NULL)
		ReleaseSemaphore(memory_semaphore);

	PROFILE_ON();
}

/****************************************************************************/

#endif /* __THREAD_SAFE */

/****************************************************************************/

STDLIB_DESTRUCTOR(stdlib_memory_exit)
{
	ENTER();

	#ifdef __MEM_DEBUG
	{
		kprintf("[%s] %ld bytes still allocated upon exit, maximum of %ld bytes allocated at a time.\n",
			__program_name,__current_memory_allocated,__maximum_memory_allocated);

		kprintf("[%s] %ld chunks of memory still allocated upon exit, maximum of %ld chunks allocated at a time.\n",
			__program_name,__current_num_memory_chunks_allocated,__maximum_num_memory_chunks_allocated);

		__check_memory_allocations(__FILE__,__LINE__);

		__never_free = FALSE;

		if(__memory_list.mlh_Head != NULL)
		{
			while(NOT IsMinListEmpty(&__memory_list))
			{
				((struct MemoryNode *)__memory_list.mlh_Head)->mn_AlreadyFree = FALSE;

				__free_memory_node((struct MemoryNode *)__memory_list.mlh_Head,__FILE__,__LINE__);
			}
		}

		#if defined(__USE_MEM_TREES)
		{
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
			__slab_exit();
		}
		else
		{
			if (__memory_pool != NULL)
			{
				NewList((struct List *)&__memory_list);

				DeletePool(__memory_pool);
				__memory_pool = NULL;
			}
			else if (__memory_list.mlh_Head != NULL)
			{
				#ifdef __MEM_DEBUG
				{
					while(NOT IsMinListEmpty(&__memory_list))
						__free_memory_node((struct MemoryNode *)__memory_list.mlh_Head,__FILE__,__LINE__);
				}
				#else
				{
					while(NOT IsMinListEmpty(&__memory_list))
						__free_memory_node((struct MemoryNode *)__memory_list.mlh_Head,NULL,0);
				}
				#endif /* __MEM_DEBUG */
			}
		}
	}
	#else
	{
		if (__memory_pool != NULL)
		{
			NewList((struct List *)&__memory_list);

			DeletePool(__memory_pool);
			__memory_pool = NULL;
		}
		else if (__memory_list.mlh_Head != NULL)
		{
			#ifdef __MEM_DEBUG
			{
				while(NOT IsMinListEmpty(&__memory_list))
					__free_memory_node((struct MemoryNode *)__memory_list.mlh_Head,__FILE__,__LINE__);
			}
			#else
			{
				while(NOT IsMinListEmpty(&__memory_list))
					__free_memory_node((struct MemoryNode *)__memory_list.mlh_Head,NULL,0);
			}
			#endif /* __MEM_DEBUG */
		}
	}
	#endif /* __USE_SLAB_ALLOCATOR */

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

	#if defined(__THREAD_SAFE)
	{
		memory_semaphore = __create_semaphore();
		if(memory_semaphore == NULL)
			goto out;
	}
	#endif /* __THREAD_SAFE */

	#if defined(__USE_MEM_TREES) && defined(__MEM_DEBUG)
	{
		__initialize_red_black_tree(&__memory_tree);
	}
	#endif /* __USE_MEM_TREES && __MEM_DEBUG */

	NewList((struct List *)&__memory_list);

	#if defined(__USE_SLAB_ALLOCATOR)
	{
		/* ZZZ this is just for the purpose of testing */
		#if DEBUG
		{
			TEXT slab_size_var[20];

			if(GetVar("SLAB_SIZE", slab_size_var, sizeof(slab_size_var), 0) > 0)
			{
				LONG value;

				if(StrToLong(slab_size_var,&value) > 0 && value > 0)
					__slab_max_size = (size_t)value;
			}
		}
		#endif

		/* Enable the slab memory allocator? */
		if(__slab_max_size > 0)
		{
			__slab_init(__slab_max_size);
		}
		else
		{
			#if defined(__amigaos4__)
			{
				__memory_pool = CreatePool(MEMF_PRIVATE,(ULONG)__default_pool_size,(ULONG)__default_puddle_size);
			}
			#else
			{
				/* There is no support for memory pools in the operating system
				 * prior to Kickstart 3.0 (V39).
				 */
				if(((struct Library *)SysBase)->lib_Version >= 39)
					__memory_pool = CreatePool(MEMF_ANY,(ULONG)__default_pool_size,(ULONG)__default_puddle_size);
			}
			#endif /* __amigaos4__ */
		}
	}
	#else
	{
		#if defined(__amigaos4__)
		{
			__memory_pool = CreatePool(MEMF_PRIVATE,(ULONG)__default_pool_size,(ULONG)__default_puddle_size);
		}
		#else
		{
			/* There is no support for memory pools in the operating system
			 * prior to Kickstart 3.0 (V39).
			 */
			if(((struct Library *)SysBase)->lib_Version >= 39)
				__memory_pool = CreatePool(MEMF_ANY,(ULONG)__default_pool_size,(ULONG)__default_puddle_size);
		}
		#endif /* __amigaos4__ */
	}
	#endif /* __USE_SLAB_ALLOCATOR) */

	success = TRUE;

 out:

	SHOWVALUE(success);
	LEAVE();

	if(success)
		CONSTRUCTOR_SUCCEED();
	else
		CONSTRUCTOR_FAIL();
}
