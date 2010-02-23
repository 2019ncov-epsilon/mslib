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

#ifndef CHARMMPARAMETERREADER_H
#define CHARMMPARAMETERREADER_H
// STL Includes
#include <vector>
#include <map>
#include <iostream>
#include <math.h>

//MSL Includes
#include "Reader.h"
#include "MslTools.h"


namespace MSL { 
class CharmmParameterReader : public Reader {

	public:
		CharmmParameterReader();
		CharmmParameterReader(const std::string & _filename);
		CharmmParameterReader(const CharmmParameterReader & _par);
		~CharmmParameterReader();

		void operator=(const CharmmParameterReader & _par);

		bool read();
/*
		double getBondMinimumDistance(std::string _type1, std::string _type2);
		double getBondSpringConstant(std::string _type1, std::string _type2);
*/
		void reset();

		//VdwParam will return a std::vector with 4 values (Eps,Rmin,Esp14,Rmin14)
		std::vector<double> vdwParam(std::string type) const;


		/*******************************************************************
		 *  VdwParam will return a std::vector with 4 values 
		 * (sqrt(Eps1*Eps2),Rmin1+Rmin2,sqrt(Esp14_1+Eps14_2),Rmin14_1+Rmin14_2)
		 * NOTE: note created automatically, need to call createVdwParamPairs() first
		 *******************************************************************/
		std::vector<double> vdwParamPair(std::string type1, std::string type2) const;
		void createVdwParamPairs();

		//bondparam will return a std::vector with 2 values Kb and B0
		std::vector<double> bondParam(std::string type1, std::string type2) const;
	
		//Angle params will return a std::vector with 2 values (Ktheta, Theta0)
		std::vector<double> angleParam(std::string type1, std::string type2, std::string type3) const;

		//Ureybradley param will return a std::vector with 2 values (Kub, S0)
		std::vector<double> ureyBradleyParam(std::string type1, std::string type2, std::string type3) const;

		//Combined angle and ureybradley param will return a std::vector with 4 values (Ktheta, Theta0, Kub, S0)
		std::vector<double> angleAndUreyBradleyParam(std::string type1, std::string type2, std::string type3) const;

		//Dihedral Params will return a std::vector of vectors with 3 values each(Kchi, N, Delta). The std::vector is for each line in the Dihedral block which contains a match for these four types
		std::vector<std::vector<double> > dihedralParam(std::string type1, std::string type2, std::string type3, std::string type4) const;
		
		//Improper Params will return a std::vector with 2 values (Kpsi,Psi0) 
		std::vector<double> improperParam(std::string type1, std::string type2, std::string type3, std::string type4) const;

		//To be implemented
		std::vector<double> EEF1Param(std::string type) const; // default solvent WATER
		std::vector<double> EEF1Param(std::string type, std::string solvent) const;
		std::vector<double> IMM1Param(std::string type) const;
		std::vector<double> IMM1Param(std::string _type, std::string _solvent1, std::string _solvent2) const;

	private:
		void addBond(std::string type1, std::string type2, double Kb, double B0);
		void addAngle(std::string type1, std::string type2, std::string type3, double Ktheta, double Theta0, double Kub=0.0, double S0=0.0);
		void addDihedral(std::string type1, std::string type2, std::string type3, std::string type4, double kchi, double N, double Delta);
		void addImproper(std::string type1, std::string type2, std::string type3, std::string type4, double Kpsi, double Psi0);
		void addVdw(std::string type1, double Eps, double Rmin, double Esp14, double Rmin14);
		void addEEF1(std::string typeType, std::string solvent, double V, double Gref, double Gfree, double Href, double CPref, double Sigw);
		
		void setup();
		void copy(const CharmmParameterReader & _par);
		
		//Bond params will contain a std::vector with 2 values Kb and B0
		std::map<std::string, std::map<std::string, std::vector<double> > > bondParamMap;
		
		//Angle params will contain a std::vector with 4 values (Ktheta, Theta0, Kub, S0)
		std::map<std::string , std::map<std::string, std::map<std::string, std::vector<double> >  >  > angleParamMap;
		
		//Dihedral Params will contain a std::vector of vectors with 3 values each(Kchi, N, Delta). The std::vector is for each line in the Dihedral block which contains a match for these four types
		std::map< std::string, std::map<std::string , std::map<std::string, std::map<std::string, std::vector<std::vector<double> > >  >  >  > dihedralParamMap;
		

		//Improper Params will contain a std::vector with 2 values (Kpsi,Psi0) 
		std::map< std::string, std::map<std::string , std::map<std::string, std::map<std::string, std::vector<double> >  >  >  > improperParamMap;
		
		//VdwParams will contain a std::vector with 4 values (Eps,Rmin,Esp14,Rmin14)
		std::map<std::string,std::vector<double> > vdwParamMap;
		std::map<std::string,std::map<std::string, std::vector<double> > > vdwParamPairMap;

};

}

#endif

