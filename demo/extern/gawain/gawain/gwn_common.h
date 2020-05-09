/*
 * ***** BEGIN GPL LICENSE BLOCK *****
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software Foundation,
 * Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 * The Original Code is Copyright (C) 2016 by Mike Erwin.
 * All rights reserved.
 *
 * Contributor(s): Blender Foundation
 *
 * ***** END GPL LICENSE BLOCK *****
 */

/** \file blender/gpu/gwn_common.h
 *  \ingroup gpu
 */

#ifndef __GWN_COMMON_H__
#define __GWN_COMMON_H__

/* bWidgets-only */
typedef unsigned int uint;

#define PROGRAM_NO_OPTI 0

#if defined(NDEBUG)
  #define TRUST_NO_ONE 0
#else
  /* strict error checking, enabled for debug builds during early development */
  #define TRUST_NO_ONE 1
#endif

#if defined(WITH_OPENGL)
  #include <GL/glew.h>
#endif

#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>

#if TRUST_NO_ONE
  #include <assert.h>
#endif

/* GWN_INLINE */
#if defined(_MSC_VER)
#  define GWN_INLINE static __forceinline
#else
#  define GWN_INLINE static inline __attribute__((always_inline)) __attribute__((__unused__))
#endif

#endif /* __GWN_COMMON_H__ */
