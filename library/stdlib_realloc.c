/*
 * $Id: stdlib_realloc.c,v 1.10 2006-02-17 10:55:03 obarthel Exp $
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

#ifndef _STDLIB_HEADERS_H
#include "stdlib_headers.h"
#endif /* _STDLIB_HEADERS_H */

/****************************************************************************/

#ifndef _STDLIB_MEMORY_H
#include "stdlib_memory.h"
#endif /* _STDLIB_MEMORY_H */

/****************************************************************************/

#undef realloc

/****************************************************************************/

__static void *
__realloc(void *ptr,size_t size,const char * file,int line)
{
	void * result = NULL;
	BOOL locked = FALSE;

	ENTER();

	SHOWPOINTER(ptr);
	SHOWVALUE(size);

	assert( (int)size >= 0 );

	if(ptr == NULL)
	{
		D(("calling malloc(%ld)",size));

		result = __malloc(size,file,line);
	}
#ifndef UNIX_PATH_SEMANTICS
	else if (size == 0)
	{
		D(("calling free(0x%08lx)",ptr));

		__free(ptr,file,line);
	}
#endif /* UNIX_PATH_SEMANTICS */
	else
	{
		struct MemoryNode * mn;
		BOOL reallocate;

		assert( ptr != NULL );

		__memory_lock();
		locked = TRUE;

		/* Try to find the allocation in the list. */
		mn = __find_memory_node(ptr);

		#ifdef __MEM_DEBUG
		{
			/* If we managed to find the memory allocation,
			   reallocate it. */
			if(mn == NULL)
			{
				SHOWMSG("allocation not found");

				kprintf("[%s] %s:%ld:Address for realloc(0x%08lx,%ld) not known.\n",__program_name,file,line,ptr,size);

				/* Apparently, the address did not qualify for
				   reallocation. */
				goto out;
			}
		}
		#else
		{
			assert( mn != NULL );
		}
		#endif /* __MEM_DEBUG */

		if(mn == NULL || mn->mn_NeverFree)
		{
			SHOWMSG("cannot free this chunk");
			goto out;
		}

		/* Don't do anything unless the size of the allocation
		   has really changed. */
		#if defined(__MEM_DEBUG)
		{
			reallocate = (mn->mn_Size != size);
		}
		#else
		{
			size_t rounded_allocation_size;

			/* Round the total allocation size to the operating system
			   granularity. */
			rounded_allocation_size = __get_allocation_size(size);

			assert( rounded_allocation_size >= size );

			if(rounded_allocation_size > mn->mn_Size)
			{
				/* Allocation size should grow. */
				reallocate = TRUE;
			}
			else
			{
				/* Optimization: If the block size shrinks by less than half the
				                 original allocation size, do not reallocate the
				                 block and do not copy over the contents of the old
				                 allocation. We also take into account that the
				                 actual size of the allocation is affected by a
				                 certain operating system imposed granularity. */
				reallocate = (rounded_allocation_size < mn->mn_Size && rounded_allocation_size <= mn->mn_Size / 2);
			}
		}
		#endif /* __MEM_DEBUG */

		if(reallocate)
		{
			void * new_ptr;

			D(("realloc() size has changed; old=%ld, new=%ld",mn->mn_Size,size));

			/* We allocate the new memory chunk before we
			   attempt to replace the old. */
			new_ptr = __malloc(size,file,line);
			if(new_ptr == NULL)
			{
				SHOWMSG("could not reallocate memory");
				goto out;
			}

			/* Copy the contents of the old allocation to the new buffer. */
			if(size > mn->mn_Size)
				size = mn->mn_Size;

			memmove(new_ptr,ptr,size);

			/* Free the old allocation. Since we already know which memory
			   node is associated with it, we don't call __free() here. */
			__free_memory_node(mn,file,line);

			result = new_ptr;
		}
		else
		{
			D(("size didn't actually change that much (%ld -> %ld); returning memory block as is.",mn->mn_Size,size));

			/* No change in size. */
			result = ptr;
		}
	}

 out:

	if(locked)
		__memory_unlock();

	if(result == NULL)
		SHOWMSG("ouch! realloc failed");

	RETURN(result);
	return(result);
}

/****************************************************************************/

void *
realloc(void *ptr,size_t size)
{
	void * result;

	result = __realloc(ptr,size,NULL,0);

	return(result);
}
