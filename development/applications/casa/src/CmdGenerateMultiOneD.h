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

   /// @brief Generate for each 1D project scenario .casa file
   void generateScenarioScripts( std::unique_ptr<casa::ScenarioAnalysis> & sa ) const;

protected:
};

#endif // CASA_GENERATE_MULTI_ONE_D_H
