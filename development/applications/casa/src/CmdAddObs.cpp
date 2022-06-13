//
// Copyright (C) 2012-2016 Shell International Exploration & Production.
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
#include "ObsGridXYLayerTopSurfaceProp.h"
#include "ObsTrapProp.h"
#include "ObsTrapDerivedProp.h"
#include "ObsValueDoubleScalar.h"
#include "ObsValueDoubleArray.h"

// LogHandler
#include "LogHandler.h"

//StringHandler
#include "StringHandler.h"

//casaCmdInterface
#include "casaCmdInterface.h"

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
   virtual ~ObsType() {}

   /// @brief create observable in scenario
   virtual casa::Observable * createObservableObject( const std::string & name, std::vector<std::string> & prms ) const = 0;

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
   ObsType() {}
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
   XYZPoint()  {}
   virtual ~XYZPoint() {}

   virtual casa::Observable * createObservableObject( const std::string & name, std::vector<std::string> & prms ) const
   {
      size_t pos = 2;
      const double x   = atof( prms[pos++].c_str() );
      const double y   = atof( prms[pos++].c_str() );
      const double z   = atof( prms[pos++].c_str() );
      const double age = atof( prms[pos++].c_str() ); // age for the observable

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
   WellTraj()  {}
   virtual ~WellTraj() {}

   virtual casa::Observable * createObservableObject( const std::string & name, std::vector<std::string> & prms ) const
   {
      const std::string & trajFileName = prms[1];                                         // well trajectory file with reference values
      const std::string & propName     = prms[2];                                         // property name
      double              age          = atof( prms[3].c_str() );                         // age for the observable
      double              stdDev       = prms.size() > 4 ? atof( prms[4].c_str() ) : 0.0; // std deviation value
      double              wgtSA        = prms.size() > 5 ? atof( prms[5].c_str() ) : 1.0; // observable weight for Sensitivity Analysis
      double              wgtUA        = prms.size() > 6 ? atof( prms[6].c_str() ) : 1.0; // observable weight for Uncertainty Analysis

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

      // check that arrays are matched
      if ( x.empty()            ||
           x.size() != y.size() ||
           x.size() != z.size() ||
           ( !r.empty()    && r.size()    != x.size() ) ||
           ( !sdev.empty() && sdev.size() != x.size() )
         )
      {
         throw ErrorHandler::Exception( ErrorHandler::OutOfRangeValue ) << "Invalid trajectory file: " << trajFileName;
      }

      // If no sdev was specified in the trajFileName, fill sdev vector with stdDev (unique value for all measurements)
      if ( !r.empty() && sdev.empty() ) { sdev.insert( sdev.begin(), x.size(), stdDev ); }

      // create observable
      casa::Observable * obsVal = casa::ObsGridPropertyWell::createNewInstance( x, y, z, propName.c_str(), age, name );

      if ( !r.empty() )
      {
         obsVal->setReferenceValue( new casa::ObsValueDoubleArray( obsVal, r ), new casa::ObsValueDoubleArray( obsVal, sdev ) );
      }

      obsVal->setSAWeight( wgtSA );
      obsVal->setUAWeight( wgtUA );

      return obsVal;
   }

   size_t expectedParametersNumber() const { return 3; }
   size_t optionalParametersNumber() const { return 3; }

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
   XYPointSorceRockMap() {}
   virtual ~XYPointSorceRockMap() {}

   virtual casa::Observable * createObservableObject( const std::string & name, std::vector<std::string> & prms ) const
   {
      size_t pos = 1;
      const  std::string & srPropName =       prms[pos++];           // property of source rock calculated in Genex
      const double x                  = atof( prms[pos++].c_str() ); // X coordinate for the point on the map
      const double y                  = atof( prms[pos++].c_str() ); // Y coordinate for the point on the map
      const std::string & srLayerName =       prms[pos++];           // source rock layer name
      const double age                = atof( prms[pos++].c_str() ); // age for the observable

      casa::Observable * obsVal = casa::ObsSourceRockMapProp::createNewInstance( x, y, srLayerName.c_str(), srPropName.c_str(), age, name );

      // optional parameters
      if ( prms.size() == 10 )
      {
         double refVal = atof( prms[pos++].c_str() ); // observable reference value
         double stdDev = atof( prms[pos++].c_str() ); // std deviation value

         obsVal->setReferenceValue( new casa::ObsValueDoubleScalar( obsVal, refVal ), new casa::ObsValueDoubleScalar( obsVal, stdDev ) );
      }
      if ( prms.size() > 7 )
      {
         double wgtSA = atof( prms[pos++].c_str() ); // observable weight for Sensitivity Analysis
         double wgtUA = atof( prms[pos++].c_str() ); // observable weight for Uncertainty Analysis

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
      oss << "    #                         type               prop name            X        Y        Layer name        Age   Ref    Dev  SWght UWght\n";
      oss << "    "<< cmdName << " XYPointSorceRockMap \"OilExpelledCumulative\" 460001.0 6750001.0 \"Lower Jurassic\"  65.0  65.7   2.0   1.0  1.0\n";
      return oss.str();
   }
};


////////////////////////////////////////////////////////////
// XYPointLayerTopSurface
////////////////////////////////////////////////////////////
//
class XYPointLayerTopSurface : public ObsType
{
public:
   XYPointLayerTopSurface() {}
   virtual ~XYPointLayerTopSurface() {}

   virtual casa::Observable * createObservableObject( const std::string & name, std::vector<std::string> & prms ) const
   {
      size_t pos = 1;
      const std::string & sthPropName  =       prms[pos++];           // Property name, for which the history of its values is extracted on top of formation surface
      const double x                   = atof( prms[pos++].c_str() ); // X coordinate
      const double y                   = atof( prms[pos++].c_str() ); // Y coordinate
      const std::string & sthLayerName =       prms[pos++];           // source rock layer name
      const double age                 = atof( prms[pos++].c_str() ); // age for the observable

      casa::Observable * obsVal = casa::ObsGridXYLayerTopSurfaceProp::createNewInstance( x, y, sthLayerName.c_str(), sthPropName.c_str(), age, name );

      // optional parameters
      if ( prms.size() == 10 )
      {
        double refVal                 = atof( prms[pos++].c_str() ); // observable reference value
        double stdDev                 = atof( prms[pos++].c_str() ); // std deviation value

        obsVal->setReferenceValue( new casa::ObsValueDoubleScalar( obsVal, refVal ), new casa::ObsValueDoubleScalar( obsVal, stdDev ) );
      }
      if ( prms.size() > 7 )
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

   virtual std::string name() const { return "XYPointLayerTopSurface"; }

   virtual std::string description() const { return "Property value for the top surface of a layer at a given location (X, Y)"; }

   virtual std::string fullDescription() const
   {
      std::ostringstream oss;
      oss << "    property value <PropName> <X> <Y> <LayerName> <Age> [<ReferenceValue> <StandardDeviationValue>] <SA weight> <UA weight>\n";
      oss << "    Where:\n";
      oss << "       PropName               - property name as it was defined in Cauldron project file\n";
      oss << "       X,Y                    - are the aerial target point coordinates\n";
      oss << "       LayerName              - layer (or formation) name\n";
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
      oss << "    #                         type             prop name       X        Y         Layer name       Age    Ref   Dev  SWght UWght\n";
      oss << "    "<< cmdName << " XYPointLayerTopSurface \"Temperature\" 460001.0 6750001.0 \"Lower Jurassic\"  65.0   65.7  2.0   1.0  1.0\n";
      return oss.str();
   }
};

class TrapProp : public ObsType
{
public:
   TrapProp() {}
   virtual ~TrapProp() {}

   virtual casa::Observable * createObservableObject( const std::string & name, std::vector<std::string> & prms ) const
   {
      bool logTransf  = false; // create RS for HCs volumes using logarithm of value
      bool byCompos   = true;  // calculate trap property by flashing the predicted composition

      size_t pos                   = 1;
      if ( prms[pos][0] == '[' )   // options list is given
      {
         const std::vector<std::string> & optList = CfgFileParser::list2array( prms[pos++], ',' );
         for ( auto s : optList )
         {
            if (      s == "log"    ) { logTransf = true;  } // create RS for logarithm of value
            else if ( s == "direct" ) { byCompos  = false; } // do not use composition RSs and flash for value prediction
            else    { throw ErrorHandler::Exception( ErrorHandler:: OutOfRangeValue ) << "Unknown trap property option: " << s; }
         }
      }
      std::string         propName =       prms[pos++];           // trap property
      double x                     = atof( prms[pos++].c_str() ); // trap X coord
      double y                     = atof( prms[pos++].c_str() ); // trap Y coord
      const std::string & resName  =       prms[pos++];           // reservoir name
      double age                   = atof( prms[pos++].c_str() ); // age for the observable

      if ( logTransf && propName.find( "Mass"      ) == std::string::npos &&
                        propName.find( "Volume"    ) == std::string::npos &&
                        propName.find( "API"       ) == std::string::npos &&
                        propName.find( "Density"   ) == std::string::npos &&
                        propName.find( "Viscosity" ) == std::string::npos &&
                        propName !=    "GOR"         &&
                        propName !=    "CGR"
         )
      {
         throw ErrorHandler::Exception( ErrorHandler::NotImplementedAPI ) << "Logarithmic transformation for trap property " << propName
                                                                          << " not yet implemented";
      }

      casa::Observable * obsVal = 0;
      if ( byCompos && ( propName == "GOR"    || propName == "CGR" ||
                         propName.find( "API"       ) != std::string::npos ||
                         propName.find( "Density"   ) != std::string::npos ||
                         propName.find( "Viscosity" ) != std::string::npos
                       )
         )
      {
         obsVal = casa::ObsTrapDerivedProp::createNewInstance( x, y, resName.c_str(), propName.c_str(), age, logTransf, name );
      }
      else
      {
         obsVal = casa::ObsTrapProp::createNewInstance( x, y, resName.c_str(), propName.c_str(), age, logTransf, name );
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
      oss << R"(    TrapProp [optionLst] <PropName> <X> <Y> <ReservoirName> <Age> [<SA weight> <UA weight>]
                      Where:
                        optionList - modifies how trap property RS is constructed. Implemented now options are:
                                     "log" if this option is given:
                                             for Volume/Mass trap properties casa will create RS for logarithm of target value
                                             for trap properties calculated by flashing interpolated over RS composition,
                                             casa will create RCs for logarithm of mass of components in composition
                                     "direct" if this option is given the GOR/CGR/API/Density/Viscosity calculation by flashing
                                             interpolated over RS composition will be disabled
                        PropName   - trap property name (supported properties list - see below)
                        X,Y        - are the aerial target point coordinates
                        LayerName  - source rock layer name
                        Age        - simulation age in [Ma]
                        SA weight  - weight [0:1] for this target for Sensitivity Analysis (it will used for Pareto diagram)
                        UA weight  - weight [0:1] for this target for Uncertainty Analysis (it will be used in RMSE calculation)

                    Note: If property name for Mass or Volume is prefixed by Log - response surface will approximate the logarithm of
                    property values. For GOR/CGR and all APIs properties, Log prefix means that logarthim of composition masses will
                    be approximated by response surface

                    Supported trap property list:
                       VolumeFGIIP [m3] -             Volume of free gas initially in place
                       VolumeCIIP [m3] -              Volume of condensate initially in place
                       VolumeSGIIP [m3] -             Volume of solution gas initially in place
                       VolumeSTOIIP [m3] -            Volume of stock tank oil initially in place
                       VolumeLiquid [m3] -            Volume of reservoir liquid phase
                       VolumeVapour [m3] -            Volume of reservoir vapour phase
                       DensityFGIIP [kg/m3] -         Density of free gas initially in place
                       DensityCIIP [kg/m3] -          Density of condensate initially in place
                       DensitySGIIP [kg/m3] -         Density of solution gas initially in place
                       DensitySTOIIP [kg/m3] -        Density of stock tank oil initially in place
                       DensityLiquid [kg/m3] -        Density of reservoir liquid phase
                       DensityVapour [kg/m3] -        Density of reservoir vapour phase
                       ViscosityFGIIP [Pa*s] -        Viscosity of free gas initially in place
                       ViscosityCIIP [Pa*s] -         Viscosity of condensate initially in place
                       ViscositySGIIP [Pa*s] -        Viscosity of solution gas initially in place
                       ViscositySTOIIP [Pa*s] -       Viscosity of stock tank oil initially in place
                       ViscosityLiquid [Pa*s] -       Viscosity of reservoir liquid phase
                       ViscosityVapour [Pa*s] -       Viscosity of reservoir vapour phase
                       MassFGIIP [kg] -               Mass of free gas initially in place
                       MassCIIP [kg] -                Mass of condensate initially in place
                       MassSGIIP [kg] -               Mass of solution gas initially in place
                       MassSTOIIP [kg] -              Mass of stock tank oil initially in place
                       MassLiquid [kg] -              Mass of reservoir liquid phase
                       MassVapour [kg] -              Mass of reservoir vapour phase
                       CGR [m3/m3] -                  Condensate Gas Ratio: VolumeCIIP / VolumeFGIIP
                       GOR [m3/m3] -                  Gas Oil Ratio: VolumeSGIIP / VolumeSTOIIP
                       OilAPI [] -                    API of STOIIP
                       CondensateAPI [] -             API of CIIP
                       GasWetnessFGIIP [mole/mole] -  C1 / Sum (C2 - C5) of FGIIP
                       GasWetnessSGIIP [mole/mole] -  C1 / Sum (C2 - C5) of SGIIP
                       CEPLiquid [MPa] -              CEP
                       CEPVapour [MPa] -              CEP
                       FracturePressure [MPa] -       Fracture pressure of the trap
                       ColumnHeightLiquid [m] -       Height of the liquid column
                       ColumnHeightVapour [m] -       Height of the vapour column
                       GOC [m] -                      Depth of Vapour-Liquid contact
                       OWC [m] -                      Depth of Liquid-Water contact
                       SpillDepth [m] -               Spill depth
                       SealPermeability [mD] -        Seal permeability
                       Pressure [MPa] -               Pressure at crest point
                       LithoStaticPressure [MPa] -    lithostatic pressure at crest point
                       HydroStaticPressure [MPa] -    hydrostatic pressure at crest point
                       OverPressure [MPa] -           over pressure at crest point
                       Temperature [C] -              Temperature at crest point
                       Permeability [mD] -            Permeability at crest point
                       Porosity [%] -                 Porosity at crest point\n)";
      return oss.str();
   }

   virtual std::string usingExample( const char * cmdName ) const
   {
      std::ostringstream oss;
      oss << "    #       type      prop name       X        Y          ReservName    Age   SWght UWght\n";
      oss << "    "<< cmdName << " TrapProp \"GOR\" 460001.0 6750001.0  \"Res\"       0.0    1.0  1.0\n\n";
      oss << "    #       type      prop name       X        Y          ReservName    Age   SWght UWght\n";
      oss << "    "<< cmdName << " [\"log\"] TrapProp \"GOR\" 460001.0 6750001.0  \"Res\"       0.0    1.0  1.0\n";
      oss << "    "<< cmdName << " [\"direct\"] TrapProp \"GOR\" 460001.0 6750001.0  \"Res\"    0.0    1.0  1.0\n";
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
      m_obsType["XYZPoint"]               = new XYZPoint();
      m_obsType["WellTraj"]               = new WellTraj();
      m_obsType["XYPointSorceRockMap"]    = new XYPointSorceRockMap();
      m_obsType["XYPointLayerTopSurface"] = new XYPointLayerTopSurface();
      m_obsType["TrapProp"]               = new TrapProp();
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

   m_obsName = casaCmdInterface::stringVecToStringWithNoSpaces(m_prms,"_");

   // the first parameter could be observable name
   const ObsType * ot = g_obsFactory.factory( m_prms[0] );

   if ( !ot && m_prms.size() > 1 )
   {
      ot = g_obsFactory.factory( m_prms[1] );
      if ( ot )
      {
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
      (m_obsName.empty() ? (m_prms[0] + "(" + StringHandler::implode( m_prms, ",", 1 ) + ")") : m_obsName);

   casa::Observable * obs = g_obsFactory.factory( m_prms[0] )->createObservableObject( m_obsName, m_prms );

   if ( ErrorHandler::NoError != sa->obsSpace().addObservable( obs ) )
   {
      throw ErrorHandler::Exception( sa->errorCode() ) << sa->errorMessage();
   }
}

void CmdAddObs::printHelpPage( const char * cmdName )
{
   std::cout << "   " << cmdName <<
R"(<target type name> [target parameters]
     Observable (or Target) - could be any data value from the simulation results.
     For example temperature or VRe at some position and depth for current time.

     Observable reference value - usually it is a measurement of corresponded observable
     value from the real well. Observables with reference value could be used for
     calibration workflow.

     Standard deviation value of observable reference value - contains the standard deviations
     of the measurement noise. Standard deviation (SD) measures the amount of variation or
     dispersion from the average. A low standard deviation indicates that the data points tend
     to be very close to the mean (also called expected value); a high standard deviation
     indicates that the data points are spread out over a large range of values.

     The following list of target types is implemented for this command:\n)";

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

