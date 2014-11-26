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
   if( m_prms.size() < 1 )
   {
      throw ErrorHandler::Exception( ErrorHandler::NonexistingID ) << "Observable is not defined properly";
   }

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

void CmdAddObs::execute( std::auto_ptr<casa::ScenarioAnalysis> & sa )
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

      if ( ErrorHandler::NoError != sa->obsSpace().addObservable( xyzVal ) )
      {
         throw ErrorHandler::Exception( sa->errorCode() ) << sa->errorMessage();
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

      if ( ErrorHandler::NoError != sa->obsSpace().addObservable( wellVal ) )
      {
         throw ErrorHandler::Exception( sa->errorCode() ) << sa->errorMessage();
      }
   }
}

void CmdAddObs::printHelpPage( const char * cmdName )
{
   std::cout << "  " << cmdName << " <target type name> [target parameters]\n\n";
   std::cout << "    Observable (or Target) - could be any data value from the simulation results.\n";
   std::cout << "  For example temperature or VRe at some position and depth for current time.\n\n";

   std::cout << "    Observable reference value - usually it is a measurement of corresponded observable\n";
   std::cout << "  value from the real well. Observables with reference value could be used for calibration workflow.\n\n";

   std::cout << "    Standard deviation value of observable reference value - contains the standard deviations\n";
   std::cout << "  of the measurement noise. Standard deviation (SD) measures the amount of variation or dispersion\n";
   std::cout << "  from the average. A low standard deviation indicates that the data points tend to be very close to\n";
   std::cout << "  the mean (also called expected value); a high standard deviation indicates that the data points are\n";
   std::cout << "  spread out over a large range of values.\n\n";

   std::cout << "  The following list of target types is implemented for this command:\n";
   std::cout << "    XYZPoint - a property value at one point on 3D simulation grid\n";
   std::cout << "    WellTraj - a sequence of property values along some path in 3D simulation grid coordinate system\n\n";

   std::cout << "    XYSPoint <PropName> <X> <Y> <Z> <Age> <ReferenceValue> <StandardDeviationValue> <SA weight> <UA weight>\n";
   std::cout << "    Where:\n";
   std::cout << "       PropName               - property name as it was defined in Cauldron project file\n";
   std::cout << "       X,Y,Z                  - are the target point coordinates in 3D simulation grid\n";
   std::cout << "       Age                    - simulation age in [Ma]\n";
   std::cout << "       ReferenceValue         - reference value (measurements) for this target\n";
   std::cout << "       StandardDeviationValue - std. deviation for reference value\n";
   std::cout << "       SA weight              - weight [0:1] for this target for Sensitivity Analysis (it will used for Pareto diagram)\n";
   std::cout << "       UA weight              - weight [0:1] for this target for Uncertainty Analysis (it will be used in RMSE calculation)\n";
   std::cout << "\n";
   std::cout << "    Example:\n";
   std::cout << "    #       type      prop name       X        Y           Z    Age   Ref   Dev  SWght UWght\n";
   std::cout << "    "<< cmdName << " XYZPoint \"Temperature\" 460001.0 6750001.0 1293.0   0.0  65.7   2.0   1.0  1.0\n";
   std::cout << "\n";

   std::cout << "    WellTraj <TrajFileName> <Age> <StandardDeviationValue> <SA weight> <UA weight>\n";
   std::cout << "    Where:\n";
   std::cout << "       TrajFileName           - Name of text file which keeps trajectory points description: X Y Z RefValue per point per line.\n";
   std::cout << "       PropName               - property name as it was defined in Cauldron project file\n";
   std::cout << "       Age                    - simulation age in [Ma]\n";
   std::cout << "       StandardDeviationValue - std. deviation for reference value (one for all points along trajectory)\n";
   std::cout << "       SA weight              - weight [0:1] for this target for Sensitivity Analysis (it will used for Pareto diagram)\n";
   std::cout << "       UA weight              - weight [0:1] for this target for Uncertainty Analysis (it will be used in RMSE calculation)\n";
   std::cout << "\n";
   std::cout << "    Example:\n";
   std::cout << "    #                 traj file name   prop name   age Dev SWght  UWght\n";
   std::cout << "    " << cmdName << " WellTraj  \"WellVr.in\"       \"Vr\"       0.0 0.1  1.0    1.0\n";
}

