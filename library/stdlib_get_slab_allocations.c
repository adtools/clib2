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
__get_slab_allocations(__slab_allocation_callback callback)
{
	if(__slab_data.sd_InUse)
	{
		struct __slab_allocation_information sai;

		memset(&sai,0,sizeof(sai));

		__memory_lock();

		sai.sai_num_single_allocations			= __slab_data.sd_NumSingleAllocations;
		sai.sai_total_single_allocation_size	= __slab_data.sd_TotalSingleAllocationSize;

		if(__slab_data.sd_SingleAllocations.mlh_Head->mln_Succ != NULL)
		{
			const struct SlabSingleAllocation * ssa;

			for(ssa = (struct SlabSingleAllocation *)__slab_data.sd_SingleAllocations.mlh_Head ;
			    ssa->ssa_MinNode.mln_Succ != NULL ;
			    ssa = (struct SlabSingleAllocation *)ssa->ssa_MinNode.mln_Succ)
			{
				sai.sai_allocation_index++;

				sai.sai_allocation_size			= ssa->ssa_Size - sizeof(*ssa);
				sai.sai_total_allocation_size	= ssa->ssa_Size;

				if((*callback)(&sai) != 0)
					break;
			}
		}
		else
		{
			(*callback)(&sai);
		}

		__memory_unlock();
	}
}
