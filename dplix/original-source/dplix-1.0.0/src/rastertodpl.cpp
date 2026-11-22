//
// rastertodpl.cpp
//
// by James L. Hammons
// (C) 2009 Underground Software
//
// This is the controlling software for the PS -> raster image -> DPL filter.
// The only problem with this approach is that the printer isn't capable of
// rendering images higher than 300x300 dpi--even if the image is B/W.
// Another approach is needed here.
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; version 2 or later of the License.
//
// JLH = James L. Hammons <jlhamm@acm.org>
//
// Who  When        What
// ---  ----------  -------------------------------------------------------------
// JLH  04/01/2009  Modified this file, added DPL specific bits
// JLH  04/18/2009  Major refactoring of codebase, with changes reflected here
//

/*
A better method would be:

Document and Job seem to be duplicates and/or the same concept. Perhaps the split
would be like so: Document -> representation of one copy of a document, i.e., all
the pages. Job -> How many copies of Document to print, flow control, reporting,
etc. It would also the the high level loop for rendering, where the low level is
handled by Page.

So...

Job is the top level class. Job contains a Printer, and a Document.
Printer seems unnecessary--we ARE the printer! So stuff related to Printer would go into Job.
Document: A collection of Pages.
Page: Bitmap/PS representation of a page, low level rendering code.
DPL: Values/registers to DPL format that the printer expects.
rendering: Goes to Document or Job.
ppdfile: Totally unnecessary.
*/

#include <errno.h>
#include <stdlib.h>
#include <cups/ppd.h>
#include <cups/cups.h>
#include "errlog.h"
#include "version.h"
#include "job.h"

int main(int argc, char ** argv)
{
	// Check the # of arguments
	if (argc != 6 && argc != 7)
	{
		fprintf(stderr, _("Usage: %s job-id user title copies options [file]\n"), argv[0]);
		return 1;
	}

	const char * jobID = argv[1];
	const char * user = argv[2];
	const char * title = argv[3];
	unsigned long copies = strtol(argv[4], (char **)NULL, 10);
// We don't use them, should we?
//	const char * options = argv[5];
	const char * file = (argc == 7 ? argv[6] : NULL);

	// Get more information on the DPLiX environment (for debugging)
	DEBUGMSG(_("DPLiX filter v%s by James Hammons"), VERSION);
	DEBUGMSG(_("More information at: http://openprinting.org"));

	// Open the given file, redirect stdin
	if (file && !freopen(file, "r", stdin))
	{
		ERRORMSG(_("Cannot open file %s"), file);
		return errno;
	}

	// Create the job
	Job job(jobID, user, title, copies);

	// Initialize it
	if (!job.Initialized())
		return 1;

	// Render it
	if (!job.Run())
		return 2;

	return 0;
}
