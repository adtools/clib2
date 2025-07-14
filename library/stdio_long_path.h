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

#ifndef _STDIO_LONG_PATH_H
#define _STDIO_LONG_PATH_H

/****************************************************************************/

/* This option enables support for long path names under Kickstart 2.04
 * and beyond. Because this workaround is not needed for AmigaOS4, it will
 * be ignored if you try to enable it.
 *
 * Caution: Enable this feature only here in the "stdio_headers.h" file,
 *          never as a compiler option. This is to make sure that all the
 *          source code which makes use of this feature is rebuilt when
 *          you change this file.
 */
/*#define __USE_LONG_PATHS*/

/****************************************************************************/

#ifndef __amigaos4__

/****************************************************************************/

#ifndef PROTO_DOS_H
#include <proto/dos.h>
#endif /* PROTO_DOS_H */

/****************************************************************************/

extern BPTR __new_lock(CONST TEXT * name, LONG type );
extern LONG __new_set_owner(CONST TEXT * name, LONG owner_info);
extern LONG __new_set_comment(CONST TEXT * name, CONST TEXT * comment);
extern LONG __new_set_protection(CONST TEXT * name, LONG protect);
extern LONG __new_set_file_date(CONST TEXT * name, CONST struct DateStamp * date);
extern BPTR __new_open(CONST TEXT * name, LONG mode);
extern LONG __new_delete_file(CONST TEXT * name);
extern BPTR __new_create_dir(CONST TEXT * name);
extern LONG __new_make_link(CONST TEXT * name, LONG dest, LONG soft);
extern LONG __new_rename(CONST TEXT * oldName, CONST TEXT * newName);

/****************************************************************************/

/* We don't want the replacement functions to become active when building
 * the "stdio_long_path.c" file.
 */
#if ! defined(__DISABLE_LONG_PATHS) && defined(__USE_LONG_PATHS)

/****************************************************************************/

#if defined(Lock)
#undef Lock
#endif /* Lock */

#define Lock(name, type) __new_lock((name), (type))

/****************************************************************************/

#if defined(SetOwner)
#undef SetOwner
#endif /* SetOwner */

#define SetOwner(name, owner_info) __new_set_owner((name), (owner_info))

/****************************************************************************/

#if defined(SetComment)
#undef SetComment
#endif /* SetComment */

#define SetComment(name, comment) __new_set_comment((name), (comment))

/****************************************************************************/

#if defined(SetProtection)
#undef SetProtection
#endif /* SetProtection */

#define SetProtection(name, protect) __new_set_protection((name), (protect))

/****************************************************************************/

#if defined(SetFileDate)
#undef SetFileDate
#endif /* SetFileDate */

#define SetFileDate(name, date) __new_set_file_date((name), (date))

/****************************************************************************/

#if defined(Open)
#undef Open
#endif /* Open */

#define Open(name, mode) __new_open((name), (mode))

/****************************************************************************/

#if defined(DeleteFile)
#undef DeleteFile
#endif /* DeleteFile */

#define DeleteFile(name) __new_delete_file(name)

/****************************************************************************/

#if defined(CreateDir)
#undef CreateDir
#endif /* CreateDir */

#define CreateDir(name) __new_create_dir(name)

/****************************************************************************/

#if defined(MakeLink)
#undef MakeLink
#endif /* MakeLink */

#define MakeLink(name, dest, soft) __new_make_link((name), (dest), (soft))

/****************************************************************************/

#if defined(Rename)
#undef Rename
#endif /* Rename */

#define Rename(oldName, newName) __new_rename((oldName), (newName))

/****************************************************************************/

#endif /* !__DISABLE_LONG_PATHS && __USE_LONG_PATHS */

/****************************************************************************/

#endif /* __amigaos4__ */

/****************************************************************************/

#endif /* _STDIO_LONG_PATH_H */
