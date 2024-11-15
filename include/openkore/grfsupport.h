// $Id: grfsupport.h 36 2005-05-10 20:36:16Z Rasqual $
/*
 *  libgrf
 *  grfsupport.h - commonly used functions
 *  Copyright (C) 2004  Faithful <faithful@users.sf.net>
 *  Copyright (C) 2004  Hongli Lai <h.lai@chello.nl>
 *  Copyright (C) 2004  Rasqual <rasqualtwilight@users.sf.net>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

/** @file grfsupport.h
 *
 * Various utility functions, to be used in combination with grf.h
 */

#ifndef __GRFSUPPORT_H__
#define __GRFSUPPORT_H__

#include "grftypes.h"

/* Comment this before final release */
/* #define GRF_DEBUG 1 */

GRFEXTERN_BEGIN


/* #define GRF_AlphaSort ((int(*)(const void *, const void *))GRF_AlphaSort_Func) */
#define GRF_OffsetSort ((int(*)(const void *, const void *))GRF_OffsetSort_Func)


/* GRFINLINE uint8_t LittleEndian8 (uint8_t *p); */	/* Pointless */
/* GRFINLINE uint16_t LittleEndian16 (uint8_t *p); */	/* Unused */

GRFINLINE uint32_t LittleEndian32(uint8_t *p);
GRFINLINE uint32_t ToLittleEndian32(uint32_t);

GRFEXPORT char *GRF_normalize_path(char *out, const char *in);
GRFEXPORT uint32_t GRF_NameHash(const char *name);

GRFEXPORT void grf_sort (Grf *grf, int(*compar)(const void *, const void *));
GRFEXPORT int GRF_AlphaSort_Func(const GrfFile *g1, const GrfFile *g2);
GRFEXPORT int GRF_OffsetSort_Func(const GrfFile *g1, const GrfFile *g2);

GRFEXPORT GrfFile *grf_find (Grf *grf, const char *fname, uint32_t *index);

int GRF_list_from_array(Grf *grf, GrfError *error);
int GRF_array_from_list(Grf *grf, GrfError *error);

GRFEXPORT GrfError *GRF_SetError(GrfError *err, GrfErrorType errtype, uint32_t line, const char *file, const char *func, void *extra);
GRFEXPORT const char *grf_strerror(GrfError err);


/** Macro used internally to set a GrfError structure. */
#define GRF_SETERR_ADD(a,b,e,f) GRF_SetError (a, b, __LINE__, __FILE__, #e, (uintptr_t *) f)

/** Macro used internally to set a GrfError structure. */
#define GRF_SETERR(err,type,func) GRF_SETERR_ADD (err, type, func, 0)

/** Macro used internally to set a GrfError structure. */
#define GRF_SETERR_2(err,type,func,extra) GRF_SETERR_ADD (err, type, func, extra)


GRFEXTERN_END

#endif /* !defined(__GRFSUPPORT_H__) */
