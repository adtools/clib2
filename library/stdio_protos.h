/*
 * $Id: stdio_protos.h,v 1.2 2004-08-08 10:55:57 obarthel Exp $
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

#ifndef _STDIO_PROTOS_H
#define _STDIO_PROTOS_H

/****************************************************************************/

#ifndef UTILITY_HOOKS_H
#include <utility/hooks.h>
#endif /* UTILITY_HOOKS_H */

/****************************************************************************/

#ifndef _STDIO_H
#include <stdio.h>
#endif /* _STDIO_H */

/****************************************************************************/

#ifndef _MATH_FP_SUPPORT_H
#include "math_fp_support.h"
#endif /* _MATH_FP_SUPPORT_H */

/****************************************************************************/

#ifndef _STDIO_HEADERS_H

/* Forward declarations */
struct file_hook_message;
struct iob;
struct fd;

#endif /* _STDIO_HEADERS_H */

/****************************************************************************/

/* stdio_translateioerror.c */
extern void __translate_io_error_to_errno(LONG io_error,int * errno_ptr);
extern void __translate_access_io_error_to_errno(LONG io_error,int * errno_ptr);

/****************************************************************************/

/* stdio_get_file_descriptor.c */
extern struct fd * __get_file_descriptor(int file_descriptor);

/****************************************************************************/

/* stdio_iobhookentry.c */
extern void __iob_hook_entry(struct Hook * hook,struct iob * iob,struct file_hook_message * message);

/****************************************************************************/

/* stdio_fdhookentry.c */
extern void __fd_hook_entry(struct Hook * hook,struct fd * fd,struct file_hook_message * message);

/****************************************************************************/

/* stdio_initializefd.c */
extern void __initialize_fd(struct fd * fd,HOOKFUNC hook_function,BPTR default_file,ULONG flags);

/****************************************************************************/

/* stdio_findvacantfdentry.c */
extern BOOL __is_valid_fd(struct fd * fd);
extern int __find_vacant_fd_entry(void);

/****************************************************************************/

/* stdio_initializeiob.c */
extern void __initialize_iob(struct iob * iob,HOOKFUNC hook_function,STRPTR custom_buffer,STRPTR buffer,int buffer_size,int file_descriptor,int slot_number,ULONG flags);

/****************************************************************************/

/* stdio_findvacantiobentry.c */
extern BOOL __is_valid_iob(struct iob * iob);
extern int __find_vacant_iob_entry(void);

/****************************************************************************/

/* stdio_growfdtable.c */
extern int __grow_fd_table(void);

/****************************************************************************/

/* stdio_growiobtable.c */
extern int __grow_iob_table(void);

/****************************************************************************/

/* stdio_openiob.c */
extern int __open_iob(const char *filename, const char *mode, int file_descriptor, int slot_number);

/****************************************************************************/

/* stdio_filliobreadbuffer.c */
extern int __fill_iob_read_buffer(struct iob * file);

/****************************************************************************/

/* stdio_dropiobreadbuffer.c */
extern int __drop_iob_read_buffer(struct iob * file);

/****************************************************************************/

/* stdio_flushiobwritebuffer.c */
extern int __flush_iob_write_buffer(struct iob * file);

/****************************************************************************/

/* stdio_vfscanf.c */
extern int __vfscanf(FILE *stream, const char *format, va_list arg);

/****************************************************************************/

/* fcntl_write.c */
extern ssize_t __write(int file_descriptor, const void * buffer, size_t num_bytes, int * error_ptr);

/****************************************************************************/

/* fcntl_lseek.c */
extern off_t __lseek(int file_descriptor, off_t offset, int mode, int * error_ptr);

/****************************************************************************/

/* fcntl_close.c */
extern int __close(int file_descriptor,int * error_ptr);

/****************************************************************************/

/* stdio_fgetc.c */
extern int __fgetc_check(FILE * stream);
extern int __fgetc(FILE *stream);

/****************************************************************************/

/* stdio_fputc.c */
extern int __fputc_check(FILE *stream);
extern int __fputc(int c,FILE *stream,int buffer_mode);

/****************************************************************************/

/* stdio_sscanf_hook_entry.c */
extern void __sscanf_hook_entry(struct Hook *UNUSED	unused_hook,struct iob *string,struct file_hook_message *message);

/****************************************************************************/

/* stdio_vasprintf_hook_entry.c */
extern void __vasprintf_hook_entry(struct Hook *UNUSED	unused_hook,struct iob *string,struct file_hook_message *message);

/****************************************************************************/

/* stdio_vsprintf_hook_entry.c */
extern void __vsprintf_hook_entry(struct Hook *UNUSED unused_hook,struct iob *string,struct file_hook_message *message);

/****************************************************************************/

/* stdio_vsnprintf_hook_entry.c */
extern void __vsnprintf_hook_entry(struct Hook *UNUSED unused_hook,struct iob *string,struct file_hook_message *message);

/****************************************************************************/

/* fcntl_read.c */
extern ssize_t __read(int file_descriptor, void * buffer, size_t num_bytes, int * error_ptr);

/****************************************************************************/

/* stdio_duplicate_fd.c */
extern void __duplicate_fd(struct fd * duplicate_fd,struct fd * original_fd);

/****************************************************************************/

#endif /* _STDIO_PROTOS_H */
