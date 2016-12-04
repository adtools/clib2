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

/* Free all currently unused slabs, regardless of whether they
 * are ready to be purged (SlabNode.sn_EmptyDecay == 0).
 */
void
__free_unused_slabs(void)
{
	if(__slab_data.sd_InUse)
	{
		struct MinNode * free_node;
		struct MinNode * free_node_next;
		struct SlabNode * sn;

		__memory_lock();

		for(free_node = (struct MinNode *)__slab_data.sd_EmptySlabs.mlh_Head ; 
		    free_node->mln_Succ != NULL ;
		    free_node = free_node_next)
		{
			free_node_next = (struct MinNode *)free_node->mln_Succ;

			/* free_node points to SlabNode.sn_EmptyLink, which
			 * directly follows the SlabNode.sn_MinNode.
			 */
			sn = (struct SlabNode *)&free_node[-1];

			/* Unlink from list of empty slabs. */
			Remove((struct Node *)free_node);

			/* Unlink from list of slabs of the same size. */
			Remove((struct Node *)sn);

			PROFILE_OFF();
			FreeVec(sn);
			PROFILE_ON();
		}

		__memory_unlock();
	}
}
