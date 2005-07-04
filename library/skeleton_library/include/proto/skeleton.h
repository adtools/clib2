#ifndef PROTO_SKELETON_H
#define PROTO_SKELETON_H

/*
** $Id: skeleton.h,v 1.1 2005-07-04 11:06:23 obarthel Exp $
**
** Prototype/inline/pragma header file combo
** Copyright (c) 2002-2005 by Olaf Barthel <olsen@sourcery.han.de>
** All rights reserved.
**
** Redistribution and use in source and binary forms, with or without
** modification, are permitted provided that the following conditions
** are met:
**
**   - Redistributions of source code must retain the above copyright
**     notice, this list of conditions and the following disclaimer.
**
**   - Neither the name of Olaf Barthel nor the names of contributors
**     may be used to endorse or promote products derived from this
**     software without specific prior written permission.
**
** THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
** AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
** IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
** ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
** LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
** CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
** SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
** INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
** CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
** ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
** POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef EXEC_TYPES_H
#include <exec/types.h>
#endif

/****************************************************************************/

#ifndef __NOLIBBASE__
 #ifndef __USE_BASETYPE__
  extern struct Library * SkeletonBase;
 #else
  extern struct Library * SkeletonBase;
 #endif /* __USE_BASETYPE__ */
#endif /* __NOLIBBASE__ */

/****************************************************************************/

#ifdef __amigaos4__
 #include <interfaces/skeleton.h>
 #ifdef __USE_INLINE__
  #include <inline4/skeleton.h>
 #endif /* __USE_INLINE__ */
 #ifndef CLIB_SKELETON_PROTOS_H
  #define CLIB_SKELETON_PROTOS_H 1
 #endif /* CLIB_SKELETON_PROTOS_H */
 #ifndef __NOGLOBALIFACE__
  extern struct SkeletonIFace *ISkeleton;
 #endif /* __NOGLOBALIFACE__ */
#else /* __amigaos4__ */
 #ifndef CLIB_SKELETON_PROTOS_H
  #include <clib/skeleton_protos.h>
 #endif /* CLIB_SKELETON_PROTOS_H */
 #if defined(__GNUC__)
  #ifndef __PPC__
   #include <inline/skeleton.h>
  #else
   #include <ppcinline/skeleton.h>
  #endif /* __PPC__ */
 #elif defined(__VBCC__)
  #ifndef __PPC__
   #include <inline/skeleton_protos.h>
  #endif /* __PPC__ */
 #else
  #include <pragmas/skeleton_pragmas.h>
 #endif /* __GNUC__ */
#endif /* __amigaos4__ */

/****************************************************************************/

#endif /* PROTO_SKELETON_H */
