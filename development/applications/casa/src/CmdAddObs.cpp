//                                                                      
// Copyright (C) 2012-2014 Shell International Exploration & Production.
// All rights reserved.
// 
// Developed under license for Shell by PDS BV.
// 
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
// 

// casa app
#include "CasaCommander.h"
#include "CmdAddObs.h"
#include "CfgFileParser.h"

// CASA API
#include "casaAPI.h"
#include "ObsSpace.h"
#include "ObsSourceRockMapProp.h"
#include "ObsGridPropertyWell.h"
#include "ObsGridPropertyXYZ.h"
#include "ObsTrapProp.h"
#include "ObsTrapDerivedProp.h"
#include "ObsValueDoubleScalar.h"
#include "ObsValueDoubleArray.h"

// LogHandler
#include "LogHandler.h"

// STD C
#include <cstdlib>

// STL
#include <iostream>
#include <sstream>
#include <map>

// File path
#include "FilePath.h"

// Class which define interface for observable object factory
class ObsType
{
public:
   /// @brief Destructor
   virtual ~ObsType() {;}

   /// @brief create observable in scenario
   virtual casa::Observable * createOservableObject( const std::string & name, std::vector<std::string> & prms ) const = 0;
   
   /// @brief Get expected parameters number for the observable type
   virtual size_t expectedParametersNumber() const = 0;
   /// @brief Get optional parameters number for the observable type
   virtual size_t optionalParametersNumber() const = 0;

   /// @brief Get observable type name
   virtual std::string name() const = 0;
   /// @brief Get short description for observable type
   virtual std::string description() const = 0;
   /// @brief Get full description for observable type
   virtual std::string fullDescription() const = 0;
    /// @brief Get observable example for using in commands script file
   virtual std::string usingExample( const char * cmdName ) const = 0;

protected:
   ObsType() {;}
};

////////////////////////////////////////////////////////////////
/// Observables type definitions
////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////
// XYZPoint observable 
////////////////////////////////////////////////////////////////
//
class XYZPoint : public ObsType
{
public:
   XYZPoint()  {;}
   virtual ~XYZPoint() {;}

   virtual casa::Observable * createOservableObject( const std::string & name, std::vector<std::string> & prms ) const
   {
      size_t pos = 2;
      double x   = atof( prms[pos++].c_str() );
      double y   = atof( prms[pos++].c_str() );
      double z   = atof( prms[pos++].c_str() );
      double age = atof( prms[pos++].c_str() ); // age for the observable

      casa::Observable * obsVal = casa::ObsGridPropertyXYZ::createNewInstance( x, y, z, prms[1].c_str(), age, name );
     
      if ( prms.size() == 10 )
      {
         double refVal = atof( prms[pos++].c_str() ); // observable reference value
         double stdDev = atof( prms[pos++].c_str() ); // std deviation value

         obsVal->setReferenceValue( new casa::ObsValueDoubleScalar( obsVal, refVal ), new casa::ObsValueDoubleScalar( obsVal, stdDev ) );
      }

      if ( prms.size() > 7 )
      {
         double wgtSA  = atof( prms[pos++].c_str() ); // observable weight for Sensitivity Analysis
         double wgtUA  = atof( prms[pos++].c_str() ); // observable weight for Uncertainty Analysis

         obsVal->setSAWeight( wgtSA );
         obsVal->setUAWeight( wgtUA );
      }

      return obsVal;
   }
      
   size_t expectedParametersNumber() const { return 5;  } // x, y, z, age
   size_t optionalParametersNumber() const { return 4; } // +refVal, stdDev, saW, uaW

   virtual std::string name() const { return "XYZPoint"; }

   virtual std::string description() const { return "a property value at one point on 3D simulation grid"; }

   virtual std::string fullDescription() const
   {
      std::ostringstream oss;
      oss << "    XYZPoint <PropName> <X> <Y> <Z> <Age> [<ReferenceValue>] [<StandardDeviationValue>] <SA weight> <UA weight>\n";
      oss << "    Where:\n";
      oss << "       PropName               - property name as it was defined in Cauldron project file\n";
      oss  << "       X,Y,Z                  - are the target point coordinates in 3D simulation grid\n";
      oss << "       Age                    - simulation age in [Ma]\n";
      oss << "       ReferenceValue         - (Optional) reference value (measurements) for this target\n";
      oss << "       StandardDeviationValue - (Optional) std. deviation for reference value\n";
      oss << "       SA weight              - weight [0:1] for this target for Sensitivity Analysis (it will used for Pareto diagram)\n";
      oss << "       UA weight              - weight [0:1] for this target for Uncertainty Analysis (it will be used in RMSE calculation)\n";
      return oss.str();
   }

   virtual std::string usingExample( const char * cmdName ) const
   {
      std::ostringstream oss;
      oss << "    #       type      prop name       X        Y           Z    Age   Ref   Dev  SWght UWght\n";
      oss << "    "<< cmdName << " XYZPoint \"Temperature\" 460001.0 6750001.0 1293.0   0.0  65.7   2.0   1.0  1.0\n";
      return oss.str();
   }
};

////////////////////////////////////////////////////////////////
// WellTraj observable 
////////////////////////////////////////////////////////////////
//
class WellTraj : public ObsType
{
public:
   WellTraj()  {;}
   virtual ~WellTraj() {;}

   virtual casa::Observable * createOservableObject( const std::string & name, std::vector<std::string> & prms ) const
   {
      const std::string & trajFileName =       prms[1];           // well trajectory file with reference values
      const std::string & propName     =       prms[2];           // property name
      double              age          = atof( prms[3].c_str() ); // age for the observable
      double              stdDev       = atof( prms[4].c_str() ); // std deviation value
      double              wgtSA        = atof( prms[5].c_str() ); // observable weight for Sensitivity Analysis
      double              wgtUA        = atof( prms[6].c_str() ); // observable weight for Uncertainty Analysis

      //well trajectories files must be indicated with a full path
      ibs::FilePath trj( trajFileName );
      if ( prms[1]!= trj.fullPath( ).path( )  )
      {
         prms[1] = trj.fullPath( ).path( );
      }
      
#ifdef _WIN32
      std::replace( prms[1].begin( ), prms[1].end( ), '\\', '/' );
#endif
      // read trajectory file
      std::vector<double> x, y, z, r, sdev;
      CfgFileParser::readTrajectoryFile( trajFileName, x, y, z, r, sdev );

      // If no sdev was specified in the trajFileName, fill sdev vector with stdDev (unique value for all measurements)
      if ( sdev.empty() )
         for ( size_t i = 0; i != r.size( ); ++i ) sdev.push_back( stdDev );
      

      // create observable
      casa::Observable * obsVal = casa::ObsGridPropertyWell::createNewInstance( x, y, z, propName.c_str(), age, name );
      new casa::ObsValueDoubleArray( obsVal, sdev );
      obsVal->setReferenceValue( new casa::ObsValueDoubleArray( obsVal, r ), new casa::ObsValueDoubleArray( obsVal, sdev ) );

      obsVal->setSAWeight( wgtSA );
      obsVal->setUAWeight( wgtUA );

      return obsVal;
   }
      
   size_t expectedParametersNumber() const { return 6; } 
   size_t optionalParametersNumber() const { return 0; }

   virtual std::string name() const { return "WellTraj"; }

   virtual std::string description() const { return "a sequence of property values along some path in 3D simulation grid coordinate system"; }

   virtual std::string fullDescription() const
   {
      std::ostringstream oss;
      oss << "    WellTraj <TrajFileName> <Age> <StandardDeviationValue> <SA weight> <UA weight>\n";
      oss << "    Where:\n";
      oss << "       TrajFileName           - Name of text file which keeps trajectory points description: X Y Z RefValue per point per line.\n";
      oss << "       PropName               - property name as it was defined in Cauldron project file\n";
      oss << "       Age                    - simulation age in [Ma]\n";
      oss << "       StandardDeviationValue - std. deviation for reference value (one for all points along trajectory)\n";
      oss << "       SA weight              - weight [0:1] for this target for Sensitivity Analysis (it will used for Pareto diagram)\n";
      oss << "       UA weight              - weight [0:1] for this target for Uncertainty Analysis (it will be used in RMSE calculation)\n";
      return oss.str();
   }

   virtual std::string usingExample( const char * cmdName ) const
   {
      std::ostringstream oss;
      oss << "    #                 traj file name   prop name   age Dev SWght  UWght\n";
      oss << "    " << cmdName << " WellTraj  \"WellVr.in\"       \"Vr\"       0.0 0.1  1.0    1.0\n";
      return oss.str();
   }
};

////////////////////////////////////////////////////////////
// XYPointSorceRockMap
////////////////////////////////////////////////////////////
//
class XYPointSorceRockMap : public ObsType
{
public:
   XYPointSorceRockMap() {;}
   virtual ~XYPointSorceRockMap() {;}

   virtual casa::Observable * createOservableObject( const std::string & name, std::vector<std::string> & prms ) const
   {
      size_t pos = 1;
      const  std::string & srPropName  =       prms[pos++];           // property of source rock calculated in Genex
      double x                         = atof( prms[pos++].c_str() ); // X coordinate for the point on the map
      double y                         = atof( prms[pos++].c_str() ); // Y coordinate for the point on the map
      const  std::string & srLayerName =       prms[pos++];           // source rock layer name
      double age                       = atof( prms[pos++].c_str() ); // age for the observable

      casa::Observable * obsVal = casa::ObsSourceRockMapProp::createNewInstance( x, y, srLayerName.c_str(), srPropName.c_str(), age, name );

      // optional parameters
      if ( prms.size() == 10 )
      {
         double refVal                 = atof( prms[pos++].c_str() ); // observable reference value
         double stdDev                 = atof( prms[pos++].c_str() ); // std deviation value
         
         obsVal->setReferenceValue( new casa::ObsValueDoubleScalar( obsVal, refVal ), new casa::ObsValueDoubleScalar( obsVal, stdDev ) );
      }
      if ( prms.size() > 6 )
      {
         double wgtSA                  = atof( prms[pos++].c_str() ); // observable weight for Sensitivity Analysis
         double wgtUA                  = atof( prms[pos++].c_str() ); // observable weight for Uncertainty Analysis

         obsVal->setSAWeight( wgtSA );
         obsVal->setUAWeight( wgtUA );
      }
      return obsVal;
   }
      
   size_t expectedParametersNumber() const { return 5; } 
   size_t optionalParametersNumber() const { return 4; }

   virtual std::string name() const { return "XYPointSorceRockMap"; }

   virtual std::string description() const { return "a surface property value for source rock layer"; }

   virtual std::string fullDescription() const
   {
      std::ostringstream oss;
      oss << "    XYPointSorceRockMap <PropName> <X> <Y> <LayerName> <Age> [<ReferenceValue> <StandardDeviationValue>] <SA weight> <UA weight>\n";
      oss << "    Where:\n";
      oss << "       PropName               - property name as it was defined in Cauldron project file\n";
      oss << "       X,Y                    - are the aerial target point coordinates\n";
      oss << "       LayerName              - source rock layer name\n";
      oss << "       Age                    - simulation age in [Ma]\n";
      oss << "       ReferenceValue         - (optional) reference value (measurements) for this target\n";
      oss << "       StandardDeviationValue - (optional) standard deviation for reference value\n";
      oss << "       SA weight              - weight [0:1] for this target for Sensitivity Analysis (it will used for Pareto diagram)\n";
      oss << "       UA weight              - weight [0:1] for this target for Uncertainty Analysis (it will be used in RMSE calculation)\n";
      return oss.str();
   }

   virtual std::string usingExample( const char * cmdName ) const
   {
      std::ostringstream oss;
      oss << "    #       type      prop name       X        Y           Z    Age   Ref   Dev  SWght UWght\n";
      oss << "    "<< cmdName << " XYPointSorceRockMap \"OilExpelledCumulative\" 460001.0 6750001.0 \"Lower Jurassic\"  1.0  1.0\n";
      return oss.str();
   }
};

class TrapProp : public ObsType
{
public:
   TrapProp() {;}
   virtual ~TrapProp() {;}

   virtual casa::Observable * createOservableObject( const std::string & name, std::vector<std::string> & prms ) const
   {
      size_t pos                   = 1;
      const std::string & propName =       prms[pos++];           // trap property
      double x                     = atof( prms[pos++].c_str() ); // trap X coord
      double y                     = atof( prms[pos++].c_str() ); // trap Y coord
      const std::string & resName  =       prms[pos++];           // reservoir name
      double age                   = atof( prms[pos++].c_str() ); // age for the observable

      casa::Observable * obsVal = 0;
      if ( propName == "GOR"    || propName == "CGR" ||
           propName == "OilAPI" || propName == "CondensateAPI"
         )
      {
         obsVal = casa::ObsTrapDerivedProp::createNewInstance( x, y, resName.c_str(), propName.c_str(), age, name );
      }
      else
      {
         obsVal = casa::ObsTrapProp::createNewInstance( x, y, resName.c_str(), propName.c_str(), age, name );
      }

      // optional parameters
      if ( prms.size() == 10 )
      {
         double refVal                 = atof( prms[pos++].c_str() ); // observable reference value
         double stdDev                 = atof( prms[pos++].c_str() ); // std deviation value
         
         obsVal->setReferenceValue( new casa::ObsValueDoubleScalar( obsVal, refVal ), new casa::ObsValueDoubleScalar( obsVal, stdDev ) );
      }
      if ( prms.size() > 6 )
      {
         double wgtSA                  = atof( prms[pos++].c_str() ); // observable weight for Sensitivity Analysis
         double wgtUA                  = atof( prms[pos++].c_str() ); // observable weight for Uncertainty Analysis

         obsVal->setSAWeight( wgtSA );
         obsVal->setUAWeight( wgtUA );
      }
      return obsVal;
   }
      
   size_t expectedParametersNumber() const { return 5; } 
   size_t optionalParametersNumber() const { return 4; }

   virtual std::string name() const { return "TrapProp"; }

   virtual std::string description() const { return "trap property, trap is defined by X,Y coordinates and reservoir name"; }

   virtual std::string fullDescription() const
   {
      std::ostringstream oss;
      oss << "    TrapProp <PropName> <X> <Y> <ReservoirName> <Age> [<SA weight> <UA weight>]\n";
      oss << "    Where:\n";
      oss << "       PropName               - trap property name (supported properties list - see below)\n";
      oss << "       X,Y                    - are the aerial target point coordinates\n";
      oss << "       LayerName              - source rock layer name\n";
      oss << "       Age                    - simulation age in [Ma]\n";
      oss << "       SA weight              - weight [0:1] for this target for Sensitivity Analysis (it will used for Pareto diagram)\n";
      oss << "       UA weight              - weight [0:1] for this target for Uncertainty Analysis (it will be used in RMSE calculation)\n";
      oss << "\n"; 
      oss << "    Supported trap property list:\n";
      oss << "       VolumeFGIIP [m3] -             Volume of free gas initially in place\n";
      oss << "       VolumeCIIP [m3] -              Volume of condensate initially in place\n";
      oss << "       VolumeSGIIP [m3] -             Volume of solution gas initially in place\n";
      oss << "       VolumeSTOIIP [m3] -            Volume of stock tank oil initially in place\n";
      oss << "       VolumeLiquid [m3] -            Volume of reservoir liquid phase\n";
      oss << "       VolumeVapour [m3] -            Volume of reservoir vapour phase\n";
      oss << "       DensityFGIIP [kg/m3] -         Density of free gas initially in place\n";
      oss << "       DensityCIIP [kg/m3] -          Density of condensate initially in place\n";
      oss << "       DensitySGIIP [kg/m3] -         Density of solution gas initially in place\n";
      oss << "       DensitySTOIIP [kg/m3] -        Density of stock tank oil initially in place\n";
      oss << "       DensityLiquid [kg/m3] -        Density of reservoir liquid phase\n";
      oss << "       DensityVapour [kg/m3] -        Density of reservoir vapour phase\n";
      oss << "       ViscosityFGIIP [Pa*s] -        Viscosity of free gas initially in place\n";
      oss << "       ViscosityCIIP [Pa*s] -         Viscosity of condensate initially in place\n";
      oss << "       ViscositySGIIP [Pa*s] -        Viscosity of solution gas initially in place\n";
      oss << "       ViscositySTOIIP [Pa*s] -       Viscosity of stock tank oil initially in place\n";
      oss << "       ViscosityLiquid [Pa*s] -       Viscosity of reservoir liquid phase\n";
      oss << "       ViscosityVapour [Pa*s] -       Viscosity of reservoir vapour phase\n";
      oss << "       MassFGIIP [kg] -               Mass of free gas initially in place\n";
      oss << "       MassCIIP [kg] -                Mass of condensate initially in place\n";
      oss << "       MassSGIIP [kg] -               Mass of solution gas initially in place\n";
      oss << "       MassSTOIIP [kg] -              Mass of stock tank oil initially in place\n";
      oss << "       MassLiquid [kg] -              Mass of reservoir liquid phase\n";
      oss << "       MassVapour [kg] -              Mass of reservoir vapour phase\n";
      oss << "       CGR [m3/m3] -                  Condensate Gas Ratio: VolumeCIIP / VolumeFGIIP\n";
      oss << "       GOR [m3/m3] -                  Gas Oil Ratio: VolumeSGIIP / VolumeSTOIIP\n";
      oss << "       OilAPI [] -                    API of STOIIP\n";
      oss << "       CondensateAPI [] -             API of CIIP\n";
      oss << "       GasWetnessFGIIP [mole/mole] -  C1 / Sum (C2 - C5) of FGIIP\n";
      oss << "       GasWetnessSGIIP [mole/mole] -  C1 / Sum (C2 - C5) of SGIIP\n";
      oss << "       CEPLiquid [MPa] -              CEP\n";
      oss << "       CEPVapour [MPa] -              CEP\n";
      oss << "       FracturePressure [MPa] -       Fracture pressure of the trap\n";
      oss << "       ColumnHeightLiquid [m] -       Height of the liquid column\n";
      oss << "       ColumnHeightVapour [m] -       Height of the vapour column\n";
      oss << "       GOC [m] -                      Depth of Vapour-Liquid contact\n";
      oss << "       OWC [m] -                      Depth of Liquid-Water contact\n";
      oss << "       SpillDepth [m] -               Spill depth\n";
      oss << "       SealPermeability [mD] -        Seal permeability\n";
      return oss.str();
   }

   virtual std::string usingExample( const char * cmdName ) const
   {
      std::ostringstream oss;
      oss << "    #       type      prop name       X        Y          ReservName    Age   SWght UWght\n";
      oss << "    "<< cmdName << " TrapProp \"GOR\" 460001.0 6750001.0  \"Res\"       0.0    1.0  1.0\n";
      return oss.str();
   }
};


////////////////////////////////////////////////////////////////
/// Observable types factory class
////////////////////////////////////////////////////////////////

class ObsTypesFactory
{
public:
   ObsTypesFactory()
   {
      m_obsType["XYZPoint"]            = new XYZPoint();
      m_obsType["WellTraj"]            = new WellTraj();
      m_obsType["XYPointSorceRockMap"] = new XYPointSorceRockMap();
      m_obsType["TrapProp"]            = new TrapProp();
   }

   ~ObsTypesFactory() { for ( std::map<std::string, ObsType*>::iterator it = m_obsType.begin(); it != m_obsType.end(); ++it ) { delete it->second; } };

   const ObsType * factory( const std::string & name ) const { return m_obsType.count( name ) ? m_obsType.find(name)->second : 0; }
   
   std::vector<std::string> typesNameList() const
   {
      std::vector<std::string> ret;
      for ( std::map<std::string, ObsType*>::const_iterator it = m_obsType.begin(); it != m_obsType.end(); ++it )
      {
         ret.push_back( it->first );
      }
      return ret;
   }

private:
   std::map<std::string, ObsType *> m_obsType; ///< Keeps all observables type
};

//////////////////////////////////////////
// Observables factory
//////////////////////////////////////////
static const ObsTypesFactory g_obsFactory;


//////////////////////////////////////////
// Command processing
//////////////////////////////////////////
CmdAddObs::CmdAddObs( CasaCommander & parent, const std::vector< std::string > & cmdPrms ) : CasaCmd( parent, cmdPrms )
{
   if( m_prms.size() < 1 )
   {
      throw ErrorHandler::Exception( ErrorHandler::NonexistingID ) << "Observable is not defined properly";
   }

   // the first parameter could be observable name
   const ObsType * ot = g_obsFactory.factory( m_prms[0] );
   
   if ( !ot && m_prms.size() > 1 )
   {
      ot = g_obsFactory.factory( m_prms[1] );
      if ( ot )
      {
         m_obsName = m_prms[0];
         m_prms.erase( m_prms.begin() );
      }
   }
   
   if ( !ot ) {  throw ErrorHandler::Exception( ErrorHandler::UndefinedValue ) << "Unknown observable name: " << m_prms[0]; }

   size_t expPrmsNum = ot->expectedParametersNumber() + 1;
   size_t expOptPrmNum = expPrmsNum + ot->optionalParametersNumber();

   // check number of command parameters for var parameter
   if ( m_prms.size() < expPrmsNum || m_prms.size() > expOptPrmNum )
   {
      throw ErrorHandler::Exception( ErrorHandler::OutOfRangeValue ) << "Wrong number of parameters for " << m_prms[0] << " observable";
   }
}

void CmdAddObs::execute( std::unique_ptr<casa::ScenarioAnalysis> & sa )
{
   LogHandler( LogHandler::INFO_SEVERITY ) << "Add observable: " <<
      (m_obsName.empty() ? (m_prms[0] + "(" + CfgFileParser::implode( m_prms, ",", 1 ) + ")") : m_obsName);


   casa::Observable * obs = g_obsFactory.factory( m_prms[0] )->createOservableObject( m_obsName, m_prms );

   if ( ErrorHandler::NoError != sa->obsSpace().addObservable( obs ) )
   {
      throw ErrorHandler::Exception( sa->errorCode() ) << sa->errorMessage();
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

   const std::vector<std::string> & obsTypesName = g_obsFactory.typesNameList();

   // print short description for each observable type
   for ( size_t i = 0; i < obsTypesName.size(); ++ i )
   {
      std::cout << "    " << obsTypesName[i] << " - " << g_obsFactory.factory( obsTypesName[i] )->description() << "\n";
   }
   std::cout << "\n";

   // print examples of observables definition 
   for ( size_t i = 0; i < obsTypesName.size(); ++ i )
   {
      const ObsType * ob = g_obsFactory.factory( obsTypesName[i] );

      std::cout << ob->fullDescription();
      std::cout << "\n";
      std::cout << "    Example:\n";
      std::cout << ob->usingExample( cmdName );
      std::cout << "\n";
   }
}

