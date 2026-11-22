//
// job.h
//
// by James Hammons
// (C) 2009 Underground Software
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; version 2 of the License.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the
// Free Software Foundation, Inc.,
// 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
//

#ifndef _JOB_H_
#define _JOB_H_

#include "document.h"

class Job
{
	protected:
		unsigned long numCopies;
		Document document;
		bool initialized;

		// Class variables
		static const char * userName;
		static const char * jobName;
		static const char * jobTitle;
		static uint16_t rotation;

	public:
		Job(const char * jName, const char * uName, const char * jTitle, unsigned long numCpy);
		virtual ~Job();

		bool Initialized(void);
		bool Run(void);

		// Class methods
		static void SetUserName(const char *);
		static void SetJobName(const char *);
		static void SetJobTitle(const char *);
		static void SetRotation(uint16_t);
		static const char * UserName(void);
		static const char * JobName(void);
		static const char * JobTitle(void);
		static uint16_t Rotation(void);
};

#endif	// _JOB_H_
