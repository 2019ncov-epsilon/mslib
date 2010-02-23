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

#ifndef MINIMIZER_H
#define MINIMIZER_H

// Standard Includes
#include <vector>
#include <map>

#include "MslTools.h"
#include "AtomPointerVector.h"
#include "EnergySet.h"

namespace MSL {
template <class T> class Minimizer {
	public:
		Minimizer();  
		Minimizer(T* _tObj);  
		~Minimizer();


		// Minimizer atoms using energySet
		void Minimize();
		
		// Get, Sets
		void setObjectInstance(T *_tObj) { ptTObj = _tObj;}

		void setStepSize(double _stepsize);
		double getStepSize();
		
		void setTolerance(double _tol);
		double getTolerance();

		void setMaxIterations(int _maxIter);
		int getMaxIterations();

		void setMinimizeAlgorithm(int _minAlgo);
		int getMinimizeAlgorithm();

		void setData(std::vector<double *> &_data);
		void addData(double &_data);
		void addData(AtomPointerVector &_av);
		std::vector<double *>& getData();     
		

		// Function + Derivative Function
		void setFunction(double (T::*_funcPtr)());
		void setDerivative(double (T::*_dfPtr)());



		// Restrict Minimization when using atoms...
		void freezeAtoms(AtomPointerVector &_av, double _springConstant);
		

		void printData();

		
		double CallFunc();

        protected: 

		// DATA 
		std::vector<double *> data;


		// Function to minimize
		double (T::*func)();

		// Derivative function
		double (T::*df)();

		// Instance of object
		T* ptTObj;

		double stepsize;
		double tolerance;
		int    maxIterations;
		int    minimizeAlgorithm;    


		AtomPointerVector springControlledAtoms;
		EnergySet springEnergy;

		//  Not using yet..
		static std::map<std::string,int> algorithmList;
		enum MinimizingAlgorithms { SIMPLEX=0 };


		// Protected Functions
		void create();



};

}
#endif

