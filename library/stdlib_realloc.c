/*
 * $Id: stdlib_realloc.c,v 1.10 2006-02-17 10:55:03 obarthel Exp $
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

#undef realloc

/****************************************************************************/

__static void *
__realloc(void *ptr, size_t size, const char * file, int line)
{
	void * result = NULL;
	BOOL locked = FALSE;

	ENTER();

	SHOWPOINTER(ptr);
	SHOWVALUE(size);

	assert( (int)size >= 0 );

	if (ptr == NULL)
	{
		D(("calling malloc(%ld)", size));

		result = __malloc(size, file, line);
	}
#ifndef UNIX_PATH_SEMANTICS
	else if (size == 0)
	{
		D(("calling free(0x%08lx)",ptr));

		__free(ptr, file, line);
	}
#endif /* UNIX_PATH_SEMANTICS */
	else
	{
		size_t new_allocation_size UNUSED;
		size_t old_allocation_size;
		struct MemoryNode * mn;
		BOOL reallocate;

		assert( ptr != NULL );

		__memory_lock();
		locked = TRUE;

		/* Try to find the allocation in the list. */
		mn = __find_memory_node(ptr);

		#ifdef __MEM_DEBUG
		{
			/* Quit if we failed to find the memory allocation. */
			if (mn == NULL)
			{
				SHOWMSG("allocation not found");

				kprintf("[%s] %s:%ld:Address for realloc(0x%08lx,%ld) not known.\n",
					__program_name, file, line, ptr, size);

				goto out;
			}
		}
		#else
		{
			assert( mn != NULL );
		}
		#endif /* __MEM_DEBUG */

		if (mn == NULL || FLAG_IS_SET(mn->mn_Flags, MNF_NEVER_FREE))
		{
			SHOWMSG("cannot free this chunk");
			goto out;
		}

		assert( FLAG_IS_CLEAR(mn->mn_Flags, MNF_NEVER_FREE) );

		old_allocation_size = mn->mn_AllocationSize;

		/* If the memory debug option is enabled, just check if
		 * requested allocation size has changed.
		 */
		#if defined(__MEM_DEBUG)
		{
			reallocate = (mn->mn_OriginalSize != size);

			new_allocation_size = (sizeof(*mn) + MALLOC_HEAD_SIZE + size + MALLOC_TAIL_SIZE + MEM_BLOCKMASK) & ~MEM_BLOCKMASK;
		}
		#else
		{
			/* The actual size of the allocation is affected by the
			   granularity and minimum allocation size used by the
			   operating system. */
			new_allocation_size = (sizeof(*mn) + size + MEM_BLOCKMASK) & ~MEM_BLOCKMASK;

			if (new_allocation_size > old_allocation_size)
			{
				/* Allocation size should grow. */
				reallocate = TRUE;
			}
			else if (new_allocation_size < old_allocation_size)
			{
				/* Optimization: If the block size shrinks by less than half the
				                 original allocation size, do not reallocate the
				                 block. */
				reallocate = (size <= old_allocation_size / 2);
			}
			else
			{
				reallocate = FALSE;
			}
		}
		#endif /* __MEM_DEBUG */

		if (reallocate)
		{
			void * new_ptr;

			D(("realloc() allocation size has changed; old=%ld, new=%ld", old_allocation_size, new_allocation_size));

			/* We allocate the new memory chunk before we
			   attempt to replace the old one. */
			new_ptr = __malloc(size, file, line);
			if (new_ptr == NULL)
			{
				SHOWMSG("could not reallocate memory");
				goto out;
			}

			/* With memory debugging enabled, the size of the allocation made
			 * will use the requested and not the rounded size of the
			 * allocation, which can be shorter. We need to deal with this.
			 */
			#if defined(__MEM_DEBUG)
			{
				struct MemoryNode * new_mn;

				new_mn = __find_memory_node(new_ptr);
				if (new_mn == NULL)
				{
					free(new_ptr);

					SHOWMSG("Could not find memory node for new allocation");
					goto out;
				}

				/* Make sure that if the new allocation size is smaller than
				 * the old allocation, we only copy as much data as will fit
				 * into the new allocation.
				 */
				if (size > new_mn->mn_OriginalSize)
					size = new_mn->mn_OriginalSize;
			}
			#else
			{
				/* We assume that the total size of the allocation will
				 * include padding. The requested size does not include
				 * the memory node, of course, which is why it is added
				 * here.
				 */
				if (size + sizeof(*mn) > old_allocation_size)
					size = old_allocation_size - sizeof(*mn);
			}
			#endif /* __MEM_DEBUG */

			memmove(new_ptr, ptr, size);

			/* Free the old allocation. Since we already know which memory
			   node is associated with it, we don't call __free() here. */
			__free_memory_node(mn, file, line);

			result = new_ptr;
		}
		else
		{
			D(("size didn't actually change that much (%ld -> %ld); returning memory block as is.",
				old_allocation_size, new_allocation_size));

			/* No change in size. */
			result = ptr;
		}
	}

 out:

	if (locked)
		__memory_unlock();

	if (result == NULL)
		SHOWMSG("ouch! realloc failed");

	RETURN(result);
	return result;
}

/****************************************************************************/

void *
realloc(void *ptr,size_t size)
{
	void * result;

	result = __realloc(ptr,size,NULL,0);

	return(result);
}
