/*! \file readfromfile.cpp
\brief Function for reading data into MeshParameter object

\section LICENSE
Copyright (c) 2017 Sijme-Jan Paardekooper

This file is part of Astrix.

Astrix is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.

Astrix is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with Astrix.  If not, see <http://www.gnu.org/licenses/>.*/

#include <cuda_runtime_api.h>
#include <iostream>
#include <fstream>
#include <sstream>
#include <stdexcept>
#include <cmath>
#include <cstdlib>

#include "../../Common/definitions.h"
#include "./meshparameter.h"

namespace astrix {

//#########################################################################
/*! Read in data from input file. File is read line by line; input parameters can appear in any order but all must be present. An exception is thrown if any parameter is missing or has an invalid value.

\param *fileName Pointer to input file name*/
//#########################################################################

void MeshParameter::ReadFromFile(const char *fileName)
{
  // Open mesh input file
  std::ifstream inFile(fileName);
  if (!inFile) {
    std::cout << "Error opening file " << fileName << std::endl;
    throw std::runtime_error("");
  }

  // Read file line by line
  std::string line;
  while (getline(inFile, line)) {
    std::string firstWord, secondWord;

    // Extract first two words from line
    std::istringstream iss(line);
    iss >> firstWord;
    iss >> secondWord;

    // Problem definition
    if (firstWord == "problemDefinition") {
      if (secondWord == "LIN") problemDef = PROBLEM_LINEAR;
      if (secondWord == "CYL") problemDef = PROBLEM_CYL;
      if (secondWord == "KH") problemDef = PROBLEM_KH;
      if (secondWord == "RIEMANN") problemDef = PROBLEM_RIEMANN;
      if (secondWord == "SOD") problemDef = PROBLEM_SOD;
      if (secondWord == "BLAST") problemDef = PROBLEM_BLAST;
      if (secondWord == "VORTEX") problemDef = PROBLEM_VORTEX;
      if (secondWord == "NOH") problemDef = PROBLEM_NOH;
      if (secondWord == "SOURCE") problemDef = PROBLEM_SOURCE;
    }

    // Equivalent no of points x-direction (check if number)
    if (firstWord == "equivalentPointsX") {
      if (!secondWord.empty() &&
          secondWord.find_first_not_of("0123456789") == std::string::npos)
        equivalentPointsX = atof(secondWord.c_str());
    }

    if (firstWord == "minX") {
      if (!secondWord.empty() &&
          secondWord.find_first_not_of("0123456789-.e") == std::string::npos)
        minx = atof(secondWord.c_str());
    }
    if (firstWord == "maxX") {
      if (!secondWord.empty() &&
          secondWord.find_first_not_of("0123456789-.e") == std::string::npos)
        maxx = atof(secondWord.c_str());
    }
    if (firstWord == "minY") {
      if (!secondWord.empty() &&
          secondWord.find_first_not_of("0123456789-.e") == std::string::npos)
        miny = atof(secondWord.c_str());
    }
    if (firstWord == "maxY") {
      if (!secondWord.empty() &&
          secondWord.find_first_not_of("0123456789-.e") == std::string::npos)
        maxy = atof(secondWord.c_str());
    }

    if (firstWord == "periodicFlagX") {
      if (!secondWord.empty() &&
          secondWord.find_first_not_of("01") == std::string::npos)
        periodicFlagX = atof(secondWord.c_str());
    }
    if (firstWord == "periodicFlagY") {
      if (!secondWord.empty() &&
          secondWord.find_first_not_of("01") == std::string::npos)
        periodicFlagY = atof(secondWord.c_str());
    }

    if (firstWord == "adaptiveMeshFlag") {
      if (!secondWord.empty() &&
          secondWord.find_first_not_of("01") == std::string::npos)
        adaptiveMeshFlag = atof(secondWord.c_str());
    }
    if (firstWord == "maxRefineFactor") {
      if (!secondWord.empty() &&
          secondWord.find_first_not_of("0123456789") == std::string::npos)
        maxRefineFactor = atof(secondWord.c_str());
    }
    if (firstWord == "nStepSkipRefine") {
      if (!secondWord.empty() &&
          secondWord.find_first_not_of("0123456789") == std::string::npos)
        nStepSkipRefine = atof(secondWord.c_str());
    }
    if (firstWord == "nStepSkipCoarsen") {
      if (!secondWord.empty() &&
          secondWord.find_first_not_of("0123456789") == std::string::npos)
        nStepSkipCoarsen = atof(secondWord.c_str());
    }
    if (firstWord == "minError") {
      if (!secondWord.empty() &&
          secondWord.find_first_not_of("0123456789.-e") == std::string::npos)
        minError = atof(secondWord.c_str());
    }
    if (firstWord == "maxError") {
      if (!secondWord.empty() &&
          secondWord.find_first_not_of("0123456789.-e") == std::string::npos)
        maxError = atof(secondWord.c_str());
    }
    if (firstWord == "qualityBound") {
      if (!secondWord.empty() &&
          secondWord.find_first_not_of("0123456789.-e") == std::string::npos)
        qualityBound = atof(secondWord.c_str());
    }
    if (firstWord == "structuredFlag") {
      if (!secondWord.empty() &&
          secondWord.find_first_not_of("012") == std::string::npos)
        structuredFlag = atof(secondWord.c_str());
    }
  }

  inFile.close();

  // Check validity of parameters
  try {
    CheckValidity();
  }
  catch(...) {
    std::cout << "Some Mesh parameters not valid, exiting..." << std::endl;
    throw;
  }

  // Need equivalentPointsX > 1
  real nx = (real) (equivalentPointsX - 1);

  // Convert nx into base resolution requirement
  baseResolution = 0.565*((maxx - minx)/nx)*((maxx - minx)/nx);
  // Maximum adaptive resolution
  maxResolution = baseResolution/((real)(maxRefineFactor*maxRefineFactor));
}

}  // namespace astrix
