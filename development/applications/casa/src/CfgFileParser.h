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
   CfgFileParser() {}

   // load command file 
   void parseFile( const std::string & cmdFile, CasaCommander & cmdQueue );

   // split list of strings divided by sep in to array of strings
   static std::vector<std::string> list2array( const std::string & listOfStr, char sep = ' ' );

   // parse matlab like vector: [1,3,4,5] to vector of doubles
   static std::vector<double> set2array( const std::string & listOfVal, char sep = ',' );

   // read well trajectory file with reference values
   static void readTrajectoryFile( const std::string & fileName, 
                                   std::vector<double> & x,
                                   std::vector<double> & y,
                                   std::vector<double> & z,
                                   std::vector<double> & ref,
                                   std::vector<double> & sdev );

   static bool isNumericPrm( const std::string & prm );

   // join array of string to one string separated with given delimiter
   static std::string implode( const std::vector<std::string> & vos, const char * delim, size_t st = 0 );

protected:
};

#endif
