/*
 * $Id: stdio_fdhookentry.c,v 1.10 2005-02-07 11:19:31 obarthel Exp $
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
#include "stdlib_mem_debug.h"
#endif /* _STDLIB_MEM_DEBUG_H */

/****************************************************************************/

#ifndef _STDIO_HEADERS_H
#include "stdio_headers.h"
#endif /* _STDIO_HEADERS_H */

#ifndef _UNISTD_HEADERS_H
#include "unistd_headers.h"
#endif /* _UNISTD_HEADERS_H */

/****************************************************************************/

#include <strings.h>
#include <limits.h>

/****************************************************************************/

#if defined(__amigaos4__) && !defined(Flush)
#define Flush(fh) FFlush(fh)
#endif /* __amigaos4__ && !Flush */

/****************************************************************************/

/* This is used in place of ExamineFH() in order to work around a bug in
 * dos.library V40 and below: a "NIL:" file handle will crash the
 * ExamineFH() function.
 */
static LONG
safe_examine_file_handle(BPTR file_handle,struct FileInfoBlock *fib)
{
	LONG result = DOSFALSE;

	assert( fib != NULL );

	#ifndef __amigaos4__
	{
		struct FileHandle * fh = (struct FileHandle *)BADDR(file_handle);

		if(fh == NULL || fh->fh_Type == NULL)
		{
			SetIoErr(ERROR_OBJECT_WRONG_TYPE);
			goto out;
		}
	}
	#endif /* __amigaos4__ */

	PROFILE_OFF();
	result = ExamineFH(file_handle,fib);
	PROFILE_ON();

 out:

	return(result);
}

/****************************************************************************/

/* Same as above, only for ParentOfFH(). */
static BPTR
safe_parent_of_file_handle(BPTR file_handle)
{
	BPTR result = ZERO;

	#ifndef __amigaos4__
	{
		struct FileHandle * fh = (struct FileHandle *)BADDR(file_handle);

		if(fh == NULL || fh->fh_Type == NULL)
		{
			SetIoErr(ERROR_OBJECT_WRONG_TYPE);
			goto out;
		}
	}
	#endif /* __amigaos4__ */

	PROFILE_OFF();
	result = ParentOfFH(file_handle);
	PROFILE_ON();

 out:

	return(result);
}

/****************************************************************************/

#if defined(UNIX_PATH_SEMANTICS)

/****************************************************************************/

/* System-wide global data structure all programs which use advisory file record
 * locking with this runtime library will use.
 */
struct FileLockSemaphore
{
	struct SignalSemaphore	fls_Semaphore;	/* Standard signal semaphore part */
	UWORD					fls_Size;		/* Size of this data structure (may grow) */
	struct MinList			fls_LockList;	/* List of lock nodes */
};

/* Each file which has regions locked is registered through the following
 * data structure.
 */
struct FileLockNode
{
	struct MinNode	fln_MinNode;			/* Standard node */
	struct MinList	fln_LockedRegionList;	/* List of locked regions */

	BPTR			fln_FileParentDir;		/* Refers to the file's parent directory */
	UBYTE			fln_FileName[1];		/* Name of the file; together with the
											 * parent directory, this should uniquely
											 * identify the file.
											 */
};

/* A single locked region, as associated with a file. */
struct LockedRegionNode
{
	struct MinNode	lrn_MinNode;			/* Standard node */
	LONG			lrn_Start;				/* Where the region begins */
	LONG			lrn_Stop;				/* Where the region ends (inclusive) */
	LONG			lrn_Length;				/* Original length requested */
	pid_t			lrn_Owner;				/* Which process owns the region */
	BOOL			lrn_Shared;				/* Whether or not this region has been locked
											 * for shared access.
											 */
};

/****************************************************************************/

static struct FileLockSemaphore * FileLockSemaphore;

/****************************************************************************/

static void release_file_lock_semaphore(struct FileLockSemaphore *fls);
static struct FileLockSemaphore *obtain_file_lock_semaphore(BOOL shared);
static void remove_locked_region_node(struct FileLockSemaphore *fls, struct fd *fd, LONG start, LONG stop, LONG original_length);
static void delete_locked_region_node(struct LockedRegionNode *lrn);
static long create_locked_region_node(struct LockedRegionNode **result_ptr);
static void delete_file_lock_node(struct FileLockNode *fln);
static long create_file_lock_node(struct fd *fd, struct FileLockNode **result_ptr);
static long find_file_lock_node_by_file_handle(struct FileLockSemaphore *fls, BPTR file_handle, struct FileLockNode **result_ptr);
static long find_file_lock_node_by_drawer_and_name(struct FileLockSemaphore *fls, BPTR dir_lock, STRPTR file_name, struct FileLockNode **result_ptr);
static struct LockedRegionNode *find_colliding_region(struct FileLockNode *fln, LONG start, LONG stop, BOOL shared);

/****************************************************************************/

static void
release_file_lock_semaphore(struct FileLockSemaphore * fls)
{
	ENTER();

	SHOWPOINTER(fls);

	if(fls != NULL)
		ReleaseSemaphore((struct SignalSemaphore *)fls);

	LEAVE();
}

/****************************************************************************/

static struct FileLockSemaphore *
obtain_file_lock_semaphore(BOOL shared)
{
	struct FileLockSemaphore * result = NULL;

	ENTER();

	if(FileLockSemaphore == NULL && __file_lock_semaphore_name != NULL && __file_lock_semaphore_name[0] != '\0')
	{
		SHOWMSG("try to find the locking semaphore");

		Forbid();

		FileLockSemaphore = (struct FileLockSemaphore *)FindSemaphore((STRPTR)__file_lock_semaphore_name);
		if(FileLockSemaphore == NULL)
		{
			SHOWMSG("didn't find it; we're going to make our own");

			FileLockSemaphore = AllocMem(sizeof(*FileLockSemaphore) + strlen(__file_lock_semaphore_name)+1,MEMF_ANY|MEMF_PUBLIC);
			if(FileLockSemaphore != NULL)
			{
				SHOWMSG("adding our own semaphore");

				memset(FileLockSemaphore,0,sizeof(*FileLockSemaphore));

				InitSemaphore(&FileLockSemaphore->fls_Semaphore);

				FileLockSemaphore->fls_Semaphore.ss_Link.ln_Name = (char *)(FileLockSemaphore + 1);
				strcpy(FileLockSemaphore->fls_Semaphore.ss_Link.ln_Name,__file_lock_semaphore_name);

				FileLockSemaphore->fls_Semaphore.ss_Link.ln_Pri = 1;

				FileLockSemaphore->fls_Size = sizeof(*FileLockSemaphore);
				NewList((struct List *)&FileLockSemaphore->fls_LockList);

				AddSemaphore(&FileLockSemaphore->fls_Semaphore);
			}
			else
			{
				SHOWMSG("not enough memory");
			}
		}
		else if (FileLockSemaphore->fls_Size < sizeof(*FileLockSemaphore))
		{
			SHOWMSG("semaphore found, but it's too short");

			SHOWVALUE(FileLockSemaphore->fls_Size);
			SHOWVALUE(sizeof(*FileLockSemaphore));

			FileLockSemaphore = NULL;
		}

		Permit();
	}

	if(FileLockSemaphore != NULL)
	{
		SHOWMSG("got a semaphore, using it...");

		PROFILE_OFF();

		if(shared)
		{
			#if defined(__amigaos4__)
			{
				ObtainSemaphoreShared((struct SignalSemaphore *)FileLockSemaphore);
			}
			#else
			{
				if(((struct Library *)SysBase)->lib_Version >= 39)
				{
					ObtainSemaphoreShared((struct SignalSemaphore *)FileLockSemaphore);
				}
				else
				{
					/* Workaround for shared semaphore nesting problem. */
					if(CANNOT AttemptSemaphoreShared((struct SignalSemaphore *)FileLockSemaphore))
					{
						if(CANNOT AttemptSemaphore((struct SignalSemaphore *)FileLockSemaphore))
							ObtainSemaphoreShared((struct SignalSemaphore *)FileLockSemaphore);
					}
				}
			}
			#endif /* __amigaos4__ */
		}
		else
		{
			ObtainSemaphore((struct SignalSemaphore *)FileLockSemaphore);
		}

		PROFILE_ON();

		result = FileLockSemaphore;
	}
	else
	{
		SHOWMSG("didn't get the semaphore");
	}

	RETURN(result);
	return(result);
}

/****************************************************************************/

static void
remove_locked_region_node(struct FileLockSemaphore * fls,struct fd * fd,LONG start,LONG UNUSED stop,LONG original_length)
{
	ENTER();

	assert( fls != NULL && fd != NULL );

	if(FLAG_IS_SET(fd->fd_Flags,FDF_IS_LOCKED))
	{
		struct FileLockNode * which_lock = NULL;

		SHOWMSG("found a locked file");

		/* Find the locked file this descriptor
		 * buffer belongs to.
		 */
		if(find_file_lock_node_by_file_handle(fls,fd->fd_DefaultFile,&which_lock) == OK)
		{
			struct LockedRegionNode * lrn;
			struct LockedRegionNode * lrn_next;
			pid_t this_task = getpid();

			assert( which_lock != NULL );

			D(("trying to unlock the region %ld..%ld",start,stop));

			/* Find the region to unlock and remove it. */
			for(lrn = (struct LockedRegionNode *)which_lock->fln_LockedRegionList.mlh_Head ;
			    (lrn_next = (struct LockedRegionNode *)lrn->lrn_MinNode.mln_Succ) != NULL ;
			    lrn = lrn_next)
			{
				if(lrn->lrn_Owner	== this_task &&
				   lrn->lrn_Start	== start &&
				   lrn->lrn_Length	== original_length)
				{
					SHOWMSG("unlocking all regions on this file");

					Remove((struct Node *)lrn);
					delete_locked_region_node(lrn);
				}
			}

			/* Check if there are any locked regions left.
			 * If not, mark the entire file as unlocked.
			 */
			if(IsListEmpty((struct List *)&which_lock->fln_LockedRegionList))
			{
				SHOWMSG("no more regions are locked; removing the file lock node");

				Remove((struct Node *)which_lock);
				delete_file_lock_node(which_lock);

				/* If this is an alias, move up to the real thing. */
				if(fd->fd_Original != NULL)
					fd = fd->fd_Original;

				do
				{
					CLEAR_FLAG(fd->fd_Flags,FDF_IS_LOCKED);
				}
				while((fd = fd->fd_NextLink) != NULL);
			}
		}
	}

	LEAVE();
}

/****************************************************************************/

static void
delete_locked_region_node(struct LockedRegionNode * lrn)
{
	ENTER();

	SHOWPOINTER(lrn);

	FreeVec(lrn);

	LEAVE();
}

/****************************************************************************/

static long
create_locked_region_node(struct LockedRegionNode ** result_ptr)
{
	struct LockedRegionNode * lrn;
	LONG error = OK;

	ENTER();

	assert( result_ptr != NULL );

	lrn = AllocVec(sizeof(*lrn),MEMF_ANY|MEMF_PUBLIC|MEMF_CLEAR);
	if(lrn == NULL)
	{
		SHOWMSG("not enough memory for locked region node");

		error = ERROR_NO_FREE_STORE;
		goto out;
	}

	lrn->lrn_Owner = getpid();

 out:

	(*result_ptr) = lrn;

	RETURN(error);
	return(error);
}

/****************************************************************************/

static void
delete_file_lock_node(struct FileLockNode * fln)
{
	ENTER();

	SHOWPOINTER(fln);

	if(fln != NULL)
	{
		PROFILE_OFF();
		UnLock(fln->fln_FileParentDir);
		PROFILE_ON();

		FreeVec(fln);
	}

	LEAVE();
}

/****************************************************************************/

static long
create_file_lock_node(struct fd * fd,struct FileLockNode ** result_ptr)
{
	struct FileLockNode * result = NULL;
	struct FileLockNode * fln = NULL;
	D_S(struct FileInfoBlock,fib);
	LONG error = OK;

	ENTER();

	assert( fd != NULL && result_ptr != NULL );

	/* We store a lock on the file's parent directory
	 * and the name of the file for later use in
	 * comparisons.
	 */
	if(CANNOT safe_examine_file_handle(fd->fd_DefaultFile,fib))
	{
		SHOWMSG("couldn't examine file handle");

		error = IoErr();
		goto out;
	}

	fln = AllocVec(sizeof(*fln) + strlen(fib->fib_FileName),MEMF_ANY|MEMF_PUBLIC);
	if(fln == NULL)
	{
		SHOWMSG("not enough memory for lock node");

		error = ERROR_NO_FREE_STORE;
		goto out;
	}

	memset(fln,0,sizeof(*fln));

	fln->fln_FileParentDir = safe_parent_of_file_handle(fd->fd_DefaultFile);
	if(fln->fln_FileParentDir == ZERO)
	{
		SHOWMSG("couldn't get parent directory");

		error = IoErr();
		goto out;
	}

	strcpy(fln->fln_FileName,fib->fib_FileName);

	NewList((struct List *)&fln->fln_LockedRegionList);

	result = fln;
	fln = NULL;

 out:

	delete_file_lock_node(fln);

	(*result_ptr) = result;

	RETURN(error);
	return(error);
}

/****************************************************************************/

static long
find_file_lock_node_by_drawer_and_name(
	struct FileLockSemaphore *	fls,
	BPTR						dir_lock,
	STRPTR						file_name,
	struct FileLockNode **		result_ptr)
{
	DECLARE_UTILITYBASE();
	struct FileLockNode * result = NULL;
	struct FileLockNode * fln;
	LONG status;
	LONG error;

	ENTER();

	assert( fls != NULL && dir_lock != ZERO && file_name != NULL && result_ptr != NULL );
	assert( UtilityBase != NULL );

	#if DEBUG
	{
		char name[FILENAME_MAX];

		if(NameFromLock(dir_lock,name,sizeof(name)))
		{
			if(AddPart(name,file_name,sizeof(name)))
				D(("Looking for a lock on file |%s|",name));
		}
	}
	#endif /* DEBUG */

	error = ERROR_OBJECT_NOT_FOUND;

	for(fln = (struct FileLockNode *)fls->fls_LockList.mlh_Head ;
	    fln->fln_MinNode.mln_Succ != NULL ;
	    fln = (struct FileLockNode *)fln->fln_MinNode.mln_Succ)
	{
		PROFILE_OFF();
		status = SameLock(fln->fln_FileParentDir,dir_lock);
		PROFILE_ON();

		if(status == LOCK_SAME)
		{
			if(Stricmp(fln->fln_FileName,file_name) == SAME)
			{
				error = OK;

				result = fln;

				break;
			}
		}
	}

	if(result != NULL)
		SHOWMSG("found one");
	else
		SHOWMSG("didn't find one");

	(*result_ptr) = result;

	RETURN(error);
	return(error);
}

/****************************************************************************/

static LONG
find_file_lock_node_by_file_handle(
	struct FileLockSemaphore *	fls,
	BPTR						file_handle,
	struct FileLockNode **		result_ptr)
{
	D_S(struct FileInfoBlock,this_fib);
	BPTR parent_dir = ZERO;
	LONG error;

	ENTER();

	assert( fls != NULL && file_handle != ZERO && result_ptr != NULL );

	(*result_ptr) = NULL;

	if(CANNOT safe_examine_file_handle(file_handle,this_fib))
	{
		SHOWMSG("couldn't examine file handle");

		error = IoErr();
		goto out;
	}

	/* Determine the file's parent directory and
	 * name. These will be compared against the
	 * global file lock data.
	 */
	parent_dir = safe_parent_of_file_handle(file_handle);
	if(parent_dir == ZERO)
	{
		SHOWMSG("couldn't get parent directory");

		error = IoErr();
		goto out;
	}

	error = find_file_lock_node_by_drawer_and_name(fls,parent_dir,this_fib->fib_FileName,result_ptr);
	if(error != OK)
	{
		SHOWMSG("couldn't find a lock node for the file");
		goto out;
	}

 out:

	PROFILE_OFF();
	UnLock(parent_dir);
	PROFILE_ON();

	RETURN(error);
	return(error);
}

/****************************************************************************/

static struct LockedRegionNode *
find_colliding_region(struct FileLockNode * fln,LONG start,LONG stop,BOOL shared)
{
	struct LockedRegionNode * result = NULL;
	struct LockedRegionNode * lrn;
	pid_t this_task = getpid();

	ENTER();

	assert( fln != NULL && start <= stop );

	SHOWVALUE(start);
	SHOWVALUE(stop);
	SHOWVALUE(shared);

	/* This routine looks for a locked region that overlaps
	 * with the specified region. It returns a pointer to the
	 * region that would collide with the specified region if
	 * a new lock were to be added.
	 */
	for(lrn = (struct LockedRegionNode *)fln->fln_LockedRegionList.mlh_Head ;
	    lrn->lrn_MinNode.mln_Succ != NULL ;
	    lrn = (struct LockedRegionNode *)lrn->lrn_MinNode.mln_Succ)
	{
		/* Do the regions overlap? */
		if(lrn->lrn_Start <= stop && start <= lrn->lrn_Stop)
		{
			/* Two shared regions may always overlap.
			 * How about the rest?
			 */
			if(NOT shared || NOT lrn->lrn_Shared)
			{
				/* The lock owner may add as many exclusive
				 * or shared locks to the same region as
				 * necessary.
				 */
				if(lrn->lrn_Owner == this_task)
				{
					/*kprintf("lock collision [%ld..%ld] : [%ld..%ld]\n",start,stop,lrn->lrn_Start,lrn->lrn_Stop);*/
					continue;
				}

				/* So we found a region that would
				 * cause a collision.
				 */
				result = lrn;
				break;
			}
		}
	}

	RETURN(result);
	return(result);
}

/****************************************************************************/

static void
cleanup_locked_records(struct fd * fd)
{
	ENTER();

	assert( fd != NULL );

	/* This routine removes all locked regions from a file
	 * before it is eventually closed.
	 */

	if(FLAG_IS_SET(fd->fd_Flags,FDF_IS_LOCKED))
	{
		struct FileLockSemaphore * fls;

		fls = obtain_file_lock_semaphore(FALSE);
		if(fls != NULL)
		{
			BPTR file_handle = fd->fd_DefaultFile;
			struct FileLockNode * which_lock = NULL;
			pid_t this_task = getpid();
			LONG error;

			error = find_file_lock_node_by_file_handle(fls,file_handle,&which_lock);
			if(error == OK)
			{
				struct LockedRegionNode * lrn_this;
				struct LockedRegionNode * lrn_next;

				assert( which_lock != NULL );

				SHOWMSG("unlocking all regions on this file");

				for(lrn_this = (struct LockedRegionNode *)which_lock->fln_LockedRegionList.mlh_Head ;
				    (lrn_next = (struct LockedRegionNode *)lrn_this->lrn_MinNode.mln_Succ) != NULL ;
				    lrn_this = lrn_next)
				{
					if(lrn_this->lrn_Owner == this_task)
					{
						Remove((struct Node *)lrn_this);

						delete_locked_region_node(lrn_this);
					}
				}

				if(IsListEmpty((struct List *)&which_lock->fln_LockedRegionList))
				{
					SHOWMSG("no more regions are locked; removing the file lock node");

					Remove((struct Node *)which_lock);

					delete_file_lock_node(which_lock);
				}
			}

			if(error == OK || error == ERROR_OBJECT_NOT_FOUND)
			{
				/* If this is an alias, move up to the real thing. */
				if(fd->fd_Original != NULL)
					fd = fd->fd_Original;

				do
				{
					CLEAR_FLAG(fd->fd_Flags,FDF_IS_LOCKED);
				}
				while((fd = fd->fd_NextLink) != NULL);
			}

			release_file_lock_semaphore(fls);
		}
	}

	LEAVE();
}

/****************************************************************************/

static int
handle_record_locking(int cmd,struct flock * l,struct fd * fd,int * error_ptr)
{
	struct FileLockSemaphore * fls = NULL;
	BPTR file_handle = fd->fd_DefaultFile;
	struct LockedRegionNode * lrn = NULL;
	struct FileLockNode * fln = NULL;
	D_S(struct FileInfoBlock,fib);
	BOOL fib_is_valid = FALSE;
	BPTR parent_dir = ZERO;
	LONG current_position;
	int result = ERROR;
	LONG original_len;
	LONG error = OK;
	LONG start = 0;
	LONG len = 0;
	LONG stop;

	ENTER();

	/* This routine implements advisory file segment locking
	 * similar to 4.4BSD, but not quite the same. The functionality
	 * is a subset, somewhat similar to the functionality offered
	 * by the AmigaDOS LockRecord() and UnlockRecord() functions.
	 * This means for example that every unlock request must
	 * match the size and position of the corresponding locking
	 * request.
	 *
	 * This implementation was chosen because not every Amiga
	 * filing system implements record locking and Samba
	 * absolutely requires this functionality to work.
	 */
	assert( l != NULL && fd != NULL);
	assert( F_RDLCK <= l->l_type && l->l_type <= F_WRLCK );
	assert( SEEK_SET <= l->l_whence && l->l_whence <= SEEK_END );
	assert( error_ptr != NULL );

	if((cmd == F_SETLK || cmd == F_SETLKW) && (l->l_type != F_UNLCK))
	{
		SHOWMSG("this is a lock request");

		error = create_file_lock_node(fd,&fln);
		if(error != OK)
		{
			SHOWMSG("could not create lock node");
			goto out;
		}

		error = create_locked_region_node(&lrn);
		if(error != OK)
		{
			SHOWMSG("could not create region node");
			goto out;
		}
	}
	else
	{
		SHOWMSG("this is not a lock request");
	}

	original_len = l->l_len;

	/* Now calculate the position of the first byte to lock and the number
	 * of bytes to lock.
	 */
	switch(l->l_whence)
	{
		case SEEK_SET:

			SHOWMSG("SEEK_SET");

			start = l->l_start;

			if(l->l_len == 0)
				len = LONG_MAX;
			else
				len = l->l_len;

			SHOWVALUE(start);
			SHOWVALUE(len);

			break;

		case SEEK_CUR:

			SHOWMSG("SEEK_CUR");

			PROFILE_OFF();

			current_position = Seek(file_handle,0,OFFSET_CURRENT);

			PROFILE_ON();

			if(current_position == SEEK_ERROR)
			{
				SHOWMSG("could not obtain current seek position");

				error = IoErr();
				goto out;
			}

			start = current_position + l->l_start;

			if(l->l_len == 0)
				len = LONG_MAX;
			else
				len = l->l_len;

			SHOWVALUE(start);
			SHOWVALUE(len);

			break;

		case SEEK_END:

			SHOWMSG("SEEK_END");

			if(CANNOT safe_examine_file_handle(file_handle,fib))
			{
				SHOWMSG("could not examine file");

				error = IoErr();
				goto out;
			}

			fib_is_valid = TRUE;

			start = fib->fib_Size + l->l_start;

			if(l->l_len == 0)
				len = LONG_MAX;
			else
				len = l->l_len;

			SHOWVALUE(start);
			SHOWVALUE(len);

			break;
	}

	SHOWVALUE(start);
	SHOWVALUE(len);

	if(start < 0)
	{
		SHOWMSG("invalid start");

		(*error_ptr) = EINVAL;
		goto out;
	}

	if(len < 0)
	{
		start += len;
		if(start < 0)
		{
			SHOWMSG("invalid start");

			(*error_ptr) = EINVAL;
			goto out;
		}

		stop = start - len - 1;
		if(stop < start)
			stop = LONG_MAX;
	}
	else
	{
		stop = start - 1 + len;
		if(stop < start - 1) /* Check for overflow */
			stop = LONG_MAX;
	}

	if(l->l_type == F_UNLCK)
	{
		SHOWMSG("F_UNLCK");

		fls = obtain_file_lock_semaphore(FALSE);
		if(fls == NULL)
		{
			SHOWMSG("couldn't obtain file locking semaphore");
			(*error_ptr) = EBADF;
			goto out;
		}

		D(("unlocking %ld..%ld",start,stop));

		remove_locked_region_node(fls,fd,start,stop,original_len);
	}
	else if (cmd == F_SETLKW)
	{
		struct FileLockNode * existing_fln;
		BOOL locked;
		BOOL shared;

		SHOWMSG("F_SETLKW");

		D(("  locking %ld..%ld",start,stop));

		if(NOT fib_is_valid && CANNOT safe_examine_file_handle(file_handle,fib))
		{
			SHOWMSG("couldn't read this file's name");

			error = IoErr();
			goto out;
		}

		parent_dir = safe_parent_of_file_handle(file_handle);
		if(parent_dir == ZERO)
		{
			SHOWMSG("couldn't get a lock on the file's parent directory");

			error = IoErr();
			goto out;
		}

		shared = (BOOL)(l->l_type == F_RDLCK);

		if(shared)
			D(("this is a shared lock; waiting for completion"));
		else
			D(("this is an exclusive lock; waiting for completion"));

		/* Shared locks require readable files, exclusive locks require
		   writable files. */
		if((shared && FLAG_IS_CLEAR(fd->fd_Flags,FDF_READ)) ||
		   (NOT shared && FLAG_IS_CLEAR(fd->fd_Flags,FDF_WRITE)))
		{
			(*error_ptr) = EBADF;
			goto out;
		}

		lrn->lrn_Start	= start;
		lrn->lrn_Stop	= stop;
		lrn->lrn_Length	= original_len;
		lrn->lrn_Shared	= shared;

		/* Retry until we manage to lock the record. */
		locked = FALSE;

		do
		{
			fls = obtain_file_lock_semaphore(FALSE);
			if(fls == NULL)
			{
				SHOWMSG("couldn't obtain file locking semaphore");
				(*error_ptr) = EBADF;
				goto out;
			}

			if(find_file_lock_node_by_drawer_and_name(fls,parent_dir,fib->fib_FileName,&existing_fln) == OK)
			{
				SHOWMSG("that file is already locked by someone");

				if(find_colliding_region(existing_fln,start,stop,shared) == NULL)
				{
					SHOWMSG("but the locks don't collide");

					AddTail((struct List *)&existing_fln->fln_LockedRegionList,(struct Node *)lrn);
					lrn = NULL;

					locked = TRUE;
				}
				else
				{
					SHOWMSG("and the locks collide");
				}
			}
			else
			{
				SHOWMSG("nobody has any locks on this file");

				AddTail((struct List *)&fls->fls_LockList,(struct Node *)fln);
				AddTail((struct List *)&fln->fln_LockedRegionList,(struct Node *)lrn);

				fln = NULL;
				lrn = NULL;

				locked = TRUE;
			}

			release_file_lock_semaphore(fls);
			fls = NULL;

			if(NOT locked)
			{
				const int rand_max = RAND_MAX / 65536;
				int num_random_ticks;

				if(__check_abort_enabled && (SetSignal(0,0) & SIGBREAKF_CTRL_C) != 0)
				{
					SHOWMSG("lock polling loop stopped");

					delete_file_lock_node(fln);
					fln = NULL;

					delete_locked_region_node(lrn);
					lrn = NULL;

					PROFILE_OFF();
					UnLock(parent_dir);
					PROFILE_ON();

					parent_dir = ZERO;

					(*error_ptr) = EINTR;

					__check_abort();
					goto out;
				}

				/* Wait a little before retrying
				 * the locking operation. We add
				 * a little randomness here to
				 * reduce the likelihood of two
				 * competing processes trying to
				 * lock the same file at the
				 * same time.
				 */

				num_random_ticks = ((TICKS_PER_SECOND / 2) * (rand() / 65536)) / rand_max;

				if(num_random_ticks > 0)
				{
					PROFILE_OFF();
					Delay(num_random_ticks);
					PROFILE_ON();
				}
			}
		}
		while(NOT locked);

		SHOWMSG("the file now has a lock set");

		/* If this is an alias, move up to the real thing. */
		if(fd->fd_Original != NULL)
			fd = fd->fd_Original;

		do
		{
			SET_FLAG(fd->fd_Flags,FDF_IS_LOCKED);
		}
		while((fd = fd->fd_NextLink) != NULL);
	}
	else if (cmd == F_SETLK)
	{
		BOOL shared = (BOOL)(l->l_type == F_RDLCK);
		struct FileLockNode * existing_fln = NULL;

		SHOWMSG("F_SETLK");

		if(shared)
			D(("this is a shared lock"));
		else
			D(("this is an exclusive lock"));

		/* Shared locks require readable files, exclusive locks require
		   writable files. */
		if((shared && FLAG_IS_CLEAR(fd->fd_Flags,FDF_READ)) ||
		   (NOT shared && FLAG_IS_CLEAR(fd->fd_Flags,FDF_WRITE)))
		{
			(*error_ptr) = EBADF;
			goto out;
		}

		lrn->lrn_Start	= start;
		lrn->lrn_Stop	= stop;
		lrn->lrn_Length	= original_len;
		lrn->lrn_Shared	= shared;

		fls = obtain_file_lock_semaphore(FALSE);
		if(fls == NULL)
		{
			SHOWMSG("couldn't obtain file locking semaphore");
			(*error_ptr) = EBADF;
			goto out;
		}

		error = find_file_lock_node_by_file_handle(fls,file_handle,&existing_fln);
		if(error == OK)
		{
			SHOWMSG("that file is already locked by someone else");

			if(find_colliding_region(existing_fln,start,stop,shared) != NULL)
			{
				SHOWMSG("and the locks collide");

				(*error_ptr) = EACCES;
				goto out;
			}

			SHOWMSG("but the locks don't collide");

			AddTail((struct List *)&existing_fln->fln_LockedRegionList,(struct Node *)lrn);
			lrn = NULL;
		}
		else
		{
			if(error != ERROR_OBJECT_NOT_FOUND)
				goto out;

			SHOWMSG("nobody has any locks on this file");

			AddTail((struct List *)&fls->fls_LockList,(struct Node *)fln);
			AddTail((struct List *)&fln->fln_LockedRegionList,(struct Node *)lrn);

			fln = NULL;
			lrn = NULL;
		}

		SHOWMSG("the file now has a lock set");

		/* If this is an alias, move up to the real thing. */
		if(fd->fd_Original != NULL)
			fd = fd->fd_Original;

		do
		{
			SET_FLAG(fd->fd_Flags,FDF_IS_LOCKED);
		}
		while((fd = fd->fd_NextLink) != NULL);
	}
	else if (cmd == F_GETLK)
	{
		struct FileLockNode * fln_found = NULL;

		SHOWMSG("F_GETLK");

		fls = obtain_file_lock_semaphore(TRUE);
		if(fls == NULL)
		{
			SHOWMSG("couldn't obtain file locking semaphore");
			(*error_ptr) = EBADF;
			goto out;
		}

		SHOWMSG("checking for possible lock collision");

		error = find_file_lock_node_by_file_handle(fls,file_handle,&fln_found);
		if(error == OK)
		{
			BOOL shared = (BOOL)(l->l_type == F_RDLCK);
			struct LockedRegionNode * lrn_found;

			SHOWMSG("somebody has locked this file");

			lrn_found = find_colliding_region(fln_found,start,stop,shared);
			if(lrn_found != NULL)
			{
				SHOWMSG("there is a possible lock collision");

				l->l_type	= (lrn_found->lrn_Shared ? F_RDLCK : F_WRLCK);
				l->l_whence	= SEEK_SET;
				l->l_start	= lrn_found->lrn_Start;
				l->l_len	= lrn_found->lrn_Length;
				l->l_pid	= lrn_found->lrn_Owner;

				SHOWVALUE(l->l_type);
				SHOWVALUE(l->l_whence);
				SHOWVALUE(l->l_start);
				SHOWVALUE(l->l_len);
				SHOWVALUE(l->l_pid);
			}
			else
			{
				SHOWMSG("there is no lock collision");

				l->l_type = F_UNLCK;
			}
		}
		else
		{
			if(error != ERROR_OBJECT_NOT_FOUND)
				goto out;

			SHOWMSG("nobody has locked this file");

			l->l_type = F_UNLCK;
		}
	}

	result = OK;

 out:

	delete_file_lock_node(fln);
	delete_locked_region_node(lrn);

	release_file_lock_semaphore(fls);

	PROFILE_OFF();
	UnLock(parent_dir);
	PROFILE_ON();

	if(result != OK && error != OK)
	{
		SetIoErr(error);

		(*error_ptr) = __translate_io_error_to_errno(error);
	}

	RETURN(result);
	return(result);
}

/****************************************************************************/

#endif /* UNIX_PATH_SEMANTICS */

/****************************************************************************/

/* Seek to the end of a file, then add a certain number of 0 bytes. */
static int
grow_file_size(struct fd * fd,int num_bytes,int * error_ptr)
{
	unsigned char * aligned_buffer;
	unsigned char * buffer;
	struct FileHandle * fh;
	D_S(struct InfoData,id);
	LONG block_size;
	int bytes_written;
	int buffer_size;
	int size;
	int result = ERROR;
	int position;
	int current_position;
	int alignment_skip;

	assert( fd != NULL );
	assert( error_ptr != NULL );

	D(("we have to grow the file by %ld bytes",num_bytes));

	block_size = 0;

	PROFILE_OFF();

	fh = BADDR(fd->fd_DefaultFile);
	if(fh != NULL && fh->fh_Type != NULL && DoPkt(fh->fh_Type,ACTION_DISK_INFO,MKBADDR(id),0,0,0,0))
		block_size = id->id_BytesPerBlock;

	PROFILE_ON();

	if(block_size < 512)
		block_size = 512;

	/* We have to fill up the file with zero bytes.
	 * That data comes from a local buffer. How
	 * large can we make it?
	 */
	buffer_size = 8 * block_size;
	if(buffer_size > num_bytes)
		buffer_size = num_bytes;

	/* Allocate a little more memory than required to allow for
	 * the buffer to be aligned to a cache line boundary.
	 */
	buffer = malloc((size_t)buffer_size + (CACHE_LINE_SIZE-1));
	if(buffer == NULL)
	{
		SHOWMSG("not enough memory for write buffer");

		(*error_ptr) = ENOMEM;
		goto out;
	}

	/* Align the buffer to a cache line boundary. */
	aligned_buffer = (unsigned char *)(((ULONG)(buffer + (CACHE_LINE_SIZE-1))) & ~(CACHE_LINE_SIZE-1));

	memset(aligned_buffer,0,(size_t)buffer_size);

	PROFILE_OFF();

	position = Seek(fd->fd_DefaultFile,0,OFFSET_END);

	PROFILE_ON();

	if(position == -1)
	{
		SHOWMSG("could not move to the end of the file");

		(*error_ptr) = __translate_io_error_to_errno(IoErr());
		goto out;
	}

	PROFILE_OFF();

	current_position = Seek(fd->fd_DefaultFile,0,OFFSET_CURRENT);

	PROFILE_ON();

	/* Try to make the first write access align the file position
	 * to a block offset. Subsequent writes will then access the
	 * file at positions that are multiples of the block size.
	 */
	if(num_bytes > block_size && (current_position % block_size) != 0)
		alignment_skip = block_size - (current_position % block_size);
	else
		alignment_skip = 0;

	while(num_bytes > 0)
	{
		if(__check_abort_enabled)
			__check_abort();

		size = buffer_size;
		if(size > num_bytes)
			size = num_bytes;

		/* If possible, even out the block offset. */
		if(alignment_skip > 0 && size > alignment_skip)
			size = alignment_skip;

		alignment_skip = 0;

		PROFILE_OFF();

		bytes_written = Write(fd->fd_DefaultFile,aligned_buffer,size);

		PROFILE_ON();

		if(bytes_written != size)
		{
			(*error_ptr) = __translate_io_error_to_errno(IoErr());
			goto out;
		}

		num_bytes -= size;
	}

	SHOWMSG("all done.");

	result = OK;

 out:

	if(buffer != NULL)
		free(buffer);

	return(result);
}

/****************************************************************************/

static void
sync_fd(struct fd * fd,int mode)
{
	assert( fd != NULL );

	if(fd->fd_DefaultFile != ZERO)
	{
		/* The mode tells us what to flush. 0 means "flush just the data", and
		   everything else means "flush everything. */
		Flush(fd->fd_DefaultFile);

		if(mode != 0)
		{
			struct FileHandle * fh = BADDR(fd->fd_DefaultFile);

			/* Verify that this file is not bound to "NIL:". */
			if(fh->fh_Type != NULL)
				DoPkt(fh->fh_Type,ACTION_FLUSH,	0,0,0,0,0);
		}
	}
}

/****************************************************************************/

void
__fd_hook_entry(
	struct Hook *				UNUSED	unused_hook,
	struct fd *							fd,
	struct file_hook_message *			message)
{
	char * buffer = NULL;
	int result = -1;
	int error = OK;
	LONG position;

	ENTER();

	assert( message != NULL && fd != NULL );
	assert( __is_valid_fd(fd) );

	switch(message->action)
	{
		case file_hook_action_read:

			SHOWMSG("file_hook_action_read");

			if(fd->fd_DefaultFile == ZERO)
			{
				SHOWMSG("file is closed");

				error = EBADF;
				break;
			}

			assert( message->data != NULL );
			assert( message->size > 0 );

			D(("read %ld bytes from position %ld to 0x%08lx",message->size,Seek(fd->fd_DefaultFile,0,OFFSET_CURRENT),message->data));

			PROFILE_OFF();

			result = Read(fd->fd_DefaultFile,message->data,message->size);

			PROFILE_ON();

			if(result < 0)
			{
				D(("read failed ioerr=%ld",IoErr()));

				error = __translate_io_error_to_errno(IoErr());
				break;
			}

			if(FLAG_IS_SET(fd->fd_Flags,FDF_CACHE_POSITION))
				fd->fd_Position += result;

			break;

		case file_hook_action_write:

			SHOWMSG("file_hook_action_write");

			if(fd->fd_DefaultFile == ZERO)
			{
				SHOWMSG("file is closed");

				error = EBADF;
				break;
			}

			assert( message->data != NULL );
			assert( message->size > 0 );

			if(FLAG_IS_SET(fd->fd_Flags,FDF_APPEND))
			{
				SHOWMSG("appending data");

				PROFILE_OFF();

				if(Seek(fd->fd_DefaultFile,0,OFFSET_END) >= 0)
				{
					if(FLAG_IS_SET(fd->fd_Flags,FDF_CACHE_POSITION))
						fd->fd_Position = Seek(fd->fd_DefaultFile,0,OFFSET_CURRENT);
				}

				PROFILE_ON();
			}

			D(("write %ld bytes to position %ld from 0x%08lx",message->size,Seek(fd->fd_DefaultFile,0,OFFSET_CURRENT),message->data));

			PROFILE_OFF();

			result = Write(fd->fd_DefaultFile,message->data,message->size);

			PROFILE_ON();

			if(result < 0)
			{
				D(("write failed ioerr=%ld",IoErr()));

				error = __translate_io_error_to_errno(IoErr());
				break;
			}

			if(FLAG_IS_SET(fd->fd_Flags,FDF_CACHE_POSITION))
				fd->fd_Position += result;

			break;

		/* This one implements Unix-like lseek() functionality. We report the
		 * new file position upon return and will try to extend the size of a
		 * file if the new seek position lies beyond the end of the file.
		 */
		case file_hook_action_seek_and_extend:
		case file_hook_action_seek:

			SHOWMSG("file_hook_action_seek(_and_extend)");

			if(fd->fd_DefaultFile != ZERO)
			{
				D_S(struct FileInfoBlock,fib);
				BOOL fib_is_valid = FALSE;
				LONG current_position;
				LONG new_position;
				LONG mode;

				assert( message->mode == SEEK_CUR ||
				        message->mode == SEEK_SET ||
				        message->mode == SEEK_END );

				if(message->mode == SEEK_CUR)
					mode = OFFSET_CURRENT;
				else if (message->mode == SEEK_SET)
					mode = OFFSET_BEGINNING;
				else
					mode = OFFSET_END;

				D(("seek&extended to offset %ld, mode %ld; current position = %ld",message->position,mode,Seek(fd->fd_DefaultFile,0,OFFSET_CURRENT)));

				if(FLAG_IS_SET(fd->fd_Flags,FDF_CACHE_POSITION))
				{
					current_position = fd->fd_Position;
				}
				else
				{
					PROFILE_OFF();

					current_position = Seek(fd->fd_DefaultFile,0,OFFSET_CURRENT);

					PROFILE_ON();

					if(current_position < 0)
					{
						error = EBADF;
						break;
					}
				}

				new_position = current_position;

				switch(mode)
				{
					case OFFSET_CURRENT:

						new_position += message->position;
						break;

					case OFFSET_BEGINNING:

						new_position = message->position;
						break;

					case OFFSET_END:

						if(safe_examine_file_handle(fd->fd_DefaultFile,fib))
						{
							new_position = fib->fib_Size + message->position;

							fib_is_valid = TRUE;
						}

						break;
				}

				if(new_position == current_position)
				{
					result = new_position;

					if(FLAG_IS_SET(fd->fd_Flags,FDF_CACHE_POSITION))
						fd->fd_Position = new_position;
				}
				else
				{
					PROFILE_OFF();

					position = Seek(fd->fd_DefaultFile,message->position,mode);

					PROFILE_ON();

					if(position >= 0)
					{
						if(FLAG_IS_SET(fd->fd_Flags,FDF_CACHE_POSITION))
							fd->fd_Position = new_position;

						result = new_position;
					}
					else
					{
						D(("seek failed, mode=%ld (%ld), offset=%ld, ioerr=%ld",mode,message->mode,message->position,IoErr()));

						error = __translate_io_error_to_errno(IoErr());

						#if defined(UNIX_PATH_SEMANTICS)
						{
							/* Check if the file would have become larger as a
							 * result of this action.
							 */
							if(message->action != file_hook_action_seek_and_extend)
								break;

							if(NOT fib_is_valid && CANNOT safe_examine_file_handle(fd->fd_DefaultFile,fib))
								break;

							if(new_position <= fib->fib_Size)
								break;

							if(grow_file_size(fd,new_position - fib->fib_Size,&error) != OK)
								break;

							if(FLAG_IS_SET(fd->fd_Flags,FDF_CACHE_POSITION))
								fd->fd_Position = new_position;

							result = new_position;
						}
						#endif /* UNIX_PATH_SEMANTICS */
					}
				}
			}
			else
			{
				SHOWMSG("file is closed");

				error = EBADF;
			}

			break;

		case file_hook_action_close:

			SHOWMSG("file_hook_action_close");

			if(fd->fd_DefaultFile != ZERO)
			{
				BOOL name_and_path_valid = FALSE;
				D_S(struct FileInfoBlock,fib);
				BPTR parent_dir;

				memset(fib,0,sizeof(*fib));

				#if defined(UNIX_PATH_SEMANTICS)
				{
					cleanup_locked_records(fd);
				}
				#endif /* UNIX_PATH_SEMANTICS */

				PROFILE_OFF();

				parent_dir = safe_parent_of_file_handle(fd->fd_DefaultFile);
				if(parent_dir != ZERO)
				{
					if(safe_examine_file_handle(fd->fd_DefaultFile,fib))
						name_and_path_valid = TRUE;
				}

				if(CANNOT Close(fd->fd_DefaultFile))
					error = __translate_io_error_to_errno(IoErr());

				PROFILE_ON();

				fd->fd_DefaultFile = ZERO;

				#if defined(UNIX_PATH_SEMANTICS)
				{
					DECLARE_UTILITYBASE();

					assert( UtilityBase != NULL );

					/* Now that we have closed this file, know where it is and what its
					 * name would be, check if we tried to unlink it earlier. If we did,
					 * we'll try to finish the job here and now.
					 */
					if(name_and_path_valid)
					{
						struct UnlinkNode * node;
						struct UnlinkNode * uln_next;
						struct UnlinkNode * uln;
						BOOL file_deleted = FALSE;

						assert( __unlink_list.mlh_Head != NULL );

						/* Check all files to be unlinked when this program exits. */
						for(uln = (struct UnlinkNode *)__unlink_list.mlh_Head ;
						    (uln_next = (struct UnlinkNode *)uln->uln_MinNode.mln_Succ) != NULL ;
						    uln = uln_next)
						{
							node = NULL;

							/* If the file name matches, check if the path matches, too. */
							if(Stricmp(FilePart(uln->uln_Name),fib->fib_FileName) == SAME)
							{
								BPTR old_dir;
								BPTR node_lock;
								BPTR path_lock = ZERO;

								PROFILE_OFF();

								/* Try to get a lock on the file first, then move on to
								 * the directory it is stored in.
								 */
								old_dir = CurrentDir(uln->uln_Lock);

								node_lock = Lock(uln->uln_Name,SHARED_LOCK);
								if(node_lock != ZERO)
								{
									path_lock = ParentDir(node_lock);

									UnLock(node_lock);
								}

								CurrentDir(old_dir);

								/* If we found the file's parent directory, check if it matches
								 * the parent directory of the file we just closed.
								 */
								if(path_lock != ZERO)
								{
									if(SameLock(path_lock,parent_dir) == LOCK_SAME)
										node = uln;

									UnLock(path_lock);
								}

								PROFILE_ON();
							}

							/* If we found that this file was set up for deletion,
							 * delete it here and now.
							 */
							if(node != NULL)
							{
								if(NOT file_deleted)
								{
									BPTR old_dir;

									PROFILE_OFF();

									old_dir = CurrentDir(parent_dir);

									if(DeleteFile(fib->fib_FileName))
									{
										file_deleted		= TRUE;
										name_and_path_valid	= FALSE;
									}

									CurrentDir(old_dir);

									PROFILE_ON();
								}

								if(file_deleted)
								{
									Remove((struct Node *)node);
									free(node);
								}
							}
						}
					}
				}
				#endif /* UNIX_PATH_SEMANTICS */

				if(FLAG_IS_SET(fd->fd_Flags,FDF_CREATED) && name_and_path_valid)
				{
					ULONG flags;
					BPTR old_dir;

					PROFILE_OFF();

					old_dir = CurrentDir(parent_dir);

					flags = fib->fib_Protection ^ (FIBF_READ|FIBF_WRITE|FIBF_EXECUTE|FIBF_DELETE);

					CLEAR_FLAG(flags,FIBF_EXECUTE);
					CLEAR_FLAG(flags,FIBF_OTR_EXECUTE);
					CLEAR_FLAG(flags,FIBF_GRP_EXECUTE);

					SetProtection(fib->fib_FileName,(LONG)(flags ^ (FIBF_READ|FIBF_WRITE|FIBF_EXECUTE|FIBF_DELETE)));

					CurrentDir(old_dir);

					PROFILE_ON();
				}

				PROFILE_OFF();
				UnLock(parent_dir);
				PROFILE_ON();

				result = 0;
			}
			else
			{
				SHOWMSG("file is closed");

				error = EBADF;
			}

			break;

		case file_hook_action_lock_record:

			SHOWMSG("file_hook_action_lock_record");

			#if defined(UNIX_PATH_SEMANTICS)
			{
				SHOWMSG("performing record locking");

				if(fd->fd_DefaultFile == ZERO)
				{
					SHOWMSG("file is closed");

					error = EBADF;
					break;
				}

				result = handle_record_locking(message->command,message->lock,fd,&error);
			}
			#else
			{
				SHOWMSG("no record locking support in this library");

				error = EBADF;
			}
			#endif /* UNIX_PATH_SEMANTICS */

			break;

		case file_hook_action_set_blocking:

			SHOWMSG("file_hook_action_set_blocking");

			PROFILE_OFF();

			if(FLAG_IS_SET(fd->fd_Flags,FDF_IS_INTERACTIVE))
			{
				LONG mode;

				SHOWMSG("changing the mode");

				if(message->arg != 0)
					mode = DOSFALSE;	/* buffered mode */
				else
					mode = DOSTRUE;		/* single character mode */

				if(CANNOT SetMode(fd->fd_DefaultFile,mode))
				{
					error = EBADF;
					break;
				}

				result = 0;
			}
			else
			{
				SHOWMSG("can't do anything here");

				error = EBADF;
			}

			PROFILE_ON();

			break;

		case file_hook_action_change_owner:

			SHOWMSG("file_hook_action_change_owner");

			if(message->owner > 65535 || message->group > 65535)
			{
				SHOWMSG("owner or group not OK");

				SHOWVALUE(message->owner);
				SHOWVALUE(message->group);

				error = EINVAL;
				break;
			}

			if(fd->fd_DefaultFile != ZERO)
			{
				D_S(struct FileInfoBlock,fib);

				PROFILE_OFF();

				if(safe_examine_file_handle(fd->fd_DefaultFile,fib))
				{
					BPTR parent_dir;

					parent_dir = safe_parent_of_file_handle(fd->fd_DefaultFile);
					if(parent_dir != ZERO)
					{
						BPTR old_current_dir;

						old_current_dir = CurrentDir(parent_dir);

						#if defined(__amigaos4__)
						{
							if(SetOwner(fib->fib_FileName,(LONG)((((ULONG)message->owner) << 16) | message->group)))
							{
								result = 0;
							}
							else
							{
								SHOWMSG("that didn't work");

								error = __translate_io_error_to_errno(IoErr());
							}
						}
						#else
						{
							if(((struct Library *)DOSBase)->lib_Version >= 39)
							{
								SHOWMSG("changing owner");

								if(SetOwner(fib->fib_FileName,(LONG)((((ULONG)message->owner) << 16) | message->group)))
								{
									result = 0;
								}
								else
								{
									SHOWMSG("that didn't work");

									error = __translate_io_error_to_errno(IoErr());
								}
							}
							else
							{
								D_S(struct bcpl_name,new_name);
								struct DevProc * dvp;
								unsigned int len;

								SHOWMSG("have to do this manually...");

								len = strlen(fib->fib_FileName);

								assert( len < sizeof(new_name->name) );

								dvp = GetDeviceProc(fib->fib_FileName,NULL);
								if(dvp != NULL)
								{
									new_name->name[0] = len;
									memmove(&new_name->name[1],fib->fib_FileName,len);

									if(DoPkt(dvp->dvp_Port,ACTION_SET_OWNER,dvp->dvp_Lock,MKBADDR(new_name),(LONG)((((ULONG)message->owner) << 16) | message->group),0,0))
										result = 0;
									else
										error = __translate_io_error_to_errno(IoErr());

									FreeDeviceProc(dvp);
								}
								else
								{
									error = __translate_io_error_to_errno(IoErr());
								}
							}
						}
						#endif /* __amigaos4__ */

						CurrentDir(old_current_dir);

						UnLock(parent_dir);
					}
					else
					{
						SHOWMSG("couldn't find parent directory");
	
						error = __translate_io_error_to_errno(IoErr());
					}
				}
				else
				{
					SHOWMSG("couldn't examine file handle");

					error = __translate_io_error_to_errno(IoErr());
				}

				PROFILE_ON();
			}
			else
			{
				SHOWMSG("file is closed");

				error = EBADF;
			}

			break;

		case file_hook_action_truncate:

			SHOWMSG("file_hook_action_truncate");

			if(fd->fd_DefaultFile != ZERO)
			{
				D_S(struct FileInfoBlock,fib);

				SHOWMSG("trying to change file size");

				if(CANNOT safe_examine_file_handle(fd->fd_DefaultFile,fib))
				{
					error = __translate_io_error_to_errno(IoErr());
					break;
				}

				PROFILE_OFF();

				if(message->size < fib->fib_Size)
				{
					/* Careful: seek to a position where the file can be safely truncated. */
					if(Seek(fd->fd_DefaultFile,message->size,OFFSET_BEGINNING) != -1 && SetFileSize(fd->fd_DefaultFile,message->size,OFFSET_BEGINNING) != -1)
					{
						if(FLAG_IS_SET(fd->fd_Flags,FDF_CACHE_POSITION))
							fd->fd_Position = message->size;

						result = 0;
					}
					else
					{
						error = __translate_io_error_to_errno(IoErr());
					}
				}
				else if (message->size > fib->fib_Size)
				{
					if(Seek(fd->fd_DefaultFile,fib->fib_Size,OFFSET_BEGINNING) != -1 && grow_file_size(fd,message->size - fib->fib_Size,&error) == OK)
					{
						if(FLAG_IS_SET(fd->fd_Flags,FDF_CACHE_POSITION))
							fd->fd_Position = message->size;

						result = 0;
					}
					else
					{
						error = __translate_io_error_to_errno(IoErr());
					}
				}
				else
				{
					/* No action */
					result = 0;
				}

				PROFILE_ON();
			}
			else
			{
				SHOWMSG("file is already closed");

				error = EBADF;
			}

			break;

		case file_hook_action_examine:

			SHOWMSG("file_hook_action_examine");

			if(fd->fd_DefaultFile != ZERO)
			{
				struct FileHandle * fh;

				if(CANNOT safe_examine_file_handle(fd->fd_DefaultFile,message->file_info))
				{
					SHOWMSG("couldn't examine the file");

					error = __translate_io_error_to_errno(IoErr());
					break;
				}

				fh = BADDR(fd->fd_DefaultFile);

				message->file_system = fh->fh_Type;

				result = 0;
			}
			else
			{
				SHOWMSG("file is already closed");

				error = EBADF;
			}

			break;

		case file_hook_action_info:

			SHOWMSG("file_hook_action_info");

			if(fd->fd_DefaultFile != ZERO)
			{
				BPTR parent_dir;

				PROFILE_OFF();

				parent_dir = safe_parent_of_file_handle(fd->fd_DefaultFile);
				if(parent_dir != ZERO)
				{
					if(Info(parent_dir,message->info_data))
					{
						result = 0;
					}
					else
					{
						SHOWMSG("couldn't get info on drive");

						error = __translate_io_error_to_errno(IoErr());
					}

					UnLock(parent_dir);
				}
				else
				{
					SHOWMSG("couldn't find parent directory");

					error = __translate_io_error_to_errno(IoErr());
				}

				PROFILE_ON();
			}
			else
			{
				SHOWMSG("file is already closed");

				error = EBADF;
			}

			break;

		case file_hook_action_change_attributes:

			SHOWMSG("file_hook_action_change_attributes");

			if(fd->fd_DefaultFile != ZERO)
			{
				BPTR parent_dir;

				PROFILE_OFF();

				parent_dir = safe_parent_of_file_handle(fd->fd_DefaultFile);
				if(parent_dir != ZERO)
				{
					D_S(struct FileInfoBlock,fib);

					if(safe_examine_file_handle(fd->fd_DefaultFile,fib))
					{
						BPTR old_current_dir;

						old_current_dir = CurrentDir(parent_dir);

						if(SetProtection(fib->fib_FileName,(LONG)message->attributes))
						{
							result = 0;
						}
						else
						{
							SHOWMSG("couldn't change protection bits; oh well, it was worth the effort...");

							error = __translate_io_error_to_errno(IoErr());
						}

						CurrentDir(old_current_dir);
					}
					else
					{
						SHOWMSG("couldn't examine file");

						error = __translate_io_error_to_errno(IoErr());
					}

					UnLock(parent_dir);
				}
				else
				{
					SHOWMSG("couldn't get a lock on the parent directory");

					error = __translate_io_error_to_errno(IoErr());
				}

				PROFILE_ON();
			}
			else
			{
				SHOWMSG("file is already closed");

				error = EBADF;
			}

			break;

		case file_hook_action_duplicate_fd:

			SHOWMSG("file_hook_action_duplicate_fd");

			__duplicate_fd(message->duplicate_fd,fd);

			result = 0;
			break;

		case file_hook_action_flush:

			SHOWMSG("file_hook_action_flush");

			sync_fd(fd,message->arg);

			result = 0;
			break;

		default:

			SHOWVALUE(message->action);

			error = EBADF;
			break;
	}

	if(buffer != NULL)
		free(buffer);

	SHOWVALUE(result);

	message->result	= result;
	message->error	= error;

	LEAVE();
}
