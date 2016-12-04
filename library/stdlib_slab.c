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

struct SlabChunk
{
	struct SlabNode * sc_Parent;
};

/****************************************************************************/

void *
__slab_allocate(size_t allocation_size)
{
	struct SlabChunk * chunk;
	void * allocation = NULL;
	size_t allocation_size_with_chunk_header;

	D(("allocating %lu bytes of memory",allocation_size));

	assert( __slab_data.sd_StandardSlabSize > 0 );

	/* Check for integer overflow. */
	allocation_size_with_chunk_header = sizeof(*chunk) + allocation_size;
	if(allocation_size_with_chunk_header < allocation_size)
		return(NULL);

	/* Number of bytes to allocate exceeds the slab size?
	 * If so, allocate this memory chunk separately and
	 * keep track of it.
	 */
	if(allocation_size_with_chunk_header > __slab_data.sd_StandardSlabSize)
	{
		struct SlabSingleAllocation * ssa;
		ULONG total_single_allocation_size = sizeof(*ssa) + allocation_size;

		D(("allocation size is > %ld; this will be stored separately",__slab_data.sd_StandardSlabSize));
		D(("allocating %ld (MinNode+Size) + %ld = %ld bytes",sizeof(*ssa),allocation_size,total_single_allocation_size));

		/* No integer overflow? */
		if(allocation_size < total_single_allocation_size)
		{
			PROFILE_OFF();

			#if defined(__amigaos4__)
			{
				ssa = AllocMem(total_single_allocation_size,MEMF_PRIVATE);
			}
			#else
			{
				ssa = AllocMem(total_single_allocation_size,MEMF_ANY);
			}
			#endif /* __amigaos4__ */

			PROFILE_ON();
		}
		/* Integer overflow has occured. */
		else
		{
			ssa = NULL;
		}

		if(ssa != NULL)
		{
			ssa->ssa_Size = total_single_allocation_size;

			allocation = &ssa[1];

			D(("single allocation = 0x%08lx",allocation));

			AddTail((struct List *)&__slab_data.sd_SingleAllocations,(struct Node *)ssa);

			__slab_data.sd_NumSingleAllocations++;
			__slab_data.sd_TotalSingleAllocationSize += total_single_allocation_size;

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
		BOOL slab_reused = FALSE;
		ULONG entry_size;
		ULONG chunk_size;
		int slab_index;

		D(("allocation size is <= %ld; this will be allocated from a slab",__slab_data.sd_StandardSlabSize));

		/* Add room for a pointer back to the slab which
		 * the chunk belongs to.
		 */
		entry_size = sizeof(*chunk) + allocation_size;

		/* Chunks must be at least as small as a MinNode, because
		 * that's what we use for keeping track of the chunks which
		 * are available for allocation within each slab.
		 */
		if(entry_size < sizeof(struct MinNode))
			entry_size = sizeof(struct MinNode);

		D(("final entry size prior to picking slab size = %ld bytes",entry_size));

		/* Find a slab which keeps track of chunks that are no
		 * larger than the amount of memory which needs to be
		 * allocated. We end up picking the smallest chunk
		 * size that still works.
		 */
		for(slab_index = 2, chunk_size = (1UL << slab_index) ;
		    slab_index < (int)NUM_ENTRIES(__slab_data.sd_Slabs) ;
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

			/* The slab list is organized in such a way that the first
			 * entry always has a free chunk ready for allocation. If
			 * there is no such free chunk, it means that no other
			 * slab nodes in this list have any free chunks.
			 */
			sn = (struct SlabNode *)slab_list->mlh_Head;

			/* Make sure that the slab list is not empty. */
			if(sn->sn_MinNode.mln_Succ != NULL)
			{
				D(("slab = 0x%08lx, chunk size = %ld", sn, sn->sn_ChunkSize));

				assert( sn->sn_ChunkSize == chunk_size );

				chunk = (struct SlabChunk *)RemHead((struct List *)&sn->sn_FreeList);
				if(chunk != NULL)
				{
					/* Keep track of this chunk's parent slab. */
					chunk->sc_Parent = sn;

					allocation = &chunk[1];

					D(("allocation succeeded at 0x%08lx in slab 0x%08lx (slab use count = %lu)",allocation,sn,sn->sn_UseCount));

					/* Was this slab empty before we began using it again? */
					if(sn->sn_UseCount == 0)
					{
						D(("slab is no longer empty"));

						/* Pull it out of the list of slabs available for reuse. */
						Remove((struct Node *)&sn->sn_EmptyLink);

						sn->sn_EmptyDecay = 0;
						sn->sn_NumReused++;
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

						/* If the chunk size of the reused slab matches
						 * exactly what we need then we won't have to
						 * completely reinitialize it again.
						 */
						if(sn->sn_ChunkSize == chunk_size)
						{
							slab_reused = TRUE;
						}
						else
						{
							/* Unlink from list of slabs which keep chunks
							 * of the same size. It will be added there
							 * again, at a different position.
							 */
							Remove((struct Node *)sn);
						}

						D(("reusing a slab"));

						sn->sn_NumReused++;

						new_sn = sn;
						break;
					}
				}

				/* We couldn't reuse an empty slab? Then we'll have to allocate
				 * memory for another one.
				 */
				if(new_sn == NULL)
				{
					D(("no slab is available for reuse; allocating a new slab (%lu bytes)",sizeof(*new_sn) + __slab_data.sd_StandardSlabSize));

					PROFILE_OFF();

					#if defined(__amigaos4__)
					{
						new_sn = (struct SlabNode *)AllocVec(sizeof(*new_sn) + __slab_data.sd_StandardSlabSize,MEMF_PRIVATE);
					}
					#else
					{
						new_sn = (struct SlabNode *)AllocVec(sizeof(*new_sn) + __slab_data.sd_StandardSlabSize,MEMF_ANY);
					}
					#endif /* __amigaos4__ */

					PROFILE_ON();

					if(new_sn == NULL)
						D(("slab allocation failed"));

					/* If this allocation went well, try to free all currently unused
					 * slabs which are ready for purging. This is done so that we don't
					 * keep allocating new memory all the time without cutting back on
					 * unused slabs.
					 */
					purge = TRUE;
				}

				if(new_sn != NULL)
				{
					D(("setting up slab 0x%08lx", new_sn));

					assert( chunk_size <= __slab_data.sd_StandardSlabSize );

					/* Do we have to completely initialize this slab from scratch? */
					if(NOT slab_reused)
					{
						struct SlabChunk * free_chunk;
						ULONG num_free_chunks = 0;
						BYTE * first_byte;
						BYTE * last_byte;

						memset(new_sn,0,sizeof(*new_sn));

						NewList((struct List *)&new_sn->sn_FreeList);

						/* This slab has room for new allocations, so make sure that
						 * it goes to the front of the slab list. It will be used
						 * by the next allocation request of this size.
						 */
						AddHead((struct List *)slab_list,(struct Node *)new_sn);

						/* Split up the slab memory into individual chunks
						 * of the same size and keep track of them
						 * in the free list. The memory managed by
						 * this slab immediately follows the
						 * SlabNode header.
						 */
						first_byte	= (BYTE *)&new_sn[1];
						last_byte	= &first_byte[__slab_data.sd_StandardSlabSize - chunk_size];

						for(free_chunk = (struct SlabChunk *)first_byte ;
						    free_chunk <= (struct SlabChunk *)last_byte;
						    free_chunk = (struct SlabChunk *)(((BYTE *)free_chunk) + chunk_size))
						{
							AddTail((struct List *)&new_sn->sn_FreeList, (struct Node *)free_chunk);
							num_free_chunks++;
						}

						new_sn->sn_Count		= num_free_chunks;
						new_sn->sn_ChunkSize	= chunk_size;

						D(("new slab contains %lu chunks, %lu bytes each",num_free_chunks,chunk_size));
					}
					/* This slab was reused and need not be reinitialized from scratch. */
					else
					{
						assert( new_sn->sn_FreeList.mlh_Head != NULL );
						assert( new_sn->sn_ChunkSize == chunk_size );
						assert( new_sn->sn_Count == 0 );
					}

					/* Grab the first free chunk (there has to be one). */
					chunk = (struct SlabChunk *)RemHead((struct List *)&new_sn->sn_FreeList);

					/* Keep track of this chunk's parent slab. */
					chunk->sc_Parent = new_sn;

					assert( chunk != NULL );
					assert( chunk->sc_Parent == new_sn );

					allocation = &chunk[1];

					/* This slab is now in use. */
					new_sn->sn_UseCount = 1;

					D(("allocation succeeded at 0x%08lx in slab 0x%08lx (slab use count = %lu)",allocation,new_sn,new_sn->sn_UseCount));
				}

				/* Mark unused slabs for purging, and purge those which
				 * are ready to be purged.
				 */
				if(purge)
				{
					size_t total_purged = 0;

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

							PROFILE_OFF();
							FreeVec(sn);
							PROFILE_ON();

							total_purged += sizeof(*sn) + __slab_data.sd_StandardSlabSize;

							/* Stop releasing memory if we reach the threshold. If no
							 * threshold has been set, we will free as much memory
							 * as possible.
							 */
							if(__slab_purge_threshold > 0 && total_purged >= __slab_purge_threshold)
								break;
						}
						/* Give it another chance. */
						else
						{
							sn->sn_EmptyDecay--;

							/* Is this slab ready for reuse now? */
							if(sn->sn_EmptyDecay == 0)
							{
								/* Move it to the front of the list, so that
								 * it will be collected as soon as possible.
								 */
								if(free_node != (struct MinNode *)__slab_data.sd_EmptySlabs.mlh_Head)
								{
									Remove((struct Node *)free_node);
									AddHead((struct List *)&__slab_data.sd_EmptySlabs,(struct Node *)free_node);
								}
							}
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
	struct SlabChunk * chunk;

	D(("freeing allocation at 0x%08lx, %lu bytes",address,allocation_size));

	assert( __slab_data.sd_StandardSlabSize > 0 );

	/* Number of bytes allocated exceeds the slab size?
	 * Then the chunk was allocated separately.
	 */
	if(sizeof(*chunk) + allocation_size > __slab_data.sd_StandardSlabSize)
	{
		struct SlabSingleAllocation * ssa = address;
		ULONG size;

		D(("allocation size is > %ld; this was stored separately",__slab_data.sd_StandardSlabSize));

		assert( __slab_data.sd_NumSingleAllocations > 0 );

		/* Management information (MinNode linkage, size in bytes) precedes
		 * the address returned by malloc(), etc.
		 */
		ssa--;

		/* Verify that the allocation is really on the list we
		 * will remove it from.
		 */
		#if DEBUG
		{
			struct MinNode * mln;
			BOOL found_allocation_in_list = FALSE;

			for(mln = __slab_data.sd_SingleAllocations.mlh_Head ;
			    mln->mln_Succ != NULL ;
			    mln = mln->mln_Succ)
			{
				if(mln == (struct MinNode *)ssa)
				{
					found_allocation_in_list = TRUE;
					break;
				}
			}

			assert( found_allocation_in_list );
		}
		#endif /* DEBUG */

		size = ssa->ssa_Size;

		assert( size > 0 );
		assert( sizeof(*ssa) + allocation_size == size );
		assert( size <= __slab_data.sd_TotalSingleAllocationSize );

		Remove((struct Node *)ssa);

		PROFILE_OFF();
		FreeMem(ssa, size);
		PROFILE_ON();

		__slab_data.sd_NumSingleAllocations--;
		__slab_data.sd_TotalSingleAllocationSize -= size;

		D(("number of single allocations = %ld", __slab_data.sd_NumSingleAllocations));
	}
	/* Otherwise the allocation should have come from a slab. */
	else
	{
		struct MinList * slab_list = NULL;
		size_t entry_size;
		ULONG chunk_size;
		int slab_index;

		D(("allocation size is <= %ld; this was allocated from a slab",__slab_data.sd_StandardSlabSize));

		/* Add room for a pointer back to the slab which
		 * the chunk belongs to.
		 */
		entry_size = sizeof(*chunk) + allocation_size;

		/* Chunks must be at least as small as a MinNode, because
		 * that's what we use for keeping track of the chunks which
		 * are available for allocation within each slab.
		 */
		if(entry_size < sizeof(struct MinNode))
			entry_size = sizeof(struct MinNode);

		/* Find a slab which keeps track of chunks that are no
		 * larger than the amount of memory which needs to be
		 * released. We end up picking the smallest chunk
		 * size that still works.
		 */
		for(slab_index = 2, chunk_size = (1UL << slab_index) ;
		    slab_index < (int)NUM_ENTRIES(__slab_data.sd_Slabs) ;
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

		/* Pick the slab which contains the memory chunk. */
		if(slab_list != NULL)
		{
			struct SlabNode * sn;

			assert( chunk_size <= __slab_data.sd_StandardSlabSize );

			/* The pointer back to the slab which this chunk belongs
			 * to precedes the address which __slab_allocate()
			 * returned.
			 */
			chunk = address;
			chunk--;

			sn = chunk->sc_Parent;

			#if DEBUG
			{
				struct SlabNode * other_sn;
				BOOL slab_found = FALSE;
				BOOL chunk_found = FALSE;

				for(other_sn = (struct SlabNode *)slab_list->mlh_Head ;
				    other_sn->sn_MinNode.mln_Succ != NULL ;
				    other_sn = (struct SlabNode *)other_sn->sn_MinNode.mln_Succ)
				{
					if(other_sn == sn)
					{
						slab_found = TRUE;
						break;
					}
				}

				assert( slab_found );

				if(slab_found)
				{
					struct MinNode * free_chunk;
					BYTE * first_byte;
					BYTE * last_byte;

					first_byte	= (BYTE *)&sn[1];
					last_byte	= &first_byte[__slab_data.sd_StandardSlabSize - chunk_size];

					for(free_chunk = (struct MinNode *)first_byte ;
					    free_chunk <= (struct MinNode *)last_byte;
					    free_chunk = (struct MinNode *)(((BYTE *)free_chunk) + chunk_size))
					{
						if(free_chunk == (struct MinNode *)chunk)
						{
							chunk_found = TRUE;
							break;
						}
					}
				}

				assert( chunk_found );
			}
			#endif /* DEBUG */

			SHOWVALUE(sn->sn_ChunkSize);

			assert( sn->sn_ChunkSize != 0 );

			assert( sn->sn_ChunkSize == chunk_size );

			D(("allocation is part of slab 0x%08lx (slab use count = %ld)",sn,sn->sn_UseCount));

			#if DEBUG
			{
				struct MinNode * mln;
				BOOL chunk_already_free = FALSE;

				for(mln = sn->sn_FreeList.mlh_Head ;
				    mln->mln_Succ != NULL ;
				    mln = mln->mln_Succ)
				{
					if(mln == (struct MinNode *)chunk)
					{
						chunk_already_free = TRUE;
						break;
					}
				}

				assert( NOT chunk_already_free );
			}
			#endif /* DEBUG */

			AddHead((struct List *)&sn->sn_FreeList, (struct Node *)chunk);

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
	const size_t max_slab_size = (1UL << (NUM_ENTRIES(__slab_data.sd_Slabs)));
	size_t size;

	SETDEBUGLEVEL(2);

	D(("slab_size = %ld",slab_size));

	/* Do not allow for a slab size that is larger than
	 * what we support.
	 */
	if(slab_size > max_slab_size)
		slab_size = max_slab_size;

	/* If the maximum allocation size to be made from the slab
	 * is not already a power of 2, round it up. We do not
	 * support allocations larger than 2^17, and the maximum
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

		memset(&__slab_data,0,sizeof(__slab_data));

		assert( size <= slab_size );

		/* Start with an empty list of slabs for each chunk size. */
		for(i = 0 ; i < (int)NUM_ENTRIES(__slab_data.sd_Slabs) ; i++)
			NewList((struct List *)&__slab_data.sd_Slabs[i]);

		NewList((struct List *)&__slab_data.sd_SingleAllocations);
		NewList((struct List *)&__slab_data.sd_EmptySlabs);

		__slab_data.sd_StandardSlabSize	= size;
		__slab_data.sd_InUse			= TRUE;
	}
}

/****************************************************************************/

#if DEBUG

static int print_json(void * ignore,const char * buffer,size_t len)
{
	extern void kputs(const char * str);

	kputs(buffer);

	return(0);
}

#endif /* DEBUG */

/****************************************************************************/

void
__slab_exit(void)
{
	ENTER();

	if(__slab_data.sd_InUse)
	{
		struct SlabSingleAllocation * ssa;
		struct SlabNode * sn;
		struct SlabNode * sn_next;
		struct MinNode * mn;
		struct MinNode * mn_next;
		size_t slab_count = 0, total_slab_size = 0;
		size_t single_allocation_count = 0, total_single_allocation_size = 0;
		int i, j;

		#if DEBUG
		{
			kprintf("---BEGIN JSON DATA ---\n");

			__get_slab_stats(NULL, print_json);

			kprintf("---END JSON DATA ---\n\n");
		}
		#endif /* DEBUG */

		D(("freeing slabs"));

		/* Free the memory allocated for each slab. */
		for(i = 0 ; i < (int)NUM_ENTRIES(__slab_data.sd_Slabs) ; i++)
		{
			if(__slab_data.sd_Slabs[i].mlh_Head->mln_Succ != NULL)
				D(("freeing slab slot #%ld (%lu bytes per chunk)", i, (1UL << i)));

			for(sn = (struct SlabNode *)__slab_data.sd_Slabs[i].mlh_Head, j = 0 ;
			    sn->sn_MinNode.mln_Succ != NULL ;
			    sn = sn_next)
			{
				sn_next = (struct SlabNode *)sn->sn_MinNode.mln_Succ;

				D((" slab #%ld.%ld at 0x%08lx",i, ++j, sn));
				D(("  fragmentation = %ld%%",100 * (__slab_data.sd_StandardSlabSize - sn->sn_Count * sn->sn_ChunkSize) / __slab_data.sd_StandardSlabSize));
				D(("  total space used = %ld (%ld%%)",sn->sn_UseCount * sn->sn_ChunkSize, 100 * sn->sn_UseCount / sn->sn_Count));
				D(("  number of chunks total = %ld",sn->sn_Count));
				D(("  number of chunks used = %ld%s",sn->sn_UseCount,sn->sn_UseCount == 0 ? " (empty)" : (sn->sn_UseCount == sn->sn_Count) ? " (full)" : ""));
				D(("  how often reused = %ld",sn->sn_NumReused));

				total_slab_size += sizeof(*sn) + __slab_data.sd_StandardSlabSize;
				slab_count++;

				PROFILE_OFF();
				FreeVec(sn);
				PROFILE_ON();
			}
		}

		if(slab_count > 0)
			D(("number of slabs = %ld, total slab size = %ld bytes",slab_count, total_slab_size));

		if(__slab_data.sd_SingleAllocations.mlh_Head->mln_Succ != NULL)
			D(("freeing single allocations"));

		/* Free the memory allocated for each allocation which did not
		 * go into a slab.
		 */
		for(mn = __slab_data.sd_SingleAllocations.mlh_Head, j = 0 ;
		    mn->mln_Succ != NULL ;
		    mn = mn_next)
		{
			mn_next = mn->mln_Succ;

			ssa = (struct SlabSingleAllocation *)mn;

			D((" allocation #%ld at 0x%08lx, %lu bytes", ++j, ssa, ssa->ssa_Size));

			total_single_allocation_size += ssa->ssa_Size;
			single_allocation_count++;

			PROFILE_OFF();
			FreeMem(ssa, ssa->ssa_Size);
			PROFILE_ON();
		}

		if(single_allocation_count > 0)
			D(("number of single allocations = %ld, total single allocation size = %ld", single_allocation_count, total_single_allocation_size));

		__slab_data.sd_InUse = FALSE;
	}

	LEAVE();
}
