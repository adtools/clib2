/*
 * $Id: stdlib_red_black.c,v 1.4 2005-03-18 12:38:24 obarthel Exp $
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

#ifndef _STDLIB_HEADERS_H
#include "stdlib_headers.h"
#endif /* _STDLIB_HEADERS_H */

/****************************************************************************/

#ifndef _STDLIB_MEMORY_H
#include "stdlib_memory.h"
#endif /* _STDLIB_MEMORY_H */

/****************************************************************************/

#if defined(__USE_MEM_TREES) && defined(__MEM_DEBUG)

/****************************************************************************/

/* Red-Black Tree 'C' code adapted from Emin Martinian's implementation
   (see <http://www.csua.berkeley.edu/~emin/source_code/red_black_tree>) */

/****************************************************************************/

STATIC VOID 
help_insertion (struct MemoryTree * tree, struct MemoryNode * z)
{
	struct MemoryNode *nil = &tree->mt_NullNode;
	struct MemoryNode *x;
	struct MemoryNode *y;

	z->mn_Left = z->mn_Right = nil;

	y = tree->mt_Root;
	x = tree->mt_Root->mn_Left;

	while (x != nil)
	{
		y = x;

		if (x->mn_Allocation > z->mn_Allocation)
			x = x->mn_Left;
		else
			x = x->mn_Right;
	}

	z->mn_Parent = y;

	if (y == tree->mt_Root || y->mn_Allocation > z->mn_Allocation)
		y->mn_Left = z;
	else
		y->mn_Right = z;
}

STATIC VOID 
rotate_left (struct MemoryTree * tree, struct MemoryNode * x)
{
	struct MemoryNode *nil = &tree->mt_NullNode;
	struct MemoryNode *y;

	y = x->mn_Right;
	x->mn_Right = y->mn_Left;

	if (y->mn_Left != nil)
		y->mn_Left->mn_Parent = x;

	y->mn_Parent = x->mn_Parent;

	if (x == x->mn_Parent->mn_Left)
		x->mn_Parent->mn_Left = y;
	else
		x->mn_Parent->mn_Right = y;

	y->mn_Left = x;
	x->mn_Parent = y;
}

STATIC VOID 
rotate_right (struct MemoryTree * tree, struct MemoryNode * y)
{
	struct MemoryNode *nil = &tree->mt_NullNode;
	struct MemoryNode *x;

	x = y->mn_Left;
	y->mn_Left = x->mn_Right;

	if (nil != x->mn_Right)
		x->mn_Right->mn_Parent = y;

	x->mn_Parent = y->mn_Parent;

	if (y == y->mn_Parent->mn_Left)
		y->mn_Parent->mn_Left = x;
	else
		y->mn_Parent->mn_Right = x;

	x->mn_Right = y;
	y->mn_Parent = x;
}

/****************************************************************************/

STATIC STRUCT MemoryNode *
get_successor (struct MemoryTree * tree, struct MemoryNode * x)
{
	struct MemoryNode *nil = &tree->mt_NullNode;
	struct MemoryNode *root = tree->mt_Root;
	struct MemoryNode *result;
	struct MemoryNode *y;

	if (nil != (y = x->mn_Right))
	{
		while (y->mn_Left != nil)
			y = y->mn_Left;

		result = y;
	}
	else
	{
		y = x->mn_Parent;

		while (x == y->mn_Right)
		{
			x = y;
			y = y->mn_Parent;
		}

		if (y == root)
			result = nil;
		else
			result = y;
	}

	return (result);
}

STATIC VOID 
remove_fixup (struct MemoryTree * tree, struct MemoryNode * x)
{
	struct MemoryNode *root = tree->mt_Root->mn_Left;
	struct MemoryNode *w;

	while (NOT x->mn_IsRed && root != x)
	{
		if (x == x->mn_Parent->mn_Left)
		{
			w = x->mn_Parent->mn_Right;

			if (w->mn_IsRed)
			{
				w->mn_IsRed = FALSE;
				x->mn_Parent->mn_IsRed = TRUE;

				rotate_left (tree, x->mn_Parent);

				w = x->mn_Parent->mn_Right;
			}

			if (NOT w->mn_Right->mn_IsRed && NOT w->mn_Left->mn_IsRed)
			{
				w->mn_IsRed = TRUE;

				x = x->mn_Parent;
			}
			else
			{
				if (NOT w->mn_Right->mn_IsRed)
				{
					w->mn_Left->mn_IsRed = FALSE;
					w->mn_IsRed = TRUE;

					rotate_right (tree, w);

					w = x->mn_Parent->mn_Right;
				}

				w->mn_IsRed = x->mn_Parent->mn_IsRed;
				x->mn_Parent->mn_IsRed = FALSE;
				w->mn_Right->mn_IsRed = FALSE;

				rotate_left (tree, x->mn_Parent);

				x = root;
			}
		}
		else
		{
			w = x->mn_Parent->mn_Left;

			if (w->mn_IsRed)
			{
				w->mn_IsRed = FALSE;
				x->mn_Parent->mn_IsRed = TRUE;

				rotate_right (tree, x->mn_Parent);

				w = x->mn_Parent->mn_Left;
			}

			if (NOT w->mn_Right->mn_IsRed && NOT w->mn_Left->mn_IsRed)
			{
				w->mn_IsRed = TRUE;

				x = x->mn_Parent;
			}
			else
			{
				if (NOT w->mn_Left->mn_IsRed)
				{
					w->mn_Right->mn_IsRed = FALSE;
					w->mn_IsRed = TRUE;

					rotate_left (tree, w);

					w = x->mn_Parent->mn_Left;
				}

				w->mn_IsRed = x->mn_Parent->mn_IsRed;
				x->mn_Parent->mn_IsRed = FALSE;
				w->mn_Left->mn_IsRed = FALSE;

				rotate_right (tree, x->mn_Parent);

				x = root;
			}
		}
	}

	x->mn_IsRed = FALSE;
}

/****************************************************************************/

void
__initialize_red_black_tree (struct MemoryTree *new_tree)
{
	struct MemoryNode *temp;

	temp = &new_tree->mt_NullNode;
	temp->mn_Parent = temp->mn_Left = temp->mn_Right = temp;
	temp->mn_IsRed = FALSE;
	temp->mn_Allocation = NULL;

	temp = new_tree->mt_Root = &new_tree->mt_RootNode;
	temp->mn_Parent = temp->mn_Left = temp->mn_Right = &new_tree->mt_NullNode;
	temp->mn_IsRed = FALSE;
	temp->mn_Allocation = NULL;
}

/****************************************************************************/

void
__red_black_tree_insert (struct MemoryTree * tree, struct MemoryNode *x)
{
	struct MemoryNode *y;

	help_insertion (tree, x);

	x->mn_IsRed = TRUE;

	while (x->mn_Parent->mn_IsRed)
	{
		if (x->mn_Parent == x->mn_Parent->mn_Parent->mn_Left)
		{
			y = x->mn_Parent->mn_Parent->mn_Right;

			if (y->mn_IsRed)
			{
				x->mn_Parent->mn_IsRed = FALSE;
				y->mn_IsRed = FALSE;
				x->mn_Parent->mn_Parent->mn_IsRed = TRUE;
				x = x->mn_Parent->mn_Parent;
			}
			else
			{
				if (x == x->mn_Parent->mn_Right)
				{
					x = x->mn_Parent;

					rotate_left (tree, x);
				}

				x->mn_Parent->mn_IsRed = FALSE;
				x->mn_Parent->mn_Parent->mn_IsRed = TRUE;

				rotate_right (tree, x->mn_Parent->mn_Parent);
			}
		}
		else
		{
			y = x->mn_Parent->mn_Parent->mn_Left;

			if (y->mn_IsRed)
			{
				x->mn_Parent->mn_IsRed = FALSE;
				y->mn_IsRed = FALSE;
				x->mn_Parent->mn_Parent->mn_IsRed = TRUE;
				x = x->mn_Parent->mn_Parent;
			}
			else
			{
				if (x == x->mn_Parent->mn_Left)
				{
					x = x->mn_Parent;

					rotate_right (tree, x);
				}

				x->mn_Parent->mn_IsRed = FALSE;
				x->mn_Parent->mn_Parent->mn_IsRed = TRUE;

				rotate_left (tree, x->mn_Parent->mn_Parent);
			}
		}
	}

	tree->mt_Root->mn_Left->mn_IsRed = FALSE;
}

/****************************************************************************/

struct MemoryNode *
__red_black_tree_find(struct MemoryTree * tree,void * allocation)
{
	struct MemoryNode * x = tree->mt_Root->mn_Left;
	struct MemoryNode * nil = &tree->mt_NullNode;
	struct MemoryNode * result = NULL;

	while(x != nil)
	{
		if(x->mn_Allocation > allocation)
		{
			x = x->mn_Left;
		}
		else if (x->mn_Allocation < allocation)
		{
			x = x->mn_Right;
		}
		else
		{
			result = x;
			break;
		}
	}

	return(result);
}

/****************************************************************************/

void
__red_black_tree_remove (struct MemoryTree * tree, struct MemoryNode * z)
{
	struct MemoryNode *nil = &tree->mt_NullNode;
	struct MemoryNode *root = tree->mt_Root;
	struct MemoryNode *y;
	struct MemoryNode *x;

	y = (z->mn_Left == nil || z->mn_Right == nil) ? z : get_successor (tree, z);
	x = (y->mn_Left == nil) ? y->mn_Right : y->mn_Left;

	if (root == (x->mn_Parent = y->mn_Parent))
	{
		root->mn_Left = x;
	}
	else
	{
		if (y == y->mn_Parent->mn_Left)
			y->mn_Parent->mn_Left = x;
		else
			y->mn_Parent->mn_Right = x;
	}

	if (NOT y->mn_IsRed)
		remove_fixup (tree, x);

	if (y != z)
	{
		y->mn_Left = z->mn_Left;
		y->mn_Right = z->mn_Right;
		y->mn_Parent = z->mn_Parent;
		y->mn_IsRed = z->mn_IsRed;
		z->mn_Left->mn_Parent = z->mn_Right->mn_Parent = y;

		if (z == z->mn_Parent->mn_Left)
			z->mn_Parent->mn_Left = y;
		else
			z->mn_Parent->mn_Right = y;
	}
}

/****************************************************************************/

#endif /* __USE_MEM_TREES && __MEM_DEBUG */
