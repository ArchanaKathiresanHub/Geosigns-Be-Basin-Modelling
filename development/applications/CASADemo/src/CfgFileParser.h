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

class CfgFileParser
{
public:
   typedef enum
   {
      app,          // add cauldron application to pipeline
      base_project, // set scenario base case 
      varprm,       // create variable parameter
      target,       // create observable
      doe,          // create doe
      location,     // where cases will be generated, run mutator
      run,          // run planned DoE experiments
      response,     // calculate coefficients for response surface approximation
      evaluate      // calculate observables value using response surface approximation
   } CfgCommand;

   CfgFileParser() {;}

   // load command file 
   void parseFile( const std::string & cmdFile );

   // number of commands in file
   size_t cmdsNum() { return m_cmdList.size(); }
   
   // access to commands
   CfgCommand cmd( size_t i ) { return m_cmdList[i]; }
   // access to command parameters
   const std::vector< std::string > & parameters( size_t i ) { return m_cmdPrms[i]; }

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
   


   // printing parsed file
   friend std::ostream & operator << ( std::ostream & ost, const CfgFileParser & cfg );

protected:
   std::vector< CfgCommand >               m_cmdList;
   std::vector< std::vector<std::string> > m_cmdPrms;

};

std::ostream & operator << ( std::ostream & ost, const CfgFileParser & cfg );

#endif
