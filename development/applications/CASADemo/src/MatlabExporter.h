//                                                                      
// Copyright (C) 2012-2014 Shell International Exploration & Production.
// All rights reserved.
// 
// Developed under license for Shell by PDS BV.
// 
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
// 
#ifndef MATLAB_EXPORTER_H
#define MATLAB_EXPORTER_H

#include <string>
#include <fstream>
#include <vector>

namespace casa
{
   class ScenarioAnalysis;
   class RunCase;
}

/// @brief This class allows to export ScenarioAnalysis data as a Matlab command file
/// User should execute this command file in Matlab/octave to load data calculated bu
class MatlabExporter
{
public:
   MatlabExporter( const std::string & dataFileName );
   ~MatlabExporter();

   void exportScenario( casa::ScenarioAnalysis & sc, const std::string & baseCaseName, const std::string & location );
   
   static void exportObsValues( const std::string & fName, const std::vector<casa::RunCase*> & rcs );

private:
   std::string m_fname;

   void exportDoEInfo(         casa::ScenarioAnalysis & sc, std::ofstream & ofs );
   void exportParametersInfo(  casa::ScenarioAnalysis & sc, std::ofstream & ofs );
   void exportObservablesInfo( casa::ScenarioAnalysis & sc, std::ofstream & ofs );
   void exportRSAProxies(      casa::ScenarioAnalysis & sc, std::ofstream & ofs );
   void exportMCResults(       casa::ScenarioAnalysis & sc, std::ofstream & ofs );
};

#endif // MATLAB_EXPORTER_H
