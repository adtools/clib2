/*
 * $Id: dirent_rewinddir.c,v 1.2 2004-08-07 09:15:32 obarthel Exp $
 *
 * :ts=4
 *
 * Portable ISO 'C' (1994) runtime library for the Amiga computer
 * Copyright (c) 2002-2004 by Olaf Barthel <olsen@sourcery.han.de>
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

#ifndef _DIRENT_HEADERS_H
#include "dirent_headers.h"
#endif /* _DIRENT_HEADERS_H */

/****************************************************************************/

/* The following is not part of the ISO 'C' (1994) standard. */

/****************************************************************************/

void
rewinddir(DIR * directory_pointer)
{
	ENTER();

	SHOWPOINTER(directory_pointer);

	if(__check_abort_enabled)
		__check_abort();

	PROFILE_OFF();

	if(directory_pointer != NULL)
	{
		struct DirectoryHandle * dh;

		dh = (struct DirectoryHandle *)directory_pointer;

		dh->dh_Position = 0;

		if(dh->dh_ScanVolumeList)
		{
			SHOWMSG("returning to first volume");

			dh->dh_VolumeNode = NULL;
		}
		else
		{
			SHOWMSG("calling Examine() again");

			assert( (((ULONG)&dh->dh_FileInfo) & 3) == 0 );

			if(CANNOT Examine(dh->dh_DirLock,&dh->dh_FileInfo))
			{
				SHOWMSG("ouch. that didn't work");
				__translate_io_error_to_errno(IoErr(),&errno);
			}
		}
	}

	PROFILE_ON();

	LEAVE();
}
