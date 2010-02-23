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

#include "AtomDistanceRelationship.h"

using namespace MSL;


AtomDistanceRelationship::AtomDistanceRelationship() {
	value = 0;
	stamp = 0;
	selected = false;
}

AtomDistanceRelationship::AtomDistanceRelationship(const AtomPointerVector & _atoms) {
	value = 0;
	atoms = _atoms;
	selected = false;
}

AtomDistanceRelationship::AtomDistanceRelationship(const AtomDistanceRelationship & _ADR) {
	value = _ADR.value;
	stamp = _ADR.stamp;
	atoms = _ADR.atoms;
}

AtomDistanceRelationship::~AtomDistanceRelationship() {
}

