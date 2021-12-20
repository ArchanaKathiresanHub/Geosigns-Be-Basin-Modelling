//
// Copyright (C) 2015-2016 Shell International Exploration & Production.
// All rights reserved.
//
// Developed under license for Shell by PDS BV.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//
#include "PropertyManager.h"

//Derived property library
#include "PrimaryOutputPropertyMap.h"
#include "DerivedOutputPropertyMap.h"
#include "DerivedOutputPropertyMapFactory.h"

#include "PropertyRetriever.h"
#include "OutputUtilities.h"

//DataAccess library
#include "Surface.h"
#include "Snapshot.h"
#include "Formation.h"

#include "ComponentManager.h"

using namespace Basin_Modelling;

PropertyManager* PropertyManager::s_propertyManager = 0;
std::string PropertyManager::s_nullString = "";

//------------------------------------------------------------//

PropertyManager::PropertyManager():
  m_derivedPropertyManager()
{
  // Mapping from the time-io-table property name to the name that appears in the time-filter old gui.
  // Would it be better to have only those that are not an identity mapping on the map?
  // e.g. Porosity -> PorosityVec, and not Depth -> Depth.
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
  m_propertyName2OutputName [ "GammaRay" ]               = "GammaRay";
  m_propertyName2OutputName [ "HeatFlow" ]               = "HeatFlow";
  m_propertyName2OutputName [ "HeatFlowX" ]              = "HeatFlow";
  m_propertyName2OutputName [ "HeatFlowY" ]              = "HeatFlow";
  m_propertyName2OutputName [ "HeatFlowZ" ]              = "HeatFlow";  
  m_propertyName2OutputName [ "HydroStaticPressure" ]    = "HydroStaticPressure";  
  m_propertyName2OutputName [ "Lithology" ]              = "Lithology";
  m_propertyName2OutputName [ "LithoStaticPressure" ]    = "LithoStaticPressure";
  m_propertyName2OutputName [ "MassFlux" ]               = "MassFlux";
  m_propertyName2OutputName [ "MassFluxX" ]              = "MassFlux";
  m_propertyName2OutputName [ "MassFluxY" ]              = "MassFlux";
  m_propertyName2OutputName [ "MassFluxZ" ]              = "MassFlux";
  m_propertyName2OutputName [ "MaxVesHighRes" ]          = "MaxVesVec";
  m_propertyName2OutputName [ "MaxVes" ]                 = "MaxVesVec";
  m_propertyName2OutputName [ "Overburden" ]             = s_nullString;
  m_propertyName2OutputName [ "OverPressure" ]           = "OverPressure";
  m_propertyName2OutputName [ "HorizontalPermeability" ] = "PermeabilityHVec";
  m_propertyName2OutputName [ "Permeability" ]           = "PermeabilityVec";
  m_propertyName2OutputName [ "Porosity" ]               = "PorosityVec";
  m_propertyName2OutputName [ "Pressure" ]               = "Pressure";
  m_propertyName2OutputName [ "FracturePressure" ]       = "FracturePressure";
  m_propertyName2OutputName [ "Reflectivity" ]           = "ReflectivityVec";
  m_propertyName2OutputName [ "SonicSlowness" ]          = "SonicVec";
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

  for( int i = 0; i < NumberOfPVTComponents; ++i)
  {
    const string propertyName = CBMGenerics::ComponentManager::getInstance ().getSpeciesName ( i ) + "Concentration";
    m_propertyName2OutputName [ propertyName ] = propertyName;
  }


  m_propertyName2OutputName [ "ElementMass" ] = "ElementMass";

  m_propertyName2OutputName [ "BrineSaturation" ]      = "BrineSaturation";
  m_propertyName2OutputName [ "HcLiquidSaturation" ]   = "HcLiquidSaturation";
  m_propertyName2OutputName [ "HcVapourSaturation" ]   = "HcVapourSaturation";
  m_propertyName2OutputName [ "ImmobileSaturation" ]   = "ImmobileSaturation";

  //average saturations
  m_propertyName2OutputName [ "AverageBrineSaturation" ]      = "AverageBrineSaturation";
  m_propertyName2OutputName [ "AverageHcLiquidSaturation" ]   = "AverageHcLiquidSaturation";
  m_propertyName2OutputName [ "AverageHcVapourSaturation" ]   = "AverageHcVapourSaturation";
  m_propertyName2OutputName [ "AverageImmobileSaturation" ]   = "AverageImmobileSaturation";


  m_propertyName2OutputName [ "HcVapourDensity" ]   = "HcVapourDensity";
  m_propertyName2OutputName [ "HcLiquidDensity" ]   = "HcLiquidDensity";
  m_propertyName2OutputName [ "HcVapourViscosity" ] = "HcVapourViscosity";
  m_propertyName2OutputName [ "HcLiquidViscosity" ] = "HcLiquidViscosity";

  m_propertyName2OutputName [ "HcVapourVelocityX" ]         = "HcVapourVelocityX";
  m_propertyName2OutputName [ "HcVapourVelocityY" ]         = "HcVapourVelocityY";
  m_propertyName2OutputName [ "HcVapourVelocityZ" ]         = "HcVapourVelocityZ";
  m_propertyName2OutputName [ "HcVapourVelocityMagnitude" ] = "HcVapourVelocityMagnitude";

  m_propertyName2OutputName [ "HcLiquidVelocityX" ]         = "HcLiquidVelocityX";
  m_propertyName2OutputName [ "HcLiquidVelocityY" ]         = "HcLiquidVelocityY";
  m_propertyName2OutputName [ "HcLiquidVelocityZ" ]         = "HcLiquidVelocityZ";
  m_propertyName2OutputName [ "HcLiquidVelocityMagnitude" ] = "HcLiquidVelocityMagnitude";



  m_propertyName2OutputName [ "HcVapourVolume" ] = "HcVapourVolume";
  m_propertyName2OutputName [ "HcLiquidVolume" ] = "HcLiquidVolume";

  m_propertyName2OutputName [ "ElementVolume" ]                = "ElementVolume";
  m_propertyName2OutputName [ "ElementPoreVolume" ]            = "ElementPoreVolume";
  m_propertyName2OutputName [ "TransportedMass" ]              = "TransportedMass";
  m_propertyName2OutputName [ "BrineRelativePermeability" ]    = "BrineRelativePermeability";
  m_propertyName2OutputName [ "HcLiquidRelativePermeability" ] = "HcLiquidRelativePermeability";
  m_propertyName2OutputName [ "HcVapourRelativePermeability" ] = "HcVapourRelativePermeability";


  m_propertyName2OutputName [ "HcLiquidBrineCapillaryPressure" ] = "HcLiquidBrineCapillaryPressure";
  m_propertyName2OutputName [ "HcVapourBrineCapillaryPressure" ] = "HcVapourBrineCapillaryPressure";
  m_propertyName2OutputName [ "GOR" ]                            = "GOR";
  m_propertyName2OutputName [ "CGR" ]                            = "CGR";
  m_propertyName2OutputName [ "OilAPI" ]                         = "OilAPI";
  m_propertyName2OutputName [ "CondensateAPI" ]                  = "CondensateAPI";
  m_propertyName2OutputName [ "BrineDensity" ]                   = "BrineDensity";
  m_propertyName2OutputName [ "BrineViscosity" ]                 = "BrineViscosity";
  m_propertyName2OutputName [ "TimeOfInvasion" ]                 = "TimeOfInvasion";

  //ALC default properties
  m_propertyName2OutputName["ALCStepContCrustThickness"] = "ALCStepContCrustThickness";
  m_propertyName2OutputName["ALCStepBasaltThickness"]    = "ALCStepBasaltThickness";
  m_propertyName2OutputName["ALCStepTopBasaltDepth"]     = "ALCStepTopBasaltDepth";
  m_propertyName2OutputName["ALCStepMohoDepth"]          = "ALCStepMohoDepth";

  //ALC debug properties
  m_propertyName2OutputName["ALCOrigLithMantleDepth"]    = "ALCOrigLithMantleDepth";
  m_propertyName2OutputName["ALCMaxAsthenoMantleDepth"]  = "ALCMaxAsthenoMantleDepth";
  m_propertyName2OutputName["ALCSmContCrustThickness"]   = "ALCSmContCrustThickness";
  m_propertyName2OutputName["ALCSmBasaltThickness"]      = "ALCSmBasaltThickness";
  m_propertyName2OutputName["ALCSmTopBasaltDepth"]       = "ALCSmTopBasaltDepth";
  m_propertyName2OutputName["ALCSmMohoDepth"]            = "ALCSmMohoDepth";


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

const std::string& PropertyManager::findPropertyName ( const std::string& name ) const {

  PropertyNameMap::const_iterator propertyNameIt;

  for ( propertyNameIt = m_propertyName2OutputName.begin (); propertyNameIt != m_propertyName2OutputName.end(); ++ propertyNameIt ) {
    if( propertyNameIt->second == name ) {
      return propertyNameIt->first;
    }
  }
  return s_nullString;
}

//------------------------------------------------------------//

void PropertyManager::createSurfacePropertyMaps ( const PropertyIdentifier& requiredProperty,
                                                  const Interface::Snapshot* snapshot,
                                                  const LayerProps_Ptr formation,
                                                  const LayerProps_Ptr formationBelow,
                                                  const LayerProps_Ptr formationAbove,
                                                  const bool createForTopSurfaceOnly,
                                                  const bool isPrimary )
{
  if ( formation->isSediment ())
  {
    if ( isPrimary )
    {
      addMapPropertyMap ( SURFACE_ASSOCIATION, requiredProperty,
                          m_derivedProperties.allocateMap( requiredProperty, formation, formation->getTopSurface(), snapshot) );
    }
    else
    {
      createDerivedProperty( requiredProperty, snapshot, formation, formation->getTopSurface() );
    }

    // Must get the bottom-most surface if sediments-only is selected.
    /// createForTopSurfaceOnly applies only to sediment formations
    if ( not createForTopSurfaceOnly and ( formationBelow == 0 or formationBelow->isCrust ()))
    {
      if ( isPrimary )
      {
        addMapPropertyMap ( SURFACE_ASSOCIATION, requiredProperty,
                            m_derivedProperties.allocateMap( requiredProperty, formation, formation->getBottomSurface(), snapshot ) );
      }
      else
      {
        createDerivedProperty( requiredProperty, snapshot, formation, formation->getBottomSurface() );
      }
    }
  }
  else
  {
    if ( formationAbove == 0 and formation->isCrust ())
    {
      if ( isPrimary )
      {
        addMapPropertyMap( SURFACE_ASSOCIATION, requiredProperty,
                           m_derivedProperties.allocateMap( requiredProperty, formation, formation->getTopSurface(), snapshot ) );
      }
      else
      {
        createDerivedProperty( requiredProperty, snapshot, formation, formation->getTopSurface() );
      }
    }

    if ( isPrimary )
    {
      addMapPropertyMap( SURFACE_ASSOCIATION, requiredProperty,
                         m_derivedProperties.allocateMap ( requiredProperty, formation, formation->getBottomSurface (), snapshot ) );
    }
    else
    {
      createDerivedProperty( requiredProperty, snapshot, formation, formation->getBottomSurface() );
    }
  }
}

//------------------------------------------------------------//

void PropertyManager::createSurfaceFormationPropertyMaps ( const PropertyIdentifier& requiredProperty,
                                                           const Interface::Snapshot* snapshot,
                                                           const LayerProps_Ptr formation,
                                                           const bool createForTopSurfaceOnly,
                                                           const bool isPrimary )
{

  if ( isPrimary )
  {
    addMapPropertyMap( SURFACE_FORMATION_ASSOCIATION, requiredProperty,
                       m_derivedProperties.allocateMap ( requiredProperty, formation, formation->getTopSurface(), snapshot ) );
  }
  else
  {
    createDerivedProperty( requiredProperty, snapshot, formation, formation->getTopSurface() );
  }

  if ( !createForTopSurfaceOnly )
  {
    // It property is a vector-quantity then it must also be output on the bottom surface of the formation.
    if ( isPrimary )
    {
      addMapPropertyMap( SURFACE_FORMATION_ASSOCIATION, requiredProperty,
                         m_derivedProperties.allocateMap ( requiredProperty, formation, formation->getBottomSurface(), snapshot ) );
    }
    else
    {
      createDerivedProperty( requiredProperty, snapshot, formation, formation->getBottomSurface() );
    }
  }
}

void PropertyManager::createDerivedProperty( const PropertyIdentifier& requiredProperty,
                                             const Interface::Snapshot* snapshot,
                                             const LayerProps_Ptr formation,
                                             const Interface::Surface* surface )
{
  for ( const Interface::Property* property : getProperties( requiredProperty ) )
  {
    if ( allowOutput( property->getName(), formation, surface ) )
    {
      OutputPropertyValuePtr propertyValue = DerivedProperties::allocateOutputProperty( *m_derivedPropertyManager, property, snapshot, {formation, surface} );
      if ( propertyValue )
      {
        DerivedProperties::createSnapshotResultPropertyValue( FastcauldronSimulator::getInstance(), propertyValue, snapshot, formation, surface );
      }
    }
  }
}

//------------------------------------------------------------//

void PropertyManager::createFormationMapPropertyMaps ( const PropertyIdentifier& requiredProperty,
                                                       const Interface::Snapshot* snapshot,
                                                       const LayerProps_Ptr formation,
                                                       const bool isPrimary )
{
  if ( isPrimary )
  {
    addMapPropertyMap( FORMATION_ASSOCIATION, requiredProperty,
                       m_derivedProperties.allocateMap ( requiredProperty, formation, formation->getTopSurface (), snapshot) );
  }
  else
  {
    createDerivedProperty( requiredProperty, snapshot, formation, 0 );
  }
}


//------------------------------------------------------------//

void PropertyManager::computePropertyMaps ( AppCtx*                    cauldron,
                                            const PropertyIdentifier&        requiredProperty,
                                            const Interface::Snapshot* snapshot,
                                            const Interface::PropertyOutputOption maximumOutputOption,
                                            const bool isPrimary )
{
  if ( not m_derivedProperties.isMapDefined ( requiredProperty )) {
    // Error?
    return;
  }

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

  for ( layers.Initialise_Iterator (); ! layers.Iteration_Is_Done (); layers++ )
  {
    LayerProps_Ptr formation = layers.Current_Layer();

    switch ( m_derivedProperties.getMapAssociation ( requiredProperty ) )
    {
      case SURFACE_ASSOCIATION :
        createSurfacePropertyMaps ( requiredProperty, snapshot, formation, layers.Layer_Below(), layers.Layer_Above(), topSurfaceOnly, isPrimary );
        break;

      case FORMATION_ASSOCIATION :
        createFormationMapPropertyMaps ( requiredProperty, snapshot, formation, isPrimary );
        break;

      case SURFACE_FORMATION_ASSOCIATION :
        createSurfaceFormationPropertyMaps ( requiredProperty, snapshot, formation, topSurfaceOnly, isPrimary );
        break;

      default :
        break;
    }

  }

}

//------------------------------------------------------------//

void PropertyManager::computePropertyVolumes ( AppCtx*                               cauldron,
                                               const PropertyIdentifier&                   requiredProperty,
                                               const Interface::Snapshot*            snapshot,
                                               const Interface::PropertyOutputOption maximumOutputOption,
                                               const bool isPrimary )
{
  Layer_Iterator layers;

  if ( maximumOutputOption == Interface::SEDIMENTS_AND_BASEMENT_OUTPUT ) {
    layers.Initialise_Iterator ( cauldron->layers, Descending, Basement_And_Sediments, Active_Layers_Only );
  } else if ( maximumOutputOption == Interface::SEDIMENTS_ONLY_OUTPUT ) {
    layers.Initialise_Iterator ( cauldron->layers, Descending, Sediments_Only, Active_Layers_Only );
  } else if ( maximumOutputOption == Interface::SOURCE_ROCK_ONLY_OUTPUT ) {
    layers.Initialise_Iterator ( cauldron->layers, Descending, Source_Rocks_Only, Active_Layers_Only );
  } else {
    return;
  }

  for ( layers.Initialise_Iterator (); ! layers.Iteration_Is_Done (); layers++ )
  {
    const LayerProps_Ptr formation = layers.Current_Layer();


    if ( isPrimary )
    {
      if ( OutputPropertyMap* outputProperty = m_derivedProperties.allocateVolume ( requiredProperty, formation, snapshot ) )
      {
        m_volumeProperties.push_back ( outputProperty );
      }
    }
    else
    {
      createDerivedProperty( requiredProperty, snapshot, formation, 0 );
    }
  }
}

//------------------------------------------------------------//

void PropertyManager::computeSourceRockPropertyMaps ( AppCtx*                    cauldron,
                                                      const Interface::Snapshot* snapshot,
                                                      const PropListVec&         genexProperties,
                                                      const PropListVec&         shaleGasProperties )
{
  m_derivedPropertyManager.reset();

  for ( const PropertyIdentifier& genexProperty : genexProperties )
  {
    computePropertyMaps ( cauldron, genexProperty, snapshot, DataAccess::Interface::SOURCE_ROCK_ONLY_OUTPUT );
  }

  for ( const PropertyIdentifier& shaleGasProperty : shaleGasProperties )
  {
    if ( std::find ( genexProperties.begin (), genexProperties.end (), shaleGasProperty ) == genexProperties.end ())
    {
      computePropertyMaps ( cauldron, shaleGasProperty, snapshot, DataAccess::Interface::SHALE_GAS_ONLY_OUTPUT );
    }
  }

  initialisePropertyMaps();
  calculatePropertyMaps();
  finalisePropertyMaps();
  clear();
}
//------------------------------------------------------------//

void PropertyManager::computeSourceRockPropertyVolumes ( AppCtx*                    cauldron,
                                                         const Interface::Snapshot* snapshot,
                                                         const PropListVec&         genexProperties,
                                                         const PropListVec&         shaleGasProperties ) {

  // Some of the properties (as Vr) should be output to the primary properties file
  if(( FastcauldronSimulator::getInstance ().isPrimaryDouble()) and cauldron->projectSnapshots.projectPrescribesMinorSnapshots ())
  {
    m_derivedPropertyManager.reset();

    for ( const PropertyIdentifier& genexProperty : genexProperties )
    {
      computePropertyVolumes ( cauldron, genexProperty, snapshot, DataAccess::Interface::SOURCE_ROCK_ONLY_OUTPUT );
    }

    for ( const PropertyIdentifier& shaleGasProperty : shaleGasProperties )
    {
      if ( std::find ( genexProperties.begin (), genexProperties.end (), shaleGasProperty ) == genexProperties.end ())
      {
        computePropertyVolumes ( cauldron, shaleGasProperty, snapshot, DataAccess::Interface::SOURCE_ROCK_ONLY_OUTPUT );
      }
    }

    initialisePropertyVolumes();
    calculatePropertyVolumes();
    finalisePropertyVolumes();
    clear();
  }
}

void PropertyManager::computeProperties ( AppCtx*                               cauldron,
                                          const PropListVec&                    mapProperties,
                                          const PropListVec&                    volumeProperties,
                                          const Interface::Snapshot*            snapshot,
                                          const Interface::PropertyOutputOption maximumOutputOption )
{
  PropListVec requiredVolumeProperties = volumeProperties;
  PropListVec requiredMapProperties = mapProperties;

  computePrimaryPropertyVolumes( cauldron, requiredVolumeProperties, snapshot, maximumOutputOption );
  computePrimaryPropertyMaps( cauldron, requiredMapProperties, snapshot, maximumOutputOption );

  // Derived volume properties dependent on ALC need primary map properties already calculated
  computeDerivedPropertyVolumes( cauldron, requiredVolumeProperties, snapshot, maximumOutputOption );
  computeDerivedPropertyMaps( cauldron, requiredMapProperties, snapshot, maximumOutputOption );
}

void PropertyManager::computeVolumeProperties( AppCtx* cauldron,
                                               const PropListVec& volumeProperties,
                                               const Interface::Snapshot* snapshot,
                                               const Interface::PropertyOutputOption maximumOutputOption )
{
  PropListVec requiredVolumeProperties = volumeProperties;
  computePrimaryPropertyVolumes( cauldron, requiredVolumeProperties, snapshot, maximumOutputOption );
  computeDerivedPropertyVolumes( cauldron, requiredVolumeProperties, snapshot, maximumOutputOption );
}


void PropertyManager::computeMapProperties( AppCtx* cauldron,
                                            const PropListVec& mapProperties,
                                            const Interface::Snapshot* snapshot,
                                            const Interface::PropertyOutputOption maximumOutputOption )
{
  PropListVec requiredMapProperties = mapProperties;
  computePrimaryPropertyMaps( cauldron, requiredMapProperties, snapshot, maximumOutputOption );
  computeDerivedPropertyMaps( cauldron, requiredMapProperties, snapshot, maximumOutputOption );
}

//------------------------------------------------------------//

void PropertyManager::computePrimaryPropertyMaps ( AppCtx*                    cauldron,
                                                   PropListVec&         requiredProperties,
                                                   const Interface::Snapshot* snapshot,
                                                   const Interface::PropertyOutputOption maximumOutputOption )
{
  PropListVec derivedProperties;
  int mapSize = m_mapProperties.size();
  for ( const PropertyIdentifier& property : requiredProperties )
  {
    computePropertyMaps ( cauldron, property, snapshot, maximumOutputOption );
    if ( mapSize == m_mapProperties.size() )
    {
      derivedProperties.push_back(property);
    }
    mapSize = m_mapProperties.size();
  }
  requiredProperties = derivedProperties;

  initialisePropertyMaps ();
  calculatePropertyMaps ();
  finalisePropertyMaps ();
  clear();
}

void PropertyManager::computeDerivedPropertyMaps ( AppCtx*                    cauldron,
                                                   const PropListVec&         requiredProperties,
                                                   const Interface::Snapshot* snapshot,
                                                   const Interface::PropertyOutputOption maximumOutputOption )
{
  database::Record * tempRecord = FastcauldronSimulator::getInstance().addCurrentSimulationDetails();

  m_derivedPropertyManager.reset(new DerivedProperties::DerivedPropertyManager(FastcauldronSimulator::getInstance(), false));

  for ( const PropertyIdentifier& property : requiredProperties )
  {
    computePropertyMaps ( cauldron, property, snapshot, maximumOutputOption, false );
  }

  FastcauldronSimulator::getInstance().removeCurrentSimulationDetails( tempRecord );
}


//------------------------------------------------------------//

void PropertyManager::computePrimaryPropertyVolumes ( AppCtx*                               cauldron,
                                                      PropListVec&                    requiredProperties,
                                                      const Interface::Snapshot*            snapshot,
                                                      const Interface::PropertyOutputOption maximumOutputOption )
{
  m_derivedPropertyManager.reset();

  PropListVec derivedProperties;
  int volumeSize = m_volumeProperties.size();
  for ( const PropertyIdentifier& property : requiredProperties )
  {
    computePropertyVolumes ( cauldron, property, snapshot, maximumOutputOption );
    if ( volumeSize == m_volumeProperties.size() )
    {
      derivedProperties.push_back(property);
    }
    volumeSize = m_volumeProperties.size();
  }
  requiredProperties = derivedProperties;

  initialisePropertyVolumes();
  calculatePropertyVolumes();
  finalisePropertyVolumes();
  clear();
}

void PropertyManager::computeDerivedPropertyVolumes ( AppCtx*                               cauldron,
                                                      const PropListVec&                    requiredProperties,
                                                      const Interface::Snapshot*            snapshot,
                                                      const Interface::PropertyOutputOption maximumOutputOption )
{
  database::Record * tempRecord = FastcauldronSimulator::getInstance().addCurrentSimulationDetails();

  m_derivedPropertyManager.reset(new DerivedProperties::DerivedPropertyManager(FastcauldronSimulator::getInstance(), false));

  for ( const PropertyIdentifier& requiredPropertyVolume : requiredProperties )
  {
    computePropertyVolumes ( cauldron, requiredPropertyVolume, snapshot, maximumOutputOption, false );
  }

  FastcauldronSimulator::getInstance().removeCurrentSimulationDetails( tempRecord );
}

//------------------------------------------------------------//

void PropertyManager::finalisePropertyMaps ()
{
  for ( OutputPropertyMap* mapProperty : m_mapProperties )
  {
    mapProperty->finalise ();
  }
}

//------------------------------------------------------------//

void PropertyManager::initialisePropertyMaps ()
{
  for ( OutputPropertyMap* mapProperty : m_mapProperties )
  {
    mapProperty->initialise ();
  }
}

//------------------------------------------------------------//

void PropertyManager::calculatePropertyMaps ()
{
  for ( OutputPropertyMap* mapProperty : m_mapProperties )
  {
    if (mapProperty->anyIsRequired ())
    {
      mapProperty->calculate ();
    }
  }
}

//------------------------------------------------------------//

void PropertyManager::finalisePropertyVolumes ()
{
  for ( OutputPropertyMap* volumeProperty : m_volumeProperties )
  {
    volumeProperty->finalise ();
  }
}

//------------------------------------------------------------//

void PropertyManager::initialisePropertyVolumes ()
{
  for ( OutputPropertyMap* volumeProperty : m_volumeProperties )
  {
    volumeProperty->initialise ();
  }
}

//------------------------------------------------------------//

void PropertyManager::calculatePropertyVolumes ()
{
  for ( OutputPropertyMap* volumeProperty : m_volumeProperties )
  {
    if (volumeProperty->anyIsRequired ())
    {
      volumeProperty->calculate ();
    }
  }
}

//------------------------------------------------------------//

void PropertyManager::clear ()
{
  for ( OutputPropertyMap* mapProperty : m_mapProperties )
  {
    delete mapProperty;
  }

  m_mapProperties.clear ();
  m_allMapProperties.clear ();

  for ( OutputPropertyMap* volumeProperty : m_volumeProperties )
  {
    delete volumeProperty;
  }

  m_volumeProperties.clear ();
}

//------------------------------------------------------------//

OutputPropertyMap* PropertyManager::findOutputPropertyMap ( const std::string&         propertyName,
                                                            const LayerProps*          formation,
                                                            const Interface::Surface*  surface,
                                                            const Interface::Snapshot* snapshot ) const {

  PropertyIdentifier property = FastcauldronSimulator::getInstance ().getCauldron ()->timefilter.getPropertylist ( findOutputPropertyName ( propertyName ));
  OutputPropertyMapAssociation propertyAssociation = m_derivedProperties.getMapAssociation ( property );

  assert ( property != ENDPROPERTYLIST );
  assert ( propertyAssociation != UNKNOWN_ASSOCIATION );

  const LayerProps* requiredFormation = ( propertyAssociation == SURFACE_ASSOCIATION ? 0 : formation );
  const Interface::Surface* requiredSurface = ( propertyAssociation == FORMATION_ASSOCIATION ? 0 : surface );

  for ( OutputPropertyMap* outputProperty : m_allMapProperties.at(propertyAssociation).at(property) )
  {
    if (outputProperty->matches ( propertyName, requiredFormation, requiredSurface, snapshot ))
    {
      return outputProperty;
    }
  }

  return 0;
}

//------------------------------------------------------------//

OutputPropertyMap* PropertyManager::findOutputPropertyVolume ( const std::string&         propertyName,
                                                               const LayerProps*          formation,
                                                               const Interface::Snapshot* snapshot ) const
{
  for ( OutputPropertyMap* volumeProperty : m_volumeProperties )
  {
    if (volumeProperty->matches ( propertyName, formation, 0, snapshot ))
    {
      return volumeProperty;
    }
  }

  return 0;
}

Interface::PropertyList PropertyManager::getProperties(const PropertyIdentifier& requiredProperty)
{
  Interface::PropertyList propertyList;
  const std::string& listName = propertyListName(requiredProperty);
  for ( auto& namePair : m_propertyName2OutputName )
  {
    if ( namePair.second == listName )
    {
      propertyList.push_back( FastcauldronSimulator::getInstance().findProperty( namePair.first ) );
    }
  }
  return propertyList;
}

//------------------------------------------------------------//

void PropertyManager::addMapPropertyMap ( const OutputPropertyMapAssociation association,
                                          const PropertyIdentifier&          property,
                                          OutputPropertyMap*           propertyMap )
{
  if ( !propertyMap )
  {
    return;
  }

  m_allMapProperties[association][property].push_back ( propertyMap );
  m_mapProperties.push_back ( propertyMap );
}

//------------------------------------------------------------//

OutputPropertyMap* property_manager::FindOutputPropertyMap ( const std::string&         propertyName,
                                                             const LayerProps*          formation,
                                                             const Interface::Surface*  surface,
                                                             const Interface::Snapshot* snapshot )
{
  return  PropertyManager::getInstance().findOutputPropertyMap ( propertyName, formation, surface, snapshot );
}


bool PropertyManager::allowOutput( const string & propertyName,
                                   const Interface::Formation* formation,
                                   const Interface::Surface* surface ) const
{
  const bool decompactionMode = ( FastcauldronSimulator::getInstance().getCalculationMode () == COUPLED_HIGH_RES_DECOMPACTION_MODE ||
                                  FastcauldronSimulator::getInstance().getCalculationMode () == HYDROSTATIC_HIGH_RES_DECOMPACTION_MODE ||
                                  FastcauldronSimulator::getInstance().getCalculationMode () == HYDROSTATIC_DECOMPACTION_MODE );

  if ( ( propertyName == "BrineDensity" || propertyName == "BrineViscosity" ) && surface != 0 )
  {
    return false;
  }

  if ( decompactionMode and ( propertyName == "BulkDensity" ) and surface == 0 )
  {
    return false;
  }

  const bool basementFormation = ( dynamic_cast<const GeoPhysics::GeoPhysicsFormation*>( formation ) != 0 &&
      dynamic_cast<const GeoPhysics::GeoPhysicsFormation*>( formation )->kind () == DataAccess::Interface::BASEMENT_FORMATION );

  // The top of the crust is a part of the sediment
  if ( basementFormation && surface != 0 &&
       ( propertyName == "Depth" || propertyName == "Temperature" ) &&
       dynamic_cast<const GeoPhysics::GeoPhysicsFormation*>( formation )->isCrust() &&
       formation->getTopSurface() && ( formation->getTopSurface() == surface ) )
  {
    return true;
  }

  const string outputPropertyName = findOutputPropertyName( propertyName );
  const OutputOption option = FastcauldronSimulator::getInstance().getCauldron()->timefilter.getPropertyOutputOption( outputPropertyName );
  const Interface::PropertyOutputOption fastcauldronOption = FastcauldronSimulator::getInstance().getOutputPropertyOption( outputPropertyName );

  if ( ( fastcauldronOption == Interface::NO_OUTPUT && option == NOOUTPUT ) ||
       basementFormation && ( fastcauldronOption < Interface::SEDIMENTS_AND_BASEMENT_OUTPUT || option < SEDIMENTSPLUSBASEMENT ) )
  {
    return false;
  }

  return true;
}

