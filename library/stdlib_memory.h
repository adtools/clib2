/*
 * $Id: stdlib_memory.h,v 1.2 2005-03-20 11:18:06 obarthel Exp $
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

#ifndef _STDLIB_MEMORY_H
#define _STDLIB_MEMORY_H

/****************************************************************************/

/*
 * Uncomment this to build a library which has the memory debugging features
 * enabled.
 */
/*#define __MEM_DEBUG*/

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

#ifndef	EXEC_MEMORY_H
#include <exec/memory.h>
#endif /* EXEC_MEMORY_H */

#ifndef EXEC_NODES_H
#include <exec/nodes.h>
#endif /* EXEC_NODES_H */

/****************************************************************************/

#ifndef _MACROS_H
#include "macros.h"
#endif /* _MACROS_H */

/****************************************************************************/

#include <stddef.h>

/****************************************************************************/

/* We shuffle things around a bit for the debug code. This works by joining
   related code which shares the same name. The debug code symbols also have
   to be completely separate from the "regular" code. */
#if defined(__MEM_DEBUG)

#define __static

#define __alloca_cleanup __alloca_cleanup_debug

#define __find_memory_node __find_memory_node_debug
#define __free_memory_node __free_memory_node_debug
#define __force_free __force_free_debug

#define __get_allocation_size __get_allocation_size_debug
#define __allocate_memory __allocate_memory_debug

#define __memory_pool __memory_pool_debug
#define __memory_list __memory_list_debug

#define __vasprintf_hook_entry __vasprintf_hook_entry_debug

extern void * __alloca(size_t size,const char * file,int line);
extern void * __calloc(size_t num_elements,size_t element_size,const char * file,int line);
extern void __free(void * ptr,const char * file,int line);
extern void * __malloc(size_t size,const char * file,int line);
extern void * __realloc(void *ptr,size_t size,const char * file,int line);
extern char * __strdup(const char *s,const char * file,int line);
extern char * __getcwd(char * buffer,size_t buffer_size,const char *file,int line);

#else

#define __static STATIC

#define __free(mem,file,line) free(mem)
#define __malloc(size,file,line) malloc(size)

#endif /* __MEM_DEBUG */

/****************************************************************************/

struct MemoryNode
{
	struct MinNode		mn_MinNode;
	size_t				mn_Size;

	UBYTE				mn_NeverFree;

#ifdef __MEM_DEBUG
	UBYTE				mn_AlreadyFree;
	UBYTE				mn_Pad0[2];

	void *				mn_Allocation;
	size_t				mn_AllocationSize;

	char *				mn_FreeFile;
	int					mn_FreeLine;

	char *				mn_File;
	int					mn_Line;

#ifdef __USE_MEM_TREES
	struct MemoryNode *	mn_Left;
	struct MemoryNode *	mn_Right;
	struct MemoryNode *	mn_Parent;
	UBYTE				mn_IsRed;
	UBYTE				mn_Pad1[3];
#endif /* __USE_MEM_TREES */

#else
	UBYTE				mn_Pad0[3];
#endif /* __MEM_DEBUG */
};

#ifdef __USE_MEM_TREES

struct MemoryTree
{
	struct MemoryNode *	mt_Root;
	struct MemoryNode	mt_RootNode;
	struct MemoryNode	mt_NullNode;
};

#endif /* __USE_MEM_TREES */

/****************************************************************************/

extern struct MemoryTree NOCOMMON	__memory_tree;
extern struct MinList NOCOMMON		__memory_list;
extern APTR NOCOMMON				__memory_pool;

/****************************************************************************/

extern unsigned long NOCOMMON __maximum_memory_allocated;
extern unsigned long NOCOMMON __current_memory_allocated;
extern unsigned long NOCOMMON __maximum_num_memory_chunks_allocated;
extern unsigned long NOCOMMON __current_num_memory_chunks_allocated;

/****************************************************************************/

extern int NOCOMMON __default_pool_size;
extern int NOCOMMON __default_puddle_size;

/****************************************************************************/

#if defined(__THREAD_SAFE)

/****************************************************************************/

extern void __memory_lock(void);
extern void __memory_unlock(void);

/****************************************************************************/

#else

/****************************************************************************/

#define __memory_lock()		((void)0)
#define __memory_unlock()	((void)0)

/****************************************************************************/

#endif /* __THREAD_SAFE */

/****************************************************************************/

#endif /* _STDLIB_MEMORY_H */
