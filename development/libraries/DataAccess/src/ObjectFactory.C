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

#include "Interface/AllochthonousLithology.h"
#include "Interface/AllochthonousLithologyDistribution.h"
#include "Interface/AllochthonousLithologyInterpolation.h"
#include "Interface/BasementSurface.h"
#include "Interface/BiodegradationParameters.h"
#include "Interface/ConstrainedOverpressureInterval.h"
#include "Interface/CrustFormation.h"
#include "Interface/DiffusionLeakageParameters.h"
#include "Interface/FaultCollection.h"
#include "Interface/FluidDensitySample.h"
#include "Interface/FluidHeatCapacitySample.h"
#include "Interface/FluidType.h"
#include "Interface/FluidThermalConductivitySample.h"
#include "Interface/Formation.h"
#include "Interface/FracturePressureFunctionParameters.h"
#include "Interface/Grid.h"
#include "Interface/GridMap.h"
#include "Interface/IgneousIntrusionEvent.h"
#include "Interface/InputValue.h"
#include "Interface/IrreducibleWaterSaturationSample.h"
#include "Interface/LangmuirAdsorptionIsothermSample.h"
#include "Interface/LangmuirAdsorptionTOCEntry.h"
#include "Interface/LithoType.h"
#include "Interface/LithologyHeatCapacitySample.h"
#include "Interface/LithologyThermalConductivitySample.h"
#include "Interface/MantleFormation.h"
#include "Interface/MobileLayer.h"
#include "Interface/ObjectFactory.h"
#include "Interface/OutputProperty.h"
#include "Interface/PaleoFormationProperty.h"
#include "Interface/PaleoProperty.h"
#include "Interface/PaleoSurfaceProperty.h"
#include "Interface/PermafrostEvent.h"
#include "Interface/PointAdsorptionHistory.h"
#include "Interface/ProjectData.h"
#include "Interface/ProjectHandle.h"
#include "Interface/Property.h"
#include "Interface/PropertyValue.h"
#include "Interface/RelatedProject.h"
#include "Interface/Reservoir.h"
#include "Interface/ReservoirOptions.h"
#include "Interface/RunParameters.h"
#include "Interface/SGDensitySample.h"
#include "Interface/SimulationDetails.h"
#include "Interface/Snapshot.h"
#include "Interface/SourceRock.h"
#include "Interface/Surface.h"
#include "Interface/TouchstoneMap.h"
#include "Interface/Trap.h"
#include "Interface/Trapper.h"
#include "Interface/Migration.h"

#include "Interface/CrustalThicknessData.h"
#include "Interface/CrustalThicknessRiftingHistoryData.h"
#include "Interface/OceanicCrustThicknessHistoryData.h"

#include "ProjectFileHandler.h"

using namespace DataAccess;
using namespace Interface;

ProjectHandle * ObjectFactory::produceProjectHandle (database::ProjectFileHandlerPtr pfh, const string & name, const string & accessMode)
{
   return new ProjectHandle ( pfh, name, accessMode, this);
}

Snapshot * ObjectFactory::produceSnapshot (ProjectHandle * projectHandle, database::Record * record)
{
   return new Snapshot (projectHandle, record);
}

Snapshot * ObjectFactory::produceSnapshot (ProjectHandle * projectHandle, double time)
{
   return new Snapshot (projectHandle, time);
}

Formation * ObjectFactory::produceFormation (ProjectHandle * projectHandle, database::Record * record)
{
   return new Formation (projectHandle, record);
}

LithoType * ObjectFactory::produceLithoType (ProjectHandle * projectHandle, database::Record * record)
{
   return new LithoType (projectHandle, record);
}

SourceRock * ObjectFactory::produceSourceRock (ProjectHandle * projectHandle, database::Record * record)
{
   return new SourceRock (projectHandle, record);
}

TouchstoneMap * ObjectFactory::produceTouchstoneMap (ProjectHandle * projectHandle, database::Record * record)
{
   return new TouchstoneMap (projectHandle, record);
}

Surface * ObjectFactory::produceSurface (ProjectHandle * projectHandle, database::Record * record)
{
   return new Surface (projectHandle, record);
}

BasementSurface * ObjectFactory::produceBasementSurface (ProjectHandle * projectHandle, const std::string& name )
{
   return new BasementSurface (projectHandle, name);
}

Reservoir * ObjectFactory::produceReservoir (ProjectHandle * projectHandle, database::Record * record)
{
   return new Reservoir (projectHandle, record);
}

std::shared_ptr<ReservoirOptions> ObjectFactory::produceReservoirOptions (ProjectHandle * projectHandle, database::Record * record)
{
   return std::make_shared<ReservoirOptions> (projectHandle, record);
}

MobileLayer * ObjectFactory::produceMobileLayer (ProjectHandle * projectHandle, database::Record * record)
{
   return new MobileLayer (projectHandle, record);
}

PaleoProperty * ObjectFactory::producePaleoProperty (ProjectHandle * projectHandle, database::Record * record )
{
   return new PaleoProperty (projectHandle, record);
}

#if 0
PaleoFormationProperty * ObjectFactory::producePaleoFormationProperty (ProjectHandle * projectHandle, database::Record * record, const std::string& formationName )
{
   return new PaleoFormationProperty (projectHandle, record, formationName);
}
#endif

PaleoFormationProperty * ObjectFactory::producePaleoFormationProperty (ProjectHandle * projectHandle, database::Record * record, const Formation* formation )
{
   return new PaleoFormationProperty (projectHandle, record, formation);
}

PaleoFormationProperty* ObjectFactory::producePaleoFormationProperty ( ProjectHandle * projectHandle,
                                                                       const Formation* formation,
                                                                       const PaleoFormationProperty* startProperty,
                                                                       const PaleoFormationProperty* endProperty,
                                                                       const Snapshot*               interpolationTime ) {
   return new PaleoFormationProperty (projectHandle, formation, startProperty, endProperty, interpolationTime );
}


PaleoSurfaceProperty * ObjectFactory::producePaleoSurfaceProperty (ProjectHandle * projectHandle, database::Record * record, const Surface* surface )
{
   return new PaleoSurfaceProperty (projectHandle, record, surface);
}

CrustFormation * ObjectFactory::produceCrustFormation (ProjectHandle * projectHandle, database::Record * record) {
   return new CrustFormation ( projectHandle, record );
}

MantleFormation * ObjectFactory::produceMantleFormation (ProjectHandle * projectHandle, database::Record * record) {
   return new MantleFormation ( projectHandle, record );
}

RunParameters * ObjectFactory::produceRunParameters (ProjectHandle * projectHandle, database::Record * record)
{
   return new RunParameters ( projectHandle, record );
}

ProjectData * ObjectFactory::produceProjectData (ProjectHandle * projectHandle, database::Record * record)
{
   return new ProjectData ( projectHandle, record );
}

SimulationDetails* ObjectFactory::produceSimulationDetails ( ProjectHandle * projectHandle, database::Record * record ) {
   return new SimulationDetails ( projectHandle, record );
}


AllochthonousLithology * ObjectFactory::produceAllochthonousLithology (ProjectHandle * projectHandle, database::Record * record)
{
  return new AllochthonousLithology (projectHandle, record);
}

AllochthonousLithologyDistribution * ObjectFactory::produceAllochthonousLithologyDistribution (ProjectHandle * projectHandle, database::Record * record)
{
  return new AllochthonousLithologyDistribution (projectHandle, record);
}

AllochthonousLithologyInterpolation * ObjectFactory::produceAllochthonousLithologyInterpolation (ProjectHandle * projectHandle, database::Record * record)
{
  return new AllochthonousLithologyInterpolation (projectHandle, record);
}

OutputProperty * ObjectFactory::produceOutputProperty (ProjectHandle * projectHandle, database::Record * record)
{
   return new OutputProperty ( projectHandle, record );
}

OutputProperty * ObjectFactory::produceOutputProperty (ProjectHandle * projectHandle, const ModellingMode mode, const PropertyOutputOption option, const std::string& name ) {
   return new OutputProperty ( projectHandle, mode, option, name );
}


LithologyHeatCapacitySample * ObjectFactory::produceLithologyHeatCapacitySample (ProjectHandle * projectHandle, database::Record * record)
{
   return new LithologyHeatCapacitySample ( projectHandle, record );
}

LithologyThermalConductivitySample * ObjectFactory::produceLithologyThermalConductivitySample (ProjectHandle * projectHandle, database::Record * record)
{
   return new LithologyThermalConductivitySample ( projectHandle, record );
}

ConstrainedOverpressureInterval* ObjectFactory::produceConstrainedOverpressureInterval ( ProjectHandle * projectHandle, database::Record * record, const Formation* formation ) {
   return new ConstrainedOverpressureInterval ( projectHandle, record, formation );
}

FluidHeatCapacitySample * ObjectFactory::produceFluidHeatCapacitySample (ProjectHandle * projectHandle, database::Record * record)
{
   return new FluidHeatCapacitySample ( projectHandle, record );
}

FluidThermalConductivitySample * ObjectFactory::produceFluidThermalConductivitySample (ProjectHandle * projectHandle, database::Record * record)
{
   return new FluidThermalConductivitySample ( projectHandle, record );
}

FluidDensitySample * ObjectFactory::produceFluidDensitySample (ProjectHandle * projectHandle, database::Record * record)
{
   return new FluidDensitySample ( projectHandle, record );
}

RelatedProject * ObjectFactory::produceRelatedProject (ProjectHandle * projectHandle, database::Record * record)
{
   return new RelatedProject ( projectHandle, record );
}

Trap * ObjectFactory::produceTrap (ProjectHandle * projectHandle, database::Record * record)
{
   return new Trap (projectHandle, record);
}

Trapper * ObjectFactory::produceTrapper (ProjectHandle * projectHandle, database::Record * record)
{
   return new Trapper (projectHandle, record);
}

Migration * ObjectFactory::produceMigration (ProjectHandle * projectHandle, database::Record * record)
{
   return new Migration (projectHandle, record);
}

IgneousIntrusionEvent* ObjectFactory::produceIgneousIntrusionEvent (ProjectHandle * projectHandle, database::Record * record )
{
   return new IgneousIntrusionEvent ( projectHandle, record );
}

PermafrostEvent* ObjectFactory::producePermafrostEvent (ProjectHandle * projectHandle, database::Record * record )
{
   return new PermafrostEvent ( projectHandle, record );
}

InputValue * ObjectFactory::produceInputValue (ProjectHandle * projectHandle, database::Record * record)
{
   return new InputValue (projectHandle, record);
}

Property * ObjectFactory::produceProperty (ProjectHandle * projectHandle, database::Record * record,
                                           const string & userName, const string & cauldronName,
                                           const string & unit, PropertyType type,
                                           const DataModel::PropertyAttribute attr, const DataModel::PropertyOutputAttribute outputAttr )
{
   return new Property (projectHandle, record,
                        userName, cauldronName,
                        unit, type, attr, outputAttr);
}

PropertyValue * ObjectFactory::producePropertyValue (ProjectHandle * projectHandle, database::Record * record ,const string & name,
      const Property * property, const Snapshot * snapshot,
      const Reservoir * reservoir, const Formation * formation, const Surface * surface, PropertyStorage storage, const std::string & fileName)
{
   return new PropertyValue (projectHandle, record, name,
	property, snapshot, reservoir,
	formation, surface, storage, fileName);
}

BiodegradationParameters* ObjectFactory::produceBiodegradationParameters (
   ProjectHandle * projectHandle, database::Record* bioRecord)
{
   return new BiodegradationParameters(projectHandle, bioRecord);
}

FracturePressureFunctionParameters* ObjectFactory::produceFracturePressureFunctionParameters (
   ProjectHandle * projectHandle, database::Record* runOptionsIoTblRecord,
   database::Record* pressureIoTblRecord)
{
   return new FracturePressureFunctionParameters(projectHandle, runOptionsIoTblRecord, pressureIoTblRecord);
}

FluidType* ObjectFactory::produceFluidType ( ProjectHandle * projectHandle, database::Record*
   fluidtypeIoRecord)
{
   return new FluidType (projectHandle, fluidtypeIoRecord);
}

DiffusionLeakageParameters* ObjectFactory::produceDiffusionLeakageParameters (
   ProjectHandle * projectHandle, database::Record* bioRecord)
{
   return new DiffusionLeakageParameters(projectHandle, bioRecord);
}

FaultCollection * ObjectFactory::produceFaultCollection (ProjectHandle * projectHandle, const string & mapName)
{
   return new FaultCollection (projectHandle, mapName);
}


IrreducibleWaterSaturationSample* ObjectFactory::produceIrreducibleWaterSaturationSample (ProjectHandle * projectHandle, database::Record * record) {
   return new IrreducibleWaterSaturationSample ( projectHandle, record );
}

LangmuirAdsorptionIsothermSample* ObjectFactory::produceLangmuirAdsorptionIsothermSample (ProjectHandle * projectHandle, database::Record * record) {
   return new LangmuirAdsorptionIsothermSample ( projectHandle, record );
}

LangmuirAdsorptionTOCEntry* ObjectFactory::produceLangmuirAdsorptionTOCEntry (ProjectHandle * projectHandle, database::Record * record) {
   return new LangmuirAdsorptionTOCEntry ( projectHandle, record );
}

PointAdsorptionHistory* ObjectFactory::producePointAdsorptionHistory (ProjectHandle * projectHandle, database::Record * record) {
   return new PointAdsorptionHistory ( projectHandle, record );
}

SGDensitySample* ObjectFactory::produceSGDensitySample (ProjectHandle * projectHandle, database::Record * record) {
   return new SGDensitySample ( projectHandle, record );
}

shared_ptr<const CrustalThicknessData> ObjectFactory::produceCrustalThicknessData( ProjectHandle * projectHandle, database::Record * record ) const {
   return shared_ptr<const CrustalThicknessData>( new CrustalThicknessData( projectHandle, record ));
}

shared_ptr<const CrustalThicknessRiftingHistoryData> ObjectFactory::produceCrustalThicknessRiftingHistoryData( ProjectHandle * projectHandle, database::Record * record ) const {
   return shared_ptr<const CrustalThicknessRiftingHistoryData>(new CrustalThicknessRiftingHistoryData( projectHandle, record ));
}

shared_ptr<const OceanicCrustThicknessHistoryData> ObjectFactory::produceOceanicCrustThicknessHistoryData( ProjectHandle * projectHandle, database::Record * record ) const {
   return shared_ptr<const OceanicCrustThicknessHistoryData>( new OceanicCrustThicknessHistoryData( projectHandle, record ) );
}
