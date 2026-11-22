//
// job.cpp
//
// by James Hammons
// (C) 2009 Underground Software
//
// This is the overall controlling class of the driver. Low level stuff happens
// in the Page class.
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; version 2 or later of the License.
//
// JLH = James L. Hammons <jlhamm@acm.org>
//
// Who  When        What
// ---  ----------  -------------------------------------------------------------
// JLH  04/18/2009  Rearrange this class so that it makes sense, add class
//                  methods to restore some sanity to the driver. :-)
//

#include "job.h"
#include <cups/ppd.h>							// For getenv()
#include "errlog.h"

// Initialize class vars
const char * Job::jobName = NULL;
const char * Job::userName = NULL;
const char * Job::jobTitle = NULL;
uint16_t Job::rotation = 0;

//
// Constructor & Destructor
//
Job::Job(const char * jName, const char * uName, const char * jTitle, unsigned long numCpy):
	numCopies(numCpy), initialized(false)
{
	SetJobName(jName ? jName : _("Unknown"));
	SetUserName(uName ? uName : getenv("USER"));
	SetJobTitle(jTitle ? jTitle : _("Unknown job title"));

	if (!document.Initialize())
	{
		ERRORMSG(_("Job: Failed to initialize the Document."));
		return;
	}

	initialized = true;
}

Job::~Job()
{
}

bool Job::Run(void)
{
	// Need to do stuff like handle number of copies, etc...

	if (!document.Render())
	{
		ERRORMSG(_("Job: Attempt to render the document failed!"));
		return false;
	}

	return true;
}

bool Job::Initialized(void)
{
	return initialized;
}

//
// Class methods
//

void Job::SetUserName(const char * un)
{
	userName = un;
}

void Job::SetJobName(const char * jn)
{
	jobName = jn;
}

void Job::SetJobTitle(const char * jt)
{
	jobTitle = jt;
}

void Job::SetRotation(uint16_t r)
{
	rotation = r;
}

const char * Job::UserName(void)
{
	return userName;
}

const char * Job::JobName(void)
{
	return jobName;
}

const char * Job::JobTitle(void)
{
	return jobTitle;
}

//Actually, this should go in the document...
uint16_t Job::Rotation(void)
{
	return rotation;
}
