//                                                                      
// Copyright (C) 2012-2014 Shell International Exploration & Production.
// All rights reserved.
// 
// Developed under license for Shell by PDS BV.
// 
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
// 

#include "CasaCommander.h"
#include "CmdAddObs.h"
#include "CfgFileParser.h"

#include "casaAPI.h"
#include "ObsSpace.h"
#include "ObsGridPropertyWell.h"
#include "ObsGridPropertyXYZ.h"
#include "ObsValueDoubleScalar.h"
#include "ObsValueDoubleArray.h"

#include <cstdlib>
#include <iostream>

CmdAddObs::CmdAddObs( CasaCommander & parent, const std::vector< std::string > & cmdPrms ) : CasaCmd( parent, cmdPrms )
{
   assert( m_prms.size() > 1 );

   if ( m_prms[0] != "XYZPoint" && m_prms[0] != "WellTraj" )
   {
      throw ErrorHandler::Exception( ErrorHandler::UndefinedValue ) << "Unknown target name: " << m_prms[0];
   }
   // check number of command parameters for var parameter
   if ( m_prms[0] == "XYZPoint" && m_prms.size() != 10 || m_prms[0] == "WellTraj" && m_prms.size() != 7 )
   {
      throw ErrorHandler::Exception( ErrorHandler::OutOfRangeValue ) << "Wrong number of parameters for " << m_prms[0];
   }
}

void CmdAddObs::execute( casa::ScenarioAnalysis & sa )
{
   if ( m_commander.verboseLevel() > CasaCommander::Quiet )
   {
      std::cout << "Add observable: " << m_prms[0] << "(";
      for ( size_t i = 1; i < m_prms.size(); ++i )
      {
         std::cout << m_prms[i] << ((i == m_prms.size() - 1) ? "" : ",");
      }
      std::cout << ")" << std::endl;
   }

   // Add observable
   if ( m_prms[0] == "XYZPoint" )
   {
      double x      = atof( m_prms[2].c_str() );
      double y      = atof( m_prms[3].c_str() );
      double z      = atof( m_prms[4].c_str() );
      double age    = atof( m_prms[5].c_str() ); // age for the observable
      double refVal = atof( m_prms[6].c_str() ); // observable reference value
      double stdDev = atof( m_prms[7].c_str() ); // std deviation value
      double wgtSA  = atof( m_prms[8].c_str() ); // observable weight for Sensitivity Analysis
      double wgtUA  = atof( m_prms[9].c_str() ); // observable weight for Uncertainty Analysis

      casa::Observable * xyzVal = casa::ObsGridPropertyXYZ::createNewInstance( x, y, z, m_prms[1].c_str(), age );
      xyzVal->setReferenceValue( new casa::ObsValueDoubleScalar( xyzVal, refVal ), stdDev );
      xyzVal->setSAWeight( wgtSA );
      xyzVal->setUAWeight( wgtUA );

      if ( ErrorHandler::NoError != sa.obsSpace().addObservable( xyzVal ) )
      {
         throw ErrorHandler::Exception( sa.errorCode() ) << sa.errorMessage();
      }
   }
   else if ( m_prms[0] == "WellTraj" ) // file  format X Y Z RefVal
   {
      const std::string & trajFileName = m_prms[1];                 // well trajectory file with reference values
      const std::string & propName     = m_prms[2];                 // property name
      double              age          = atof( m_prms[3].c_str() ); // age for the observable
      double              stdDev       = atof( m_prms[4].c_str() ); // std deviation value
      double              wgtSA        = atof( m_prms[5].c_str() ); // observable weight for Sensitivity Analysis
      double              wgtUA        = atof( m_prms[6].c_str() ); // observable weight for Uncertainty Analysis

      // read trajectory file
      std::vector<double> x;
      std::vector<double> y;
      std::vector<double> z;
      std::vector<double> r;

      CfgFileParser::readTrajectoryFile( trajFileName, x, y, z, r );
      
      // create observable
      casa::Observable * wellVal = casa::ObsGridPropertyWell::createNewInstance( x, y, z, propName.c_str(), age );
      wellVal->setReferenceValue( new casa::ObsValueDoubleArray( wellVal, r ), stdDev );
      wellVal->setSAWeight( wgtSA );
      wellVal->setUAWeight( wgtUA );

      if ( ErrorHandler::NoError != sa.obsSpace().addObservable( wellVal ) )
      {
         throw ErrorHandler::Exception( sa.errorCode() ) << sa.errorMessage();
      }
   }
}
