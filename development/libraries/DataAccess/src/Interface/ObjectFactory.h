#ifndef _INTERFACE_OBJECTFACTORY_H_
#define _INTERFACE_OBJECTFACTORY_H_

#include <string>
// #include <vector>

// #include "hdf5.h"

#include "PropertyAttribute.h"

using namespace std;
#include "Interface/Interface.h"

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
	    virtual ProjectHandle * produceProjectHandle (database::Database * database, const string & name, const string & accessMode);

	    virtual Snapshot * produceSnapshot (ProjectHandle * projectHandle, database::Record * record);
	    virtual Snapshot * produceSnapshot (ProjectHandle * projectHandle, double time);

         virtual GridMap * produceGridMap (const Parent * owner, unsigned int childIndex, const Grid * grid, double undefinedValue, unsigned int depth, float *** values);
         virtual GridMap * produceGridMap (const Parent * owner, unsigned int childIndex, const Grid * grid, double value, unsigned int depth = 1);
         virtual GridMap * produceGridMap (const Parent * owner, unsigned int childIndex, const GridMap * operand1, const GridMap * operand2, BinaryOperator binaryOperator);
         virtual GridMap * produceGridMap ( const Parent * owner, unsigned int childIndex, const GridMap * operand, UnaryOperator unaryOperator);
         virtual GridMap * produceGridMap ( const Parent * owner, unsigned int childIndex, const GridMap * operand1, const GridMap * operand2, BinaryFunctor& binaryFunctor );
         virtual GridMap * produceGridMap (const Parent * owner, unsigned int childIndex, const GridMap * operand1, UnaryFunctor& unaryFunctor );

	    virtual Grid * produceGrid (double minI, double minJ, double maxI, double maxJ, unsigned int numI, unsigned int numJ, unsigned int lowResNumI, unsigned int lowResNumJ);
	    virtual Grid * produceGrid (const Grid * referenceGrid, double minI, double minJ, double maxI, double maxJ, unsigned int numI, unsigned int numJ);

	    virtual Formation * produceFormation (ProjectHandle * projectHandle, database::Record * record);

	    virtual LithoType * produceLithoType (ProjectHandle * projectHandle, database::Record * record);

	    virtual TouchstoneMap * produceTouchstoneMap (ProjectHandle * projectHandle, database::Record * record);

	    virtual SourceRock * produceSourceRock (ProjectHandle * projectHandle, database::Record * record);

	    virtual Surface * produceSurface (ProjectHandle * projectHandle, database::Record * record);

	    virtual BasementSurface * produceBasementSurface (ProjectHandle * projectHandle, const std::string& name );

	    virtual Reservoir * produceReservoir (ProjectHandle * projectHandle, database::Record * record);

	    virtual MobileLayer * produceMobileLayer (ProjectHandle * projectHandle, database::Record * record);

	    virtual PaleoProperty * producePaleoProperty (ProjectHandle * projectHandle, database::Record * record );

	    virtual PaleoFormationProperty * producePaleoFormationProperty (ProjectHandle * projectHandle, database::Record * record, const Formation* formation );

	    virtual PaleoFormationProperty * producePaleoFormationProperty ( ProjectHandle * projectHandle,
                                                                             const Formation* formation,
                                                                             const PaleoFormationProperty* startProperty,
                                                                             const PaleoFormationProperty* endProperty,
                                                                             const Snapshot*               interpolationTime );

	    virtual PaleoSurfaceProperty * producePaleoSurfaceProperty (ProjectHandle * projectHandle, database::Record * record, const Surface* surface );

            virtual CrustFormation * produceCrustFormation (ProjectHandle * projectHandle, database::Record * record);

            virtual MantleFormation * produceMantleFormation (ProjectHandle * projectHandle, database::Record * record);

            virtual RunParameters * produceRunParameters (ProjectHandle * projectHandle, database::Record * record);

            virtual ProjectData * produceProjectData (ProjectHandle * projectHandle, database::Record * record);

            virtual SimulationDetails* produceSimulationDetails ( ProjectHandle * projectHandle, database::Record * record );

	    virtual AllochthonousLithology * produceAllochthonousLithology (ProjectHandle * projectHandle, database::Record * record);

	    virtual AllochthonousLithologyDistribution * produceAllochthonousLithologyDistribution (ProjectHandle * projectHandle, database::Record * record);

	    virtual AllochthonousLithologyInterpolation * produceAllochthonousLithologyInterpolation (ProjectHandle * projectHandle, database::Record * record);

            virtual OutputProperty * produceOutputProperty (ProjectHandle * projectHandle, database::Record * record);

            virtual OutputProperty * produceOutputProperty (ProjectHandle * projectHandle, const ModellingMode mode, const PropertyOutputOption option, const std::string& name );

            virtual LithologyHeatCapacitySample * produceLithologyHeatCapacitySample (ProjectHandle * projectHandle, database::Record * record);

            virtual LithologyThermalConductivitySample * produceLithologyThermalConductivitySample (ProjectHandle * projectHandle, database::Record * record);

            virtual ConstrainedOverpressureInterval* produceConstrainedOverpressureInterval ( ProjectHandle * projectHandle, database::Record * record, const Formation* formation );

            virtual FluidHeatCapacitySample * produceFluidHeatCapacitySample (ProjectHandle * projectHandle, database::Record * record);

            virtual FluidThermalConductivitySample * produceFluidThermalConductivitySample (ProjectHandle * projectHandle, database::Record * record);

            virtual FluidDensitySample * produceFluidDensitySample (ProjectHandle * projectHandle, database::Record * record);


            virtual RelatedProject * produceRelatedProject (ProjectHandle * projectHandle, database::Record * record);

	    virtual Trap * produceTrap (ProjectHandle * projectHandle, database::Record * record);
	    virtual Trapper * produceTrapper (ProjectHandle * projectHandle, database::Record * record);
	    virtual Migration * produceMigration (ProjectHandle * projectHandle, database::Record * record);

	    virtual InputValue * produceInputValue (ProjectHandle * projectHandle, database::Record * record);

            /// \brief Function for allocating igneous-intrusion events.
            virtual IgneousIntrusionEvent* produceIgneousIntrusionEvent (ProjectHandle * projectHandle, database::Record * record );

            /// \brief Function for allocating permafrost events.
            virtual PermafrostEvent* producePermafrostEvent (ProjectHandle * projectHandle, database::Record * record );

	    virtual Property * produceProperty (ProjectHandle * projectHandle, database::Record * record,
                                                const string & userName, const string & cauldronName,
                                                const string & unit, PropertyType type,
                                                const DataModel::PropertyAttribute attr );

	    virtual PropertyValue * producePropertyValue (ProjectHandle * projectHandle, database::Record * record,
		  const string & name, const Property * property, const Snapshot * snapshot,
		  const Reservoir * reservoir, const Formation * formation, const Surface * surface, PropertyStorage storage);

	    virtual BiodegradationParameters* produceBiodegradationParameters (ProjectHandle * projectHandle, 
               database::Record* bioRecord);

	    virtual DiffusionLeakageParameters* produceDiffusionLeakageParameters (ProjectHandle * projectHandle, 
               database::Record* diffusionLeakageRecord);

            virtual FracturePressureFunctionParameters* produceFracturePressureFunctionParameters (
               ProjectHandle * projectHandle, database::Record* runOptionsIoTblRecord, database::Record* pressureIoTblRecord);

            virtual FluidType* produceFluidType ( ProjectHandle * projectHandle, database::Record* 
               fluidtypeIoRecord);

	    virtual FaultCollection * produceFaultCollection (ProjectHandle * projectHandle, const string & mapName);
	    // virtual Fault * produceFault (string & name);


            virtual IrreducibleWaterSaturationSample* produceIrreducibleWaterSaturationSample (ProjectHandle * projectHandle, database::Record * record);

            virtual LangmuirAdsorptionIsothermSample* produceLangmuirAdsorptionIsothermSample (ProjectHandle * projectHandle, database::Record * record);

            virtual LangmuirAdsorptionTOCEntry* produceLangmuirAdsorptionTOCEntry (ProjectHandle * projectHandle, database::Record * record);

            virtual PointAdsorptionHistory* producePointAdsorptionHistory (ProjectHandle * projectHandle, database::Record * record);

            virtual SGDensitySample* produceSGDensitySample (ProjectHandle * projectHandle, database::Record * record);
            
			virtual MapWriter* produceMapWriter (void);
			
            virtual CrustalThicknessData* produceCrustalThicknessData (ProjectHandle * projectHandle, database::Record * record);


      };
   }
}

#endif // _IMPLEMENTATION_OBJECTFACTORY_H_
