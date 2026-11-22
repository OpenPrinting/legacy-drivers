//
// printer.h
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

#ifndef _PRINTER_H_
#define _PRINTER_H_

class Printer
{
	public:
		Printer();
		virtual ~Printer();

		// Class methods
		static bool SendPJLHeader(void);
		static bool SendPJLFooter(void);
		static bool SendDPLJobHeader(void);
};

#endif	// _PRINTER_H_
