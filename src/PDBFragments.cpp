/*
----------------------------------------------------------------------------
This file is part of MSL (Molecular Software Libraries) 
 Copyright (C) 2008-2012 The MSL Developer Group (see README.TXT)
 MSL Libraries: http://msl-libraries.org

If used in a scientific publication, please cite: 
Kulp DW et al. "Structural informatics, modeling and design with a open 
source Molecular Software Library (MSL)" (2012) J. Comp. Chem, in press
DOI: 10.1002/jcc.22968

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


#include "PDBFragments.h"
#include "BBQTable.h"
#include "Transforms.h"
#include "AtomSelection.h"
#include "MslOut.h"

// BOOST Includes
#include <boost/regex.hpp>


static MslOut MSLOUT("PDBFragments");
using namespace MSL;
using namespace std;

int PDBFragments::searchForMatchingFragmentsSpots(System &_sys, std::vector<std::string> &_stemResidues, int _maxResiduesBetweenStems, double _rmsdTol){
	if (fragDB.size() <= 4){
		cerr << "ERROR 1232 PDBFragments::searchForMatchingFragments(), fragment database is too small, maybe you forgot to load it using loadFragmentDatabase() ?"<<endl;
		exit(1232);
	}
	int numFrags = 0;

	// Remove last set of results
	lastResults.clear();

	// Clear our set of matched sequences
	matchedSequences.clear();


	if (_stemResidues.size() != 3){
	  cerr << "ERROR 249582 PDBFragments::searchForMatchingFragments3, need exactly 3 stem residues, got: "<<_stemResidues.size()<<endl;
	  exit(249582);
	}

	if (!_sys.positionExists(_stemResidues[0]) ||!_sys.positionExists(_stemResidues[1]) ||!_sys.positionExists(_stemResidues[2])){
	  cerr << "ERROR 23423 PDBFragments::searchForMatchingFragments3, couldn't find a stem residue in pdb file: "<<_stemResidues[0]<<" , "<<_stemResidues[1]<<" , "<<_stemResidues[2]<<endl;
	  exit(23423);
	}
	Position &pos1 = _sys.getPosition(_stemResidues[0]);
	Position &pos2 = _sys.getPosition(_stemResidues[1]);
	Position &pos3 = _sys.getPosition(_stemResidues[2]);

	AtomPointerVector stemBBats;
	stemBBats.push_back(&pos1.getAtom("N"));
	stemBBats.push_back(&pos1.getAtom("CA"));
	stemBBats.push_back(&pos1.getAtom("C"));

	stemBBats.push_back(&pos2.getAtom("N"));
	stemBBats.push_back(&pos2.getAtom("CA"));
	stemBBats.push_back(&pos2.getAtom("C"));

	stemBBats.push_back(&pos3.getAtom("N"));
	stemBBats.push_back(&pos3.getAtom("CA"));
	stemBBats.push_back(&pos3.getAtom("C"));

         
        // Store stem-to-stem distance-squared vector (for filtering candidates below)
        AtomPointerVector stems;
	stems.push_back(&pos1.getAtom("CA"));
	stems.push_back(&pos2.getAtom("CA"));
	stems.push_back(&pos3.getAtom("CA"));
	
	vector<double> stemDistanceSq;
	for (uint c = 0; c < stems.size();c++){
	  for (uint n = c+1; n < stems.size();n++){
	    double distSq = stems(c).distance2(stems(n));			
	    stemDistanceSq.push_back(distSq);
	  }
	}

	MSLOUT.stream() << "FragDB.size(): "<<fragDB.size()<<endl;


	// Now loop over all fragments in database checking for ones of the correct size
	double tol = 64; // Tolerance of distance to be deviant from stems in Angstroms^2
	int matchIndex = 0; // Index for keeping track of matches
	for (uint i = 0 ; i < fragDB.size()-(2*_maxResiduesBetweenStems+stems.size());i++){


			bool validTriplet = true;
			int secondPositionIndex = -1;

			// Scan ahead for second position
			int closestDistanceIndex = 0;
			double minDiff           = MslTools::doubleMax;
			for (uint j = 1; j <= _maxResiduesBetweenStems;j++){

			  // Second position is no good due to segid match
			  if (fragDB[i]->getSegID() != fragDB[i+j]->getSegID()){
			    validTriplet = false;
			    break;
			  }

			  // Second position is no good due to chain match
			  if (fragDB[i]->getChainId() != fragDB[i+j]->getChainId()){
			    validTriplet = false;
			    break;
			  }

			  // Second position is no good due to gap..
			  if ( abs(fragDB[i]->getResidueNumber() - fragDB[i+j]->getResidueNumber()) > j){
			    validTriplet = false;
			    break;
			  }


			  // Second position is no good due to distance tolerance.., don't break loop!
			  double distSq = fragDB[i]->distance2(*fragDB[i+j]);
			  if (abs(distSq - stemDistanceSq[0]) < minDiff){
			    minDiff              = abs(distSq - stemDistanceSq[0]);
			    closestDistanceIndex = j;
			  }
			}

			if (minDiff > tol) continue;
			secondPositionIndex = i + closestDistanceIndex;

			if (secondPositionIndex+_maxResiduesBetweenStems > fragDB.size()) continue;
			int thirdPositionIndex = -1;
			closestDistanceIndex = 0;
			minDiff = MslTools::doubleMax;
			double closestDistance2   = MslTools::doubleMax;
			double closestDistance3   = MslTools::doubleMax;
			for (uint j = 1; j <= _maxResiduesBetweenStems;j++){

			  // Second position is no good due to segid match
			  if (fragDB[secondPositionIndex]->getSegID() != fragDB[secondPositionIndex+j]->getSegID()){
			    validTriplet = false;
			    break;
			  }

			  // Second position is no good due to chain match
			  if (fragDB[secondPositionIndex]->getChainId() != fragDB[secondPositionIndex+j]->getChainId()){
			    validTriplet = false;
			    break;
			  }

			  // Second position is no good due to gap..
			  if ( abs(fragDB[secondPositionIndex]->getResidueNumber() - fragDB[secondPositionIndex+j]->getResidueNumber()) > j){
			    validTriplet = false;
			    break;
			  }


			  // Second position is no good due to distance tolerance.., don't break loop!
			  double distSq2 = fragDB[i]->distance2(*fragDB[secondPositionIndex+j]);
			  double distSq3 = fragDB[secondPositionIndex]->distance2(*fragDB[secondPositionIndex+j]);
			  double diffFound = abs(distSq2 - stemDistanceSq[1]) + abs(distSq3 - stemDistanceSq[2]);
			  if (diffFound < minDiff){
			    closestDistanceIndex = j;
			    closestDistance2      = distSq2;
			    closestDistance3      = distSq3;
			    minDiff               = diffFound;
			  }

			}			  

			if (abs(closestDistance2-stemDistanceSq[1]) > tol && abs(closestDistance3 - stemDistanceSq[2]) > tol) continue;
			thirdPositionIndex = secondPositionIndex + closestDistanceIndex;

			if (thirdPositionIndex >= fragDB.size()) { cerr << "ERROR 3rd position index too large"<<endl; continue;}
			
			// MSLOUT.stream() << MslTools::stringf("%06d %s-%1s_%04d%s ",
			// 				     matchIndex,
			// 				     fragDB(i).getSegID().c_str(),
			// 				     fragDB(i).getChainId().c_str(),
			// 				     fragDB(i).getResidueNumber(),
			// 				     fragDB(i).getResidueIcode().c_str());
			// MSLOUT.stream() << MslTools::stringf(" %s-%1s_%04d%s ",
			// 			       fragDB(secondPositionIndex).getSegID().c_str(),
			// 			       fragDB(secondPositionIndex).getChainId().c_str(),
			// 			       fragDB(secondPositionIndex).getResidueNumber(),
  			// 			       fragDB(secondPositionIndex).getResidueIcode().c_str());
			// MSLOUT.stream() << MslTools::stringf(" %s-%1s_%04d%s \n",
			// 			       fragDB(thirdPositionIndex).getSegID().c_str(),
			// 			       fragDB(thirdPositionIndex).getChainId().c_str(),
			// 			       fragDB(thirdPositionIndex).getResidueNumber(),
			// 			       fragDB(thirdPositionIndex).getResidueIcode().c_str());


			// align and print winning fragment
			// Make a copy of frag stem, so not to effect original atoms
			AtomPointerVector fragStem; 
			fragStem.push_back(fragDB[i]);
			fragStem.push_back(fragDB[secondPositionIndex]);
			fragStem.push_back(fragDB[thirdPositionIndex]);
			// Align the fragStem to stem to see if it matches well enough...
			Transforms tm;
			fragStem.saveCoor("pre");
			tm.rmsdAlignment(fragStem,stems);

			double rmsd = fragStem.rmsd(stems);


			// Continue if the RMSD filter not passed
			if (rmsd > 0.5){
				continue;
			}

			matchIndex++;
			fragStem.applySavedCoor("pre");

			// Create new atoms for middle atoms..
			AtomContainer fullFragCA;
			string matchSeq = "";
			for (uint f = i; f <= thirdPositionIndex;f++){
			        fullFragCA.addAtom(*fragDB[f]);
				matchSeq += MslTools::getOneLetterCode(fragDB[f]->getResidueName());
			}



			string key = MslTools::stringf("%06d-%s-%1s_%04d%s-%1s_%04d%s",
						       matchIndex,
						       fragDB(i).getSegID().c_str(),
						       fragDB(i).getChainId().c_str(),
						       fragDB(i).getResidueNumber(),
						       fragDB(i).getResidueIcode().c_str(),
						       fragDB(thirdPositionIndex).getChainId().c_str(),
						       fragDB(thirdPositionIndex).getResidueNumber(),
						       fragDB(thirdPositionIndex).getResidueIcode().c_str());

			matchedSequences[key] = matchSeq;

			// if (_regex != ""){
			//   if (!boost::regex_search(matchSeq.c_str(),boost::regex(_regex))){
			//     continue;
			//   } else {
			//     MSLOUT.stream() << "RegEx Matched."<<endl;
			//   }
			// }
			

			
			tm.rmsdAlignment(fragStem,stems,fullFragCA.getAtomPointers());




			
			bool successful = true;

			  if (pdbDir != ""){

			              Atom &at1 = fullFragCA.getAtom(0);
			              Atom &at2 = fullFragCA.getAtom(fullFragCA.size()-1);

				      string allAtomFileName = MslTools::stringf("%s/%s.pdb",pdbDir.c_str(),at1.getSegID().c_str());

				      System allAtomSys;
				      allAtomSys.readPdb(allAtomFileName);
				      for (uint ats = 0; ats < allAtomSys.getAtomPointers().size();ats++){
					allAtomSys.getAtom(ats).setSegID("");
				      }

				      AtomSelection sel2(allAtomSys.getAtomPointers());

			    	      stringstream ss;
				      char tmpstr[100];
				      //sprintf(tmpstr,"chain %1s and resi %d-%-d and name CA",at1.getChainId().c_str(),at1.getResidueNumber(),at2.getResidueNumber());
				      sprintf(tmpstr,"chain %1s and resi %d-%-d and name CA",	fragDB[i]->getChainId().c_str(),fragDB[i]->getResidueNumber(),fragDB[thirdPositionIndex]->getResidueNumber());
				      ss << tmpstr;
				      AtomPointerVector caAts = sel2.select(ss.str());


				      if (!tm.rmsdAlignment(caAts,fullFragCA.getAtomPointers(),allAtomSys.getAtomPointers())){
					MSLOUT.stream() << "Problem aligning all atoms using the C-alpha trace"<<endl;
					MSLOUT.stream() << "PDB: "<<allAtomFileName<<endl;
					MSLOUT.stream() << "\tTrying to align with: "<<ss.str()<<endl;
					MSLOUT.stream() << "\tSystem: "<<allAtomSys.getSizes();
					MSLOUT.stream() << "\tSelected: "<<caAts.size()<<" atoms using '"<<ss.str()<<"'"<<endl;
					MSLOUT.stream() << "\tReference: "<<fullFragCA.getAtomPointers().size()<<" atoms"<<endl;
					MSLOUT.stream() << fullFragCA.getAtomPointers();
					continue;
				      } 

				      // Get BB atoms from stem-equivalent residues
				      AtomPointerVector allAts_stemBBats;
				      Position &stem1eq  = allAtomSys.getPosition(MslTools::getPositionId(fragDB[i]->getChainId(),fragDB[i]->getResidueNumber(),fragDB[i]->getResidueIcode()));
				      allAts_stemBBats.push_back(&stem1eq.getAtom("N"));
				      allAts_stemBBats.push_back(&stem1eq.getAtom("CA"));
				      allAts_stemBBats.push_back(&stem1eq.getAtom("C"));

				      Position &stem2eq  = allAtomSys.getPosition(MslTools::getPositionId(fragDB[secondPositionIndex]->getChainId(),fragDB[secondPositionIndex]->getResidueNumber(),fragDB[secondPositionIndex]->getResidueIcode()));
				      allAts_stemBBats.push_back(&stem2eq.getAtom("N"));
				      allAts_stemBBats.push_back(&stem2eq.getAtom("CA"));
				      allAts_stemBBats.push_back(&stem2eq.getAtom("C"));

				      Position &stem3eq  = allAtomSys.getPosition(MslTools::getPositionId(fragDB[thirdPositionIndex]->getChainId(),fragDB[thirdPositionIndex]->getResidueNumber(),fragDB[thirdPositionIndex]->getResidueIcode()));
				      allAts_stemBBats.push_back(&stem3eq.getAtom("N"));
				      allAts_stemBBats.push_back(&stem3eq.getAtom("CA"));
				      allAts_stemBBats.push_back(&stem3eq.getAtom("C"));
				      	
				      double bbRMSD = allAts_stemBBats.rmsd(stemBBats);


				      if (bbRMSD > _rmsdTol){
					continue;
				      }
				      fprintf(stdout,"(%4s and chain %1s and resi %3d-%3d)  %8.3f ",
					      fragDB[i]->getSegID().c_str(),
					      fragDB[i]->getChainId().c_str(),
					      fragDB[i]->getResidueNumber(),
					      fragDB[thirdPositionIndex]->getResidueNumber(),
					      rmsd);

				      fprintf(stdout, "%8.3f",bbRMSD);

				      ss.str("");
				      char tmpstr2[100];
				      sprintf(tmpstr2,"chain %1s and resi %d-%-d",at1.getChainId().c_str(),at1.getResidueNumber(),at2.getResidueNumber());
				      ss << tmpstr2;
				      AtomPointerVector allAts = sel2.select(ss.str());
				      lastResults.push_back(new AtomContainer());
				      lastResults.back()->addAtoms(allAts);




			  }
	 
			fprintf(stdout,"\n");
			fragStem.applySavedCoor("pre");

			if (successful){
				numFrags++;
			}
			
	}
	MSLOUT.stream() << "Done."<<endl;
	
	return numFrags;
}


int PDBFragments::searchForMatchingFragmentsLinear(System &_sys, string &_startRes, string &_endRes, string _regex, double _rmsdTol){

  if (  ! ( _sys.positionExists(_startRes)  && _sys.positionExists(_endRes) ) ) {
    cerr << "ERROR 2342 residue(s) don't exist: "<<_startRes<<" "<<_endRes<<endl;
    return -1;
  }


  int numFrags = 0;
  
  // Remove last set of results
  lastResults.clear();

  // Clear our set of matched sequences
  matchedSequences.clear();


  // AtomVector of CA/backbone atoms
  int residueSeparation = _sys.getPositionIndex(_endRes) - _sys.getPositionIndex(_startRes);
  AtomPointerVector bbAts;
  for (uint i = _sys.getPositionIndex(_startRes); i <= _sys.getPositionIndex(_endRes);i++){
    if (_sys.getPosition(i).atomExists("CA")){
      bbAts.push_back(&_sys.getPosition(i).getAtom("CA"));
    } else {
      cerr << "Position: "<<_sys.getPosition(i).toString()<<" doesn't have a CA atom"<<endl;
      return -1;
    }
  }


  MSLOUT.stream() << "FragDB.size(): "<<fragDB.size()<<endl;
  // Now loop over all fragments in database checking for ones of the correct size
  int matchIndex = 0; // Index for keeping track of matches
  Transforms tm;
  stringstream ss;
  for (uint i = 0 ; i < fragDB.size()-residueSeparation;i++){

    // Filter by pdb/chain breaks
    if (fragDB(i).getSegID() != fragDB(i+residueSeparation).getSegID()){
      continue;
    }
    if (fragDB(i).getChainId() != fragDB(i+residueSeparation).getChainId()){
      continue;
    }

    AtomPointerVector fragBB;
    string matchSeq = "";
    for (uint j = 0; j <= residueSeparation;j++){
      fragBB.push_back(&fragDB(i+j));
      matchSeq += MslTools::getOneLetterCode(fragDB(i+j).getResidueName());
    }


    fragBB.saveCoor("pre");
    if (!tm.rmsdAlignment(fragBB,bbAts)){
      cerr << "ERROR in alignment: "<<fragBB.size()<<" "<<bbAts.size()<<endl;
      continue;
    }

    double rmsd = fragBB.rmsd(bbAts);
    if (rmsd > _rmsdTol){
      continue;
    }

    
    matchIndex++;
    


    if (_regex != ""){
      if (!boost::regex_search(matchSeq.c_str(),boost::regex(_regex))){
	MSLOUT.stream() << "RegEx NOT Matched. "<<matchSeq<<endl;
	continue;
      } else {
	MSLOUT.stream() << "RegEx Matched. "<<matchSeq<<endl;
      }

      string key = MslTools::stringf("%06d-%s-%1s_%04d%s-%1s_%04d%s",
				   matchIndex,
				   fragDB(i).getSegID().c_str(),
				   fragDB(i).getChainId().c_str(),
				   fragDB(i).getResidueNumber(),
				   fragDB(i).getResidueIcode().c_str(),
				   fragBB(fragBB.size()-1).getChainId().c_str(),
				   fragBB(fragBB.size()-1).getResidueNumber(),
				   fragBB(fragBB.size()-1).getResidueIcode().c_str());
      matchedSequences[key] = matchSeq;
    }


    fprintf(stdout,"(%4s and chain %1s and resi %3d-%3d)  %8.3f \n",
	    fragDB[i]->getSegID().c_str(),
	    fragDB[i]->getChainId().c_str(),
	    fragDB[i]->getResidueNumber(),
	    fragBB[fragBB.size()-1]->getResidueNumber(),
	    rmsd);


    if (pdbDir != ""){
      fragBB.applySavedCoor("pre");
      Atom &at1 = fragBB(0);
      Atom &at2 = fragBB(fragBB.size()-1);
      string allAtomFileName = MslTools::stringf("%s/%s.pdb",pdbDir.c_str(),at1.getSegID().c_str());


      System allAtomSys;
      allAtomSys.readPdb(allAtomFileName);
      for (uint ats = 0; ats < allAtomSys.getAtomPointers().size();ats++){
	allAtomSys.getAtom(ats).setSegID("");
      }

      if (!tm.rmsdAlignment(fragBB,bbAts,allAtomSys.getAtomPointers())){
	cerr << "ERROR in alignment2: "<<fragBB.size()<<" "<<bbAts.size()<<endl;
	continue;
      }


      lastResults.push_back(new AtomContainer());
      if (includeFullFile){
	lastResults.back()->addAtoms(allAtomSys.getAtomPointers());
      } else {
	AtomSelection sel2(allAtomSys.getAtomPointers());
	ss.str("");
	char tmpstr2[100];
	sprintf(tmpstr2,"chain %1s and resi %d-%-d",at1.getChainId().c_str(),at1.getResidueNumber(),at2.getResidueNumber());
	ss << tmpstr2;
	AtomPointerVector allAts = sel2.select(ss.str());
	lastResults.back()->addAtoms(allAts);
      }

    } else {

      // Add CA only atoms..
      lastResults.push_back(new AtomContainer());
      lastResults.back()->addAtoms(fragDB);

    }

    numFrags++;
  }


  return numFrags;
}

/*
Input:    
    _sys                    :   MSL System
    _stemResidues           :   id of "stem" residues inside chain   (current needs to be size of 4, 2 on each side of the locally sampling fragment)
    _numResiduesInFragment :*  number of residues wanted between "stems" (default is calculated from firstOfLastTwoStems.getResidueNumber() - lastOfFirstTwoStems.getResidueNumber() )

Output:
    int number of fragments found.
 */
int PDBFragments::searchForMatchingFragmentsStems(System &_sys, vector<string> &_stemResidues,int _numResiduesInFragment, string _regex, double _rmsdTol){

	if (fragDB.size() <= 4){
		cerr << "ERROR 1232 PDBFragments::searchForMatchingFragments(), fragment database is too small, maybe you forgot to load it using loadFragmentDatabase() ?"<<endl;
		exit(1232);
	}
	int numFrags = 0;
	int illegalQuads = 0;

	// Remove last set of results
	lastResults.clear();

	// Clear our set of matched sequences
	matchedSequences.clear();


	// If stem residues is 0, the use all the atoms of _ats to search
	if (_stemResidues.size() == 2){
	} else {

		// Get two stem vectors
		if (_stemResidues.size() % 2 != 0){
			cerr << "ERROR 1967 PDBFragments::searchForMatchingFragments() , number of stem residues is not even\n";
			exit(1967);
		}


		// Create 2 lists (AtomPointerVectors) of the cterm and nterm stem.
		int numStemResidues =  _stemResidues.size() / 2;
		AtomPointerVector stem1;
		AtomPointerVector stem2;
		for (uint i = 0; i < _stemResidues.size();i++){
			if (i < numStemResidues){
				stem1.push_back(&_sys.getPosition(_stemResidues[i]).getAtom("CA"));
			} else {
				stem2.push_back(&_sys.getPosition(_stemResidues[i]).getAtom("CA"));
			}

		}

		AtomPointerVector stemBBats;
		stemBBats.push_back(&_sys.getPosition(_stemResidues[0]).getAtom("N"));
		stemBBats.push_back(&_sys.getPosition(_stemResidues[0]).getAtom("CA"));
		stemBBats.push_back(&_sys.getPosition(_stemResidues[0]).getAtom("C"));
		stemBBats.push_back(&_sys.getPosition(_stemResidues[_stemResidues.size()-1]).getAtom("N"));
		stemBBats.push_back(&_sys.getPosition(_stemResidues[_stemResidues.size()-1]).getAtom("CA"));
		stemBBats.push_back(&_sys.getPosition(_stemResidues[_stemResidues.size()-1]).getAtom("C"));

		//MSLOUT.stream() << "STEM sizes: "<<stem1.size()<<","<<stem2.size()<<endl;

		// Check for small stems
		if (stem1.size() <= 1 || stem2.size() <= 1){
			cerr<< "PDBFragments::searchForMatchingFragments() . Stems are too small."<<stem1.size()<<","<<stem2.size()<<endl;
		}

		// Use natural sequence length if num residues in fragment not set
		if (_numResiduesInFragment == -1){
			_numResiduesInFragment = stem2(0).getResidueNumber() - stem1(stem1.size()-1).getResidueNumber() - 1;
		}

		MSLOUT.stream() << "Number of residues between stems: "<<_numResiduesInFragment<<endl;

		// Store stem-to-stem distance-squared vector (for filtering candidates below)
		AtomPointerVector stems = stem1 + stem2;
		vector<double> stemDistanceSq;
		for (uint c = 0; c < stem1.size();c++){
			  fprintf(stdout,"C: %1s %4d %3s\n",
			  stem1[c]->getChainId().c_str(),
			  stem1[c]->getResidueNumber(),
			  stem1[c]->getResidueName().c_str());
			for (uint n = 0; n < stem2.size();n++){

				double distSq = stem1(c).distance2(stem2(n));			
				  fprintf(stdout,"\tN: %1s %4d %3s = %8.3f\n",
				  stem2[n]->getChainId().c_str(),
				  stem2[n]->getResidueNumber(),
				  stem2[n]->getResidueName().c_str(),distSq);

				stemDistanceSq.push_back(distSq);
			}
		}


		// BBQTable is used for adding backbone-atoms to a c-alpha trace.
		BBQTable bbqT;
		//bbqT.setDebugFlag(true);
		if (fragType == caOnly){

			// BBQ Table for adding backbone atoms
			bbqT.openReader(bbqTable);
		}

		MSLOUT.stream() << "FragDB.size(): "<<fragDB.size()<<endl;
		// Now loop over all fragments in database checking for ones of the correct size
		double tol = 64; // Tolerance of distance to be deviant from stems in Angstroms^2
		int matchIndex = 0; // Index for keeping track of matches
		for (uint i = 0 ; i < fragDB.size()-(_numResiduesInFragment+stem1.size()+stem2.size());i++){

			// Get proposed ctermStem
			AtomPointerVector ctermStem;
			for (uint n = 0; n < stem1.size();n++){
				ctermStem.push_back(fragDB[i+n]);
			}

			// Get proposed ntermStem
			AtomPointerVector ntermStem;
			for (uint n = 0; n < stem2.size();n++){
				ntermStem.push_back(fragDB[i+stem1.size()+_numResiduesInFragment+n]);
			}

			// Check for same PDB
			if (ctermStem(0).getSegID() != ntermStem(ntermStem.size()-1).getSegID()){
			  //fprintf(stdout,"New PDB: %4s to %4s\n",ctermStem[0]->getSegID().c_str(),ntermStem(ntermStem.size()-1).getSegID().c_str());

				// Jump ahead to move past all pdb1 v pdb2 tests
				i = i + _numResiduesInFragment+stem1.size();
				continue;
			}


			// Check for same Chain
			if (ctermStem(0).getChainId() != ntermStem(ntermStem.size()-1).getChainId()){
				/*
				fprintf(stdout,"New Chain: %4s %1s to %4s %1s\n",
					ctermStem[0]->getSegID().c_str(),
					ctermStem[0]->getChainId().c_str(),					
					ntermStem(ntermStem.size()-1).getSegID().c_str(),
					ntermStem(ntermStem.size()-1).getChainId().c_str());
				*/
				// Jump ahead to move past all chain1 v chain2 tests
				i = i + _numResiduesInFragment+stem1.size();
				continue;
			}


			// Check for a gap (PDB Structures have gaps).
			if ( abs(ctermStem[ctermStem.size()-1]->getResidueNumber() - ntermStem[0]->getResidueNumber()) != _numResiduesInFragment+1){
				/*
				fprintf(stdout," Gap found at residue %4s %1s %4d %3s and %4s %1s %4d %3s\n",
					ctermStem[ctermStem.size()-1]->getSegID().c_str(),
					ctermStem[ctermStem.size()-1]->getChainId().c_str(),
					ctermStem[ctermStem.size()-1]->getResidueNumber(),
					ctermStem[ctermStem.size()-1]->getResidueName().c_str(),
					ntermStem[0]->getSegID().c_str(),
					ntermStem[0]->getChainId().c_str(),
					ntermStem[0]->getResidueNumber(),
					ntermStem[0]->getResidueName().c_str());
				*/
				// Jump ahead to move past gap
				i = i + _numResiduesInFragment+stem1.size();
				continue;
			}
			

			// Distance Filter... are the proposed stems close enough ?
			bool passDistanceFilter = true;
			int index = 0;

			// Check against compiled list of distance-squared's
			for (uint c = 0; c < ctermStem.size();c++){

			  /*
				  fprintf(stdout," Checking residue %1s %4d %3s\n",
				  ctermStem[c]->getChainId().c_str(),
				  ctermStem[c]->getResidueNumber(),
				  ctermStem[c]->getResidueName().c_str());
			  */
				for (uint n = 0; n < ntermStem.size();n++){

					double distSq = ctermStem[c]->distance2(*ntermStem[n]);

					/*
					  fprintf(stdout," \t%1s %4d %3s = %8.3f vs %8.3f\n",
					  ntermStem[n]->getChainId().c_str(),
					  ntermStem[n]->getResidueNumber(),
					  ntermStem[n]->getResidueName().c_str(),distSq,stemDistanceSq[index]);
					*/
					if (abs(stemDistanceSq[index++] - distSq) > tol){
						passDistanceFilter = false;
						break;
					}
				}

				if (!passDistanceFilter){
					break;
				}
			}



			// Continue if the distance filter not passed
			if (!passDistanceFilter) {
				continue;
			}
			MSLOUT.stream() << "PASSED DISTANCE FILTER!"<<endl;

			// align and print winning fragment

			// Make a copy of frag stem, so not to effect original atoms
			AtomPointerVector fragStem;
			for (uint ct = 0; ct < ctermStem.size();ct++){
				fragStem.push_back(new Atom(ctermStem(ct)));
			}
			for (uint nt = 0; nt < ntermStem.size();nt++){
				fragStem.push_back(new Atom(ntermStem(nt)));
			}

			// Align the fragStem to stem to see if it matches well enough...
			Transforms tm;
			fragStem.saveCoor("pre");
			tm.rmsdAlignment(fragStem,stems);
			
			double rmsd = fragStem.rmsd(stems);
			MSLOUT.stream() << "RMSD: "<<rmsd<<endl;

			// Continue if the RMSD filter not passed
			if (rmsd > _rmsdTol){
				continue;
			}




			matchIndex++;
			fragStem.applySavedCoor("pre");

			//lastResults->writePdb("/tmp/preAdd.pdb");

			// Create new atoms for middle atoms..
			AtomPointerVector tmp;
			index = 1;
			string matchSeq = "";
			for (uint f = 0; f < ctermStem.size();f++){
			        //fragStem(f).setResidueNumber(index++);
				//fragStem(f).setResidueName("FRG");
				//fragStem(f).setChainId("A");
				//fragStem(f).setSegID("");
				tmp.push_back(&fragStem(f));
				//cout << "C-ADD: "<<fragStem(f)<<" "<<fragStem(f).getSegID()<<endl;
				matchSeq += MslTools::getOneLetterCode(fragStem(f).getResidueName());
			}


			for (int f = 0; f < _numResiduesInFragment;f++){
			        Atom *a =new Atom(fragDB(i+ctermStem.size()+f));
				//a->setCoor(fragDB(i+ctermStem.size()+f).getCoor());
				//a->setResidueNumber(index++);
				//a->setResidueName("FRG");
				//a->setChainId("A");
				//a->setSegID("");
				tmp.push_back(a);
				//cout << "F-ADD: "<<*a<<" "<<a->getSegID()<<endl;

				matchSeq += MslTools::getOneLetterCode(fragDB(i+ctermStem.size()+f).getResidueName());
			} 		


			for (uint f = 0; f < ntermStem.size();f++){
				//fragStem(f+ctermStem.size()).setResidueNumber(index++);
				//fragStem(f+ctermStem.size()).setResidueName("FRG");
				//fragStem(f+ctermStem.size()).setChainId("A");
				//fragStem(f+ctermStem.size()).setSegID("");
				tmp.push_back(&fragStem(f+ctermStem.size()));
				matchSeq += MslTools::getOneLetterCode(fragStem(f+ctermStem.size()).getResidueName());
				//cout << "N-ADD: "<<fragStem(f+ctermStem.size())<<" "<<fragStem(f+ctermStem.size()).getSegID()<<endl;
			}
			string key = MslTools::stringf("%06d-%s-%1s_%04d%s-%1s_%04d%s",
						       matchIndex,
						       fragDB(i+ctermStem.size()).getSegID().c_str(),
						       fragDB(i+ctermStem.size()).getChainId().c_str(),
						       fragDB(i+ctermStem.size()).getResidueNumber(),
						       fragDB(i+ctermStem.size()).getResidueIcode().c_str(),
						       fragDB(i+ctermStem.size()+_numResiduesInFragment-1).getChainId().c_str(),
						       fragDB(i+ctermStem.size()+_numResiduesInFragment-1).getResidueNumber(),
						       fragDB(i+ctermStem.size()+_numResiduesInFragment-1).getResidueIcode().c_str());

			matchedSequences[key] = matchSeq;

			if (_regex != ""){


			  if (!boost::regex_search(matchSeq.c_str(),boost::regex(_regex))){
			    MSLOUT.stream() << "RegEx NOT Matched. "<<matchSeq<<endl;
			    continue;
			  } else {
			    MSLOUT.stream() << "RegEx Matched."<<endl;
			  }

			}
			

			Chain tmpChain;
			tmpChain.addAtoms(tmp);			

			
			tm.rmsdAlignment(fragStem,stems,tmpChain.getAtomPointers());


			fprintf(stdout,"(%4s and chain %1s and resi %3d-%3d)  %8.3f ",
				ctermStem[0]->getSegID().c_str(),
				ctermStem[0]->getChainId().c_str(),
				ctermStem[0]->getResidueNumber(),
				ntermStem[ntermStem.size()-1]->getResidueNumber(),
			        rmsd);

			
			bool successful = true;
			if (fragType == caOnly){
			  if (pdbDir != ""){
			    
			              Atom &at1 = tmpChain.getAtom(0);
			              Atom &at2 = tmpChain.getAtom(tmpChain.atomSize()-1);

				      string allAtomFileName = MslTools::stringf("%s/%s.pdb",pdbDir.c_str(),at1.getSegID().c_str());
				      cout << "Opening "<<allAtomFileName<<endl;
				      System allAtomSys;
				      allAtomSys.readPdb(allAtomFileName);
				      for (uint ats = 0; ats < allAtomSys.getAtomPointers().size();ats++){
					allAtomSys.getAtom(ats).setSegID("");
				      }

				      AtomSelection sel2(allAtomSys.getAtomPointers());

			    	      stringstream ss;
				      char tmpstr[100];
				      //sprintf(tmpstr,"chain %1s and resi %d-%-d and name CA",at1.getChainId().c_str(),at1.getResidueNumber(),at2.getResidueNumber());
				      sprintf(tmpstr,"chain %1s and resi %d-%-d and name CA",	ctermStem[0]->getChainId().c_str(),ctermStem[0]->getResidueNumber(),ntermStem[ntermStem.size()-1]->getResidueNumber());
				      ss << tmpstr;
				      AtomPointerVector caAts = sel2.select(ss.str());


				      if (!tm.rmsdAlignment(caAts,tmpChain.getAtomPointers(),allAtomSys.getAtomPointers())){
					MSLOUT.stream() << "Problem aligning all atoms using the C-alpha trace"<<endl;
					MSLOUT.stream() << "PDB: "<<allAtomFileName<<endl;
					MSLOUT.stream() << "\tTrying to align with: "<<ss.str()<<endl;
					MSLOUT.stream() << "\tSystem: "<<allAtomSys.getSizes();
					MSLOUT.stream() << "\tSelected: "<<caAts.size()<<" atoms using '"<<ss.str()<<"'"<<endl;
					MSLOUT.stream() << "\tReference: "<<tmpChain.getAtomPointers().size()<<" atoms"<<endl;
					MSLOUT.stream() << tmpChain.getAtomPointers();
					successful = false;
					continue;
				      } 

				      // Get BB atoms from stem-equivalent residues
				      AtomPointerVector allAts_stemBBats;
				      //Position &stem1eq  = allAtomSys.getPosition(MslTools::getPositionId(at1.getChainId(),at1.getResidueNumber(),at1.getResidueIcode()));
				      Position &stem1eq  = allAtomSys.getPosition(MslTools::getPositionId(ctermStem[0]->getChainId(),ctermStem[0]->getResidueNumber(),ctermStem[0]->getResidueIcode()));
				      allAts_stemBBats.push_back(&stem1eq.getAtom("N"));
				      allAts_stemBBats.push_back(&stem1eq.getAtom("CA"));
				      allAts_stemBBats.push_back(&stem1eq.getAtom("C"));

				      //Position &stem2eq  = allAtomSys.getPosition(MslTools::getPositionId(at2.getChainId(),at2.getResidueNumber()+stem2.size()-1,at2.getResidueIcode()));
				      Position &stem2eq  = allAtomSys.getPosition(MslTools::getPositionId(ntermStem[ntermStem.size()-1]->getChainId(),ntermStem[ntermStem.size()-1]->getResidueNumber(),ntermStem[ntermStem.size()-1]->getResidueIcode()));
				      allAts_stemBBats.push_back(&stem2eq.getAtom("N"));
				      allAts_stemBBats.push_back(&stem2eq.getAtom("CA"));
				      allAts_stemBBats.push_back(&stem2eq.getAtom("C"));
				      	
				      double bbRMSD = allAts_stemBBats.rmsd(stemBBats);

				      MSLOUT.stream() << "BB RMSD: "<<bbRMSD<<endl;
				      fprintf(stdout, "%8.3f",bbRMSD);
				      if (bbRMSD > 1.51){
					successful = false;
					continue;
				      }
				      cout << "ADDDDDDDDDDDDDDDDDING COORDS"<<endl;


				      lastResults.push_back(new AtomContainer());
				      if (includeFullFile){
					lastResults.back()->addAtoms(allAtomSys.getAtomPointers());
				      } else {
					ss.str("");
					char tmpstr2[100];
					sprintf(tmpstr2,"chain %1s and resi %d-%-d",at1.getChainId().c_str(),at1.getResidueNumber(),at2.getResidueNumber());
					ss << tmpstr2;
					AtomPointerVector allAts = sel2.select(ss.str());
					lastResults.back()->addAtoms(allAts);
				      }






			  } else if (bbqTable != ""){
			  
				illegalQuads += bbqT.fillInMissingBBAtoms(tmpChain);

				if (illegalQuads > 0){
				  successful = false;
			  	  exit(0);
				}

				lastResults.push_back(new AtomContainer(tmpChain.getAtomPointers()));
			  }
				
			} else {
			  MSLOUT.stream() <<" ALL ATOMS "<<endl;
			}
			fprintf(stdout,"\n");

			fragStem.applySavedCoor("pre");

			if (successful){
				numFrags++;
			}

			/*					
			stringstream ss;
			PDBWriter pout;
			pout.open(ss);
			pout.write(sys.getAtomPointers());
			pout.close();

			matchingFragments.push(pair<double,string>(rmsd,ss.str()));
			*/


		}

		//fprintf(stdout, "Best RMSD: %8.3f\n",matchingFragments.top().first);


		fprintf(stdout,"Number of succesful fragments: %10d, illegal quads: %10d\n",numFrags,illegalQuads);

	}
	
	
	return numFrags;
}


