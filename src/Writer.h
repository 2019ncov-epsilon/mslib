/*
----------------------------------------------------------------------------
This file is part of MSL (Molecular Simulation Library)n
 Copyright (C) 2009 Dan Kulp, Alessandro Senes, Jason Donald, Brett Hannigan

This library is free software; you can redistribute it and/or
 modify it under the terms of the GNU Lesser General Public
 License as published by the Free Software Foundation; either
 version 2.1 of the License, or (at your option) any later version.

This library is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public
 License along with this library; if not, write to the Free Software
 Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307, 
 USA, or go to http://www.gnu.org/copyleft/lesser.txt.
----------------------------------------------------------------------------
*/

#ifndef WRITER_H
#define WRITER_H

/*
  A simple base class for writers.
 */



// MSL Includes
#include "File.h"

namespace MSL { 
class Writer : public File {

	public:
		Writer();
		Writer(const std::string &_filename);
		Writer(const std::string &_filename, int _mode);
		virtual ~Writer() ;

		bool write(std::string &_formatedString);
		bool writeln(std::string &_formatedString);
		virtual void writeREMARKS() {};
	protected:		
	private:
		
};

//INLINES GO HERE
	inline Writer::Writer():File("",1) {}
	inline Writer::Writer(const std::string &_filename) : File(_filename,1) {}
	inline Writer::Writer(const std::string &_filename, int _mode) : File(_filename,_mode) {}
	inline Writer::~Writer() {}
}

#endif
