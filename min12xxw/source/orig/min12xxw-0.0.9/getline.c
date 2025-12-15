/* getline.c
 *
 * implements a getline version for systems that don't have it
 * for BSDish systems, we use the fgetln libary function, for systems
 * which lack it, we do things the hard way
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

#include "config.h"

#ifndef HAVE_GETLINE
#ifdef HAVE_FGETLN
/* getline replacement for BSDish systems that have fgetln */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int getline(char **lineptr, size_t *n, FILE *f)
{
	char *line, *p;
	size_t len;

	/* make sure we can do our work... */
	if ((lineptr == NULL) || (n == NULL) || (f == NULL))
		return -1;

	/* read in a line */
	line = fgetln(f, &len);

	/* if there was an error, we bail out */
	if (ferror(f) || !line) return -1;
	/* if we couldn't read anything and we have reached EOF, we give up */
	if (!len && feof(f)) return -1;

	/* make sure there's enough space in the buffer */
	if (*n < (len + 1)) {
		char *tmp;

		tmp = (char *) realloc(*lineptr, len + 1);

		if (tmp == NULL) return -1;
		*lineptr = tmp;
		*n = len + 1;
	}
	/* copy line and zero-terminate it */
	for (p = *lineptr; p < &((*lineptr)[len]); *p++ = *line++);
	*p = 0;

	return len;
}
#else /* HAVE_FGETLN */
/* we need to do things the hard way... */
#include <stdio.h>
#include <stdlib.h>

/* our internal buffer is BUFSZ bytes big, and reallocation will also be
 * to line buffer sizes which are multiples of BUFSZ bytes */
#define BUFSZ	256

int getline(char **lineptr, size_t *n, FILE *f)
{
	char buf[BUFSZ], *p, *q;
	int c;
	size_t m, total = 0;
	
	/* make sure our input is valid */
	if ((lineptr == NULL) || (n == NULL)|| (f == NULL))
		return -1;
	/* return on error and end of file */
	if (ferror(f)) return -1;
	if (feof(f)) return -1;

	do {
		/* read in a piece of our line */
		m = 0;
		while ((m < BUFSZ) && !ferror(f) && !feof(f)) {
			buf[m] = c = fgetc(f);
			if (c == EOF) break;
			if (buf[m++] == '\n')	
				break;
		}
		/* in case of a "hard" error, we just bail out */
		if (ferror(f)) return -1;
		
		total += m;
		
		/* make sure destination is large enough */
		if (total >= *n) {
			size_t newsz = total + 1;
			/* round up to the nearest multiple of BUFSZ
			 * to avoid frequent reallocation as longer
			 * and longer lines are read in
			 * we also make sure we have space for the
			 * terminating zero character */
			newsz += (BUFSZ - (newsz % BUFSZ));
			p = realloc(*lineptr, newsz);
			if ((p == NULL))
			       return -1;
			*lineptr = p;
			*n = newsz;
		}

		/* copy to destination */
		q = &(*lineptr)[total - m];
		p = buf;
		while (p < &buf[m]) *q++ = *p++;

		/* check for feof again */
		if (feof(f)) {
			/* if we couldn't read a thing, return -1 */
			if (total == 0) return -1;
			/* we could read something, null-terminate it
			 * and return... */
			*q++ = 0;
			return total;
		}
		/* if we just copied our end-of-line character, we're
		 * done - note: we did read at least one byte if we reached
		 * this point, so *--p is perfectly legal to deference */
	} while (*--p != '\n');

	/* zero-terminate our string - we have space in the buffer! */
	*q++ = 0;
	return total;
}

#endif /* HAVE_FGETLN */
#endif /* HAVE_GETLINE */
