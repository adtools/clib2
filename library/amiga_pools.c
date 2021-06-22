/*
 * :ts=4
 *
 * Portable ISO 'C' (1994) runtime library for the Amiga computer
 * Copyright (c) 2002-2021 by Olaf Barthel <obarthel (at) gmx.net>
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

#include <exec/memory.h>
#include <exec/alerts.h>
#include <exec/lists.h>

/****************************************************************************/

#include <string.h>

/****************************************************************************/

#include <proto/exec.h>
#include <clib/alib_protos.h>

/****************************************************************************/

#include "debug.h"

/****************************************************************************/

/* This signature identifies a large allocation on the list
 * which contains both puddles and such large memory allocations.
 */
#define IS_LARGE_ALLOCATION 0

/****************************************************************************/

/* A private memory pool, as used by this implementation of the
 * pool memory management functions.
 */
struct MemoryPool
{
	struct MinList	mp_PuddleMinList;		/* Both puddles and large allocations
											 * are stored in this list. The puddles
											 * are stored near the head of the list
											 * and the puddles are stored near the
											 * tail of the list.
											 */

	ULONG			mp_MemoryFlags;			/* Memory attributes for allocation */
	ULONG			mp_PuddleSize;			/* Largest allocation which will fit
											 * into a single puddle.
											 */
	ULONG			mp_PuddleSizeThreshold;	/* Allocations which exceed this size
											 * will be allocated separately and not
											 * make use of the puddles.
											 */
};

/****************************************************************************/

/* The management data structure associated with a separate large memory
 * allocation. This is distinct from the puddles.
 */
struct LargeAllocation
{
	struct MinNode	la_MinNode;

	ULONG			la_Signature;	/* Must be set to IS_LARGE_ALLOCATION */
};

/****************************************************************************/

/* This combines a plain Node, a MemHeader and a LargeAllocation. They
 * all overlap to some degree. The "puddle" is really a MemHeader from
 * which the individual memory allocations are made.
 *
 * For reference, here is how the individual data structures look like.
 * The 'struct MinNode' is used by the 'struct LargeAllocation' and you
 * can see that the LargeAllocation.la_Signature overlaps the
 * Node.ln_Type and Node.ln_Pri fields. The 'struct MemHeader' begins
 * with a 'struct Node' and is initialized so that the Node.ln_Type and
 * Node.ln_Pri are never zero. This is why LargeAllocation.la_Signature
 * being zero is key to identifying large allocations which along with
 * the MemHeaders share the same storage list.
 *
 * struct MinNode {
 *     struct MinNode *  mln_Succ;
 *     struct MinNode *  mln_Pred;
 * };
 *
 * struct LargeAllocation {
 *     struct MinNode    la_MinNode;
 *     ULONG             la_Signature;
 * };
 *
 * struct Node {
 *     struct Node *     ln_Succ;
 *     struct Node *     ln_Pred;
 *     UBYTE             ln_Type;
 *     BYTE              ln_Pri;
 *     char *            ln_Name;
 * };
 *
 * struct MemHeader {
 *     struct Node       mh_Node;
 *     UWORD             mh_Attributes;
 *     struct MemChunk * mh_First;
 *     APTR              mh_Lower;
 *     APTR              mh_Upper;
 *     ULONG             mh_Free;
 * };
 */
union PuddleUnion
{
	struct Node				pu_Node;
	struct MemHeader		pu_MemHeader;
	struct LargeAllocation	pu_LargeAllocation;
};

/****************************************************************************/

/* Our local Kickstart 1.x compatible implementation of FreeVec(). */
static VOID free_vec(APTR allocation)
{
	if (allocation != NULL)
	{
		ULONG * mem = allocation;

		FreeMem(&mem[-1], mem[-1]);
	}
}

/****************************************************************************/

/* Our local Kickstart 1.x compatible implementation of AllocVec(). */
static APTR alloc_vec(ULONG size, ULONG flags)
{
	ULONG * mem = NULL;

	if (size > 0)
	{
		size += sizeof(*mem);

		mem = AllocMem(size, flags);
		if (mem != NULL)
			(*mem++) = size;
	}

	return mem;
}

/****************************************************************************/

VOID LibDeletePool(APTR pool)
{
	/* If possible, use the operating system implementation
	 * instead of this reimplementation.
	 */
#if ! defined(__amigaos4__)
	if (SysBase->LibNode.lib_Version >= 39)
	{
		DeletePool(pool);
		return;
	}
#endif /* ! defined(__amigaos4__) */

	if (pool != NULL)
	{
		struct MemoryPool * mp = pool;
		struct MinNode * mln_next;
		struct MinNode * mln;

		for (mln = mp->mp_PuddleMinList.mlh_Head ;
		     mln->mln_Succ != NULL ;
		     mln = mln_next)
		{
			mln_next = mln->mln_Succ;

			free_vec(mln);
		}

		FreeMem(mp, sizeof(*mp));
	}
}

/****************************************************************************/

/* Note: puddle size should not be 0 or every single memory allocation
 *       will result in an AllocVec() call.
 */
APTR LibCreatePool(
	ULONG memory_flags,
	ULONG puddle_size,
	ULONG threshold_size)
{
	struct MemoryPool * mp = NULL;
	APTR result = NULL;

	/* If possible, use the operating system implementation
	 * instead of this reimplementation.
	 */
#if ! defined(__amigaos4__)
	if (SysBase->LibNode.lib_Version >= 39)
	{
		return CreatePool(memory_flags, puddle_size, threshold_size);
	}
#endif /* ! defined(__amigaos4__) */

	/* The threshold size must be less than or equal
	 * to the puddle size.
	 */
	if (threshold_size > puddle_size)
		goto out;

	/* Round up the puddle size to the size of a
	 * memory block, as managed by Allocate().
	 */
	puddle_size = (puddle_size + MEM_BLOCKMASK) & ~MEM_BLOCKMASK;

	mp = AllocMem(sizeof(*mp), MEMF_ANY);
	if (mp == NULL)
		goto out;

	NewList((struct List *)&mp->mp_PuddleMinList);

	mp->mp_MemoryFlags			= memory_flags;
	mp->mp_PuddleSize			= puddle_size;
	mp->mp_PuddleSizeThreshold	= threshold_size;

	result = mp;
	mp = NULL;

 out:

	if (mp != NULL)
		LibDeletePool(mp);

	return result;
}

/****************************************************************************/

/* Creates a new puddle from which memory allocations up to a certain size
 * will be made. Returns the new puddle or NULL for failure.
 *
 * Note: No overflow testing is performed when allocating the puddle.
 */
static union PuddleUnion * create_new_puddle(struct MemoryPool * mp)
{
	ULONG memory_flags = mp->mp_MemoryFlags;
	ULONG puddle_size = mp->mp_PuddleSize;
	union PuddleUnion * result = NULL;
	union PuddleUnion * pu;
	struct MemHeader * mh;
	struct MemChunk * mc;

	/* The extra sizeof(APTR) is needed for aligning the
	 * allocatable memory chunks within the memory header.
	 */
	pu = alloc_vec(sizeof(pu->pu_MemHeader) + puddle_size + sizeof(APTR), memory_flags);
	if (pu == NULL)
		goto out;

	mh = &pu->pu_MemHeader;

	/* The first allocatable memory chunk follows the memory header
	 * and must be aligned to a 64 bit address. This happens to be
	 * the smallest allocatable memory unit (MEM_BLOCKSIZE == 8).
	 */
	mc = (struct MemChunk *)(((ULONG)&mh[1] + MEM_BLOCKMASK) & ~MEM_BLOCKMASK);

	mc->mc_Next		= NULL;
	mc->mc_Bytes	= puddle_size;

	/* Both ln_Type and ln_Pri must be non-zero! This allows them to
	 * be identified as small puddles as compared to the large puddles
	 * which have the la_Signature member set to IS_LARGE_ALLOCATION.
	 * The 'struct Node' which introduces the 'struct MemHeader'
	 * overlaps the 'struct LargeAllocation' in the Node.ln_Type/ln_Pri
	 * fields. This is why the test for la_Signature == IS_LARGE_ALLOCATION
	 * works.
	 */
	mh->mh_Node.ln_Type	= NT_MEMORY;
	mh->mh_Node.ln_Pri	= mh->mh_Node.ln_Type;

	mh->mh_Node.ln_Name	= (char *)&"\0Pool"[1]; /* The memory name must be an odd address. */
	mh->mh_Attributes	= memory_flags;
	mh->mh_First		= mc;
	mh->mh_Lower		= mc;
	mh->mh_Upper		= &((BYTE *)mc)[puddle_size];
	mh->mh_Free			= puddle_size;

	/* Puddles are always added at the head of the list. */
	AddHead((struct List *)&mp->mp_PuddleMinList, &pu->pu_Node);

	result = pu;

 out:

	return result;
}

/****************************************************************************/

/* Note: No overflow testing is performed when making a large allocation. */
APTR LibAllocPooled(APTR pool, ULONG mem_size)
{
	struct MemoryPool * mp;
	union PuddleUnion * pu;
	APTR result = NULL;

	/* If possible, use the operating system implementation
	 * instead of this reimplementation.
	 */
#if ! defined(__amigaos4__)
	if (SysBase->LibNode.lib_Version >= 39)
	{
		return AllocPooled(pool, mem_size);
	}
#endif /* ! defined(__amigaos4__) */

	/* No pool or no memory to allocate? */
	if (pool == NULL || mem_size == 0)
		goto out;

	mp = pool;

	/* Requested allocation size is still within the threshold limit? */
	if (mem_size <= mp->mp_PuddleSizeThreshold)
	{
		/* Search for the first puddle from which memory may be
		 * allocated. If no such puddle exists, it will have
		 * to be created. Instead of puddles we might end up
		 * finding a large memory allocation instead. This is
		 * also an indication that a new puddle will have to
		 * be created.
		 */
		pu = (union PuddleUnion *)mp->mp_PuddleMinList.mlh_Head;

		while (TRUE)
		{
			/* We just reached the end of the puddle list or this
			 * is a large allocation? We can't use an existing puddle to
			 * allocate memory, so we have to make a new one.
			 */
			if (pu->pu_Node.ln_Succ == NULL || pu->pu_LargeAllocation.la_Signature == IS_LARGE_ALLOCATION)
			{
				/* We need to create another puddle, which is added to
				 * the head of the list. Then the search for allocatable
				 * memory in the puddle list will resume there.
				 */
				pu = create_new_puddle(mp);
				if (pu == NULL)
					goto out;
			}

			/* Try to allocate memory from this puddle.*/
			result = Allocate(&pu->pu_MemHeader, mem_size);
			if (result != NULL)
			{
				/* If the allocation needs to be zeroed, clear
				 * the entire allocated memory which Allocate()
				 * returned. It's been rounded up to be a
				 * multiple of MEM_BLOCKSIZE.
				 */
				if ((mp->mp_MemoryFlags & MEMF_CLEAR) != 0)
					memset(result, 0, (mem_size + MEM_BLOCKMASK) & ~MEM_BLOCKMASK);

				/* And we're good. */
				break;
			}

			/* Try the next puddle on the list. */
			pu = (union PuddleUnion *)pu->pu_Node.ln_Succ;
		}
	}
	/* No, we need to allocate this memory chunk separately. */
	else
	{
		struct LargeAllocation * la;

		pu = alloc_vec(sizeof(pu->pu_LargeAllocation) + mem_size, mp->mp_MemoryFlags);
		if (pu == NULL)
			goto out;

		/* This identifies it as a separate large allocation. */
		pu->pu_LargeAllocation.la_Signature = IS_LARGE_ALLOCATION;

		/* The separate large allocations are stored near the end
		 * of the list. The puddles are always stored at the beginning
		 * of the list.
		 */
		AddTail((struct List *)&mp->mp_PuddleMinList, &pu->pu_Node);

		la = &pu->pu_LargeAllocation;

		result = &la[1];
	}
	
 out:

	return result;
}

/****************************************************************************/

/* Note: The size of the allocation to free must match
 *       exactly or memory will remain unfreed. Furthermore,
 *       the wrong allocation size may lead to general
 *       memory corruption.
 */
VOID LibFreePooled(APTR pool, APTR memory, ULONG size)
{
	/* If possible, use the operating system implementation
	 * instead of this reimplementation.
	 */
#if ! defined(__amigaos4__)
	if (SysBase->LibNode.lib_Version >= 39)
	{
		FreePooled(pool, memory, size);
		return;
	}
#endif /* ! defined(__amigaos4__) */

	if (pool != NULL && memory != NULL)
	{
		struct MemoryPool * mp = pool;

		/* This allocation was made from a puddle? */
		if (size <= mp->mp_PuddleSizeThreshold)
		{
			union PuddleUnion * pu_deallocation = NULL;
			union PuddleUnion * pu;

			/* Try to find the puddle which contains this
			 * allocation.
			 */
			for (pu = (union PuddleUnion *)mp->mp_PuddleMinList.mlh_Head ;
			     pu->pu_Node.ln_Succ != NULL ;
				 pu = (union PuddleUnion *)pu->pu_Node.ln_Succ)
			{
				/* Did we reach the end of the puddle list? Then
				 * the given memory address and/or size may be
				 * invalid...
				 */
				if (pu->pu_LargeAllocation.la_Signature == IS_LARGE_ALLOCATION)
					break;

				/* Is this the memory header which contains the
				 * memory allocation?
				 */
				if (pu->pu_MemHeader.mh_Lower <= memory && memory < pu->pu_MemHeader.mh_Upper)
				{
					/* Free the allocation and remember where it
					 * came from. We will make use of this below to
					 * free now unused puddles.
					 */
					Deallocate(&pu->pu_MemHeader, memory, size);

					pu_deallocation = pu;
					break;
				}
			}

			/* Did we succeed in releasing this memory allocation? */
			if (pu_deallocation != NULL)
			{
				APTR upper;

				/* The more frequently memory is freed from a
				 * specific puddle, the easier it should become
				 * to find it on the list again. We try to move it
				 * closer to the beginning of the list unless it
				 * is already at the head of the list.
				 */
				if (mp->mp_PuddleMinList.mlh_Head != (struct MinNode *)pu)
				{
					/* This puddle precedes the one from which we
					 * just released an allocation.
					 */
					struct Node * pred = pu->pu_Node.ln_Pred;
					struct Node * before;

					/* Put this puddle at the head of the list? */
					if (pred == (struct Node *)mp->mp_PuddleMinList.mlh_Head)
						before = NULL;
					/* No, insert it in front of its predecessor. */
					else
						before = pred->ln_Pred;

					Remove(&pu->pu_Node);
					Insert((struct List *)&mp->mp_PuddleMinList, &pu->pu_Node, before);
				}

				/* Has this puddle been emptied? */
				upper = &((BYTE *)pu->pu_MemHeader.mh_Lower)[pu->pu_MemHeader.mh_Free];
				if (upper == pu->pu_MemHeader.mh_Upper)
				{
					/* Then we may remove and free it now. */
					Remove(&pu->pu_Node);

					free_vec(pu);
				}
			}
			/* Something's wrong :-( */
			else
			{
				Alert(AN_BadFreeAddr);
			}
		}
		/* No, this is a large allocation which must be freed as it is. */
		else
		{
			struct LargeAllocation * la = &((struct LargeAllocation *)memory)[-1];

			Remove((struct Node *)&la->la_MinNode);

			free_vec(la);
		}
	}
}
