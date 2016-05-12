//                                                                      
// Copyright (C) 2012-2016 Shell International Exploration & Production.
// All rights reserved.
// 
// Developed under license for Shell by PDS BV.
// 
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
// 
#ifndef CASA_GENERATE_MULTI_ONE_D_H
#define CASA_GENERATE_MULTI_ONE_D_H

#include "CasaCmd.h"

/// @brief Define GenerateMultiOneD algorithm for the scenario and run it
class CmdGenerateMultiOneD : public CasaCmd
{
public:
   /// @brief Constructor
   /// @param cmdPrms list of command parameters as set of strings
   CmdGenerateMultiOneD( CasaCommander & parent, const std::vector< std::string > & cmdPrms );

   virtual ~CmdGenerateMultiOneD() { ; }

   /// @brief Run command
   virtual void execute( std::unique_ptr<casa::ScenarioAnalysis> & sa );

   /// @brief Print help page about command purpose and it parameters
   /// @param cmdName - command name - how it is defined in CasaCommander
   static void printHelpPage( const char * cmdName );


   /// @brief Generate for each 1D project scenario .casa file
   void generateScenarioScripts( std::unique_ptr<casa::ScenarioAnalysis> & sa ) const;

protected:
   std::string  m_cldVer;                 // cauldron version
   bool         m_keepHist;               // keep history for 1D runs
   std::string  m_transformation;         // the parameter transformation
};

#endif // CASA_GENERATE_MULTI_ONE_D_H
