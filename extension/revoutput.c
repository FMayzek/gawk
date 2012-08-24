/*
 * revoutput.c --- Provide an output wrapper that reverses lines.
 *
 * Arnold Robbins
 * arnold@skeeve.com
 * Written 8/2012
 */

/*
 * Copyright (C) 2012 the Free Software Foundation, Inc.
 * 
 * This file is part of GAWK, the GNU implementation of the
 * AWK Programming Language.
 * 
 * GAWK is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 * 
 * GAWK is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <sys/types.h>
#include <sys/stat.h>

#include "config.h"

#include "gawkapi.h"

#include "gettext.h"
#define _(msgid)  gettext(msgid)
#define N_(msgid) msgid

static const gawk_api_t *api;	/* for convenience macros to work */
static awk_ext_id_t *ext_id;

static awk_bool_t init_revout(void);
static awk_bool_t (*init_func)(void) = init_revout;

int plugin_is_GPL_compatible;

/* rev_fwrite --- write out characters in reverse order */

static size_t
rev_fwrite(const void *buf, size_t size, size_t count, FILE *fp, void *opaque)
{
	const char *cp = buf;
	int nbytes = size * count;

	(void) opaque;

	for (; nbytes >= 1; nbytes--)
		putc(cp[nbytes-1], fp);

	return (size * count);
}


/* revout_can_take_file --- return true if we want the file */

static int
revout_can_take_file(const awk_output_buf_t *outbuf)
{
	awk_value_t value;

	if (outbuf == NULL)
		return 0;

	if (! sym_lookup("REVOUT", AWK_NUMBER, & value))
		return 0;

	return (value.num_value != 0);
}

/*
 * revout_take_control_of --- set up output wrapper.
 * We can assume that revout_can_take_file just returned true,
 * and no state has changed since then.
 */

static int
revout_take_control_of(awk_output_buf_t *outbuf)
{
	if (outbuf == NULL)
		return 0;

	outbuf->gawk_fwrite = rev_fwrite;
	outbuf->redirected = 1;
	return 1;
}

static awk_output_wrapper_t output_wrapper = {
	"revout",
	revout_can_take_file,
	revout_take_control_of,
	NULL
};

/* init_revout --- set things ups */

static awk_bool_t
init_revout()
{
	awk_value_t value;

	register_output_wrapper(& output_wrapper);

	make_number(0.0, & value);	/* init to false */
	if (! sym_update("REVOUT", & value)) {
		warning(ext_id, _("revout: could not initialize REVOUT variable"));

		return 0;
	}

	return 1;
}

static awk_ext_func_t func_table[] = {
	{ NULL, NULL, 0 }
};

/* define the dl_load function using the boilerplate macro */

dl_load_func(func_table, revout, "")
