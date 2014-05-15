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

#include "RunCase.h"
#include "ErrorHandler.h"
#include "Observable.h"

/// @page CASA_DataDiggerPage Data Digger
///
/// @link casa::DataDigger Data digger @endlink provides set of API functions to create various types of @link casa::Observable Observable @endlink objects.
/// It also allows to collect observables value from the simulation results.
/// 

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

      /// @brief Create an observable object which will keep given property value for give IJK grid position
      /// @param layerName name of the layer
      /// @param i I grid coordinate 
      /// @param j J grid coordinate
      /// @param k K grid coordinate in layer
      /// @param propName name of the property
      /// @param simTime simulation time for property value extraction
      /// @return Pointer to new observable object
      static Observable * newObsPropertyIJK( const char * layerName, int i, int j, int k, const char * propName, double simTime = 0.0 );

      /// @brief Create an observable object which will keep given property value for given XYZ coordinates
      /// @param x X coordinate 
      /// @param y Y coordinate
      /// @param z depth value
      /// @param propName name of the property
      /// @param simTime simulation time for property value extraction
      /// @return Pointer to new observable object
      static Observable * newObsPropertyXYZ( double x, double y, double z, const char * propName, double simTime = 0.0 );

      /// @brief Collect observables value from simulation results for given case
      /// @param cs RunCase object which keeps list of observables and reference to Cauldron model
      /// @return ErrorHandler::NoError on success, or error code otherwise
      virtual ErrorHandler::ReturnCode collectRunResults( RunCase & cs ) = 0;

   protected:
      DataDigger() {;}
   };
}

#endif // CASA_API_DATA_DIGGER_H
