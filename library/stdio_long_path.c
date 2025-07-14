/*
 * :ts=4
 *
 * Portable ISO 'C' (1994) runtime library for the Amiga computer
 * Copyright (c) 2002-2023 by Olaf Barthel <obarthel (at) gmx.net>
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

#ifndef __amigaos4__

/****************************************************************************/

/* We don't want the replacement functions to become active when buliding
 * this file.
 */
#define __DISABLE_LONG_PATHS

#ifndef _STDIO_LONG_PATH_H
#include "stdio_long_path.h"
#endif /* _STDIO_LONG_PATH_H */

/****************************************************************************/

#include <proto/exec.h>
#include <proto/dos.h>

#include <string.h>

/****************************************************************************/

/* Split an AmigaDOS path name into a sequence of the individual path
 * pieces. A path name may be empty, it may include a device name,
 * a series of directory names or even "/" references to the
 * parent directory and a file, directory or link name.
 *
 * split_path() is modeled on the SplitName() function in that it
 * will copy each path piece into the supplied buffer, returning -1
 * if the last piece has been copied. If -2 is returned, it means
 * that the piece buffer is too short for the path piece to be
 * copied.
 */
static LONG
split_path(CONST TEXT * path, TEXT * piece, LONG old_position, LONG size)
{
	LONG length_copied = 0;
	TEXT c;

	/* The piece has to be NUL-terminated, which reduces the
	 * usable size of the piece buffer.
	 */
	size--;

	/* Is the piece buffer too short? */
	if (size <= 0)
		return -2;

	/* Is this not an empty string, which stands for the
	 * current directory?
	 */
	if (path[0] != '\0')
	{
		/* Adjust the starting position for scanning the path name,
		 * which simplifies the loop below.
		 */
		path += old_position;

		while (size-- >= 0)
		{
			c = path[length_copied];

			/* Was this the end of the path name? */
			if (c == '\0')
				break;

			(*piece++) = c;
			length_copied++;

			/* Is this a device name separator, which may appear
			 * exactly once in a path name? Then we will have to
			 * keep it.
			 */
			if (c == ':' && old_position == 0)
			{
				break;
			}
			/* Is there a '/' at the end of this path piece? */
			else if (c == '/')
			{
				/* Is this in fact a single '/' which represents a
				 * reference to the parent directory? Then we need
				 * to keep it, otherwise it will have to be removed.
				 */
				if (length_copied > 1)
				{
					/* Make sure that the trailing '/' will
					 * be removed from the piece.
					 */
					piece--;
					size++;
				}

				break;
			}
		}

		/* Is the piece buffer too short? */
		if (size < 0)
			return -2;
	}

	/* Make sure that the piece is NUL-terminated. */
	(*piece) = '\0';

	/* Is the separator character at the end of the path name,
	 * or is this an empty string (with path_length == 0)?
	 */
	if (path[length_copied] == '\0')
		return -1; /* This is the last piece of the path name. */

	/* Continue path processing at this position. */
	return old_position + length_copied;
}

/****************************************************************************/

/* If a path name exceeds 255 characters, break down the path into individual
 * pieces, traveling to the end of the path and invoke a callback function
 * with the current directory set and the name of the file/directory/link
 * provided along with additional parameters needed to carry out a file
 * system operation.
 *
 * For path names shorter than 256 characters, no extra work will be done
 * and the callback function will be invoked directly.
 */

typedef LONG (*callback_f)(CONST TEXT * name, LONG arg1, LONG arg2);

static void
long_path_workaround(
	CONST TEXT * path_name, LONG arg1, LONG arg2,
	LONG * result_ptr,
	callback_f callback)
{
	TEXT piece[256];
	LONG path_name_len;
	LONG len;
	CONST TEXT * colon;
	BPTR reference_dir = (BPTR)NULL;
	BPTR old_current_dir = (BPTR)NULL;
	LONG position = 0;
	BPTR lock;

	path_name_len = strlen(path_name);

	/* Only go through with the extra effort to resolve the path
	 * if this job requires a file system and not a handler (such
	 * as "NIL:" or "CON:") and if the path name exceeds 255
	 * characters.
	 */
	if (path_name_len > 255 && IsFileSystem(path_name))
	{
		/* Does the path begin with a device name, or a reference
		 * to the root directory of a volume?
		 */
		colon = strchr(path_name, ':');
		if (colon != NULL)
		{
			/* We need to copy the name of the device, which must
			 * not exceed 255 characters, including the colon
			 * character. Volume and device names should never
			 * exceed 30 characters, but accidents may still
			 * happen...
			 */
			len = (LONG)(colon - path_name) + 1;
			if (len > 255)
			{
				SetIoErr(ERROR_LINE_TOO_LONG);
				goto out;
			}

			/* Obtain a shared lock on the device or root
			 * directory. We are using Lock() because it
			 * implies that the handler is a file system
			 * and not, for example, "NIL:" or "CON:".
			 */
			CopyMem(path_name, piece, len);
			piece[len] = '\0';

			reference_dir = Lock(piece, SHARED_LOCK);
			if (reference_dir == (BPTR)NULL)
				goto out;

			/* We took care of the device or root directory name. */
			path_name += len;
		}

		/* Process the path name, one piece at a time, relative
		 * to a Lock on its parent directory. This repeats
		 * until the last piece of the path has been found.
		 * Each loop iteration will reuse the Lock of the
		 * previous iteration as the parent directory.
		 */
		while (TRUE)
		{
			/* Obtain the next piece of the path. */
			position = split_path(path_name, piece, position, sizeof(piece));

			/* Is the piece too long? */
			if (position == -2)
			{
				SetIoErr(ERROR_LINE_TOO_LONG);
				goto out;
			}

			/* Access the next piece of the path relative to a
			 * specific directory? Otherwise, we use the current
			 * directory of the current Process. Because the current
			 * directory may be an exclusive lock we cannot
			 * conveniently fall back onto DupLock() or
			 * Lock("", SHARED_LOCK) instead.
			 */
			if (reference_dir != (BPTR)NULL)
				old_current_dir = CurrentDir(reference_dir);

			/* Unless this is the final piece of the path
			 * (split_path() will return -1), we will need to call
			 * Lock() and make use of shared locks.
			 */
			if (position != -1)
			{
				lock = Lock(piece, SHARED_LOCK);
			}
			else
			{
				lock = (BPTR)NULL;

				/* Time to do what we came here for. */
				(*result_ptr) = (*callback)(piece, arg1, arg2);
			}

			if (reference_dir != (BPTR)NULL)
				CurrentDir(old_current_dir);

			/* Stop as soon as we reach the end of the path or
			 * if the Lock could not be obtained.
			 */
			if (position == -1 || lock == (BPTR)NULL)
				break;

			/* Look at the next piece of the remaining path.
			 * Free the current reference directory Lock and
			 * replace it with the Lock obtained above.
			 */
			UnLock(reference_dir);
			reference_dir = lock;
		}
	}
	else
	{
		(*result_ptr) = (*callback)(path_name, arg1, arg2);
	}

 out:

	UnLock(reference_dir);
}

/****************************************************************************/

static LONG
lock_callback(CONST TEXT * name, LONG type)
{
	return Lock(name, type);
}

BPTR
__new_lock(CONST TEXT * name, LONG type )
{
	BPTR result = (BPTR)NULL;

	long_path_workaround(name, type, 0, (LONG *)&result, (callback_f)lock_callback);

	return result;
}

/****************************************************************************/

static LONG
set_owner_callback(CONST TEXT * name, LONG owner_info)
{
	return SetOwner(name, owner_info);
}

LONG
__new_set_owner(CONST TEXT * name, LONG owner_info)
{
	LONG success = FALSE;

	long_path_workaround(name, owner_info, 0, &success, (callback_f)set_owner_callback);

	return success;
}

/****************************************************************************/

static LONG
set_comment_callback(CONST TEXT * name, LONG comment)
{
	return SetComment(name, (STRPTR)comment);
}

LONG
__new_set_comment(CONST TEXT * name, CONST TEXT * comment)
{
	LONG success = FALSE;

	long_path_workaround(name, (LONG)comment, 0, &success, (callback_f)set_comment_callback);

	return success;
}

/****************************************************************************/

static LONG
set_protect_callback(CONST TEXT * name, LONG protect)
{
	return SetProtection(name, protect);
}

LONG
__new_set_protection(CONST TEXT * name, LONG protect)
{
	LONG success = FALSE;

	long_path_workaround(name, protect, 0, &success, (callback_f)set_protect_callback);

	return success;
}

/****************************************************************************/

static LONG
set_file_date_callback(CONST TEXT * name, LONG date)
{
	return SetFileDate(name, (struct DateStamp *)date);
}

LONG
__new_set_file_date(CONST TEXT * name, CONST struct DateStamp * date)
{
	LONG success = FALSE;

	long_path_workaround(name, (LONG)date, 0, &success, (callback_f)set_file_date_callback);

	return success;
}

/****************************************************************************/

static LONG
open_callback(CONST TEXT * name, LONG mode)
{
	return Open(name, mode);
}

BPTR
__new_open(CONST TEXT * name, LONG mode)
{
	BPTR file = (BPTR)NULL;

	long_path_workaround(name, mode, 0, (LONG *)&file, (callback_f)open_callback);

	return file;
}

/****************************************************************************/

static LONG
delete_file_callback(CONST TEXT * name)
{
	return DeleteFile(name);
}

LONG
__new_delete_file(CONST TEXT * name)
{
	LONG successful = FALSE;

	long_path_workaround(name, 0, 0, &successful, (callback_f)delete_file_callback);

	return successful;
}

/****************************************************************************/

static LONG
create_dir_callback(CONST TEXT * name)
{
	return CreateDir(name);
}

BPTR
__new_create_dir(CONST TEXT * name)
{
	BPTR lock = (BPTR)NULL;

	long_path_workaround(name, 0, 0, (LONG *)&lock, (callback_f)create_dir_callback);

	return lock;
}

/****************************************************************************/

static LONG
make_link_callback(CONST TEXT * name, LONG dest, LONG soft)
{
	return MakeLink(name, dest, soft);
}

LONG
__new_make_link(CONST TEXT * name, LONG dest, LONG soft)
{
	LONG success = FALSE;

	long_path_workaround(name, dest, soft, (LONG *)&success, (callback_f)make_link_callback);

	return success;
}

/****************************************************************************/

struct rename_arg
{
	BPTR dir_lock;
	TEXT name[256+4];
};

static struct rename_arg *
create_rename_arg(void)
{
	struct rename_arg * arg;

	arg = AllocMem(sizeof(*arg), MEMF_PUBLIC);
	if (arg != NULL)
		arg->dir_lock = (BPTR)NULL;

	return arg;
}

static void
delete_rename_arg(struct rename_arg * arg)
{
	if (arg != NULL)
	{
		UnLock(arg->dir_lock);
		FreeMem(arg, sizeof(*arg));
	}
}

static LONG
rename_callback(CONST TEXT * name, struct rename_arg * arg)
{
	BOOL successful = FALSE;
	LONG name_len;
	BPTR old_lock;

	old_lock = CurrentDir((BPTR)NULL);
	CurrentDir(old_lock);

	if (old_lock != (BPTR)NULL)
	{
		arg->dir_lock = DupLock(old_lock);
		if (arg->dir_lock != (BPTR)NULL)
			successful = TRUE;
	}
	else
	{
		successful = TRUE;
	}

	if (successful)
	{
		name_len = strlen(name);
		if (name_len > 255)
			name_len = 255;

		arg->name[0] = name_len;
		CopyMem(name, &arg->name[1], name_len);
		arg->name[1+name_len] = '\0';
	}

	return successful;
}

LONG
__new_rename(CONST TEXT * oldName, CONST TEXT * newName)
{
	struct rename_arg * from = NULL;
	struct rename_arg * to = NULL;
	struct MsgPort * filesys_port;
	LONG successful = FALSE;

	if (IsFileSystem(oldName) && IsFileSystem(newName) && (strlen(oldName) > 255 || strlen(newName) > 255))
	{
		from = create_rename_arg();
		if (from == NULL)
			goto out;

		to = create_rename_arg();
		if (to == NULL)
			goto out;

		long_path_workaround(oldName, (LONG)from, 0, (LONG *)&successful, (callback_f)rename_callback);
		if (successful == FALSE)
			goto out;

		long_path_workaround(newName, (LONG)to, 0, (LONG *)&successful, (callback_f)rename_callback);
		if (successful == FALSE)
			goto out;

		if (from->dir_lock == (BPTR)NULL && to->dir_lock == (BPTR)NULL)
		{
			filesys_port = GetFileSysTask();
		}
		else
		{
			if (from->dir_lock == (BPTR)NULL || to->dir_lock == (BPTR)NULL ||
			    SameLock(from->dir_lock, to->dir_lock) == LOCK_DIFFERENT)
			{
				SetIoErr(ERROR_RENAME_ACROSS_DEVICES);

				successful = FALSE;
				goto out;
			}

			filesys_port = ((struct FileLock *)BADDR(from->dir_lock))->fl_Task;
		}

		successful = DoPkt4(filesys_port, ACTION_RENAME_OBJECT,
			from->dir_lock, MKBADDR(from->name),
			to->dir_lock, MKBADDR(to->name));
	}
	else
	{
		successful = Rename(oldName, newName);
	}

 out:

	delete_rename_arg(from);
	delete_rename_arg(to);

	return successful;
}

/****************************************************************************/

#endif /* __amigaos4__ */
