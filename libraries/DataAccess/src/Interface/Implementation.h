#ifndef _IMPLEMENTATION_H_
#define _IMPLEMENTATION_H_

#include <vector>
#include <string>
using namespace std;

typedef int PetscInt;
typedef int PetscMPIInt;

namespace ddd
{
   int GetRank (void);
   string & GetRankString (void);
}

#ifndef USEINTERFACE
#define INTERFACE Implementation
#define IMPLEMENTATION Implementation
#else
#define INTERFACE Interface
#define IMPLEMENTATION Implementation
#endif


namespace DataAccess
{
   namespace IMPLEMENTATION
   {
      /// operator type definition used in the computation of GridMaps from other GridMaps
      typedef double (*BinaryOperator) (double, double);
      /// operator type definition used in the computation of GridMaps from other GridMaps
      typedef double (*UnaryOperator) (double);

      class BinaryFunctor {
      public :

         virtual ~BinaryFunctor () {}

         virtual double operator ()( const double left, const double right ) const = 0;

      };

      class SubtractionFunctor : public BinaryFunctor {

      public :

         double operator ()( const double left, const double right ) const;

      };

      class AdditionFunctor : public BinaryFunctor {

      public :

         double operator ()( const double left, const double right ) const;

      };


      class UnaryFunctor {
      public :

         virtual ~UnaryFunctor () {}

         virtual double operator ()( const double operand ) const = 0;

      };


      class IdentityFunctor : public UnaryFunctor {
      public :

         double operator ()( const double operand ) const;

      private :

      };


      class SubtractConstant : public UnaryFunctor {
      public :

         SubtractConstant ( const double value ) : m_value ( value ) {};

         double operator ()( const double operand ) const;

      private :

         const double m_value;

      };

      class SubtractFromConstant : public UnaryFunctor {
      public :

         SubtractFromConstant ( const double value ) : m_value ( value ) {};

         double operator ()( const double operand ) const;

      private :

         const double m_value;

      };

      class AddConstant : public UnaryFunctor {
      public :

         AddConstant ( const double value ) : m_value ( value ) {};

         double operator ()( const double operand ) const;

      private :

         const double m_value;

      };

      class AllochthonousLithology;
      class AllochthonousLithologyDistribution;
      class AllochthonousLithologyInterpolation;
      class AttributeValue;
      class BasementSurface;
      class BiodegradationParameters;
      class ConstrainedOverpressureInterval;
      class CrustFormation;
      class DiffusionLeakageParameters;
      class Fault;
      class FaultCollection;
      class FluidDensitySample;
      class FluidHeatCapacitySample;
      class FluidThermalConductivitySample;
      class FluidType;
      class Formation;
      class FracturePressureFunctionParameters;
      class GlobalGrid;
      class Grid;
      class Grid;
      class GridMap;
      class GridMap;
      class InputValue;
      class IrreducibleWaterSaturationSample;
      class LangmuirAdsorptionIsothermSample;
      class LangmuirAdsorptionTOCEntry;
      class Lead;
      class LeadTrap;
      class LithoType;
      class LithologyHeatCapacitySample;
      class LithologyThermalConductivitySample;
      class MantleFormation;
      class MapWriter;
      class MobileLayer;
      class ObjectFactory;
      class OutputProperty;
      class PaleoFormationProperty;
      class PaleoProperty;
      class PaleoSurfaceProperty;
      class Parent;
      class Point;
      class PointAdsorptionHistory;
      class ProjectData;
      class ProjectHandle;
      class Property;
      class PropertyValue;
      class RelatedProject;
      class Reservoir;
      class RunParameters;
      class SGDensitySample;
      class Snapshot;
      class SourceRock;
      class Surface;
      class SurfaceDepthHistoryInstance;
      class SurfaceTemperatureHistoryInstance;
      class TouchstoneMap;
      class Trap;
      class Trapper;
      class Migration;
      class TrapPhase;
      class CrustalThicknessData;


      /*
      /// list type for FaultCollections
      typedef vector<FaultCollection *> FaultCollectionList;

      /// list type for Faults
      typedef vector<Fault *> FaultList;
      */

      /// mutable list type for the snapshots
      typedef vector<Snapshot *> MutableSnapshotList;
      /// mutable list type for the lithotypes
      typedef vector<LithoType *> MutableLithoTypeList;
      /// mutable list type for the SourceRocks
      typedef vector<SourceRock *> MutableSourceRockList;
      /// mutable list type for the TouchstoneMaps
      typedef vector<TouchstoneMap *> MutableTouchstoneMapList;
      /// mutable list type for the formations
      typedef vector<Formation *> MutableFormationList;
      /// mutable list type for the surfaces
      typedef vector<Surface *> MutableSurfaceList;
      /// mutable list type for the reservoirs
      typedef vector<Reservoir *> MutableReservoirList;
      /// mutable list type for the MobileLayers
      typedef vector<MobileLayer *> MutableMobileLayerList;
      ///  mutable list type for the FluidTypes.
      typedef vector<FluidType*> MutableFluidTypeList;
      /// mutable list type for the SurfaceTemperatureHistoryInstances.
      typedef vector<SurfaceTemperatureHistoryInstance *> MutableSurfaceTemperatureHistoryInstanceList;
      /// mutable list type for the SurfaceDepthHistoryInstances.
      typedef vector<SurfaceDepthHistoryInstance *> MutableSurfaceDepthHistoryInstanceList;
      /// mutable list type for the AllochthonousLithologies
      typedef vector<AllochthonousLithology *> MutableAllochthonousLithologyList;
      /// mutable list type for the AllochthonousLithologyDistributions
      typedef vector<AllochthonousLithologyDistribution *> MutableAllochthonousLithologyDistributionList;
      /// mutable list type for the AllochthonousLithologyInterpolations
      typedef vector<AllochthonousLithologyInterpolation *> MutableAllochthonousLithologyInterpolationList;
      /// mutable list type for the traps
      typedef vector<Trap *> MutableTrapList;
      /// mutable list type for the trappers
      typedef vector<Trapper *> MutableTrapperList;
      /// mutable list type for the Leads
      typedef vector<Migration *> MutableMigrationList;
      /// mutable list type for the Leads
      typedef vector<Lead *> MutableLeadList;
      /// mutable list type for the (Mapped)InputValues
      typedef vector<InputValue *> MutableInputValueList;
      /// mutable list type for the Properties
      typedef vector<Property *> MutablePropertyList;
      /// mutable list type for the PropertyValues
      typedef vector<PropertyValue *> MutablePropertyValueList;
      /// mutable list type for the output properties.
      typedef vector<OutputProperty *> MutableOutputPropertyList;
      /// mutable list type for the lithology heat-capacity samples.
      typedef vector<LithologyHeatCapacitySample *> MutableLithologyHeatCapacitySampleList;
      /// list type for the lithology thermal-conductivity samples.
      typedef vector<LithologyThermalConductivitySample *> MutableLithologyThermalConductivitySampleList;
      /// mutable list type for the fluid density  samples.
      typedef vector<FluidDensitySample *> MutableFluidDensitySampleList;
      /// mutable list type for the fluid thermal-conductivity samples.
      typedef vector<FluidThermalConductivitySample *> MutableFluidThermalConductivitySampleList;
      /// mutable list type for the fluid heat-capacity samples.
      typedef vector<FluidHeatCapacitySample *> MutableFluidHeatCapacitySampleList;
      /// list type for related-projects.
      typedef vector<RelatedProject *> MutableRelatedProjectList;
      /// list type for the PaleoProperties.
      typedef vector<PaleoProperty *> MutablePaleoPropertyList;
      /// list type for the PaleoFormationProperties.
      typedef vector<PaleoFormationProperty *> MutablePaleoFormationPropertyList;
      /// list type for the PaleoSurfaceProperties.
      typedef vector<PaleoSurfaceProperty *> MutablePaleoSurfacePropertyList;
      /// list type for constrained overpressure intervals.
      typedef vector<ConstrainedOverpressureInterval*> MutableConstrainedOverpressureIntervalList;

      /// mutable list type for FaultCollections
      typedef vector<FaultCollection *> MutableFaultCollectionList;

      /// mutable list type for the Crustal Thickness input data
      typedef vector< CrustalThicknessData *> MutableCrustalThicknessDataList;
	  
      /// mutable list type for the formations
      typedef vector< const Formation *> ConstFormationList;

      // Shale-gas specific classes.

      /// \typedef MutableIrreducibleWaterSaturationSampleList
      /// \brief Mutable list of all irreducible-water-saturation samples.
      typedef vector<IrreducibleWaterSaturationSample*> MutableIrreducibleWaterSaturationSampleList;

      /// \typedef MutableLangmuirAdsorptionIsothermSampleList
      /// \brief Mutable list of all Langmuir-adsorption-isotherm samples.
      typedef vector<LangmuirAdsorptionIsothermSample*> MutableLangmuirAdsorptionIsothermSampleList;

      /// \typedef MutableLangmuirAdsorptionTOCEntryList
      /// \brief Mutable list of all Langmuir-adsorption-toc entries.
      typedef vector<LangmuirAdsorptionTOCEntry*> MutableLangmuirAdsorptionTOCEntryList;

      /// \typedef MutablePointAdsorptionHistoryList
      /// \brief Mutable list of all adsorption-history points.
      typedef vector<PointAdsorptionHistory*> MutablePointAdsorptionHistoryList;

      /// \typedef MutableSGDensitySampleList
      /// \brief Mutable list of all sg-density samples.
      typedef vector<SGDensitySample*> MutableSGDensitySampleList;


      /// Value for undefined used in maps
      const double DefaultUndefinedMapValue = 99999;
      /// Value for undefined used in project files
      const double DefaultUndefinedScalarValue = -9999;
      const double RecordValueUndefined = DefaultUndefinedScalarValue;


      /// Must correspond to the FaultStatus in Migration
      typedef enum {
         NoFault = 0, Seal, Pass, Waste, SealOil, PassOil, NumFaults
      } FaultStatus;

      /// Used to specify whether a PropertyValue is stored in the TimeIoTbl (2D PropertyValues) or
      /// in the SnapshotIoTbl (3D PropertyValues).
      enum PropertyStorage { SNAPSHOTIOTBL, TIMEIOTBL };

      /// \var CrustBottomSurfaceName
      /// The name of the surface at the bottom of the crust.
      static const std::string CrustBottomSurfaceName  = "Bottom of Crust";

      /// \var MantleBottomSurfaceName
      /// The name of the surface at the bottom of the mantle.
      static const std::string MantleBottomSurfaceName = "Bottom of Lithospheric Mantle";

      /// \var CrustFormationName
      /// The name of the crust formation.
      static const std::string CrustFormationName = "Crust";

      /// \var CrustLithologyName
      /// The name of the lithology used in the crust formation.
      static const std::string CrustLithologyName = "Crust";

      /// \var MantleFormationName
      /// The name of the mantle formation.
      static const std::string MantleFormationName = "Mantle";

      /// \var MantleLithologyName
      /// The name of the lithology used in the mantle formation.
      static const std::string MantleLithologyName = "Litho. Mantle";


   }
}


inline double DataAccess::IMPLEMENTATION::SubtractionFunctor::operator ()( const double op1, const double op2 ) const {
   return op1 - op2;
}

inline double DataAccess::IMPLEMENTATION::AdditionFunctor::operator ()( const double op1, const double op2 ) const {
   return op1 + op2;
}

inline double DataAccess::IMPLEMENTATION::IdentityFunctor::operator ()( const double operand ) const {
   return operand;
}

inline double DataAccess::IMPLEMENTATION::SubtractConstant::operator ()( const double operand ) const {
   return operand - m_value;
}

inline double DataAccess::IMPLEMENTATION::SubtractFromConstant::operator ()( const double operand ) const {
   return m_value - operand;
}

inline double DataAccess::IMPLEMENTATION::AddConstant::operator ()( const double operand ) const {
   return operand + m_value;
}


#endif // _IMPLEMENTATION_H_
