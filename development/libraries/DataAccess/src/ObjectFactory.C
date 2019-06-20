//
// Copyright (C) 2015-2016 Shell International Exploration & Production.
// All rights reserved.
//
// Developed under license for Shell by PDS BV.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#include <iostream>
using namespace std;

#include "AllochthonousLithology.h"
#include "AllochthonousLithologyDistribution.h"
#include "AllochthonousLithologyInterpolation.h"
#include "BasementSurface.h"
#include "BiodegradationParameters.h"
#include "ConstrainedOverpressureInterval.h"
#include "CrustFormation.h"
#include "DiffusionLeakageParameters.h"
#include "FaultCollection.h"
#include "FluidDensitySample.h"
#include "FluidHeatCapacitySample.h"
#include "FluidType.h"
#include "FluidThermalConductivitySample.h"
#include "Formation.h"
#include "FracturePressureFunctionParameters.h"
#include "Grid.h"
#include "GridMap.h"
#include "IgneousIntrusionEvent.h"
#include "InputValue.h"
#include "IrreducibleWaterSaturationSample.h"
#include "LangmuirAdsorptionIsothermSample.h"
#include "LangmuirAdsorptionTOCEntry.h"
#include "LithoType.h"
#include "LithologyHeatCapacitySample.h"
#include "LithologyThermalConductivitySample.h"
#include "MantleFormation.h"
#include "MobileLayer.h"
#include "ObjectFactory.h"
#include "OutputProperty.h"
#include "PaleoFormationProperty.h"
#include "PaleoProperty.h"
#include "PaleoSurfaceProperty.h"
#include "PermafrostEvent.h"
#include "PointAdsorptionHistory.h"
#include "ProjectData.h"
#include "ProjectHandle.h"
#include "Property.h"
#include "PropertyValue.h"
#include "RelatedProject.h"
#include "Reservoir.h"
#include "ReservoirOptions.h"
#include "RunParameters.h"
#include "SGDensitySample.h"
#include "SimulationDetails.h"
#include "Snapshot.h"
#include "SourceRock.h"
#include "Surface.h"
#include "TouchstoneMap.h"
#include "Trap.h"
#include "Trapper.h"
#include "Migration.h"

#include "CrustalThicknessData.h"
#include "CrustalThicknessRiftingHistoryData.h"
#include "OceanicCrustThicknessHistoryData.h"

#include "ProjectFileHandler.h"

using namespace DataAccess;
using namespace Interface;

ProjectHandle * ObjectFactory::produceProjectHandle (database::ProjectFileHandlerPtr pfh, const string & name, const string & accessMode) const
{
   return new ProjectHandle ( pfh, name, accessMode, this);
}

Snapshot * ObjectFactory::produceSnapshot (ProjectHandle * projectHandle, database::Record * record) const
{
   return new Snapshot (projectHandle, record);
}

Snapshot * ObjectFactory::produceSnapshot (ProjectHandle * projectHandle, double time) const
{
   return new Snapshot (projectHandle, time);
}

Formation * ObjectFactory::produceFormation (ProjectHandle * projectHandle, database::Record * record) const
{
   return new Formation (projectHandle, record);
}

LithoType * ObjectFactory::produceLithoType (ProjectHandle * projectHandle, database::Record * record) const
{
   return new LithoType (projectHandle, record);
}

SourceRock * ObjectFactory::produceSourceRock (ProjectHandle * projectHandle, database::Record * record) const
{
   return new SourceRock (projectHandle, record);
}

TouchstoneMap * ObjectFactory::produceTouchstoneMap (ProjectHandle * projectHandle, database::Record * record) const
{
   return new TouchstoneMap (projectHandle, record);
}

Surface * ObjectFactory::produceSurface (ProjectHandle * projectHandle, database::Record * record) const
{
   return new Surface (projectHandle, record);
}

BasementSurface * ObjectFactory::produceBasementSurface (ProjectHandle * projectHandle, const std::string& name ) const
{
   return new BasementSurface (projectHandle, name);
}

Reservoir * ObjectFactory::produceReservoir (ProjectHandle * projectHandle, database::Record * record) const
{
   return new Reservoir (projectHandle, record);
}

std::shared_ptr<ReservoirOptions> ObjectFactory::produceReservoirOptions (ProjectHandle * projectHandle, database::Record * record) const
{
   return std::make_shared<ReservoirOptions> (projectHandle, record);
}

MobileLayer * ObjectFactory::produceMobileLayer (ProjectHandle * projectHandle, database::Record * record) const
{
   return new MobileLayer (projectHandle, record);
}

PaleoProperty * ObjectFactory::producePaleoProperty (ProjectHandle * projectHandle, database::Record * record ) const
{
   return new PaleoProperty (projectHandle, record);
}

#if 0
PaleoFormationProperty * ObjectFactory::producePaleoFormationProperty (ProjectHandle * projectHandle, database::Record * record, const std::string& formationName ) const
{
   return new PaleoFormationProperty (projectHandle, record, formationName);
}
#endif

PaleoFormationProperty * ObjectFactory::producePaleoFormationProperty (ProjectHandle * projectHandle, database::Record * record, const Formation* formation ) const
{
   return new PaleoFormationProperty (projectHandle, record, formation);
}

PaleoFormationProperty* ObjectFactory::producePaleoFormationProperty ( ProjectHandle * projectHandle,
                                                                       const Formation* formation,
                                                                       const PaleoFormationProperty* startProperty,
                                                                       const PaleoFormationProperty* endProperty,
                                                                       const Snapshot*               interpolationTime ) const
{
   return new PaleoFormationProperty (projectHandle, formation, startProperty, endProperty, interpolationTime );
}


PaleoSurfaceProperty * ObjectFactory::producePaleoSurfaceProperty (ProjectHandle * projectHandle, database::Record * record, const Surface* surface ) const
{
   return new PaleoSurfaceProperty (projectHandle, record, surface);
}

CrustFormation * ObjectFactory::produceCrustFormation (ProjectHandle * projectHandle, database::Record * record) const
{
   return new CrustFormation ( projectHandle, record );
}

MantleFormation * ObjectFactory::produceMantleFormation (ProjectHandle * projectHandle, database::Record * record) const
{
   return new MantleFormation ( projectHandle, record );
}

RunParameters * ObjectFactory::produceRunParameters (ProjectHandle * projectHandle, database::Record * record) const
{
   return new RunParameters ( projectHandle, record );
}

ProjectData * ObjectFactory::produceProjectData (ProjectHandle * projectHandle, database::Record * record) const
{
   return new ProjectData ( projectHandle, record );
}

SimulationDetails* ObjectFactory::produceSimulationDetails ( ProjectHandle * projectHandle, database::Record * record ) const
{
   return new SimulationDetails ( projectHandle, record );
}


AllochthonousLithology * ObjectFactory::produceAllochthonousLithology (ProjectHandle * projectHandle, database::Record * record) const
{
  return new AllochthonousLithology (projectHandle, record);
}

AllochthonousLithologyDistribution * ObjectFactory::produceAllochthonousLithologyDistribution (ProjectHandle * projectHandle, database::Record * record) const
{
  return new AllochthonousLithologyDistribution (projectHandle, record);
}

AllochthonousLithologyInterpolation * ObjectFactory::produceAllochthonousLithologyInterpolation (ProjectHandle * projectHandle, database::Record * record) const
{
  return new AllochthonousLithologyInterpolation (projectHandle, record);
}

OutputProperty * ObjectFactory::produceOutputProperty (ProjectHandle * projectHandle, database::Record * record) const
{
   return new OutputProperty ( projectHandle, record );
}

OutputProperty * ObjectFactory::produceOutputProperty (ProjectHandle * projectHandle, const ModellingMode mode, const PropertyOutputOption option, const std::string& name ) const
{
   return new OutputProperty ( projectHandle, mode, option, name );
}

LithologyHeatCapacitySample * ObjectFactory::produceLithologyHeatCapacitySample (ProjectHandle * projectHandle, database::Record * record) const
{
   return new LithologyHeatCapacitySample ( projectHandle, record );
}

LithologyThermalConductivitySample * ObjectFactory::produceLithologyThermalConductivitySample (ProjectHandle * projectHandle, database::Record * record) const
{
   return new LithologyThermalConductivitySample ( projectHandle, record );
}

ConstrainedOverpressureInterval* ObjectFactory::produceConstrainedOverpressureInterval (ProjectHandle * projectHandle, database::Record * record, const Formation* formation) const
{
   return new ConstrainedOverpressureInterval ( projectHandle, record, formation );
}

FluidHeatCapacitySample * ObjectFactory::produceFluidHeatCapacitySample (ProjectHandle * projectHandle, database::Record * record) const
{
   return new FluidHeatCapacitySample ( projectHandle, record );
}

FluidThermalConductivitySample * ObjectFactory::produceFluidThermalConductivitySample (ProjectHandle * projectHandle, database::Record * record) const
{
   return new FluidThermalConductivitySample ( projectHandle, record );
}

FluidDensitySample * ObjectFactory::produceFluidDensitySample (ProjectHandle * projectHandle, database::Record * record) const
{
   return new FluidDensitySample ( projectHandle, record );
}

RelatedProject * ObjectFactory::produceRelatedProject (ProjectHandle * projectHandle, database::Record * record) const
{
   return new RelatedProject ( projectHandle, record );
}

Trap * ObjectFactory::produceTrap (ProjectHandle * projectHandle, database::Record * record) const
{
   return new Trap (projectHandle, record);
}

Trapper * ObjectFactory::produceTrapper (ProjectHandle * projectHandle, database::Record * record) const
{
   return new Trapper (projectHandle, record);
}

Migration * ObjectFactory::produceMigration (ProjectHandle * projectHandle, database::Record * record) const
{
   return new Migration (projectHandle, record);
}

IgneousIntrusionEvent* ObjectFactory::produceIgneousIntrusionEvent (ProjectHandle * projectHandle, database::Record * record ) const
{
   return new IgneousIntrusionEvent ( projectHandle, record );
}

PermafrostEvent* ObjectFactory::producePermafrostEvent (ProjectHandle * projectHandle, database::Record * record ) const
{
   return new PermafrostEvent ( projectHandle, record );
}

InputValue * ObjectFactory::produceInputValue (ProjectHandle * projectHandle, database::Record * record) const
{
   return new InputValue (projectHandle, record);
}

Property * ObjectFactory::produceProperty (ProjectHandle * projectHandle, database::Record * record,
                                           const string & userName, const string & cauldronName,
                                           const string & unit, PropertyType type,
                                           const DataModel::PropertyAttribute attr, const DataModel::PropertyOutputAttribute outputAttr ) const
{
   return new Property (projectHandle, record,
                        userName, cauldronName,
                        unit, type, attr, outputAttr);
}

PropertyValue * ObjectFactory::producePropertyValue (ProjectHandle * projectHandle, database::Record * record ,const string & name,
      const Property * property, const Snapshot * snapshot,
      const Reservoir * reservoir, const Formation * formation, const Surface * surface, PropertyStorage storage, const std::string & fileName) const
{
	 return new PropertyValue (projectHandle, record, name,
	property, snapshot, reservoir,
	formation, surface, storage, fileName);
}

BiodegradationParameters* ObjectFactory::produceBiodegradationParameters (
   ProjectHandle * projectHandle, database::Record* bioRecord) const
{
   return new BiodegradationParameters(projectHandle, bioRecord);
}

FracturePressureFunctionParameters* ObjectFactory::produceFracturePressureFunctionParameters (
   ProjectHandle * projectHandle, database::Record* runOptionsIoTblRecord,
   database::Record* pressureIoTblRecord) const
{
   return new FracturePressureFunctionParameters(projectHandle, runOptionsIoTblRecord, pressureIoTblRecord);
}

FluidType* ObjectFactory::produceFluidType ( ProjectHandle * projectHandle, database::Record*
   fluidtypeIoRecord) const
{
   return new FluidType (projectHandle, fluidtypeIoRecord);
}

DiffusionLeakageParameters* ObjectFactory::produceDiffusionLeakageParameters (
   ProjectHandle * projectHandle, database::Record* bioRecord) const
{
   return new DiffusionLeakageParameters(projectHandle, bioRecord);
}

FaultCollection * ObjectFactory::produceFaultCollection (ProjectHandle * projectHandle, const string & mapName) const
{
   return new FaultCollection (projectHandle, mapName);
}


IrreducibleWaterSaturationSample* ObjectFactory::produceIrreducibleWaterSaturationSample (ProjectHandle * projectHandle, database::Record * record) const
{
   return new IrreducibleWaterSaturationSample ( projectHandle, record );
}

LangmuirAdsorptionIsothermSample* ObjectFactory::produceLangmuirAdsorptionIsothermSample (ProjectHandle * projectHandle, database::Record * record) const
{
   return new LangmuirAdsorptionIsothermSample ( projectHandle, record );
}

LangmuirAdsorptionTOCEntry* ObjectFactory::produceLangmuirAdsorptionTOCEntry (ProjectHandle * projectHandle, database::Record * record) const
{
   return new LangmuirAdsorptionTOCEntry ( projectHandle, record );
}

PointAdsorptionHistory* ObjectFactory::producePointAdsorptionHistory (ProjectHandle * projectHandle, database::Record * record) const
{
   return new PointAdsorptionHistory ( projectHandle, record );
}

SGDensitySample* ObjectFactory::produceSGDensitySample (ProjectHandle * projectHandle, database::Record * record) const
{
   return new SGDensitySample ( projectHandle, record );
}

shared_ptr<const CrustalThicknessData> ObjectFactory::produceCrustalThicknessData(ProjectHandle * projectHandle, database::Record * record) const
{
   return shared_ptr<const CrustalThicknessData>( new CrustalThicknessData( projectHandle, record ));
}

shared_ptr<const CrustalThicknessRiftingHistoryData> ObjectFactory::produceCrustalThicknessRiftingHistoryData(ProjectHandle * projectHandle, database::Record * record) const
{
   return shared_ptr<const CrustalThicknessRiftingHistoryData>(new CrustalThicknessRiftingHistoryData( projectHandle, record ));
}

shared_ptr<const OceanicCrustThicknessHistoryData> ObjectFactory::produceOceanicCrustThicknessHistoryData(ProjectHandle * projectHandle, database::Record * record) const
{
   return shared_ptr<const OceanicCrustThicknessHistoryData>( new OceanicCrustThicknessHistoryData( projectHandle, record ) );
}
