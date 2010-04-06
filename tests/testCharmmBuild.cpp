/*
----------------------------------------------------------------------------
This file is part of MSL (Molecular Software Libraries)
 Copyright (C) 2010 Dan Kulp, Alessandro Senes, Jason Donald, Brett Hannigan,
 Sabareesh Subramaniam, Ben Mueller

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


#include <iostream>

#include "System.h"
#include "CharmmSystemBuilder.h"
#include "PDBWriter.h"
#include "AtomSelection.h"
#include "Transforms.h"

using namespace std;

using namespace MSL;


int main() {

	System sys;

	PolymerSequence seq("\
A: ALA-ACE ILE VAL ILE\n\
B: ARG HSD THR GLY");

/*
	PolymerSequence seq("\
A: GLY GLY\n\
B: ARG HSD THR GLY");
*/

	cout << seq << endl;

	CharmmSystemBuilder CSB(sys, "/library/charmmTopPar/top_all22_prot.inp", "/library/charmmTopPar/par_all22_prot.inp");
	CSB.buildSystem(seq);
	sys.printIcTable();

	if (!sys.seed("A 1 C", "A 1 CA", "A 1 N")) {
		cerr << "Cannot seed atoms C, CA, N on residue 1 A" << endl;
	}
	if (!sys.seed("B 1 C", "B 1 CA", "B 1 N")) {
		cerr << "Cannot seed atoms C, CA, N on residue 1 B" << endl;
	}
	sys.buildAtoms();
	AtomSelection sel(sys.getAllAtomPointers());
	sel.select("chainB, chain B");

	Transforms tr;
	tr.translate(sel.getSelection("chainB"), CartesianPoint(10,5,5));
	string filename = "/tmp/buildFromCharmmTopology.pdb";

	if (!sys.writePdb(filename)) {
		cerr << "Cannot write output file " << filename << endl;
		exit(1);
	}

	cout << "Written pdb file " << filename << endl;
	cout << endl;

	cout << sys.getAtomPointers();

	cout << sys.calcEnergy() << endl;
	cout << sys.getEnergySummary();

	return 0;
}
