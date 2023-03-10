//
// Copyright (C) 2012-20166666 Shell International Exploration & Production.
// All rights reserved.
//
// Developed under license for Shell by PDS BV.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#ifndef CASA_CMD_H
#define CASA_CMD_H

// STL
#include <string>
#include <vector>
#include <memory>

namespace casa
{
   class ScenarioAnalysis;
}

class CasaCommander;


/// @brief Define interface to any CASADemo command and keeps the common part
class CasaCmd
{
public:
   virtual ~CasaCmd() { ; }

   /// @brief Run command
   virtual void execute( std::unique_ptr<casa::ScenarioAnalysis> & sa ) = 0;

   /// @brief Get the list of command parameters
   virtual std::vector< std::string >  cmdParameters() const { return m_prms; }

protected:
   /// @brief Constructor
   /// @param cmdPrms list of command parameters as set of strings
   CasaCmd( CasaCommander & parent, const std::vector< std::string > & cmdPrms ) : m_commander( parent )
   {
      m_prms.insert( m_prms.begin(), cmdPrms.begin(), cmdPrms.end() );
   }

   std::vector< std::string > m_prms;      ///< list of command parameters
   CasaCommander            & m_commander; ///< queue of all commands and intercommand data router
};

typedef std::shared_ptr<CasaCmd> SharedCmdPtr;

#endif // CASA_CMD_H
