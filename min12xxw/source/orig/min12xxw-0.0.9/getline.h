/* getline.h	header file to declare a prototype for getline
 *
 * Copyright (C) 2004-2006 Manuel Tobias Schiller <mala@hinterbergen.de>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software Foundation,
 * Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 */

#ifndef _GETLINE_H
#define _GETLINE_H

/* we may include this by accident, so we check if we really need to
 * declare the prototype... */
#include "config.h"
#ifndef HAVE_GETLINE

#include <stddef.h> /* for size_t */
#include <stdio.h> /* for FILE */

#ifdef __cplusplus
extern "C" {
#endif
	
extern int getline(char **lineptr, size_t *n, FILE *stream);
	
#ifdef __cplusplus
}
#endif

#endif /* HAVE_GETLINE */
#endif /* _GETLINE_H */
