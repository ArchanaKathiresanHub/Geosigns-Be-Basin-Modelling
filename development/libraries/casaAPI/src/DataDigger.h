//                                                                      
// Copyright (C) 2012-2014 Shell International Exploration & Production.
// All rights reserved.
// 
// Developed under license for Shell by PDS BV.
// 
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
// 

/// @file DataDigger.h
/// @brief This file keeps API declaration for data digger. 

#ifndef CASA_API_DATA_DIGGER_H
#define CASA_API_DATA_DIGGER_H

#include "RunCaseSet.h"
#include "ErrorHandler.h"
#include "Observable.h"
#include "ObsSpace.h"

/// @page CASA_DataDiggerPage Data Digger
///
/// @link casa::DataDigger Data digger @endlink provides set of API functions to create various types of @link casa::Observable Observable @endlink objects.
/// It also allows to collect observables value from the simulation results.
/// To full fill this purpose it should first add to each run case project request for observables. This must be done just after
/// cases mutation. casa::DataDigger uses the functionality of datadriller cauldron application. Call for datadriller added by casa::RunManager
/// at the end of applications pipeline.
/// 
/// The casa::DataDigger accepts the list of cases from casa::RunManager and list of observables as an input. 
/// It will load cases one by one and collect values for <i>present day</i> time from the computational results for the given list of observables.
/// Then it will return a table - cases vs observables value. 
///
/// The standard Cauldron data access API can be used to read results from the relevant mesh and trap results. 
/// However there are several specific ways to read this data which need to be implemented as an CASA - specific API.
/// -# Trap Finder.Because different experiment realizations may put traps in different places or give them different \n
///    sizes, a small program must know how to <i>find</i> a trap given a base - case trap. The simplest approach is to find \n
///    the trap in the realization which overlaps the base - case crest point.Once the trap is found, the standard API can \n
///    be used to extract relevant properties.
/// -# Well Sampling. It is rare that a user would want to predict pressure at a given XYZ point only.They will want \n
///    a pressure profile down a well (or proposed well). Therefore the data should be sampled along the length of the well, \n
///    perhaps at every surface and 50 - foot intervals in between.
/// -# Map sub-sampling/extraction. One target property may be  an entire map (e.g.reservoir formation porosity map). However \n
///    generating one response surface per pixel of the target map is likely extremely wasteful. Therefore some sampling function \n
///   will have to be described, perhaps high resolution in some places and low resolution in others.
/// -# Map Aggregate Statistics. Rather than seeing P10 & P90 maps of expulsion, the user may rather instead just view an \n
///    aggregate statistic, such as <i>total expulsion over this polygon</i>
/// -# The user will have to define the targets he has in mind. In a web - based interface, this is likely to be graphical.
///    However in a <i>data digger</i> API, this will have to be represented programmatically.
///
/// <b> For 2014, the following targets will be supported </b>
/// - Mesh properties along a vertical well, i.e.specified at a given X / Y.VRe, Temperature, Pressure, Permeability, Density, Sonic Velocity.
///
/// <b> For 2015 </b>
/// - map and map aggregated targets.
/// - Fluid properties in a trap as currently implemented in BPA1.
/// - Stock Tank Liquid : Oil Volume, Solution Gas Volume, Oil API, GOR, Oil Density, Oil Viscosity, Solution Gas Density, Solution Gas Viscosity.\n
///   Stock Tank Vapor : Condensate Volume, Free Gas Volume, CGR, Condensate Density, Condensate Viscosity, Free Gas Density, Free Gas Viscosity.
/// - Generic Reservoir Information : GOC, OWC, Spill Depth, Temperature, Pressure, Porosity, Permeability, Fracture Pressure.
/// - In Situ Conditions : Liquid volume, Liquid Density, Liquid Viscosity, Liquid Mass, Liquid CEP, Vapor Volume, Vapor Density, Vapor CEP, Vapor Mass.


namespace casa
{  
   /// @class DataDigger DataDigger.h "DataDigger.h"
   /// @brief Data digger extracts observables value from simulation results for single case
   ///        Also it performs like observables object factory.
   class DataDigger : public ErrorHandler
   {
   public:
      /// @brief Destructor
      virtual ~DataDigger() {;}

      /// @brief Create an observable object which will keep given property value for given XYZ coordinates
      /// @param x X coordinate 
      /// @param y Y coordinate
      /// @param z depth value
      /// @param propName name of the property
      /// @param simTime simulation time for property value extraction
      /// @return Pointer to new observable object
      static Observable * newObsPropertyXYZ( double x, double y, double z, const char * propName, double simTime = 0.0 );

      /// @brief Insert into project file request for observables values using datadriller cauldron app and data mining interface.\n
      /// Observables must be requested after case mutation.
      /// @param obs casaObsSpace object which keeps list of observables. For each observable, data digger will request data using datadriller table
      /// @param rcs casa::RunCaseSet object which keeps list of observables
      /// @return ErrorHandler::NoError on success, or error code otherwise
      virtual ErrorHandler::ReturnCode requestObservables( ObsSpace & obs, RunCaseSet & rcs ) = 0;

      /// @brief Collect observables value from simulation results for given case
      /// @param obs casaObsSpace object which keeps list of observables. For each observable, data digger will create ObsValue object for each run case
      /// @param rcs casa::RunCaseSet object which keeps list of observables and reference to Cauldron model
      /// @return ErrorHandler::NoError on success, or error code otherwise
      virtual ErrorHandler::ReturnCode collectRunResults( ObsSpace & obs, RunCaseSet & rcs ) = 0;

   protected:
      DataDigger() {;}
   };
}

#endif // CASA_API_DATA_DIGGER_H
