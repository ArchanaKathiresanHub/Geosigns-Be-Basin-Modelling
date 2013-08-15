#ifndef __Interface_OUTPUT_DEFS_H__
#define __Interface_OUTPUT_DEFS_H__

namespace CrustalThicknessInterface {
   const int XYZ = 0x0001;
   const int HDF = 0x0002;
   const int SUR = 0x0004;
   
   enum outputMaps {
      mohoMap = 0, thicknessBasaltMap, sedimentDensityMap, WLSadjustedMap,
      TFOnsetMap, TFOnsetLinMap, TFOnsetMigMap, PTaMap,
      basaltDensityMap, RDAadjustedMap, TFMap, thicknessCrustMap, ECTMap, 
      estimatedCrustDensityMap, WLSOnsetMap, WLSCritMap, WLSExhumeMap, WLSExhumeSerpMap,
      slopePreMelt, slopePostMelt, interceptPostMelt, thicknessCrustMeltOnset, topBasaltMap,
      WLSMap, cumSedimentBackstrip, cumSedimentThickness, isostaticBathymetry, cumBasementCompensation,
      incTectonicSubsidence,
      numberOfOutputMaps
   };
   const string outputMapsNames [CrustalThicknessInterface::numberOfOutputMaps] = {
      "Moho",             // present-day Moho depth (m)
      "BasaltThickness",  // present-day basalt thickness (m)
      "IntegratedSedimentDensity",  // present-day integrated sediment column density (kgm-3)
      "AdjustedWaterLoadedSubsidence",      // present-day water loaded basin subsidence (m)
      "TFonset",          // crustal thinning, melt onset (accurate) 
      "TFonsetLin",       // crustal thinning, melt onset
      "TFonsetMig",       // crustal thinning, melt migration
      "PTa",              // mantle potential temperature associated with user-defined HBu (max basaltic crust thickness) (C)
      "BasaltDensity",    // basaltic crust density  associated with user-defined HBu (kgm-3)
      "AdjustedResidualDepthAnomaly",      // present-day calculated residual depth anomaly (m)
      "TF",               // present-day crustal thinning factor
      "ContinentalCrustalThickness", // present-day crustal thickness (m)
      "EffectiveCrustalThickness",   // present-day effective crust thickness
      "MeanCrustDensity", // mean estimated continental crust density (kgm-3)
      "WLSonset",         // water-loaded edge of basalt magmatism
      "WLScrit",          // water-loaded depth of oceanic crust
      "WLSexhume",        // water-loaded depth of exhumed unalerted mantle
      "WLSexhumeSerp",    // water-loaded depth of exhumed serpentinised mantle
      "SlopePreMelt",     // premelt slope for linear relations between WLS and TF
      "SlopePostMelt",    // postmelt slope for linear relations between WLS and TF
      "InterceptPostMelt",// intercept for linear relations between WLS and T
      "CrustThicknessMeltOnset",  // crust thickness at melt onset (m) 
      "TopBasaltUnderplate",   // present-day depth to top basalt (m)
      "CumTectonicSubsidence", // present-day water loaded basin subsidence (m) (former WLS)
      "CumSedimentBackstrip",  // cumulative sediment backstrip (m)
      "CumSedimentThickness",  // cumulative sediment thickness (m)
      "IsostaticBathymetry",   // filling up of a basin in absence of any thermal subsidence (m)
      "CumBasementCompensation", // the amount the basement pushed down (or rebound) when sediment load is emplaced or removed 
      "IncTectonicSubsidence"    // the change in total tectonic subsidence  
   };
   const string outputMapsUnits [CrustalThicknessInterface::numberOfOutputMaps] = {
      "m", "m", "kgm-3", "m",
      "", "", "", "C", 
      "kgm-3", "m", "", "m", "m",
      "kgm-3", "m", "m", "m", "m",
      "", "", "", "m", "m", "m",
      "m", "m", "m", "m", "m"
   };
   outputMaps getPropertyId(const string & propertyName);
}

#endif
