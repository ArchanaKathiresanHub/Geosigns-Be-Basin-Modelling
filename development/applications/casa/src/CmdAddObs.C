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
#include "ObsSourceRockMapProp.h"
#include "ObsGridPropertyWell.h"
#include "ObsGridPropertyXYZ.h"
#include "ObsTrapProp.h"
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

   int expPrmsNum = 0;
   int expOptPrmNum = 0;
   if (      m_prms[0] == "XYZPoint"            ) { expPrmsNum = 10; expOptPrmNum = 10; }
   else if ( m_prms[0] == "WellTraj"            ) { expPrmsNum = 7;  expOptPrmNum = 7;  }
   else if ( m_prms[0] == "XYPointSorceRockMap" ) { expPrmsNum = 8;  expOptPrmNum = 10; }
   else if ( m_prms[0] == "TrapProp"            ) { expPrmsNum = 6;  expOptPrmNum = 8;  }
   else
   {
      throw ErrorHandler::Exception( ErrorHandler::UndefinedValue ) << "Unknown observable name: " << m_prms[0];
   }

   // check number of command parameters for var parameter
   if ( m_prms.size() != expPrmsNum && m_prms.size() != expOptPrmNum )
   {
      throw ErrorHandler::Exception( ErrorHandler::OutOfRangeValue ) << "Wrong number of parameters for " << m_prms[0] << " observable";
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

   casa::Observable * obsVal    = 0;
   casa::ObsValue   * obsRefVal = 0;
   
   double wgtSA  = 1.0;
   double wgtUA  = 1.0;
   double stdDev = 0.0;

   // Add observable
   if ( m_prms[0] == "XYZPoint" )
   {
      double x      = atof( m_prms[2].c_str() );
      double y      = atof( m_prms[3].c_str() );
      double z      = atof( m_prms[4].c_str() );
      double age    = atof( m_prms[5].c_str() ); // age for the observable
      double refVal = atof( m_prms[6].c_str() ); // observable reference value
      stdDev        = atof( m_prms[7].c_str() ); // std deviation value
      wgtSA         = atof( m_prms[8].c_str() ); // observable weight for Sensitivity Analysis
      wgtUA         = atof( m_prms[9].c_str() ); // observable weight for Uncertainty Analysis

      obsVal    = casa::ObsGridPropertyXYZ::createNewInstance( x, y, z, m_prms[1].c_str(), age );
      obsRefVal = new casa::ObsValueDoubleScalar( obsVal, refVal );
   }
   else if ( m_prms[0] == "WellTraj" ) // file  format X Y Z RefVal
   {
      const std::string & trajFileName =       m_prms[1];           // well trajectory file with reference values
      const std::string & propName     =       m_prms[2];           // property name
      double              age          = atof( m_prms[3].c_str() ); // age for the observable
      stdDev                           = atof( m_prms[4].c_str() ); // std deviation value
      wgtSA                            = atof( m_prms[5].c_str() ); // observable weight for Sensitivity Analysis
      wgtUA                            = atof( m_prms[6].c_str() ); // observable weight for Uncertainty Analysis

      // read trajectory file
      std::vector<double> x;
      std::vector<double> y;
      std::vector<double> z;
      std::vector<double> r;

      CfgFileParser::readTrajectoryFile( trajFileName, x, y, z, r );
      
      // create observable
      obsVal    = casa::ObsGridPropertyWell::createNewInstance( x, y, z, propName.c_str(), age );
      obsRefVal = new casa::ObsValueDoubleArray( obsVal, r );
   }
   else if ( m_prms[0] == "XYPointSorceRockMap" ) // X Y layer_name prop_name age ref value std_dev sa_w ua_w
   {
      const  std::string & srPropName  =       m_prms[1];           // property of source rock calculated in Genex
      double x                         = atof( m_prms[2].c_str() ); // X coordinate for the point on the map
      double y                         = atof( m_prms[3].c_str() ); // Y coordinate for the point on the map
      const  std::string & srLayerName =       m_prms[4];           // source rock layer name
      double age                       = atof( m_prms[5].c_str() ); // age for the observable

      obsVal    = casa::ObsSourceRockMapProp::createNewInstance( x, y, srLayerName.c_str(), srPropName.c_str(), age );
      // optional parameters
      if ( m_prms.size() == 10 )
      {
         double refVal                 = atof( m_prms[6].c_str() ); // observable reference value
         stdDev                        = atof( m_prms[7].c_str() ); // std deviation value
         
         wgtSA                         = atof( m_prms[8].c_str() ); // observable weight for Sensitivity Analysis
         wgtUA                         = atof( m_prms[9].c_str() ); // observable weight for Uncertainty Analysis
         obsRefVal = new casa::ObsValueDoubleScalar( obsVal, refVal );
      }
      else
      {
         wgtSA                         = atof( m_prms[6].c_str() ); // observable weight for Sensitivity Analysis
         wgtUA                         = atof( m_prms[7].c_str() ); // observable weight for Uncertainty Analysis
      }
   }
   else if ( m_prms[0] == "TrapProp" ) // X Y reservoir_name prop_name age
   {
      const std::string & propName = m_prms[1];                 // trap property
      double x                     = atof( m_prms[2].c_str() ); // trap X coord
      double y                     = atof( m_prms[3].c_str() ); // trap Y coord
      const std::string & resName  = m_prms[4];                 // reservoir name
      double age                   = atof( m_prms[5].c_str() ); // age for the observable
 
      obsVal = casa::ObsTrapProp::createNewInstance( x, y, resName.c_str(), propName.c_str(), age );

      if ( m_prms.size() == 8 )
      {
         wgtSA                     = atof( m_prms[6].c_str() ); // observable weight for Sensitivity Analysis
         wgtUA                     = atof( m_prms[7].c_str() ); // observable weight for Uncertainty Analysis
      }
   }

   if ( obsVal )
   {
      if ( obsRefVal ) obsVal->setReferenceValue( obsRefVal, stdDev );
      obsVal->setSAWeight( wgtSA );
      obsVal->setUAWeight( wgtUA );

      if ( ErrorHandler::NoError != sa->obsSpace().addObservable( obsVal ) )
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
   std::cout << "    WellTraj - a sequence of property values along some path in 3D simulation grid coordinate system\n";
   std::cout << "    XYPointSorceRockMap - a surface property value for source rock layer\n";
   std::cout << "    TrapProp - trap property, trap is defined by X,Y coordinates and reservoir name\n";
   std::cout << "\n";
   std::cout << "    XYZPoint <PropName> <X> <Y> <Z> <Age> <ReferenceValue> <StandardDeviationValue> <SA weight> <UA weight>\n";
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

   std::cout << "    XYPointSorceRockMap <PropName> <X> <Y> <LayerName> <Age> [<ReferenceValue> <StandardDeviationValue>] <SA weight> <UA weight>\n";
   std::cout << "    Where:\n";
   std::cout << "       PropName               - property name as it was defined in Cauldron project file\n";
   std::cout << "       X,Y                    - are the aerial target point coordinates\n";
   std::cout << "       LayerName              - source rock layer name\n";
   std::cout << "       Age                    - simulation age in [Ma]\n";
   std::cout << "       ReferenceValue         - (optional) reference value (measurements) for this target\n";
   std::cout << "       StandardDeviationValue - (optional) standard deviation for reference value\n";
   std::cout << "       SA weight              - weight [0:1] for this target for Sensitivity Analysis (it will used for Pareto diagram)\n";
   std::cout << "       UA weight              - weight [0:1] for this target for Uncertainty Analysis (it will be used in RMSE calculation)\n";
   std::cout << "\n";
   std::cout << "    Example:\n";
   std::cout << "    #       type      prop name       X        Y           Z    Age   Ref   Dev  SWght UWght\n";
   std::cout << "    "<< cmdName << " XYPointSorceRockMap \"OilExpelledCumulative\" 460001.0 6750001.0 \"Lower Jurassic\"  1.0  1.0\n";
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

   std::cout << "    TrapProp <PropName> <X> <Y> <ReservoirName> <Age> [<SA weight> <UA weight>]\n";
   std::cout << "    Where:\n";
   std::cout << "       PropName               - trap property name (supported properties list - see below)\n";
   std::cout << "       X,Y                    - are the aerial target point coordinates\n";
   std::cout << "       LayerName              - source rock layer name\n";
   std::cout << "       Age                    - simulation age in [Ma]\n";
   std::cout << "       SA weight              - weight [0:1] for this target for Sensitivity Analysis (it will used for Pareto diagram)\n";
   std::cout << "       UA weight              - weight [0:1] for this target for Uncertainty Analysis (it will be used in RMSE calculation)\n";
   std::cout << "\n";
   std::cout << "    Example:\n";
   std::cout << "    #       type      prop name       X        Y          ReservName    Age   SWght UWght\n";
   std::cout << "    "<< cmdName << " TrapProp \"GOR\" 460001.0 6750001.0  \"Res\"       0.0    1.0  1.0\n";
   std::cout << "\n";
   std::cout << "    Supported trap property list:\n";
   std::cout << "       VolumeFGIIP [m3] -             Volume of free gas initially in place\n";
   std::cout << "       VolumeCIIP [m3] -              Volume of condensate initially in place\n";
   std::cout << "       VolumeSGIIP [m3] -             Volume of solution gas initially in place\n";
   std::cout << "       VolumeSTOIIP [m3] -            Volume of stock tank oil initially in place\n";
   std::cout << "       VolumeLiquid [m3] -            Volume of reservoir liquid phase\n";
   std::cout << "       VolumeVapour [m3] -            Volume of reservoir vapour phase\n";
   std::cout << "       DensityFGIIP [kg/m3] -         Density of free gas initially in place\n";
   std::cout << "       DensityCIIP [kg/m3] -          Density of condensate initially in place\n";
   std::cout << "       DensitySGIIP [kg/m3] -         Density of solution gas initially in place\n";
   std::cout << "       DensitySTOIIP [kg/m3] -        Density of stock tank oil initially in place\n";
   std::cout << "       DensityLiquid [kg/m3] -        Density of reservoir liquid phase\n";
   std::cout << "       DensityVapour [kg/m3] -        Density of reservoir vapour phase\n";
   std::cout << "       ViscosityFGIIP [Pa*s] -        Viscosity of free gas initially in place\n";
   std::cout << "       ViscosityCIIP [Pa*s] -         Viscosity of condensate initially in place\n";
   std::cout << "       ViscositySGIIP [Pa*s] -        Viscosity of solution gas initially in place\n";
   std::cout << "       ViscositySTOIIP [Pa*s] -       Viscosity of stock tank oil initially in place\n";
   std::cout << "       ViscosityLiquid [Pa*s] -       Viscosity of reservoir liquid phase\n";
   std::cout << "       ViscosityVapour [Pa*s] -       Viscosity of reservoir vapour phase\n";
   std::cout << "       MassFGIIP [kg] -               Mass of free gas initially in place\n";
   std::cout << "       MassCIIP [kg] -                Mass of condensate initially in place\n";
   std::cout << "       MassSGIIP [kg] -               Mass of solution gas initially in place\n";
   std::cout << "       MassSTOIIP [kg] -              Mass of stock tank oil initially in place\n";
   std::cout << "       MassLiquid [kg] -              Mass of reservoir liquid phase\n";
   std::cout << "       MassVapour [kg] -              Mass of reservoir vapour phase\n";
   std::cout << "       CGR [m3/m3] -                  Condensate Gas Ratio: VolumeCIIP / VolumeFGIIP\n";
   std::cout << "       GOR [m3/m3] -                  Gas Oil Ratio: VolumeSGIIP / VolumeSTOIIP\n";
   std::cout << "       OilAPI [] -                    API of STOIIP\n";
   std::cout << "       CondensateAPI [] -             API of CIIP\n";
   std::cout << "       GasWetnessFGIIP [mole/mole] -  C1 / Sum (C2 - C5) of FGIIP\n";
   std::cout << "       GasWetnessSGIIP [mole/mole] -  C1 / Sum (C2 - C5) of SGIIP\n";
   std::cout << "       CEPLiquid [MPa] -              CEP\n";
   std::cout << "       CEPVapour [MPa] -              CEP\n";
   std::cout << "       FracturePressure [MPa] -       Fracture pressure of the trap\n";
   std::cout << "       ColumnHeightLiquid [m] -       Height of the liquid column\n";
   std::cout << "       ColumnHeightVapour [m] -       Height of the vapour column\n";
   std::cout << "       GOC [m] -                      Depth of Vapour-Liquid contact\n";
   std::cout << "       OWC [m] -                      Depth of Liquid-Water contact\n";
   std::cout << "       SpillDepth [m] -               Spill depth\n";
   std::cout << "       SealPermeability [mD] -        Seal permeability\n";
}

