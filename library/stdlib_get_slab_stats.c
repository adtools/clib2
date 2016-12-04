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

#include <setjmp.h>

/****************************************************************************/

struct context
{
	jmp_buf					abort_buf;
	void *					user_data;
	__slab_status_callback	callback;
	char *					buffer;
	size_t					buffer_size;
};

/****************************************************************************/

static void print(struct context * ct, const char * format, ...)
{
	va_list args;
	int len;

	va_start(args,format);
	len = vsnprintf(ct->buffer, ct->buffer_size, format, args);
	va_end(args);

	/* This shouldn't happen: the buffer ought to be large enough
	 * to hold every single line.
	 */
	if(len >= (int)ct->buffer_size)
		len = strlen(ct->buffer);

	if((*ct->callback)(ct->user_data, ct->buffer, len) != 0)
		longjmp(ct->abort_buf,-1);
}

/****************************************************************************/

void
__get_slab_stats(void * user_data, __slab_status_callback callback)
{
	if(__slab_data.sd_InUse)
	{
		static int times_checked = 1;

		const struct SlabNode * sn;
		volatile size_t num_empty_slabs = 0;
		volatile size_t num_full_slabs = 0;
		volatile size_t num_slabs = 0;
		volatile size_t slab_allocation_size = 0;
		volatile size_t total_slab_allocation_size = 0;
		struct context ct;
		char line[1024];
		char time_buffer[40];
		time_t now;
		struct tm when;
		int i;

 		memset(&ct, 0, sizeof(ct));

		ct.user_data	= user_data;
		ct.callback		= callback;
		ct.buffer		= line;
		ct.buffer_size	= sizeof(line);

		__memory_lock();

		if(setjmp(ct.abort_buf) == 0)
		{
			now = time(NULL);
			localtime_r(&now, &when);

			strftime(time_buffer, sizeof(time_buffer), "%Y-%m-%dT%H:%M:%S", &when);

			print(&ct,"{\n");

			print(&ct,"\t\"when\": \"%s\",\n", time_buffer);
			print(&ct,"\t\"times_checked\": %d,\n", times_checked++);
			print(&ct,"\t\"slab_size\": %zu,\n", __slab_data.sd_StandardSlabSize);
			print(&ct,"\t\"num_single_allocations\": %zu,\n", __slab_data.sd_NumSingleAllocations);
			print(&ct,"\t\"total_single_allocation_size\": %zu,\n", __slab_data.sd_TotalSingleAllocationSize);

			if(__slab_data.sd_SingleAllocations.mlh_Head->mln_Succ != NULL)
			{
				const struct SlabSingleAllocation * ssa;

				print(&ct,"\t\"single_allocations\": [\n");

				for(ssa = (struct SlabSingleAllocation *)__slab_data.sd_SingleAllocations.mlh_Head ;
				    ssa->ssa_MinNode.mln_Succ != NULL ;
				    ssa = (struct SlabSingleAllocation *)ssa->ssa_MinNode.mln_Succ)
				{
					print(&ct,"\t\t{ \"size\": %lu, \"total_size\": %lu }%s\n",
						ssa->ssa_Size - sizeof(*ssa), ssa->ssa_Size,
						ssa->ssa_MinNode.mln_Succ->mln_Succ != NULL ? "," : "");
				}

				print(&ct,"\t],\n");
			}
			else
			{
				print(&ct,"\t\"single_allocations\": [],\n");
			}

			for(i = 0 ; i < (int)NUM_ENTRIES(__slab_data.sd_Slabs) ; i++)
			{
				for(sn = (struct SlabNode *)__slab_data.sd_Slabs[i].mlh_Head ;
				    sn->sn_MinNode.mln_Succ != NULL ;
				    sn = (struct SlabNode *)sn->sn_MinNode.mln_Succ)
				{
					if (sn->sn_UseCount == 0)
						num_empty_slabs++;
					else if (sn->sn_UseCount == sn->sn_Count)
						num_full_slabs++;

					num_slabs++;

					slab_allocation_size += sn->sn_ChunkSize * sn->sn_UseCount;
					total_slab_allocation_size += sizeof(*sn) + __slab_data.sd_StandardSlabSize;
				}
			}

			print(&ct,"\t\"num_slabs\": %zu,\n", num_slabs);
			print(&ct,"\t\"num_empty_slabs\": %zu,\n", num_empty_slabs);
			print(&ct,"\t\"num_full_slabs\": %zu,\n", num_full_slabs);
			print(&ct,"\t\"slab_allocation_size\": %zu,\n", slab_allocation_size);
			print(&ct,"\t\"total_slab_allocation_size\": %zu,\n", total_slab_allocation_size);

			if(num_slabs > 0)
			{
				const char * eol = "";
	
				print(&ct,"\t\"slabs\": [\n");
	
				for(i = 0 ; i < (int)NUM_ENTRIES(__slab_data.sd_Slabs) ; i++)
				{
					for(sn = (struct SlabNode *)__slab_data.sd_Slabs[i].mlh_Head ;
					    sn->sn_MinNode.mln_Succ != NULL ;
					    sn = (struct SlabNode *)sn->sn_MinNode.mln_Succ)
					{
						print(&ct,"%s\t\t{ \"size\": %lu, \"chunks\": %lu, \"chunks_in_use\": %lu, \"times_reused\": %lu, \"empty_decay\": %lu }",
							eol,
							sn->sn_ChunkSize,
							sn->sn_Count,
							sn->sn_UseCount,
							sn->sn_NumReused,
							sn->sn_EmptyDecay);
	
						eol = ",\n";
					}
				}
	
				print(&ct,"\n\t]\n");
			}
			else
			{
				print(&ct,"\t\"slabs\": []\n");
			}
	
			print(&ct,"}\n");
		}

		__memory_unlock();
	}
}
