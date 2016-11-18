/*
 * $Id: stdlib_free.c,v 1.13 2006-01-08 12:04:25 obarthel Exp $
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

#ifndef _STDLIB_HEADERS_H
#include "stdlib_headers.h"
#endif /* _STDLIB_HEADERS_H */

/****************************************************************************/

#ifndef _STDLIB_MEMORY_H
#include "stdlib_memory.h"
#endif /* _STDLIB_MEMORY_H */

/****************************************************************************/

#undef free
#undef __free

/****************************************************************************/

#ifdef __MEM_DEBUG

/****************************************************************************/

STATIC char
get_hex_char(int n)
{
	char result;

	if(0 <= n && n <= 9)
		result = n + '0';
	else
		result = n + 'A' - 10;

	return(result);
}

STATIC VOID
int_to_hex(unsigned long v,char * buffer,int min_digits)
{
	int i,j,c;

	i = 0;

	do
	{
		c = (v % 16);
		v = v / 16;

		buffer[i++] = get_hex_char(c);
	}
	while(v > 0);

	while(i < min_digits)
		buffer[i++] = '0';

	for(j = 0 ; j < i / 2 ; j++)
	{
		c					= buffer[i - 1 - j];
		buffer[i - 1 - j]	= buffer[j];
		buffer[j]			= c;
	}

	buffer[i] = '\0';
}

STATIC VOID
dump_memory(unsigned char * m,int size,int ignore)
{
	const int mod = 20;
	int position,i,c;
	char buffer[120];
	char hex[10];

	buffer[0] = '\0';

	for(i = 0 ; i < size ; i++)
	{
		position = i % mod;
		if(position == 0)
		{
			if(buffer[0] != '\0')
			{
				int len = sizeof(buffer)-1;

				while(len > 0 && buffer[len-1] == ' ')
					len--;

				buffer[len] = '\0';

				kprintf("[%s] %s\n",__program_name,buffer);
			}

			memset(buffer,' ',sizeof(buffer)-1);

			int_to_hex((unsigned long)&m[i],hex,8);

			memmove(buffer,hex,8);
			hex[9] = ':';
		}

		if(m[i] != ignore)
		{
			buffer[10 + 2 * position + 0] = get_hex_char(m[i] >> 4);
			buffer[10 + 2 * position + 1] = get_hex_char(m[i] & 15);

			c = m[i];
			if(c < ' ' || (c >= 127 && c <= 160))
				c = '.';

			buffer[10 + 2 * mod + 1 + position] = c;
		}
		else
		{
			buffer[10 + 2 * position + 0] = '_';
			buffer[10 + 2 * position + 1] = '_';
		}
	}

	if(buffer[0] != '\0')
	{
		int len = sizeof(buffer)-1;

		while(len > 0 && buffer[len-1] == ' ')
			len--;

		buffer[len] = '\0';

		kprintf("[%s] %s\n",__program_name,buffer);
	}
}

/****************************************************************************/

STATIC VOID
check_memory_node(struct MemoryNode * mn,const char * file,int line)
{
	size_t size = mn->mn_Size;
	unsigned char * head = (unsigned char *)(mn + 1);
	unsigned char * body = head + MALLOC_HEAD_SIZE;
	unsigned char * tail = body + size;
	int max_head_damage = 0;
	int max_tail_damage = 0;
	int max_body_damage = 0;
	int i;

	for(i = 1 ; i <= MALLOC_HEAD_SIZE ; i++)
	{
		if(head[MALLOC_HEAD_SIZE - i] != MALLOC_HEAD_FILL)
			max_head_damage = i;
	}

	if(max_head_damage > 0)
	{
		kprintf("[%s] ",__program_name);

		if(file != NULL)
			kprintf("%s:%ld:",file,line);

		kprintf("At least %ld bytes were damaged in front of allocation 0x%08lx..0x%08lx, size = %ld",
			max_head_damage,
			body,body + size - 1,size);

		if(mn->mn_File != NULL)
			kprintf(", allocated at %s:%ld",mn->mn_File,mn->mn_Line);

		kprintf("\n");

		dump_memory(head,MALLOC_HEAD_SIZE,MALLOC_HEAD_FILL);
	}

	for(i = 0 ; i < MALLOC_TAIL_SIZE ; i++)
	{
		if(tail[i] != MALLOC_TAIL_FILL)
			max_tail_damage = i+1;
	}

	if(max_tail_damage > 0)
	{
		kprintf("[%s] ",__program_name);

		if(file != NULL)
			kprintf("%s:%ld:",file,line);

		kprintf("At least %ld bytes were damaged behind allocation 0x%08lx..0x%08lx, size = %ld (with damage = %ld)",
			max_tail_damage,
			body,body + size - 1,
			size,size+max_tail_damage);

		if(mn->mn_File != NULL)
			kprintf(", allocated at %s:%ld",mn->mn_File,mn->mn_Line);

		kprintf("\n");

		dump_memory(tail,MALLOC_TAIL_SIZE,MALLOC_TAIL_FILL);
	}

	if(mn->mn_AlreadyFree)
	{
		for(i = 0 ; i < size ; i++)
		{
			if(body[i] != MALLOC_FREE_FILL)
				max_body_damage = i+1;
		}

		if(max_body_damage > 0)
		{
			kprintf("[%s] ",__program_name);

			if(file != NULL)
				kprintf("%s:%ld:",file,line);

			kprintf("At least %ld bytes were damaged in freed allocation 0x%08lx..0x%08lx, size = %ld",
				max_body_damage,
				body,body + size - 1,size);

			if(mn->mn_File != NULL)
				kprintf(", allocated at %s:%ld",mn->mn_File,mn->mn_Line);

			kprintf("\n");

			dump_memory(body,size,MALLOC_FREE_FILL);
		}
	}
}

/****************************************************************************/

void
__check_memory_allocations(const char * file,int line)
{
	struct MemoryNode * mn;

	__memory_lock();

	for(mn = (struct MemoryNode *)__memory_list.mlh_Head ;
	    mn->mn_MinNode.mln_Succ != NULL ;
	    mn = (struct MemoryNode *)mn->mn_MinNode.mln_Succ)
	{
		check_memory_node(mn,file,line);
	}

	__memory_unlock();
}

/****************************************************************************/

struct MemoryNode *
__find_memory_node(void * address)
{
	struct MemoryNode * result;

	assert( address != NULL );

	__memory_lock();

	#if defined(__USE_MEM_TREES)
	{
		result = __red_black_tree_find(&__memory_tree,address);
	}
	#else
	{
		struct MemoryNode * mn;

		result = NULL;

		for(mn = (struct MemoryNode *)__memory_list.mlh_Head ;
		    mn->mn_MinNode.mln_Succ != NULL ;
		    mn = (struct MemoryNode *)mn->mn_MinNode.mln_Succ)
		{
			if(address == mn->mn_Allocation)
			{
				result = mn;
				break;
			}
		}
	}
	#endif /* __USE_MEM_TREES */

	__memory_unlock();

	return(result);
}

/****************************************************************************/

#else

/****************************************************************************/

struct MemoryNode *
__find_memory_node(void * address)
{
	struct MemoryNode * result;

	assert( address != NULL );

	result = &((struct MemoryNode *)address)[-1];

	return(result);
}

/****************************************************************************/

#endif /* __MEM_DEBUG */

/****************************************************************************/

STATIC VOID
remove_and_free_memory_node(struct MemoryNode * mn)
{
	size_t allocation_size;

	assert( mn != NULL );

	__memory_lock();

	Remove((struct Node *)mn);

	#if defined(__USE_MEM_TREES) && defined(__MEM_DEBUG)
	{
		__red_black_tree_remove(&__memory_tree,mn);
	}
	#endif /* __USE_MEM_TREES && __MEM_DEBUG */

	#ifdef __MEM_DEBUG
	{
		allocation_size = sizeof(*mn) + MALLOC_HEAD_SIZE + mn->mn_Size + MALLOC_TAIL_SIZE;

		assert( allocation_size == mn->mn_AllocationSize );

		memset(mn,MALLOC_FREE_FILL,allocation_size);
	}
	#else
	{
		allocation_size = sizeof(*mn) + mn->mn_Size;
	}
	#endif /* __MEM_DEBUG */

	#if defined(__USE_SLAB_ALLOCATOR)
	{
		/* Are we using the slab allocator? */
		if (__slab_data.sd_InUse)
			__slab_free(mn,allocation_size);
		else if (__memory_pool != NULL)
			FreePooled(__memory_pool,mn,allocation_size);
		else
			FreeMem(mn,allocation_size);
	}
	#else
	{
		if (__memory_pool != NULL)
			FreePooled(__memory_pool,mn,allocation_size);
		else
			FreeMem(mn,allocation_size);
	}
	#endif /* __USE_SLAB_ALLOCATOR */

	__current_memory_allocated -= allocation_size;
	__current_num_memory_chunks_allocated--;

	__memory_unlock();
}

/****************************************************************************/

void
__free_memory_node(struct MemoryNode * mn,const char * UNUSED file,int UNUSED line)
{
	assert(mn != NULL);

	#ifdef __MEM_DEBUG
	{
		size_t size = mn->mn_Size;

		check_memory_node(mn,file,line);

		if(NOT mn->mn_AlreadyFree)
		{
			#ifdef __MEM_DEBUG_LOG
			{
				kprintf("[%s] - %10ld 0x%08lx [",__program_name,mn->mn_Size,mn->mn_Allocation);

				if(mn->mn_File != NULL)
					kprintf("allocated at %s:%ld, ",mn->mn_File,mn->mn_Line);

				kprintf("freed at %s:%ld]\n",file,line);
			}
			#endif /* __MEM_DEBUG_LOG */

			if(__never_free)
			{
				mn->mn_AlreadyFree = TRUE;

				mn->mn_FreeFile	= (char *)file;
				mn->mn_FreeLine = line;

				memset(mn->mn_Allocation,MALLOC_FREE_FILL,size);
			}
			else
			{
				remove_and_free_memory_node(mn);
			}
		}
		else
		{
			#ifdef __MEM_DEBUG_LOG
			{
				kprintf("[%s] - %10ld 0x%08lx [",__program_name,mn->mn_Size,mn->mn_Allocation);

				kprintf("FAILED]\n");
			}
			#endif /* __MEM_DEBUG_LOG */

			kprintf("[%s] %s:%ld:Allocation at address 0x%08lx, size %ld",
				__program_name,file,line,mn->mn_Allocation,mn->mn_Size);

			if(mn->mn_File != NULL)
				kprintf(", allocated at %s:%ld",mn->mn_File,mn->mn_Line);

			kprintf(", has already been freed at %s:%ld.\n",mn->mn_FreeFile,mn->mn_FreeLine);
		}
	}
	#else
	{
		remove_and_free_memory_node(mn);
	}
	#endif /* __MEM_DEBUG */
}

/****************************************************************************/

VOID
__free_memory(void * ptr,BOOL force,const char * file,int line)
{
	struct MemoryNode * mn;

	assert(ptr != NULL);

	#ifdef __MEM_DEBUG
	{
		/*if((rand() % 16) == 0)
			__check_memory_allocations(file,line);*/
	}
	#endif /* __MEM_DEBUG */

	mn = __find_memory_node(ptr);

	#ifdef __MEM_DEBUG
	{
		if(mn != NULL)
		{
			if(force || (NOT mn->mn_NeverFree))
				__free_memory_node(mn,file,line);
		}
		else
		{
			#ifdef __MEM_DEBUG_LOG
			{
				kprintf("[%s] - %10ld 0x%08lx [",__program_name,0,ptr);

				kprintf("FAILED]\n");
			}
			#endif /* __MEM_DEBUG_LOG */

			kprintf("[%s] %s:%ld:Address for free(0x%08lx) not known.\n",__program_name,file,line,ptr);

			D(("memory allocation at 0x%08lx could not be freed",ptr));
		}
	}
	#else
	{
		assert( mn != NULL );

		if(mn != NULL && (force || (NOT mn->mn_NeverFree)))
			__free_memory_node(mn,file,line);
	}
	#endif /* __MEM_DEBUG */
}

/****************************************************************************/

__static void
__free(void * ptr,const char * file,int line)
{
	__memory_lock();

	/* Try to get rid of now unused memory. */
	if(__alloca_cleanup != NULL)
		(*__alloca_cleanup)(file,line);

	__memory_unlock();

	if(ptr != NULL)
		__free_memory(ptr,FALSE,file,line);
}

/****************************************************************************/

void
free(void * ptr)
{
	__free(ptr,NULL,0);
}
