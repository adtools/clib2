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

#ifndef _STDLIB_HEADERS_H
#include "stdlib_headers.h"
#endif /* _STDLIB_HEADERS_H */

/****************************************************************************/

#ifndef _STDLIB_MEMORY_H
#include "stdlib_memory.h"
#endif /* _STDLIB_MEMORY_H */

/****************************************************************************/

void
__get_slab_usage(__slab_usage_callback callback)
{
	if(__slab_data.sd_InUse)
	{
		struct __slab_usage_information sui;
		const struct SlabNode * sn;
		BOOL stop;
		int i;

		memset(&sui,0,sizeof(sui));

		__memory_lock();

		sui.sui_slab_size						= __slab_data.sd_StandardSlabSize;
		sui.sui_num_single_allocations			= __slab_data.sd_NumSingleAllocations;
		sui.sui_total_single_allocation_size	= __slab_data.sd_TotalSingleAllocationSize;

		for(i = 0 ; i < (int)NUM_ENTRIES(__slab_data.sd_Slabs) ; i++)
		{
			for(sn = (struct SlabNode *)__slab_data.sd_Slabs[i].mlh_Head ;
			    sn->sn_MinNode.mln_Succ != NULL ;
			    sn = (struct SlabNode *)sn->sn_MinNode.mln_Succ)
			{
				if (sn->sn_UseCount == 0)
					sui.sui_num_empty_slabs++;
				else if (sn->sn_Count == sn->sn_UseCount)
					sui.sui_num_full_slabs++;

				sui.sui_num_slabs++;

				sui.sui_total_slab_allocation_size += sizeof(*sn) + __slab_data.sd_StandardSlabSize;
			}
		}

		if(sui.sui_num_slabs > 0)
		{
			for(i = 0, stop = FALSE ; NOT stop && i < (int)NUM_ENTRIES(__slab_data.sd_Slabs) ; i++)
			{
				for(sn = (struct SlabNode *)__slab_data.sd_Slabs[i].mlh_Head ;
				    sn->sn_MinNode.mln_Succ != NULL ;
				    sn = (struct SlabNode *)sn->sn_MinNode.mln_Succ)
				{
					sui.sui_chunk_size		= sn->sn_ChunkSize;
					sui.sui_num_chunks		= sn->sn_Count;
					sui.sui_num_chunks_used	= sn->sn_UseCount;
					sui.sui_num_reused		= sn->sn_NumReused;

					sui.sui_slab_index++;

					if((*callback)(&sui) != 0)
					{
						stop = TRUE;
						break;
					}
				}
			}
		}
		else
		{
			(*callback)(&sui);
		}

		__memory_unlock();
	}
}
