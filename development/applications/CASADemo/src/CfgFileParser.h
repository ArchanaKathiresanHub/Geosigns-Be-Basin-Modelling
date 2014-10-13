//                                                                      
// Copyright (C) 2012-2014 Shell International Exploration & Production.
// All rights reserved.
// 
// Developed under license for Shell by PDS BV.
// 
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
// 

#ifndef CFG_FILE_PARSER_H
#define CFG_FILE_PARSER_H

#include <string>
#include <vector>
#include <iostream>

class CasaCommander;

class CfgFileParser
{
public:
   enum CfgCommand
   {
      app,          // add cauldron application to pipeline
      base_project, // set scenario base case 
      varprm,       // create variable parameter
      target,       // create observable
      doe,          // create doe
      location,     // where cases will be generated, run mutator
      run,          // run planned DoE experiments
      response,     // calculate coefficients for response surface approximation
      evaluate,     // calculate observables value using response surface approximation
      exportMatlab  // export all data to matlab file
   };

   CfgFileParser() {;}

   // load command file 
   void parseFile( const std::string & cmdFile, CasaCommander & cmdQueue );

   // split list of strings divided by sep in to array of strings
   static std::vector<std::string> list2array( const std::string & listOfStr, char sep = ' ');

   // read well trajectory file with reference values
   static void readTrajectoryFile( const std::string & fileName, 
                                   std::vector<double> & x,
                                   std::vector<double> & y,
                                   std::vector<double> & z,
                                   std::vector<double> & ref );

   // read parameters value from plain data file
   static void readParametersValueFile( const std::string & fileName, std::vector< std::vector<double> > & dataVals );

protected:
};

#endif
