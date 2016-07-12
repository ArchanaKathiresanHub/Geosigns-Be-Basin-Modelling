//                                                                      
// Copyright (C) 2015-2016 Shell International Exploration & Production.
// All rights reserved.
// 
// Developed under license for Shell by PDS BV.
// 
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#ifndef _DATAACCESS_INTERFACE_CRUSTALTHICKNESSINTERFACE_H_
#define _DATAACCESS_INTERFACE_CRUSTALTHICKNESSINTERFACE_H_

#include <string>

/// @defgroup Save file as
/// @{
#define XYZ 0x0001
#define HDF 0x0002
#define SUR 0x0004
/// @}

/// @file Contains the interface dediacted to fastctc
namespace CrustalThicknessInterface {

   enum outputMaps {
      mohoMap = 0, thicknessBasaltMap, WLSadjustedMap,
      TFOnsetMap, TFOnsetLinMap, TFOnsetMigMap, PTaMap,
      basaltDensityMap, RDAadjustedMap, TFMap, thicknessCrustMap, ECTMap,
      estimatedCrustDensityMap, WLSOnsetMap, WLSCritMap, WLSExhumeMap, WLSExhumeSerpMap,
      slopePreMelt, slopePostMelt, interceptPostMelt, thicknessCrustMeltOnset, topBasaltMap,
      WLSMap, incTectonicSubsidence,
      cumSedimentBackstrip, cumBasementCompensation, cumSedimentThickness, sedimentDensityMap,
      isostaticBathymetry, PaleowaterdepthResidual,
      numberOfOutputMaps
   };

   const std::string outputMapsNames[CrustalThicknessInterface::numberOfOutputMaps] = {
      "Moho",                               // present-day Moho depth (m)
      "BasaltThickness",                    // present-day basalt thickness (m)
      "AdjustedWaterLoadedSubsidence",      // present-day water loaded basin subsidence (m)
      "TFonset",                            // crustal thinning, melt onset (accurate) 
      "TFonsetLin",                         // crustal thinning, melt onset
      "TFonsetMig",                         // crustal thinning, melt migration
      "PTa",                                // mantle potential temperature associated with user-defined HBu (max basaltic crust thickness) (C)
      "BasaltDensity",                      // basaltic crust density  associated with user-defined HBu (kgm-3)
      "AdjustedResidualDepthAnomaly",       // present-day calculated residual depth anomaly (m)
      "TF",                                 // present-day crustal thinning factor
      "ContinentalCrustalThickness",        // present-day crustal thickness (m)
      "EffectiveCrustalThickness",          // present-day effective crust thickness
      "MeanCrustDensity",                   // mean estimated continental crust density (kgm-3)
      "WLSonset",                           // water-loaded edge of basalt magmatism
      "WLScrit",                            // water-loaded depth of oceanic crust
      "WLSexhume",                          // water-loaded depth of exhumed unalerted mantle
      "WLSexhumeSerp",                      // water-loaded depth of exhumed serpentinised mantle
      "SlopePreMelt",                       // premelt slope for linear relations between WLS and TF
      "SlopePostMelt",                      // postmelt slope for linear relations between WLS and TF
      "InterceptPostMelt",                  // intercept for linear relations between WLS and T
      "CrustThicknessMeltOnset",            // crust thickness at melt onset (m) 
      "TopBasaltUnderplate",                // present-day depth to top basalt (m)
      "TotalTectonicSubsidence",            // present-day water loaded basin subsidence (m) (former WLS)
      "IncrementalTectonicSubsidence",      // the change in total tectonic subsidence [m]
      "SedimentBackstrip",                  // cumulative sediment backstrip (m)
      "BasementCompensation",               // the amount the basement pushed down (or rebound) when sediment load is emplaced or removed [m]
      "SedimentThickness",                  // cumulative sediment thickness (m)
      "IntegratedSedimentDensity",          // present-day integrated sediment column density (kgm-3)
      "PaleowaterDepth",                    // filling up of a basin in absence of any thermal subsidence (m)
      "PaleowaterdepthResidual"             // The paleowaterdepth residual (PWD-SDH)  [m]
   };

   const std::string outputMapsUnits[CrustalThicknessInterface::numberOfOutputMaps] = {
      "m", "m", "m",
      "", "", "", "C",
      "kgm-3", "m", "", "m", "m",
      "kgm-3", "m", "m", "m", "m",
      "", "", "", "m", "m", "m",
      "m", "m", "m", "m", "kgm-3", "m", "m"
   };

   inline outputMaps getPropertyId( const std::string & propertyName ){

      for (unsigned int i = 0; i < numberOfOutputMaps; ++i) {
         if (propertyName == outputMapsNames[i]) {
            return static_cast<outputMaps>(i);
         }
      }

      return numberOfOutputMaps;
   };

}

#endif