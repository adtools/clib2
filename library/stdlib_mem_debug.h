/*
 * $Id: stdlib_mem_debug.h,v 1.2 2005-01-02 09:07:18 obarthel Exp $
 *
 * :ts=4
 *
 * Portable ISO 'C' (1994) runtime library for the Amiga computer
 * Copyright (c) 2002-2005 by Olaf Barthel <olsen@sourcery.han.de>
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

#ifndef _STDLIB_MEM_DEBUG_H
#define _STDLIB_MEM_DEBUG_H

/****************************************************************************/

/*
 * Uncomment this to build a library which has the memory debugging features
 * enabled.
 */
/*#define __MEM_DEBUG */

/****************************************************************************/

/*
 * Uncomment this to see reports of where and how much memory is allocated
 * or released.
 */
/*#define __MEM_DEBUG_LOG*/

/****************************************************************************/

/*
 * Uncomment this to speed up memory data maintenance operations when
 * the memory debugging mode is enabled.
 */
/*#define __USE_MEM_TREES*/

/****************************************************************************/

/*
 * Memory debugging parameters; note that the head/tail size must be
 * multiples of four, or you will break code that depends upon malloc()
 * and friends to return long word aligned data structures! Better
 * still, there are assert()s in the library which will blow your code
 * out of the water if the data returned by malloc() and realloc() is
 * not long word aligned...
 */

#define MALLOC_HEAD_SIZE 512	/* How many bytes to allocate in front of
								   each memory chunk */
#define MALLOC_TAIL_SIZE 512	/* How many bytes to allocate behind each
								   memory chunk */

/****************************************************************************/

#define MALLOC_NEW_FILL  0xA3	/* The byte value to fill newly created
								   memory allocations with */
#define MALLOC_FREE_FILL 0xA5	/* The byte value to fill freed memory
								   allocations with */
#define MALLOC_HEAD_FILL 0xA7	/* The byte value to fill the memory in
								   front of each allocation with */
#define MALLOC_TAIL_FILL 0xA9	/* The byte value to fill the memory behind
								   each allocation with */

/****************************************************************************/

#endif /* _STDLIB_MEM_DEBUG_H */
