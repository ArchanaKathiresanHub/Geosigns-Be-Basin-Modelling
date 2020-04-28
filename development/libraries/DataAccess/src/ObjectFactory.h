//
// Copyright (C) 2015-2016 Shell International Exploration & Production.
// All rights reserved.
//
// Developed under license for Shell by PDS BV.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//
#ifndef INTERFACE_OBJECTFACTORY_H
#define INTERFACE_OBJECTFACTORY_H

// std library
#include <string>
#include <memory>
using namespace std;


#include <ProjectFileHandler.h>
#include "PropertyAttribute.h"

#include "Interface.h"

namespace database
{
   class Database;
   class Transaction;
   class Table;
   class Record;
}

namespace DataAccess
{

   namespace Interface
   {
      /// The ObjectFactory Class is responsible for producing all objects in the DataAccess library.
      /// This class can be subclassed to produce application-defined subclasses of the classes fort which
      /// factory methods have been defined below.
      /// For a description of the methods below see the respective classes.
      class ObjectFactory
      {
      public:

         ObjectFactory() {}
         virtual ~ObjectFactory () {}

         virtual ProjectHandle * produceProjectHandle (database::ProjectFileHandlerPtr pfh, const string & name) const;

         virtual Snapshot * produceSnapshot (ProjectHandle& projectHandle, database::Record * record) const;
         virtual Snapshot * produceSnapshot (ProjectHandle& projectHandle, double time) const;

         virtual GridMap * produceGridMap (const Parent * owner, unsigned int childIndex, const Grid * grid, double undefinedValue, unsigned int depth, float *** values) const;
         virtual GridMap * produceGridMap (const Parent * owner, unsigned int childIndex, const Grid * grid, double value, unsigned int depth = 1) const;
         virtual GridMap * produceGridMap (const Parent * owner, unsigned int childIndex, const GridMap * operand1, const GridMap * operand2, BinaryOperator binaryOperator) const;
         virtual GridMap * produceGridMap ( const Parent * owner, unsigned int childIndex, const GridMap * operand, UnaryOperator unaryOperator) const;
         virtual GridMap * produceGridMap ( const Parent * owner, unsigned int childIndex, const GridMap * operand1, const GridMap * operand2, BinaryFunctor& binaryFunctor ) const;
         virtual GridMap * produceGridMap (const Parent * owner, unsigned int childIndex, const GridMap * operand1, UnaryFunctor& unaryFunctor ) const;

         virtual Grid * produceGrid (double minI, double minJ, double maxI, double maxJ, unsigned int numI, unsigned int numJ, unsigned int lowResNumI, unsigned int lowResNumJ) const;
         virtual Grid * produceGrid (const Grid * referenceGrid, double minI, double minJ, double maxI, double maxJ, unsigned int numI, unsigned int numJ) const;

         virtual Formation * produceFormation (ProjectHandle& projectHandle, database::Record * record) const;

         virtual LithoType * produceLithoType (ProjectHandle& projectHandle, database::Record * record) const;

         virtual SourceRock * produceSourceRock (ProjectHandle& projectHandle, database::Record * record) const;

         virtual Surface * produceSurface (ProjectHandle& projectHandle, database::Record * record) const;

         virtual BasementSurface * produceBasementSurface (ProjectHandle& projectHandle, const std::string& name ) const;

         virtual Reservoir * produceReservoir (ProjectHandle& projectHandle, database::Record * record) const;

         virtual std::shared_ptr<ReservoirOptions> produceReservoirOptions (ProjectHandle& projectHandle, database::Record * record) const;

         virtual MobileLayer * produceMobileLayer (ProjectHandle& projectHandle, database::Record * record) const;

         virtual PaleoProperty * producePaleoProperty (ProjectHandle& projectHandle, database::Record * record ) const;

         virtual PaleoFormationProperty * producePaleoFormationProperty (ProjectHandle& projectHandle, database::Record * record, const Formation* formation ) const;

         virtual PaleoFormationProperty * producePaleoFormationProperty ( ProjectHandle& projectHandle,
                                                                          const Formation* formation,
                                                                          const PaleoFormationProperty* startProperty,
                                                                          const PaleoFormationProperty* endProperty,
                                                                          const Snapshot*               interpolationTime ) const;

         virtual PaleoSurfaceProperty * producePaleoSurfaceProperty (ProjectHandle& projectHandle, database::Record * record, const Surface* surface ) const;

         virtual CrustFormation * produceCrustFormation (ProjectHandle& projectHandle, database::Record * record) const;

         virtual MantleFormation * produceMantleFormation (ProjectHandle& projectHandle, database::Record * record) const;

         virtual RunParameters * produceRunParameters (ProjectHandle& projectHandle, database::Record * record) const;

         virtual ProjectData * produceProjectData (ProjectHandle& projectHandle, database::Record * record) const;

         virtual SimulationDetails* produceSimulationDetails (ProjectHandle& projectHandle, database::Record * record) const;

         virtual AllochthonousLithology * produceAllochthonousLithology (ProjectHandle& projectHandle, database::Record * record) const;

         virtual AllochthonousLithologyDistribution * produceAllochthonousLithologyDistribution (ProjectHandle& projectHandle, database::Record * record) const;

         virtual AllochthonousLithologyInterpolation * produceAllochthonousLithologyInterpolation (ProjectHandle& projectHandle, database::Record * record) const;

         virtual OutputProperty * produceOutputProperty (ProjectHandle& projectHandle, database::Record * record) const;

         virtual OutputProperty * produceOutputProperty (ProjectHandle& projectHandle, const ModellingMode mode, const PropertyOutputOption option, const std::string& name ) const;

         virtual LithologyHeatCapacitySample * produceLithologyHeatCapacitySample (ProjectHandle& projectHandle, database::Record * record) const;

         virtual LithologyThermalConductivitySample * produceLithologyThermalConductivitySample (ProjectHandle& projectHandle, database::Record * record) const;

         virtual ConstrainedOverpressureInterval* produceConstrainedOverpressureInterval ( ProjectHandle& projectHandle, database::Record * record, const Formation* formation ) const;

         virtual FluidHeatCapacitySample * produceFluidHeatCapacitySample (ProjectHandle& projectHandle, database::Record * record) const;

         virtual FluidThermalConductivitySample * produceFluidThermalConductivitySample (ProjectHandle& projectHandle, database::Record * record) const;

         virtual FluidDensitySample * produceFluidDensitySample (ProjectHandle& projectHandle, database::Record * record) const;

         virtual RelatedProject * produceRelatedProject (ProjectHandle& projectHandle, database::Record * record) const;

         virtual Trap * produceTrap (ProjectHandle& projectHandle, database::Record * record) const;
         virtual Trapper * produceTrapper (ProjectHandle& projectHandle, database::Record * record) const;
         virtual Migration * produceMigration (ProjectHandle& projectHandle, database::Record * record) const;

         virtual InputValue * produceInputValue (ProjectHandle& projectHandle, database::Record * record) const;

         /// \brief Function for allocating igneous-intrusion events.
         virtual IgneousIntrusionEvent* produceIgneousIntrusionEvent (ProjectHandle& projectHandle, database::Record * record) const;

         /// \brief Function for allocating permafrost events.
         virtual PermafrostEvent* producePermafrostEvent (ProjectHandle& projectHandle, database::Record * record) const;

         virtual Property * produceProperty (ProjectHandle& projectHandle, database::Record * record,
                                             const string & userName, const string & cauldronName,
                                             const string & unit, PropertyType type,
                                             const DataModel::PropertyAttribute attr,
                                             const DataModel::PropertyOutputAttribute outputAttr) const;

         virtual PropertyValue * producePropertyValue (ProjectHandle& projectHandle, database::Record * record,
                                                       const string & name, const Property * property, const Snapshot * snapshot, const Reservoir * reservoir,
                                                       const Formation * formation, const Surface * surface, PropertyStorage storage, const std::string & fileName = "") const;

         virtual BiodegradationParameters* produceBiodegradationParameters (ProjectHandle& projectHandle,
                                                                            database::Record* bioRecord) const;

         virtual DiffusionLeakageParameters* produceDiffusionLeakageParameters (ProjectHandle& projectHandle,
                                                                                database::Record* diffusionLeakageRecord) const;

         virtual FracturePressureFunctionParameters* produceFracturePressureFunctionParameters (ProjectHandle& projectHandle,
                                                                                                database::Record* runOptionsIoTblRecord,
                                                                                                database::Record* pressureIoTblRecord) const;

         virtual FluidType* produceFluidType ( ProjectHandle& projectHandle, database::Record* fluidtypeIoRecord) const;

         virtual FaultCollection * produceFaultCollection (ProjectHandle& projectHandle, const string & mapName) const;
         // virtual Fault * produceFault (string & name);

         virtual IrreducibleWaterSaturationSample* produceIrreducibleWaterSaturationSample (ProjectHandle& projectHandle, database::Record * record) const;

         virtual LangmuirAdsorptionIsothermSample* produceLangmuirAdsorptionIsothermSample (ProjectHandle& projectHandle, database::Record * record) const;

         virtual LangmuirAdsorptionTOCEntry* produceLangmuirAdsorptionTOCEntry (ProjectHandle& projectHandle, database::Record * record) const;

         virtual PointAdsorptionHistory* producePointAdsorptionHistory (ProjectHandle& projectHandle, database::Record * record) const;

         virtual SGDensitySample* produceSGDensitySample (ProjectHandle& projectHandle, database::Record * record) const;
         virtual MapWriter* produceMapWriter (void) const;

         /// @defgroup FASTCTC
         /// @{
         /// @brief Produces the CTC data accessor for the CTCIoTbl record
         shared_ptr<const CrustalThicknessData> produceCrustalThicknessData( ProjectHandle& projectHandle, database::Record * record ) const;
         /// @brief Produces the CTC data accessor for the CTCRiftingHistoryIoTbl record
         shared_ptr<const CrustalThicknessRiftingHistoryData> produceCrustalThicknessRiftingHistoryData( ProjectHandle& projectHandle, database::Record * record ) const;
         /// @}

         /// @defgroup ALC
         /// @{
         /// @brief Produces the ALC data accessor for the OceaCrustalThicknessIoTbl record
         shared_ptr<const OceanicCrustThicknessHistoryData> produceOceanicCrustThicknessHistoryData( ProjectHandle& projectHandle, database::Record * record ) const;
         /// @}
      };
   }
}

#endif // INTERFACE_OBJECTFACTORY_H
