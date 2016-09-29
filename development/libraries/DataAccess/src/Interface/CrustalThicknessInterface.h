//                                                                      
// Copyright (C) 2015-2016 Shell International Exploration & Production.
// All rights reserved.
// 
// Developed under license for Shell by PDS BV.
// 
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#ifndef DATAACCESS_INTERFACE_CRUSTALTHICKNESSINTERFACE_H
#define DATAACCESS_INTERFACE_CRUSTALTHICKNESSINTERFACE_H

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
      mohoMap = 0,                    // Moho depth                                                                                           [m]
      thicknessBasaltMap,             // basalt thickness                                                                                     [m]
      WLSadjustedMap,                 // paleobathymetrie corrected water loaded basin subsidence                                             [m]
      TFOnsetMap,                     // crustal thinning factor at melt onset (accurate from McKenzie equations)                             []
      TFOnsetLinMap,                  // crustal thinning factor at melt onset (linearized to simplify McKenzie equations)                    []
      TFOnsetMigMap,                  // crustal thinning factor at melt migration                                                            []
      PTaMap,                         // mantle potential temperature associated with user-defined maximum basaltic crust thickness           [C]
      basaltDensityMap,               // basaltic crust density associated with user-defined maximum basaltic crust thickness                 [kgm-3]
      RDAadjustedMap,                 // calculated residual depth anomaly                                                                    [m]
      TFMap,                          // crustal thinning factor                                                                              []
      thicknessCrustMap,              // continental crustal thickness                                                                        [m]
      ECTMap,                         // effective continental crust thickness                                                                [m]
      estimatedCrustDensityMap,       // mean estimated continental crust density                                                             [kgm-3]
      WLSOnsetMap,                    // water-loaded edge of basalt magmatism (first apparition of basalt during thinning)                   [m]
      WLSCritMap,                     // water-loaded depth of oceanic crust (thinning factor is equal to 1 in this case)                     [m]
      WLSExhumeMap,                   // water-loaded depth of exhumed unalerted mantle                                                       [m]
      WLSExhumeSerpMap,               // water-loaded depth of exhumed serpentinised mantle                                                   [m]
      slopePreMelt,                   // premelt slope for linear relations between WLS and TF                                                []
      slopePostMelt,                  // postmelt slope for linear relations between WLS and TF                                               []
      interceptPostMelt,              // intercept for linear relations between WLS and T                                                     [m]
      thicknessCrustMeltOnset,        // continental crustal thickness at melt onset                                                          [m] 
      topBasaltMap,                   // depth to top basalt                                                                                  [m]
      WLSMap,                         // water loaded basin subsidence (former WLS)                                                           [m]
      incTectonicSubsidence,          // the change in total tectonic subsidence                                                              [m]
      incTectonicSubsidenceAdjusted,  // the paleobathymetrie corrected change in total tectonic subsidence                                   [m]
      cumSedimentBackstrip,           // cumulative sediment backstrip                                                                        [m]
      cumBasementCompensation,        // the amount the basement pushed down (or rebound) when sediment load is emplaced or removed           [m]
      cumSedimentThickness,           // cumulative sediment thickness                                                                        [m]
      sedimentDensityMap,             // present-day integrated sediment column density                                                       [kgm-3]
      isostaticBathymetry,            // filling up of a basin in absence of any thermal subsidence                                           [m]
      PaleowaterdepthResidual,        // the residual between the estimated paleowaterdepth on the input surface depth history (PWDR=PWD-SDH) [m]
      UpperContinentalCrustThickness, // the upper part of the continental crust thickness                                                    [m]
      LowerContinentalCrustThickness, // the lower part of the oceanic crust thickness                                                        [m]
      UpperOceanicCrustThickness,     // the upper part of the continental crust thickness                                                    [m]
      LowerOceanicCrustThickness,     // the lower part of the oceanic crust thickness                                                        [m]
      numberOfOutputMaps
   };

   const std::string outputMapsNames[CrustalThicknessInterface::numberOfOutputMaps] = {
      "Moho",                                  // Moho depth                                                                                           [m]
      "BasaltThickness",                       // basalt thickness                                                                                     [m]
      "AdjustedWaterLoadedSubsidence",         // paleobathymetrie corrected water loaded basin subsidence                                             [m]
      "TFonset",                               // crustal thinning factor at melt onset (accurate from McKenzie equations)                             []
      "TFonsetLin",                            // crustal thinning factor at melt onset (linearized to simplify McKenzie equations)                    []
      "TFonsetMig",                            // crustal thinning factor at melt migration                                                            []
      "PTa",                                   // mantle potential temperature associated with user-defined maximum basaltic crust thickness           [C]
      "BasaltDensity",                         // basaltic crust density associated with user-defined maximum basaltic crust thickness                 [kgm-3]
      "AdjustedResidualDepthAnomaly",          // present-day calculated residual depth anomaly                                                        [m]
      "TF",                                    // crustal thinning factor                                                                              []
      "ContinentalCrustalThickness",           // continental crustal thickness                                                                        [m]
      "EffectiveCrustalThickness",             // effective crust thickness                                                                            [m]
      "MeanCrustDensity",                      // mean estimated continental crust density                                                             [kgm-3]
      "WLSonset",                              // water-loaded edge of basalt magmatism (first apparition of basalt during thinning)                   [m]
      "WLScrit",                               // water-loaded depth of oceanic crust (thinning factor is equal to 1 in this case)                     [m]
      "WLSexhume",                             // water-loaded depth of exhumed unalerted mantle                                                       [m]
      "WLSexhumeSerp",                         // water-loaded depth of exhumed serpentinised mantle                                                   [m]
      "SlopePreMelt",                          // premelt slope for linear relations between WLS and TF                                                []
      "SlopePostMelt",                         // postmelt slope for linear relations between WLS and TF                                               []
      "InterceptPostMelt",                     // intercept for linear relations between WLS and T                                                     [m]
      "ContinentalCrustThicknessMeltOnset ",   // continental crustal thickness at melt onset                                                          [m] 
      "TopBasaltUnderplate",                   // depth to top basalt                                                                                  [m]
      "TotalTectonicSubsidence",               // water loaded basin subsidence (former WLS)                                                           [m]
      "IncrementalTectonicSubsidence",         // the change in total tectonic subsidence                                                              [m]
      "IncrementalTectonicSubsidenceAdjusted", // the paleobathymetrie corrected change in total tectonic subsidence                                   [m]
      "SedimentBackstrip",                     // cumulative sediment backstrip                                                                        [m]
      "BasementCompensation",                  // the amount the basement pushed down (or rebound) when sediment load is emplaced or removed           [m]
      "SedimentThickness",                     // cumulative sediment thickness                                                                        [m]
      "IntegratedSedimentDensity",             // present-day integrated sediment column density                                                       [kgm-3]
      "PaleowaterDepth",                       // filling up of a basin in absence of any thermal subsidence                                           [m]
      "PaleowaterdepthResidual",               // the residual between the estimated paleowaterdepth on the input surface depth history (PWDR=PWD-SDH) [m]
      "UpperContinentalCrustThickness",        // the upper part of the continental crust thickness                                                    [m]
      "LowerContinentalCrustThickness",        // the lower part of the oceanic crust thickness                                                        [m]
      "UpperOceanicCrustThickness",            // the upper part of the continental crust thickness                                                    [m]
      "LowerOceanicCrustThickness"             // the lower part of the oceanic crust thickness                                                        [m]
   };

   const std::string outputMapsUnits[CrustalThicknessInterface::numberOfOutputMaps] = {
      "m",     // Moho depth                                                                                           [m]
      "m",     // basalt thickness                                                                                     [m]
      "m",     // paleobathymetrie corrected water loaded basin subsidence                                             [m]
      "",      // crustal thinning factor at melt onset (accurate from McKenzie equations)                             []
      "",      // crustal thinning factor at melt onset (linearized to simplify McKenzie equations)                    []
      "",      // crustal thinning factor at melt migration                                                            []
      "C",     // mantle potential temperature associated with user-defined maximum basaltic crust thickness           [C]
      "kgm-3", // basaltic crust density associated with user-defined maximum basaltic crust thickness                 [kgm-3]
      "m",     // calculated residual depth anomaly                                                                    [m]
      "",      // crustal thinning factor                                                                              []
      "m",     // continental crustal thickness                                                                        [m]
      "m",     // effective crust thickness                                                                            [m]
      "kgm-3", // mean estimated continental crust density                                                             [kgm-3]
      "m",     // water-loaded edge of basalt magmatism (first apparition of basalt during thinning)                   [m]
      "m",     // water-loaded depth of oceanic crust (thinning factor is equal to 1 in this case)                     [m]
      "m",     // water-loaded depth of exhumed unalerted mantle                                                       [m]
      "m",     // water-loaded depth of exhumed serpentinised mantle                                                   [m]
      "",      // premelt slope for linear relations between WLS and TF                                                []
      "",      // postmelt slope for linear relations between WLS and TF                                               []
      "m",     // intercept for linear relations between WLS and T                                                     [m]
      "m",     // continental crustal thickness at melt onset                                                          [m] 
      "m",     // depth to top basalt                                                                                  [m]
      "m",     // water loaded basin subsidence (former WLS)                                                           [m]
      "m",     // the change in total tectonic subsidence                                                              [m]
      "m",     // the paleobathymetrie corrected change in total tectonic subsidence                                   [m]
      "m",     // cumulative sediment backstrip                                                                        [m]
      "m",     // the amount the basement pushed down (or rebound) when sediment load is emplaced or removed           [m]
      "m",     // cumulative sediment thickness                                                                        [m]
      "kgm-3", // present-day integrated sediment density                                                              [kgm-3]
      "m",     // filling up of a basin in absence of any thermal subsidence                                           [m]
      "m",     // the residual between the estimated paleowaterdepth on the input surface depth history (PWDR=PWD-SDH) [m]
      "m",     // the upper part of the continental crust thickness                                                    [m]
      "m",     // the lower part of the oceanic crust thickness                                                        [m]
      "m",     // the upper part of the continental crust thickness                                                    [m]
      "m"      // the lower part of the oceanic crust thickness                                                        [m]
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