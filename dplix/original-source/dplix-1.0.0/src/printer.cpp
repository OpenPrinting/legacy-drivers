//
// printer.cpp
//
// by James L. Hammons
// (C) 2009 Underground Software
//
// This is basically a convenience class that also handles sending out the
// PJL header and footer that the printer expects.
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
// JLH  04/18/2009  Added user/job/time id to PJL header, converted to class
//                  methods to bring sanity to the world, err, driver. ;-)
//

#include "printer.h"
#include <time.h>
#include <string.h>
#include <stdint.h>								// For uint8_t and the like
#include <unistd.h>								// For gethostname()
#include "dpl.h"
#include "errlog.h"
#include "job.h"

//
// Constructor/Destructor
//
Printer::Printer()
{
}

Printer::~Printer()
{
}

//
// Send the PJL header.
// The PJL header will be sent to STDOUT. Like the other methods which send data to
// the printer, if you need to redirect the data to somewhere else, use the freopen
// function to redirect STDOUT.
//
bool Printer::SendPJLHeader(void)
{
const uint8_t preamble[] = {
	"@PJL COMMENT DATE=%s\x0A"
	"@PJL COMMENT TIME=%s\x0A"
	"@PJL COMMENT DNAME=%s\x0A"
	"@PJL JOB MODE=PRINTER\x0A"
	"@PJL SET JOBATTR=\"@LUNA=%s\"\x0A"
	"@PJL SET COPIES=1\x0A"
	"@PJL SET QTY=1\x0A"
	"@PJL SET JOBATTR=\"@TRCH=OFF\"\x0A"
	"@PJL SET DUPLEX=OFF\x0A"
	"@PJL SET BINDING=LONGEDGE\x0A"
	"@PJL SET JOBATTR=\"@SPSE=AUTO\"\x0A"
	"@PJL SET JOBATTR=\"@MSIP=NORMAL\"\x0A"
	"@PJL SET RENDERMODE=COLOR\x0A"
	"@PJL SET ECONOMODE=OFF\x0A"
	"@PJL SET JOBATTR=\"@IREC=OFF\"\x0A"
	"@PJL SET JOBATTR=\"@TRAP=OFF\"\x0A"
	"@PJL SET JOBATTR=\"@HOAD=I0A00020F\"\x0A"
	"@PJL SET JOBATTR=\"@JOAU=%s\"\x0A"
	"@PJL SET JOBATTR=\"@CNAM=%s\"\x0A"
	"@PJL SET OUTBIN=FACEDOWN\x0A"
	"@PJL SET JOBATTR=\"@PODR=NORMAL\"\x0A"
	"@PJL SET SLIPSHEET=OFF\x0A"
	"@PJL SET PAPERDIRECTION=SEF\x0A"
	"@PJL SET JOBATTR=\"@APSP=OFF\"\x0A"
	"@PJL SET RESOLUTION=300\x0A"
	"@PJL SET BITSPERPIXEL=8\x0A"
	"@PJL SET JOBATTR=\"@DRDM=QCIQUALITY\"\x0A"
	"@PJL SET JOBATTR=\"@TSCR=9\"\x0A"
	"@PJL SET JOBATTR=\"@GSCR=9\"\x0A"
	"@PJL SET JOBATTR=\"@ISCR=9\"\x0A"
	"@PJL SET JOBATTR=\"@TTRC=9\"\x0A"
	"@PJL SET JOBATTR=\"@GTRC=9\"\x0A"
	"@PJL SET JOBATTR=\"@ITRC=9\"\x0A"
	"@PJL SET JOBATTR=\"@TCPR=24\"\x0A"
	"@PJL SET JOBATTR=\"@GCPR=24\"\x0A"
	"@PJL SET JOBATTR=\"@ICPR=25\"\x0A"
	"@PJL SET JOBATTR=\"@TUCR=24\"\x0A"
	"@PJL SET JOBATTR=\"@GUCR=24\"\x0A"
	"@PJL SET JOBATTR=\"@IUCR=25\"\x0A"
	"@PJL SET JOBATTR=\"@TDFT=1\"\x0A"
	"@PJL SET JOBATTR=\"@GDFT=1\"\x0A"
	"@PJL SET JOBATTR=\"@IDFT=1\"\x0A"
	"@PJL SET JOBATTR=\"@BANR=DEVICE\"\x0A"
	"@PJL ENTER LANGUAGE=DPL\x0A"
};

	time_t timestamp;
	time(&timestamp);
	struct tm * timeinfo = localtime(&timestamp);
	char date[64], time[64], host[256];
	sprintf(date, "%u/%u/%u", timeinfo->tm_mon + 1, timeinfo->tm_mday, 1900 + timeinfo->tm_year);
	sprintf(time, "%u:%u:%u", timeinfo->tm_hour, timeinfo->tm_min, timeinfo->tm_sec);
	gethostname(host, 255);
	host[255] = 0;								// Just in case...

	printf("%s", "\x1B%-12345X");
	// (1)date, (2)time, (3)jobtitle, (4)username, (5)username, (6)compname
	printf((const char *)preamble, date, time, Job::JobTitle(), Job::UserName(), Job::UserName(), host);
	fflush(stdout);

	return true;
}

bool Printer::SendPJLFooter(void)
{
	printf("%s", "IB\x1B%-12345X@PJL EOJ\x0A");
	fflush(stdout);

	return true;
}

bool Printer::SendDPLJobHeader(void)
{
	printf("%s", ") HP-PCL XL;2;1;FXXPL2-1.0.0,WINNT-5.1,XPL2DRV-1.0.6.0\x0A");
	fflush(stdout);

	try
	{
		DPL::Write(0x89, (uint16_t)600, 600);				// DPI?
		DPL::Write(0x86, (uint8_t)0);
		DPL::Write(0x8F, (uint8_t)0);
		DPL::Write(0x41);
		DPL::Write(0x88, (uint8_t)0);
//1 for RGB as well??? no...? mebbe...
		DPL::Write(0x82, (uint8_t)1);						// Can be 1 too (b/w page)...
		DPL::Write(0x48);

		fflush(stdout);
	}
	catch (DPL::Error e)
	{
		return false;
	}

	return true;
}
