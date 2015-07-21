#include "PropertyManager.h"

#include "PrimaryOutputPropertyMap.h"
#include "DerivedOutputPropertyMap.h"
#include "DerivedOutputPropertyMapFactory.h"

#include "Interface/Surface.h"
#include "Interface/Snapshot.h"
#include "Interface/Formation.h"

#include "ComponentManager.h"

using namespace Basin_Modelling;

PropertyManager* PropertyManager::s_propertyManager = 0;
std::string PropertyManager::s_nullString = "";

//------------------------------------------------------------//

PropertyManager::PropertyManager () {

   int i;

   // Mapping from the time-io-table property name to the name that appears in the time-filter gui.
   // Would it be better to have only those that are not an identity mapping on the map?
   // e.g. Porosity -> PorosityVec, and not Depth -> depth.
   m_propertyName2OutputName [ "AllochthonousLithology" ] = "AllochthonousLithology";
   m_propertyName2OutputName [ "BulkDensity" ]            = "BulkDensityVec";
   m_propertyName2OutputName [ "Depth" ]                  = "Depth"; 
   m_propertyName2OutputName [ "ChemicalCompaction" ]     = "ChemicalCompaction"; 
   m_propertyName2OutputName [ "DepthHighRes" ]           = "Depth"; 
   m_propertyName2OutputName [ "Diffusivity" ]            = "DiffusivityVec"; 
   m_propertyName2OutputName [ "ErosionFactor" ]          = "ErosionFactor"; 
   m_propertyName2OutputName [ "FaultElements" ]          = "FaultElements"; 
   m_propertyName2OutputName [ "FCTCorrection" ]          = "FCTCorrection"; 
   m_propertyName2OutputName [ "FluidVelocity" ]          = "FluidVelocity"; 
   m_propertyName2OutputName [ "FluidVelocityX" ]         = "FluidVelocity"; 
   m_propertyName2OutputName [ "FluidVelocityY" ]         = "FluidVelocity"; 
   m_propertyName2OutputName [ "FluidVelocityZ" ]         = "FluidVelocity"; 
   m_propertyName2OutputName [ "HeatFlow" ]               = "HeatFlow"; 
   m_propertyName2OutputName [ "HeatFlowX" ]              = "HeatFlow"; 
   m_propertyName2OutputName [ "HeatFlowY" ]              = "HeatFlow"; 
   m_propertyName2OutputName [ "HeatFlowZ" ]              = "HeatFlow"; 
   m_propertyName2OutputName [ "HopaneIsomerisation" ]    = "Biomarkers"; 
   m_propertyName2OutputName [ "HorizontalPermeability" ] = "HorizontalPermeability"; 
   m_propertyName2OutputName [ "HydroStaticPressure" ]    = "HydroStaticPressure"; 
   m_propertyName2OutputName [ "IlliteFraction" ]         = "IlliteFraction";
   m_propertyName2OutputName [ "Lithology" ]              = "Lithology"; 
   m_propertyName2OutputName [ "LithoStaticPressure" ]    = "LithoStaticPressure"; 
   m_propertyName2OutputName [ "MassFlux" ]               = "MassFlux"; 
   m_propertyName2OutputName [ "MassFluxX" ]              = "MassFlux"; 
   m_propertyName2OutputName [ "MassFluxY" ]              = "MassFlux"; 
   m_propertyName2OutputName [ "MassFluxZ" ]              = "MassFlux"; 
   m_propertyName2OutputName [ "MaxVesHighRes" ]          = "MaxVes"; 
   m_propertyName2OutputName [ "MaxVes" ]                 = "MaxVes"; 
   m_propertyName2OutputName [ "Overburden" ]             = s_nullString; 
   m_propertyName2OutputName [ "OverPressure" ]           = "OverPressure"; 
   m_propertyName2OutputName [ "PermeabilityH" ]          = "PermeabilityVec"; 
   m_propertyName2OutputName [ "Permeability" ]           = "PermeabilityVec"; 
   m_propertyName2OutputName [ "Porosity" ]               = "PorosityVec"; 
   m_propertyName2OutputName [ "Pressure" ]               = "Pressure"; 
   m_propertyName2OutputName [ "FracturePressure" ]       = "FracturePressure"; 
   m_propertyName2OutputName [ "Reflectivity" ]           = "ReflectivityVec"; 
   m_propertyName2OutputName [ "SonicVelocity" ]          = "SonicVec"; 
   m_propertyName2OutputName [ "SteraneAromatisation" ]   = "Biomarkers"; 
   m_propertyName2OutputName [ "SteraneIsomerisation" ]   = "Biomarkers"; 
   m_propertyName2OutputName [ "Temperature" ]            = "Temperature"; 
   m_propertyName2OutputName [ "ThCond" ]                 = "ThCondVec"; 
   m_propertyName2OutputName [ "ThicknessError" ]         = "ThicknessError"; 
   m_propertyName2OutputName [ "ThicknessHighRes" ]       = "Thickness"; 
   m_propertyName2OutputName [ "Thickness" ]              = "Thickness";
   m_propertyName2OutputName [ "TwoWayTime" ]             = "TwoWayTime";
   m_propertyName2OutputName [ "TwoWayTimeResidual" ]     = "TwoWayTimeResidual";
   m_propertyName2OutputName [ "Velocity" ]               = "VelocityVec";
   m_propertyName2OutputName [ "VesHighRes" ]             = "Ves"; 
   m_propertyName2OutputName [ "Ves" ]                    = "Ves";
   m_propertyName2OutputName [ "VreMaturity" ]            = "VreMaturity";
   m_propertyName2OutputName [ "Vre" ]                    = "Vr";
   m_propertyName2OutputName [ "Vr" ]                     = "VrVec";

   m_propertyName2OutputName [ "C1ExpelledCumulative" ]          = "C1ExpelledCumulative"; 
   m_propertyName2OutputName [ "C2ExpelledCumulative" ]          = "C2ExpelledCumulative"; 
   m_propertyName2OutputName [ "C3ExpelledCumulative" ]          = "C3ExpelledCumulative"; 
   m_propertyName2OutputName [ "C4ExpelledCumulative" ]          = "C4ExpelledCumulative"; 
   m_propertyName2OutputName [ "C5ExpelledCumulative" ]          = "C5ExpelledCumulative"; 
   m_propertyName2OutputName [ "C15+AroExpelledCumulative" ]     = "C15+AroExpelledCumulative"; 
   m_propertyName2OutputName [ "C15+SatExpelledCumulative" ]     = "C15+SatExpelledCumulative"; 
   m_propertyName2OutputName [ "C6-14AroExpelledCumulative" ]    = "C6-14AroExpelledCumulative"; 
   m_propertyName2OutputName [ "C6-14SatExpelledCumulative" ]    = "C6-14SatExpelledCumulative"; 
   m_propertyName2OutputName [ "asphaltenesExpelledCumulative" ] = "asphaltenesExpelledCumulative"; 
   m_propertyName2OutputName [ "resinsExpelledCumulative" ]      = "resinsExpelledCumulative"; 
   m_propertyName2OutputName [ "COxExpelledCumulative" ]         = "COxExpelledCumulative"; 
   m_propertyName2OutputName [ "N2ExpelledCumulative" ]          = "N2ExpelledCumulative"; 
   m_propertyName2OutputName [ "H2SExpelledCumulative"]          = "H2SExpelledCumulative";
   m_propertyName2OutputName [ "LSCExpelledCumulative"]          = "LSCExpelledCumulative";
   m_propertyName2OutputName [ "C15+ATExpelledCumulative"]       = "C15+ATExpelledCumulative";
   m_propertyName2OutputName [ "C6-14BTExpelledCumulative"]      = "C6-14BTExpelledCumulative";
   m_propertyName2OutputName [ "C6-14DBTExpelledCumulative"]     = "C6-14DBTExpelledCumulative";
   m_propertyName2OutputName [ "C6-14BPExpelledCumulative"]      = "C6-14BPExpelledCumulative";
   m_propertyName2OutputName [ "C15+AroSExpelledCumulative"]     = "C15+AroSExpelledCumulative";
   m_propertyName2OutputName [ "C15+SatSExpelledCumulative"]     = "C15+SatSExpelledCumulative";
   m_propertyName2OutputName [ "C6-14SatSExpelledCumulative"]    = "C6-14SatSExpelledCumulative";
   m_propertyName2OutputName [ "C6-14AroSExpelledCumulative"]    = "C6-14AroSExpelledCumulative";

   m_propertyName2OutputName [ "InstantaneousExpulsionApi" ]                = "InstantaneousExpulsionApi";
   m_propertyName2OutputName [ "CumulativeExpulsionApi" ]                   = "CumulativeExpulsionApi"; 
   m_propertyName2OutputName [ "InstantaneousExpulsionCondensateGasRatio" ] = "InstantaneousExpulsionCondensateGasRatio";
   m_propertyName2OutputName [ "CumulativeExpulsionCondensateGasRatio" ]    = "CumulativeExpulsionCondensateGasRatio";
   m_propertyName2OutputName [ "InstantaneousExpulsionGasOilRatio" ]        = "InstantaneousExpulsionGasOilRatio";
   m_propertyName2OutputName [ "CumulativeExpulsionGasOilRatio" ]           = "CumulativeExpulsionGasOilRatio";
   m_propertyName2OutputName [ "InstantaneousExpulsionGasWetness" ]         = "InstantaneousExpulsionGasWetness";
   m_propertyName2OutputName [ "CumulativeExpulsionGasWetness" ]            = "CumulativeExpulsionGasWetness";
   m_propertyName2OutputName [ "InstantaneousExpulsionAromaticity" ]        = "InstantaneousExpulsionAromaticity";
   m_propertyName2OutputName [ "CumulativeExpulsionAromaticity" ]           = "CumulativeExpulsionAromaticity";
   m_propertyName2OutputName [ "KerogenConversionRatio" ]                   = "KerogenConversionRatio"; 
   m_propertyName2OutputName [ "OilGeneratedCumulative" ]                   = "OilGeneratedCumulative";
   m_propertyName2OutputName [ "OilGeneratedRate" ]                         = "OilGeneratedRate";
   m_propertyName2OutputName [ "OilExpelledCumulative" ]                    = "OilExpelledCumulative";
   m_propertyName2OutputName [ "OilExpelledRate" ]                          = "OilExpelledRate";
   m_propertyName2OutputName [ "HcGasGeneratedCumulative" ]                 = "HcGasGeneratedCumulative";
   m_propertyName2OutputName [ "HcGasGeneratedRate" ]                       = "HcGasGeneratedRate";
   m_propertyName2OutputName [ "HcGasExpelledCumulative" ]                  = "HcGasExpelledCumulative";
   m_propertyName2OutputName [ "HcGasExpelledRate" ]                        = "HcGasExpelledRate";
   m_propertyName2OutputName [ "DryGasGeneratedCumulative" ]                = "DryGasGeneratedCumulative";
   m_propertyName2OutputName [ "DryGasGeneratedRate" ]                      = "DryGasGeneratedRate";
   m_propertyName2OutputName [ "DryGasExpelledCumulative" ]                 = "DryGasExpelledCumulative";
   m_propertyName2OutputName [ "DryGasExpelledRate" ]                       = "DryGasExpelledRate";
   m_propertyName2OutputName [ "WetGasGeneratedCumulative" ]                = "WetGasGeneratedCumulative";
   m_propertyName2OutputName [ "WetGasGeneratedRate" ]                      = "WetGasGeneratedRate";
   m_propertyName2OutputName [ "WetGasExpelledCumulative" ]                 = "WetGasExpelledCumulative";
   m_propertyName2OutputName [ "WetGasExpelledRate" ]                       = "WetGasExpelledRate"; 

   m_propertyName2OutputName [ "CapillaryPressureGas100" ] = "CapillaryPressureGas100"; 
   m_propertyName2OutputName [ "CapillaryPressureGas0" ]   = "CapillaryPressureGas0"; 
   m_propertyName2OutputName [ "CapillaryPressureOil100" ] = "CapillaryPressureOil100"; 
   m_propertyName2OutputName [ "CapillaryPressureOil0" ]   = "CapillaryPressureOil0"; 

   
   //for(i = 0; i < CBMGenerics::ComponentManager::NumberOfSpeciesToFlash; ++i)
   for(i = 0; i < NumberOfPVTComponents; ++i)
   {
      const string propertyName = CBMGenerics::ComponentManager::getInstance ().GetSpeciesName ( i ) + "Concentration";
      m_propertyName2OutputName [ propertyName ] = propertyName;
   }


   m_propertyName2OutputName [ "ElementMass" ] = "ElementMass";

   m_propertyName2OutputName [ "BrineSaturation" ] = "BrineSaturation";
   m_propertyName2OutputName [ "HcLiquidSaturation"   ] = "HcLiquidSaturation";
   m_propertyName2OutputName [ "HcVapourSaturation"   ] = "HcVapourSaturation";
   m_propertyName2OutputName [ "ImmobileSaturation" ] = "ImmobileSaturation";

   //average saturations
   m_propertyName2OutputName [ "AverageBrineSaturation" ] = "AverageBrineSaturation";
   m_propertyName2OutputName [ "AverageHcLiquidSaturation"   ] = "AverageHcLiquidSaturation";
   m_propertyName2OutputName [ "AverageHcVapourSaturation"   ] = "AverageHcVapourSaturation";
   m_propertyName2OutputName [ "AverageImmobileSaturation" ] = "AverageImmobileSaturation";
   

   m_propertyName2OutputName [ "HcVapourDensity" ] = "HcVapourDensity";
   m_propertyName2OutputName [ "HcLiquidDensity" ] = "HcLiquidDensity";
   m_propertyName2OutputName [ "HcVapourViscosity" ] = "HcVapourViscosity";
   m_propertyName2OutputName [ "HcLiquidViscosity" ] = "HcLiquidViscosity";

   m_propertyName2OutputName [ "HcVapourVelocityX" ] = "HcVapourVelocityX";
   m_propertyName2OutputName [ "HcVapourVelocityY" ] = "HcVapourVelocityY";
   m_propertyName2OutputName [ "HcVapourVelocityZ" ] = "HcVapourVelocityZ";
   m_propertyName2OutputName [ "HcVapourVelocityMagnitude" ] = "HcVapourVelocityMagnitude";

   m_propertyName2OutputName [ "HcLiquidVelocityX" ] = "HcLiquidVelocityX";
   m_propertyName2OutputName [ "HcLiquidVelocityY" ] = "HcLiquidVelocityY";
   m_propertyName2OutputName [ "HcLiquidVelocityZ" ] = "HcLiquidVelocityZ";
   m_propertyName2OutputName [ "HcLiquidVelocityMagnitude" ] = "HcLiquidVelocityMagnitude";



   m_propertyName2OutputName [ "HcVapourVolume" ] = "HcVapourVolume";
   m_propertyName2OutputName [ "HcLiquidVolume" ] = "HcLiquidVolume";

   m_propertyName2OutputName [ "ElementVolume" ] = "ElementVolume";
   m_propertyName2OutputName [ "ElementPoreVolume" ] = "ElementPoreVolume";
   m_propertyName2OutputName [ "TransportedMass" ] = "TransportedMass";
   m_propertyName2OutputName [ "BrineRelativePermeability" ] = "BrineRelativePermeability";
   m_propertyName2OutputName [ "HcLiquidRelativePermeability" ] = "HcLiquidRelativePermeability";
   m_propertyName2OutputName [ "HcVapourRelativePermeability" ] = "HcVapourRelativePermeability";

  
   m_propertyName2OutputName [ "HcLiquidBrineCapillaryPressure" ] = "HcLiquidBrineCapillaryPressure";
   m_propertyName2OutputName [ "HcVapourBrineCapillaryPressure" ] = "HcVapourBrineCapillaryPressure";
   m_propertyName2OutputName [ "GOR" ] = "GOR";
   m_propertyName2OutputName [ "CGR" ] = "CGR";
   m_propertyName2OutputName [ "OilAPI" ] = "OilAPI";
   m_propertyName2OutputName [ "CondensateAPI" ] = "CondensateAPI";
   m_propertyName2OutputName [ "BrineDensity" ] = "BrineDensity";
   m_propertyName2OutputName [ "BrineViscosity" ] = "BrineViscosity";
   m_propertyName2OutputName [ "TimeOfInvasion" ] = "TimeOfInvasion";
  
   m_propertyName2OutputName [ "ALCOrigLithMantleDepth" ] = "ALCOrigLithMantleDepth"; 
   m_propertyName2OutputName [ "ALCMaxAsthenoMantleDepth" ] = "ALCMaxAsthenoMantleDepth"; 
   m_propertyName2OutputName [ "ALCStepContCrustThickness" ] = "ALCStepContCrustThickness";
   m_propertyName2OutputName [ "ALCStepBasaltThickness" ] = "ALCStepBasaltThickness";
   m_propertyName2OutputName [ "ALCStepTopBasaltDepth" ] = "ALCStepTopBasaltDepth";
   m_propertyName2OutputName [ "ALCSmContCrustThickness" ] = "ALCSmContCrustThickness";
   m_propertyName2OutputName [ "ALCSmBasaltThickness" ] = "ALCSmBasaltThickness";
   m_propertyName2OutputName [ "ALCSmTopBasaltDepth" ] = "ALCSmTopBasaltDepth";
   m_propertyName2OutputName [ "ALCSmMohoDepth" ] = "ALCSmMohoDepth";
   m_propertyName2OutputName [ "ALCStepMohoDepth" ] = "ALCStepMohoDepth";
 
}

//------------------------------------------------------------//

const std::string& PropertyManager::findOutputPropertyName ( const std::string& name ) const {

   PropertyNameMap::const_iterator outputPropertyName = m_propertyName2OutputName.find ( name );

   if ( outputPropertyName != m_propertyName2OutputName.end () and outputPropertyName->second != s_nullString ) {
      return outputPropertyName->second;
   } else {
      return s_nullString;
   }
   
}

//------------------------------------------------------------//

void PropertyManager::createSurfacePropertyMaps ( AppCtx*                                cauldron,
                                                  const OutputPropertyMapAssociation     association,
                                                  const PropertyList                     requiredProperty,
                                                  const Interface::Snapshot*             snapshot,
                                                  const Basin_Modelling::Layer_Iterator& formation,
                                                  const bool                             createForTopSurfaceOnly ) {

   OutputPropertyMap* property;

   if ( formation.Current_Layer ()->isSediment ()) {
      property = m_derivedProperties.allocateMap ( requiredProperty,
                                                   formation.Current_Layer (),
                                                   formation.Current_Layer ()->getTopSurface (),
                                                   snapshot );

      if ( property != 0 ) {
         addMapPropertyMap ( association, requiredProperty, property );
      }

      // Must get the bottom-most surface if sediments-only is selected.
      /// createForTopSurfaceOnly applies only to sediment formations
      if ( not createForTopSurfaceOnly and ( formation.Layer_Below () == 0 or formation.Layer_Below ()->isCrust ())) {
         property = m_derivedProperties.allocateMap ( requiredProperty,
                                                      formation.Current_Layer (),
                                                      formation.Current_Layer ()->getBottomSurface (),
                                                      snapshot );

         if ( property != 0 ) {
            addMapPropertyMap ( association, requiredProperty, property );
         }

      }

   } else {

      if ( formation.Layer_Above () == 0 and formation.Current_Layer ()->isCrust ()) {
         property = m_derivedProperties.allocateMap ( requiredProperty,
                                                      formation.Current_Layer (),
                                                      formation.Current_Layer ()->getTopSurface (),
                                                      snapshot );

         if ( property != 0 ) {
            addMapPropertyMap ( association, requiredProperty, property );
         }

      }

      property = m_derivedProperties.allocateMap ( requiredProperty,
                                                   formation.Current_Layer (),
                                                   formation.Current_Layer ()->getBottomSurface (),
                                                   snapshot );

      if ( property != 0 ) {
         addMapPropertyMap ( association, requiredProperty, property );
      }

   }

}

//------------------------------------------------------------//

void PropertyManager::createSurfaceFormationPropertyMaps ( AppCtx*                                cauldron,
                                                           const OutputPropertyMapAssociation     association,
                                                           const PropertyList                     requiredProperty,
                                                           const Interface::Snapshot*             snapshot,
                                                           const Basin_Modelling::Layer_Iterator& formation,
                                                           const bool                             createForTopSurfaceOnly ) {

   OutputPropertyMap* property;

   property = m_derivedProperties.allocateMap ( requiredProperty,
                                                formation.Current_Layer (),
                                                formation.Current_Layer ()->getTopSurface (),
                                                snapshot );

   if ( property != 0 ) {
      addMapPropertyMap ( association, requiredProperty, property );
   }

   if ( not createForTopSurfaceOnly ) {
      // It property is a vector-quantity then it must also be output on the bottom surface of the formation.
      property = m_derivedProperties.allocateMap ( requiredProperty,
                                                   formation.Current_Layer (),
                                                   formation.Current_Layer ()->getBottomSurface (),
                                                   snapshot );

      if ( property != 0 ) {
         addMapPropertyMap ( association, requiredProperty, property );
      }

   }

}

//------------------------------------------------------------//

void PropertyManager::createFormationPropertyMaps ( AppCtx*                                cauldron,
                                                    const OutputPropertyMapAssociation     association,
                                                    const PropertyList                     requiredProperty,
                                                    const Interface::Snapshot*             snapshot,
                                                    const Basin_Modelling::Layer_Iterator& formation ) {

   OutputPropertyMap* property;

   property = m_derivedProperties.allocateMap ( requiredProperty,
                                                formation.Current_Layer (),
                                                formation.Current_Layer ()->getTopSurface (),
                                                snapshot );

   if ( property != 0 ) {
      addMapPropertyMap ( association, requiredProperty, property );
   }


}

//------------------------------------------------------------//

void PropertyManager::computePropertyMaps ( AppCtx*                    cauldron,
                                            const PropertyList         requiredProperty,
                                            const Interface::Snapshot* snapshot,
                                            const Interface::PropertyOutputOption maximumOutputOption ) {

   if ( not m_derivedProperties.isMapDefined ( requiredProperty )) {
      // Error?
      return;
   }

   const OutputPropertyMapAssociation association = m_derivedProperties.getMapAssociation ( requiredProperty );
   Layer_Iterator layers;
   bool topSurfaceOnly = maximumOutputOption == Interface::SOURCE_ROCK_ONLY_OUTPUT or
                         maximumOutputOption == Interface::SHALE_GAS_ONLY_OUTPUT;

   if ( maximumOutputOption == Interface::SEDIMENTS_AND_BASEMENT_OUTPUT ) {
      layers.Initialise_Iterator ( cauldron->layers, Descending, Basement_And_Sediments, Active_Layers_Only );
   } else if ( maximumOutputOption == Interface::SEDIMENTS_ONLY_OUTPUT ) {
      layers.Initialise_Iterator ( cauldron->layers, Descending, Sediments_Only, Active_Layers_Only );
   } else if ( maximumOutputOption == Interface::SOURCE_ROCK_ONLY_OUTPUT ) {
      layers.Initialise_Iterator ( cauldron->layers, Descending, Source_Rocks_Only, Active_Layers_Only );
   } else if ( maximumOutputOption == Interface::SHALE_GAS_ONLY_OUTPUT ) {
      layers.Initialise_Iterator ( cauldron->layers, Descending, Shale_Gas_Only, Active_Layers_Only );
   } else {
      return;
   }

   for ( layers.Initialise_Iterator (); ! layers.Iteration_Is_Done (); layers++ ) {

      switch ( association ) {

        case SURFACE_ASSOCIATION : 
           createSurfacePropertyMaps ( cauldron, association, requiredProperty, snapshot, layers, topSurfaceOnly ); 
           break;

        case FORMATION_ASSOCIATION :
           createFormationPropertyMaps ( cauldron, association, requiredProperty, snapshot, layers );
           break;

        case SURFACE_FORMATION_ASSOCIATION :
           createSurfaceFormationPropertyMaps ( cauldron, association, requiredProperty, snapshot, layers, topSurfaceOnly );
           break;

        default :
           break;
      }

   }

}

//------------------------------------------------------------//

void PropertyManager::computePropertyVolumes ( AppCtx*                               cauldron,
                                               const PropertyList                    requiredProperty,
                                               const Interface::Snapshot*            snapshot,
                                               const Interface::PropertyOutputOption maximumOutputOption ) {

   if ( not m_derivedProperties.isVolumeDefined ( requiredProperty )) {
      // Error?
      return;
   }

   Layer_Iterator layers;
   OutputPropertyMap* property;

   if ( maximumOutputOption == Interface::SEDIMENTS_AND_BASEMENT_OUTPUT ) {
      layers.Initialise_Iterator ( cauldron->layers, Descending, Basement_And_Sediments, Active_Layers_Only );
   } else if ( maximumOutputOption == Interface::SEDIMENTS_ONLY_OUTPUT ) {
      layers.Initialise_Iterator ( cauldron->layers, Descending, Sediments_Only, Active_Layers_Only );
   } else if ( maximumOutputOption == Interface::SOURCE_ROCK_ONLY_OUTPUT ) {
      layers.Initialise_Iterator ( cauldron->layers, Descending, Source_Rocks_Only, Active_Layers_Only );
   } else {
      return;
   }

   for ( layers.Initialise_Iterator (); ! layers.Iteration_Is_Done (); layers++ ) {

      property = m_derivedProperties.allocateVolume ( requiredProperty,
                                                      layers.Current_Layer (),
                                                      snapshot );

      if ( property != 0 ) {
         m_volumeProperties.push_back ( property );
      }

   }

}

//------------------------------------------------------------//

void PropertyManager::computeSourceRockPropertyMaps ( AppCtx*                    cauldron,
                                                      const Interface::Snapshot* snapshot,
                                                      const PropListVec&         genexProperties,
                                                      const PropListVec&         shaleGasProperties ) {

   PropListVec::const_iterator propertyIter;

   for ( propertyIter = genexProperties.begin (); propertyIter != genexProperties.end (); ++propertyIter ) {
      computePropertyMaps ( cauldron, *propertyIter, snapshot, DataAccess::Interface::SOURCE_ROCK_ONLY_OUTPUT );
   } 

   for ( propertyIter = shaleGasProperties.begin (); propertyIter != shaleGasProperties.end (); ++propertyIter ) {

      if ( std::find ( genexProperties.begin (), genexProperties.end (), *propertyIter ) == genexProperties.end ()) {
         computePropertyMaps ( cauldron, *propertyIter, snapshot, DataAccess::Interface::SHALE_GAS_ONLY_OUTPUT );
      }

   } 

   initialisePropertyMaps ();
   calculatePropertyMaps ();
   finalisePropertyMaps ();

}

//------------------------------------------------------------//

void PropertyManager::computePropertyMaps ( AppCtx*                    cauldron,
                                            const PropListVec&         requiredProperties,
                                            const Interface::Snapshot* snapshot,
                                            const Interface::PropertyOutputOption maximumOutputOption ) {

   PropListVec::const_iterator propertyIter;

   for ( propertyIter = requiredProperties.begin (); propertyIter != requiredProperties.end (); ++propertyIter ) {
      computePropertyMaps ( cauldron, *propertyIter, snapshot, maximumOutputOption );
   } 

   initialisePropertyMaps ();
   calculatePropertyMaps ();
   finalisePropertyMaps ();
}

//------------------------------------------------------------//

void PropertyManager::computePropertyVolumes ( AppCtx*                               cauldron,
                                               const PropListVec&                    requiredProperties,
                                               const Interface::Snapshot*            snapshot,
                                               const Interface::PropertyOutputOption maximumOutputOption ) {

   PropListVec::const_iterator propertyIter;

   for ( propertyIter = requiredProperties.begin (); propertyIter != requiredProperties.end (); ++propertyIter ) {
      computePropertyVolumes ( cauldron, *propertyIter, snapshot, maximumOutputOption );
   } 

   initialisePropertyVolumes ();
   calculatePropertyVolumes ();
   finalisePropertyVolumes  ();
}

//------------------------------------------------------------//

void PropertyManager::computeProperties ( AppCtx*                               cauldron,
                                          const PropListVec&                    requiredPropertyMaps,
                                          const PropListVec&                    requiredPropertyVolumes,
                                          const Interface::Snapshot*            snapshot,
                                          const Interface::PropertyOutputOption maximumOutputOption ) {

   PropListVec::const_iterator propertyIter;

   for ( propertyIter = requiredPropertyMaps.begin (); propertyIter != requiredPropertyMaps.end (); ++propertyIter ) {
      computePropertyMaps ( cauldron, *propertyIter, snapshot, maximumOutputOption );
   } 

   for ( propertyIter = requiredPropertyVolumes.begin (); propertyIter != requiredPropertyVolumes.end (); ++propertyIter ) {
      computePropertyVolumes ( cauldron, *propertyIter, snapshot, maximumOutputOption );
   } 

   initialisePropertyMaps ();
   initialisePropertyVolumes ();

   calculatePropertyMaps ();
   calculatePropertyVolumes ();

   finalisePropertyMaps ();
   finalisePropertyVolumes  ();

}

//------------------------------------------------------------//

void PropertyManager::finalisePropertyMaps () {

   OutputPropertyMapSet::iterator outputPropertyIter;

   for ( outputPropertyIter = m_mapProperties.begin (); outputPropertyIter != m_mapProperties.end (); ++outputPropertyIter ) {
      (*outputPropertyIter)->finalise ();
   }

}

//------------------------------------------------------------//

void PropertyManager::initialisePropertyMaps () {

   OutputPropertyMapSet::iterator outputPropertyIter;

   for ( outputPropertyIter = m_mapProperties.begin (); outputPropertyIter != m_mapProperties.end (); ++outputPropertyIter ) {
      (*outputPropertyIter)->initialise ();
   }

}

//------------------------------------------------------------//

void PropertyManager::calculatePropertyMaps () {

   OutputPropertyMapSet::iterator outputPropertyIter;

   for ( outputPropertyIter = m_mapProperties.begin (); outputPropertyIter != m_mapProperties.end (); ++outputPropertyIter ) {

      if ((*outputPropertyIter)->anyIsRequired ()) { 
         (*outputPropertyIter)->calculate ();
      }

   }

}

//------------------------------------------------------------//

void PropertyManager::finalisePropertyVolumes () {

   OutputPropertyMapSet::iterator outputPropertyIter;

   for ( outputPropertyIter = m_volumeProperties.begin (); outputPropertyIter != m_volumeProperties.end (); ++outputPropertyIter ) {
      (*outputPropertyIter)->finalise ();
   }

}

//------------------------------------------------------------//

void PropertyManager::initialisePropertyVolumes () {

   OutputPropertyMapSet::iterator outputPropertyIter;

   for ( outputPropertyIter = m_volumeProperties.begin (); outputPropertyIter != m_volumeProperties.end (); ++outputPropertyIter ) {
      (*outputPropertyIter)->initialise ();
   }

}

//------------------------------------------------------------//

void PropertyManager::calculatePropertyVolumes () {

   OutputPropertyMapSet::iterator outputPropertyIter;

   for ( outputPropertyIter = m_volumeProperties.begin (); outputPropertyIter != m_volumeProperties.end (); ++outputPropertyIter ) {

      if ((*outputPropertyIter)->anyIsRequired ()) { 
         (*outputPropertyIter)->calculate ();
      }

   }

}

//------------------------------------------------------------//

void PropertyManager::clear () {

   OutputPropertyMapSet::iterator outputPropertyIter;

   for ( outputPropertyIter = m_mapProperties.begin (); outputPropertyIter != m_mapProperties.end (); ++outputPropertyIter ) {
      delete *outputPropertyIter;
   }

   m_mapProperties.clear ();
   m_allMapProperties.clear ();

   for ( outputPropertyIter = m_volumeProperties.begin (); outputPropertyIter != m_volumeProperties.end (); ++outputPropertyIter ) {
      delete *outputPropertyIter;
   }

   m_volumeProperties.clear ();

}

//------------------------------------------------------------//

OutputPropertyMap* PropertyManager::findOutputPropertyMap ( const std::string&         propertyName,
                                                            const LayerProps*          formation,
                                                            const Interface::Surface*  surface,
                                                            const Interface::Snapshot* snapshot ) const {

   OutputPropertyMapSet::const_iterator outputPropertyIter;
   PropertyList property = FastcauldronSimulator::getInstance ().getCauldron ()->timefilter.getPropertylist ( findOutputPropertyName ( propertyName ));
   OutputPropertyMapAssociation propertyAssociation = m_derivedProperties.getMapAssociation ( property );

   assert ( property != ENDPROPERTYLIST );
   assert ( propertyAssociation != UNKNOWN_ASSOCIATION );

   const LayerProps* requiredFormation = ( propertyAssociation == SURFACE_ASSOCIATION ? 0 : formation );
   const Interface::Surface* requiredSurface = ( propertyAssociation == FORMATION_ASSOCIATION ? 0 : surface );

   // Get output-property-map-list for association and property.
   const OutputPropertyMapSet& propertyList = m_allMapProperties ( propertyAssociation, property );

   for ( outputPropertyIter = propertyList.begin (); outputPropertyIter != propertyList.end (); ++outputPropertyIter ) {

      if ((*outputPropertyIter)->matches ( propertyName, requiredFormation, requiredSurface, snapshot )) {
         return *outputPropertyIter;
      }

   }

   return 0;
}

//------------------------------------------------------------//

OutputPropertyMap* PropertyManager::findOutputPropertyVolume ( const std::string&         propertyName,
                                                               const LayerProps*          formation,
                                                               const Interface::Snapshot* snapshot ) const {

   OutputPropertyMapSet::const_iterator outputPropertyIter;

   for ( outputPropertyIter = m_volumeProperties.begin (); outputPropertyIter != m_volumeProperties.end (); ++outputPropertyIter ) {

      if ((*outputPropertyIter)->matches ( propertyName, formation, 0, snapshot )) {
         return *outputPropertyIter;
      }

   }

   return 0;
}

//------------------------------------------------------------//

void PropertyManager::addMapPropertyMap ( const OutputPropertyMapAssociation association,
                                          const PropertyList                 property,
                                                OutputPropertyMap*           propertyMap ) {

   
   m_allMapProperties ( association, property ).push_back ( propertyMap );
   m_mapProperties.push_back ( propertyMap );
}

//------------------------------------------------------------//

void PropertyManager::OutputPropertyMapSetArray::clear () {

   unsigned int association;
   unsigned int property;

   for ( association = 0; association < NumberOfAssociations; ++association ) {

      for ( property = 0; property < PropertyListSize; ++property ) {
         m_propertyMapList [ association ][ property ].clear ();
      }

   }

}

//------------------------------------------------------------//

OutputPropertyMap* property_manager::FindOutputPropertyMap ( const std::string&         propertyName,
                                                             const LayerProps*          formation,
                                                             const Interface::Surface*  surface,
                                                             const Interface::Snapshot* snapshot )
{
   return  PropertyManager::getInstance().findOutputPropertyMap ( propertyName, formation, surface, snapshot );
}
  

