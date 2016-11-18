/*
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

struct SlabData NOCOMMON __slab_data;

/****************************************************************************/

void *
__slab_allocate(size_t allocation_size)
{
	void * allocation = NULL;

	D(("allocating %lu bytes of memory",allocation_size));

	assert( __slab_data.sd_MaxSlabSize > 0 );

	/* Number of bytes to allocate exceeds the slab size?
	 * If so, allocate this memory chunk separately and
	 * keep track of it.
	 */
	if(allocation_size > __slab_data.sd_MaxSlabSize)
	{
		struct MinNode * single_allocation;

		D(("allocation size is > %ld; this will be stored separately",__slab_data.sd_MaxSlabSize));
		D(("allocating %ld (MinNode) + %ld = %ld bytes",sizeof(*single_allocation),allocation_size,sizeof(*single_allocation) + allocation_size));

		#if defined(__amigaos4__)
		{
			single_allocation = AllocVec(sizeof(*single_allocation) + allocation_size,MEMF_PRIVATE);
		}
		#else
		{
			single_allocation = AllocVec(sizeof(*single_allocation) + allocation_size,MEMF_ANY);
		}
		#endif /* __amigaos4__ */

		if(single_allocation != NULL)
		{
			D(("single allocation = 0x%08lx",allocation));

			AddTail((struct List *)&__slab_data.sd_SingleAllocations,(struct Node *)single_allocation);

			__slab_data.sd_NumSingleAllocations++;

			allocation = &single_allocation[1];

			D(("single allocation succeeded at 0x%08lx (number of single allocations = %lu)", allocation, __slab_data.sd_NumSingleAllocations));
		}
		else
		{
			D(("single allocation failed"));
		}
	}
	/* Otherwise allocate a chunk from a slab. */
	else
	{
		struct MinList * slab_list = NULL;
		ULONG entry_size;
		ULONG chunk_size;
		int slab_index;

		D(("allocation size is <= %ld; this will be allocated from a slab",__slab_data.sd_MaxSlabSize));

		/* Chunks must be at least as small as a MinNode, because
		 * that's what we use for keeping track of the chunks which
		 * are available for allocation within each slab.
		 */
		entry_size = allocation_size;
		if(entry_size < sizeof(struct MinNode))
			entry_size = sizeof(struct MinNode);

		/* Find a slab which keeps track of chunks that are no
		 * larger than the amount of memory which needs to be
		 * allocated. We end up picking the smallest chunk
		 * size that still works.
		 */
		for(slab_index = 2, chunk_size = (1UL << slab_index) ;
		    slab_index < 31 ;
		    slab_index++, chunk_size += chunk_size)
		{
			if(entry_size <= chunk_size)
			{
				D(("using slab #%ld (%lu bytes per chunk)", slab_index, chunk_size));

				assert( (chunk_size % sizeof(LONG)) == 0 );

				slab_list = &__slab_data.sd_Slabs[slab_index];
				break;
			}
		}

		if(slab_list != NULL)
		{
			struct SlabNode * sn;

			SHOWVALUE(chunk_size);

			/* Find the first slab which has a free chunk and use it. */
			for(sn = (struct SlabNode *)slab_list->mlh_Head ;
			    sn->sn_MinNode.mln_Succ != NULL ;
			    sn = (struct SlabNode *)sn->sn_MinNode.mln_Succ)
			{
				D(("slab = 0x%08lx, chunk size = %ld", sn, sn->sn_ChunkSize));

				assert( sn->sn_ChunkSize == chunk_size );

				allocation = (struct MemoryNode *)RemHead((struct List *)&sn->sn_FreeList);
				if(allocation != NULL)
				{
					D(("allocation succeeded at 0x%08lx in slab 0x%08lx (slab use count = %lu)",allocation,sn,sn->sn_UseCount));

					/* Was this slab empty before we began using it again? */
					if(sn->sn_UseCount == 0)
					{
						D(("slab is no longer empty"));

						/* Mark it as no longer empty. */
						Remove((struct Node *)&sn->sn_EmptyLink);
						sn->sn_EmptyDecay = 0;
					}

					sn->sn_UseCount++;

					/* Is this slab now fully utilized? Move it to the
					 * end of the queue so that it will not be checked
					 * before other slabs of the same size have been
					 * tested. Those at the front of the queue should
					 * still have room left.
					 */
					if(sn->sn_UseCount == sn->sn_Count && sn != (struct SlabNode *)slab_list->mlh_TailPred)
					{
						D(("slab is full"));

						Remove((struct Node *)sn);
						AddTail((struct List *)slab_list, (struct Node *)sn);
					}

					break;
				}
			}

			/* There is no slab with a free chunk? Then we might have to
			 * allocate a new one.
			 */
			if(allocation == NULL)
			{
				struct MinNode * free_node;
				struct MinNode * free_node_next;
				struct SlabNode * new_sn = NULL;
				BOOL purge = FALSE;

				D(("no slab is available which still has free room"));

				/* Try to recycle an empty (unused) slab, if possible. */
				for(free_node = (struct MinNode *)__slab_data.sd_EmptySlabs.mlh_Head ; 
				    free_node->mln_Succ != NULL ;
				    free_node = free_node_next)
				{
					free_node_next = (struct MinNode *)free_node->mln_Succ;

					/* free_node points to SlabNode.sn_EmptyLink, which
					 * directly follows the SlabNode.sn_MinNode.
					 */
					sn = (struct SlabNode *)&free_node[-1];

					/* Is this empty slab ready to be reused? */
					if(sn->sn_EmptyDecay == 0)
					{
						/* Unlink from list of empty slabs. */
						Remove((struct Node *)free_node);

						/* Unlink from list of slabs which keep chunks
						 * of the same size.
						 */
						Remove((struct Node *)sn);

						D(("reusing a slab"));

						new_sn = sn;
						break;
					}
				}

				/* We couldn't reuse an empty slab? Then we'll have to allocate
				 * memory for another one.
				 */
				if(new_sn == NULL)
				{
					D(("no slab is available for reuse; allocating a new slab (%lu bytes)",sizeof(*sn) + __slab_data.sd_MaxSlabSize));

					#if defined(__amigaos4__)
					{
						new_sn = (struct SlabNode *)AllocVec(sizeof(*sn) + __slab_data.sd_MaxSlabSize,MEMF_PRIVATE);
					}
					#else
					{
						new_sn = (struct SlabNode *)AllocVec(sizeof(*sn) + __slab_data.sd_MaxSlabSize,MEMF_ANY);
					}
					#endif /* __amigaos4__ */

					if(new_sn == NULL)
						D(("slab allocation failed"));

					purge = TRUE;
				}

				if(new_sn != NULL)
				{
					struct MinNode * free_chunk;
					ULONG num_free_chunks = 0;
					BYTE * first_byte;
					BYTE * last_byte;

					D(("setting up slab 0x%08lx", new_sn));

					assert( chunk_size <= __slab_data.sd_MaxSlabSize );

					memset(new_sn,0,sizeof(*new_sn));

					NewList((struct List *)&new_sn->sn_FreeList);

					/* Split up the slab memory into individual chunks
					 * of the same size and keep track of them
					 * in the free list. The memory managed by
					 * this slab immediately follows the
					 * SlabNode header.
					 */
					first_byte	= (BYTE *)&new_sn[1];
					last_byte	= &first_byte[__slab_data.sd_MaxSlabSize - chunk_size];

					for(free_chunk = (struct MinNode *)first_byte ;
					    free_chunk <= (struct MinNode *)last_byte;
					    free_chunk = (struct MinNode *)(((BYTE *)free_chunk) + chunk_size))
					{
						AddTail((struct List *)&new_sn->sn_FreeList, (struct Node *)free_chunk);
						num_free_chunks++;
					}

					D(("slab contains %lu chunks, %lu bytes each",num_free_chunks,chunk_size));

					/* Grab the first free chunk (there has to be one). */
					allocation = (struct MemoryNode *)RemHead((struct List *)&new_sn->sn_FreeList);

					D(("allocation succeeded at 0x%08lx in slab 0x%08lx (slab use count = %lu)",allocation,new_sn,new_sn->sn_UseCount+1));

					assert( allocation != NULL );

					/* Set up the new slab and put it where it belongs. */
					new_sn->sn_EmptyDecay	= 0;
					new_sn->sn_UseCount		= 1;
					new_sn->sn_Count		= num_free_chunks;
					new_sn->sn_ChunkSize	= chunk_size;

					SHOWVALUE(new_sn->sn_ChunkSize);

					AddHead((struct List *)slab_list,(struct Node *)new_sn);
				}

				/* Mark unused slabs for purging, and purge those which
				 * are ready to be purged.
				 */
				if(purge)
				{
					D(("purging empty slabs"));

					for(free_node = (struct MinNode *)__slab_data.sd_EmptySlabs.mlh_Head ; 
					    free_node->mln_Succ != NULL ;
					    free_node = free_node_next)
					{
						free_node_next = (struct MinNode *)free_node->mln_Succ;

						/* free_node points to SlabNode.sn_EmptyLink, which
						 * directly follows the SlabNode.sn_MinNode.
						 */
						sn = (struct SlabNode *)&free_node[-1];

						/* Is this empty slab ready to be purged? */
						if(sn->sn_EmptyDecay == 0)
						{
							D(("freeing empty slab"));

							/* Unlink from list of empty slabs. */
							Remove((struct Node *)free_node);

							/* Unlink from list of slabs of the same size. */
							Remove((struct Node *)sn);

							FreeVec(sn);
						}
						/* Give it another chance. */
						else
						{
							sn->sn_EmptyDecay--;
						}
					}
				}
			}
		}
		else
		{
			D(("no matching slab found"));
		}
	}

	return(allocation);
}

/****************************************************************************/

void
__slab_free(void * address,size_t allocation_size)
{
	D(("freeing allocation at 0x%08lx, %lu bytes",address,allocation_size));

	assert( __slab_data.sd_MaxSlabSize > 0 );

	/* Number of bytes allocated exceeds the slab size?
	 * Then the chunk was allocated separately.
	 */
	if(allocation_size > __slab_data.sd_MaxSlabSize)
	{
		struct MinNode * mn = address;

		D(("allocation size is > %ld; this was stored separately",__slab_data.sd_MaxSlabSize));

		Remove((struct Node *)&mn[-1]);

		FreeVec(&mn[-1]);

		assert( __slab_data.sd_NumSingleAllocations > 0 );

		__slab_data.sd_NumSingleAllocations--;

		D(("number of single allocations = %ld", __slab_data.sd_NumSingleAllocations));
	}
	/* Otherwise the allocation should have come from a slab. */
	else
	{
		struct MinList * slab_list = NULL;
		size_t entry_size;
		ULONG chunk_size;
		int slab_index;

		D(("allocation size is <= %ld; this was allocated from a slab",__slab_data.sd_MaxSlabSize));

		/* Chunks must be at least as small as a MinNode, because
		 * that's what we use for keeping track of the chunks which
		 * are available for allocation within each slab.
		 */
		entry_size = allocation_size;
		if(entry_size < sizeof(struct MinNode))
			entry_size = sizeof(struct MinNode);

		/* Find a slab which keeps track of chunks that are no
		 * larger than the amount of memory which needs to be
		 * released. We end up picking the smallest chunk
		 * size that still works.
		 */
		for(slab_index = 2, chunk_size = (1UL << slab_index) ;
		    slab_index < 31 ;
		    slab_index++, chunk_size += chunk_size)
		{
			if(entry_size <= chunk_size)
			{
				D(("using slab #%ld (%ld bytes per chunk)", slab_index, chunk_size));

				assert( (chunk_size % sizeof(LONG)) == 0 );

				slab_list = &__slab_data.sd_Slabs[slab_index];
				break;
			}
		}

		/* Find the slab which contains the memory chunk. */
		if(slab_list != NULL)
		{
			const size_t usable_range = __slab_data.sd_MaxSlabSize - chunk_size;
			struct SlabNode * sn;
			BYTE * first_byte;
			BYTE * last_byte;
			BOOL freed = FALSE;

			assert( chunk_size <= __slab_data.sd_MaxSlabSize );

			for(sn = (struct SlabNode *)slab_list->mlh_Head ;
			    sn->sn_MinNode.mln_Succ != NULL ;
			    sn = (struct SlabNode *)sn->sn_MinNode.mln_Succ)
			{
				SHOWVALUE(sn->sn_ChunkSize);

				assert( sn->sn_ChunkSize == chunk_size );

				first_byte	= (BYTE *)&sn[1];
				last_byte	= &first_byte[usable_range];
				
				/* Is this where the chunk belongs? */
				if(first_byte <= (BYTE *)address && (BYTE *)address <= last_byte)
				{
					D(("allocation is part of slab 0x%08lx (slab use count = %ld)",sn,sn->sn_UseCount));

					AddTail((struct List *)&sn->sn_FreeList, (struct Node *)address);

					assert( sn->sn_UseCount > 0 );

					sn->sn_UseCount--;

					/* If this slab is empty, mark it as unused and
					 * allow it to be purged.
					 */
					if(sn->sn_UseCount == 0)
					{
						D(("slab is now empty"));

						AddTail((struct List *)&__slab_data.sd_EmptySlabs,(struct Node *)&sn->sn_EmptyLink);
						sn->sn_EmptyDecay = 1;
					}

					/* This slab now has room. Move it to front of the list
					 * so that searching for a free chunk will pick it
					 * first.
					 */
					if(sn != (struct SlabNode *)slab_list->mlh_Head)
					{
						D(("moving slab to the head of the list"));

						Remove((struct Node *)sn);
						AddHead((struct List *)slab_list, (struct Node *)sn);
					}

					freed = TRUE;
					break;
				}
			}

			if(!freed)
				D(("allocation at 0x%08lx could not be freed",address));
		}
		else
		{
			D(("no matching slab found"));
		}
	}
}

/****************************************************************************/

void
__slab_init(size_t slab_size)
{
	size_t size;

	SETDEBUGLEVEL(2);

	D(("slab_size = %ld",slab_size));

	/* If the maximum allocation size to be made from the slab
	 * is not already a power of 2, round it up. We do not
	 * support allocations larger than 2^31, and the maximum
	 * allocation size should be much smaller.
	 *
	 * Note that the maximum allocation size also defines the
	 * amount of memory which each slab manages.
	 */
	size = sizeof(struct MinNode);
	while(size < slab_size && (size & 0x80000000) == 0)
		size += size;

	D(("size = %lu",size));
	
	/* If the slab size looks sound, enable the slab memory allocator. */
	if((size & 0x80000000) == 0)
	{
		int i;

		D(("activating slab allocator"));

		assert( size <= slab_size );

		/* Start with an empty list of slabs for each chunk size. */
		for(i = 0 ; i < 31 ; i++)
			NewList((struct List *)&__slab_data.sd_Slabs[i]);

		NewList((struct List *)&__slab_data.sd_SingleAllocations);
		NewList((struct List *)&__slab_data.sd_EmptySlabs);

		__slab_data.sd_MaxSlabSize	= size;
		__slab_data.sd_InUse		= TRUE;
	}
}

/****************************************************************************/

void
__slab_exit(void)
{
	struct SlabNode * sn;
	struct SlabNode * sn_next;
	struct MinNode * mn;
	struct MinNode * mn_next;
	int i;

	ENTER();

	D(("freeing slabs"));

	/* Free the memory allocated for each slab. */
	for(i = 0 ; i < 31 ; i++)
	{
		if(__slab_data.sd_Slabs[i].mlh_Head->mln_Succ != NULL)
			D(("freeing slab #%ld (%lu bytes per chunk)", i, (1UL << i)));

		for(sn = (struct SlabNode *)__slab_data.sd_Slabs[i].mlh_Head ;
		    sn->sn_MinNode.mln_Succ != NULL ;
		    sn = sn_next)
		{
			sn_next = (struct SlabNode *)sn->sn_MinNode.mln_Succ;

			FreeVec(sn);
		}
	}

	if(__slab_data.sd_SingleAllocations.mlh_Head->mln_Succ != NULL)
		D(("freeing single allocations"));

	/* Free the memory allocated for each allocation which did not
	 * go into a slab.
	 */
	for(mn = __slab_data.sd_SingleAllocations.mlh_Head ;
	    mn->mln_Succ != NULL ;
	    mn = mn_next)
	{
		mn_next = mn->mln_Succ;

		FreeVec(mn);
	}

	__slab_data.sd_InUse = FALSE;

	LEAVE();
}
