//
// document.cpp
//
// by James Hammons
// (C) 2009 Underground Software
//
// This is the document's representation. Basically, a collection of Pages.
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; version 2 or later of the License.
//
// JLH = James L. Hammons <jlhamm@acm.org>
//
// Who  When        What
// ---  ----------  -------------------------------------------------------------
// JLH  04/09/2009  Modified this file, added DPL specific bits
// JLH  04/18/2009  Major restructuring of classes, cleanup of this class to
//                  reflect overall new structure of the driver
//

#include "document.h"
#include <fcntl.h>
#include <math.h>
#include "printer.h"
#include "page.h"
#include "errlog.h"
#include "job.h"

//
// Constructor/Destructor
//
Document::Document(): raster(NULL)
{
}

Document::~Document()
{
    if (raster)
         cupsRasterClose(raster);
}


bool Document::Initialize(void)
{
	currentPage = 0;
	lastPage = false;
	raster = cupsRasterOpen(STDIN_FILENO, CUPS_RASTER_READ);

	if (!raster)
	{
		ERRORMSG(_("Document: Cannot open cupsRasterOpen()!"));
		return false;
	}

	return true;
}

//
// Document rendering. This handles rendering all pages in the document.
//
bool Document::Render(void)
{
	// Send the PJL Header
	Printer::SendPJLHeader();

	// Send the DPL job header...
	Printer::SendDPLJobHeader();

	// Send all pages
	Page * page = GetNextRawPage();

	while (page)
	{
		if (!page->Render())
			ERRORMSG(_("Error while rendering the page. Check the previous "
						"message. Trying to print the other pages."));

		fprintf(stderr, "PAGE: %lu %lu\n", page->PageNum(), page->Copies());
		delete page;
		page = GetNextRawPage();
	}

	// Send the PJL footer
	Printer::SendPJLFooter();

	return true;
}

//
// Get the next page from the CUPS rasterizer
//
Page * Document::GetNextRawPage(void)
{
	cups_page_header2_t header;

	if (!cupsRasterReadHeader2(raster, &header) || !header.cupsBytesPerLine ||
		!header.PageSize[1])
	{
		DEBUGMSG(_("No more pages"));
		lastPage = true;
		return NULL;
	}

	// This should be in the initialization phase, not here...
	Job::SetRotation(0);

	if (header.Orientation == CUPS_ORIENT_90)
		Job::SetRotation(90);
	else if (header.Orientation == CUPS_ORIENT_180)
		Job::SetRotation(180);
	else if (header.Orientation == CUPS_ORIENT_270)
		Job::SetRotation(270);

	// Right now we're assuming RGB format, hence (BytesPerLine / 3) here
	Page * page = new Page(header.cupsBytesPerLine / 3, header.cupsHeight);
	uint8_t * bitmap = page->GetBitmap();

	currentPage++;
	page->SetPageNum(currentPage);
	page->SetCopies(header.NumCopies);	// This makes no sense here...
//	fprintf(stderr, "PAGE: %lu %u\n", currentPage, header.NumCopies);

	// Read raster data into the Page's bitmap
	for(unsigned y=0; y<header.cupsHeight; y++)
	{
		if (cupsRasterReadPixels(raster, &bitmap[y * header.cupsBytesPerLine],
			header.cupsBytesPerLine) == 0)
		{
			ERRORMSG(_("Document: cupsRasterReadPixels failed!"));
			delete page;
			return NULL;
		}
	}

	return page;
}
