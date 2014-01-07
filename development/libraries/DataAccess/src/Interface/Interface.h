#ifndef _INTERFACE_H_
#define _INTERFACE_H_

#include <vector>
#include <string>
using namespace std;


// V.R. Ambati (25/07/2011)
// typedef's are not needed at the moment.
//typedef int PetscInt;
//typedef int PetscMPIInt;

namespace ddd
{
   /// \brief Rank of process.
   ///
   /// Will return 0 for serial programmes and the actual rank for mpi-distributed programmes.
   int GetRank (void);

   /// \brief Number of processes.
   ///
   /// Will return 1 for serial programmes and the actual size for mpi-distributed programmes.
   int GetSize (void);

   string & GetRankString (void);
}



/// The classes in this namespace together form the DataAccess library.
/// The purpose of this library is to provide third-party tools with access to Cauldron basin models and their simulation
/// results. The namespace contains two sub-namespaces.
/// The Interface namespace contains a number of abstract base classes with pure virtual functions only.
/// These classes provide the programming interface for the third-party tools.
/// The purpose of these classes is to form a semantic layer on top of the set of tables containing the data of a Cauldron basin model.
/// The Implementation namespace contains the classes that provide an implementation for the classes in the Interface namespace.
/// This library is a work in progress insofar that not all of the data in a Cauldron basin model is directly accessible yet.
namespace DataAccess
{
   /// Classes in this namespace contain the functions that can be used to access Cauldron Project Data.
   /// All the classes in this namespace contain pure virtual functions, only. They are comparable to Java Interfaces.
   /// These "Interfaces" are implemented by same-named classes in the Implementation namespace.
   namespace Interface
   {
      class AllochthonousLithology;
      class AllochthonousLithologyDistribution;
      class AllochthonousLithologyInterpolation;
      class AttributeValue;
      class BasementFormation;
      class BasementSurface;
      class ConstrainedOverpressureInterval;
      class CrustFormation;
      class Fault;
      class FaultCollection;
      class FluidDensitySample;
      class FluidHeatCapacitySample;
      class FluidThermalConductivitySample;
      class FluidType;
      class Formation;
      class Grid;
      class GridMap;
      class IgneousIntrusionEvent;
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
      class MobileLayer;
      class OutputProperty;
      class PaleoFormationProperty;
      class PaleoProperty;
      class PaleoSurfaceProperty;
      class PermafrostEvent;
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
      class TouchstoneMap;
      class Trap;
      class Trapper;
      class Migration;
      class CrustalThicknessData;


      /// list type for the snapshots
      typedef vector<const Snapshot *> SnapshotList;
      /// list type for the lithotypes
      typedef vector<const LithoType *> LithoTypeList;
      /// list type for the TouchstoneMaps
      typedef vector<const TouchstoneMap *> TouchstoneMapList;
      /// list type for the formations
      typedef vector<const Formation *> FormationList;
      /// list type for the surfaces
      typedef vector<const Surface *> SurfaceList;
      /// list type for the reservoirs
      typedef vector<const Reservoir *> ReservoirList;
      /// list type for the MobileLayers
      typedef vector<const MobileLayer *> MobileLayerList;
      /// list type for the PaleoProperties.
      typedef vector<const PaleoProperty *> PaleoPropertyList;
      /// list type for the PaleoFormationProperties.
      typedef vector<const PaleoFormationProperty *> PaleoFormationPropertyList;
      /// list type for the PaleoSurfaceProperties.
      typedef vector<const PaleoSurfaceProperty *> PaleoSurfacePropertyList;
      /// list type of FluidTypes.
      typedef vector<const FluidType*> FluidTypeList;
      /// list type of FaultCollection.
      typedef vector<const FaultCollection*> FaultCollectionList;
      /// list type for Faults
      typedef vector<const Fault *> FaultList;
      /// list type for the AllochthonousLithologies
      typedef vector<const Point *> PointList;
      /// list type for the AllochthonousLithologies
      typedef vector<const AllochthonousLithology *> AllochthonousLithologyList;
      /// list type for the AllochthonousLithologyDistributions
      typedef vector<const AllochthonousLithologyDistribution *> AllochthonousLithologyDistributionList;
      /// list type for the AllochthonousLithologyInterpolations
      typedef vector<const AllochthonousLithologyInterpolation *> AllochthonousLithologyInterpolationList;
      /// list type for the traps
      typedef vector<const Trap *> TrapList;
      /// list type for the trappers
      typedef vector<const Trapper *> TrapperList;
      /// list type for the Migrations
      typedef vector<const Migration *> MigrationList;
      /// list type for the Leads
      typedef vector<const Lead *> LeadList;
      /// list type for the LeadTraps
      typedef vector<const LeadTrap *> LeadTrapList;
      /// list type for the LeadTraps
      typedef vector<const LeadTrap *> LeadTrapList;
      /// list type for the (Mapped)InputValues
      typedef vector<const InputValue *> InputValueList;
      /// list type for the properties
      typedef vector<const Property *> PropertyList;
      /// list type for the property values
      typedef vector<const PropertyValue *> PropertyValueList;
      /// list type for the output properties.
      typedef vector<const OutputProperty *> OutputPropertyList;
      /// list type for the lithology heat-capacity samples.
      typedef vector<const LithologyHeatCapacitySample *> LithologyHeatCapacitySampleList;
      /// list type for the lithology thermal-conductivity samples.
      typedef vector<const LithologyThermalConductivitySample *> LithologyThermalConductivitySampleList;
      /// list type for the fluid density  samples.
      typedef vector<const FluidDensitySample *> FluidDensitySampleList;
      /// list type for the fluid thermal-conductivity samples.
      typedef vector<const FluidThermalConductivitySample *> FluidThermalConductivitySampleList;
      /// list type for the fluid heat-capacity samples.
      typedef vector<const FluidHeatCapacitySample *> FluidHeatCapacitySampleList;
      /// list type for the related-projects.
      typedef vector<const RelatedProject *> RelatedProjectList;
      /// list type for constrained overpressure intervals.
      typedef vector<const ConstrainedOverpressureInterval*> ConstrainedOverpressureIntervalList;

      const string ALCBasalt = "ALC Basalt";

      // Shale-gas specific classes.

      /// \typedef IrreducibleWaterSaturationSampleList
      /// \brief List of all irreducible-water-saturation samples.
      typedef vector<const IrreducibleWaterSaturationSample*> IrreducibleWaterSaturationSampleList;

      /// \typedef LangmuirAdsorptionIsothermSampleList
      /// \brief List of all Langmuir-adsorption-isotherm samples.
      typedef vector<const LangmuirAdsorptionIsothermSample*> LangmuirAdsorptionIsothermSampleList;

      /// \typedef LangmuirAdsorptionTOCEntryList
      /// \brief List of all Langmuir-adsorption-toc entries.
      typedef vector<const LangmuirAdsorptionTOCEntry*> LangmuirAdsorptionTOCEntryList;

      /// \typedef PointAdsorptionHistoryList
      /// \brief List of all adsorption-history points.
      typedef vector<const PointAdsorptionHistory*> PointAdsorptionHistoryList;

      /// \typedef SGDensitySampleList
      /// \brief List of all sg-density samples.
      typedef vector<const SGDensitySample*> SGDensitySampleList;


      /// Used to select Surface PropertyValues whose Properties are continuous across Formation boundaries
      /// Used to indicate 2D PropertyValues
      const int SURFACE		= 0x1;
      /// Used to select Formation PropertyValues
      const int FORMATION		= 0x2;
      /// Used to select Reservoir PropertyValues
      const int RESERVOIR		= 0x4;
      /// Used to select Surface PropertyValues whose Properties are discontinuous across Formation boundaries
      const int FORMATIONSURFACE	= 0x8;

      /// Used to indicate 2D PropertyValues
      const int MAP    = 0x1;
      /// Used to indicate 3D PropertyValues
      const int VOLUME = 0x2;

      /// Used to indicate Snapshots that are in the SnapshotIoTbl
      const int MAJOR = 0x1;
      const int MINOR = 0x2;

      enum InputValueType { PropertyMap, FaultMap };

      /// Ids for the gas and oil phase
      enum PhaseId {
	 Gas = 0, Oil, NumPhases
      };

      /// Ids for all the hydrocarbon components used.
	  enum ComponentId
		{
		  FIRST_COMPONENT = 0,
		  ASPHALTENES = 0,
		  RESINS = 1,
		  C15_ARO = 2,
		  C15_SAT = 3,
		  C6_14ARO = 4,
		  C6_14SAT = 5,
		  C5 = 6,
		  C4 = 7,
		  C3 = 8,
		  C2 = 9,
		  C1 = 10,
		  COX = 11,
		  N2 = 12,
		  H2S = 13,
		  LSC = 14,
		  C15_AT = 15,
		  C6_14BT = 16,
		  C6_14DBT = 17,
		  C6_14BP = 18,
		  C15_AROS = 19,
		  C15_SATS = 20,
		  C6_14SATS = 21,
		  C6_14AROS = 22,
		  LAST_COMPONENT = 22,
		  UNKNOWN = 23,
		  NUM_COMPONENTS = 23
      
		};


      /// Specifies whether a Property applies to Formations or Reservoirs
      enum PropertyType { FORMATIONPROPERTY, RESERVOIRPROPERTY, TRAPPROPERTY };

      /// Indicates the type of formation.
      enum FormationKind {
         SEDIMENT_FORMATION, /**< A formation indicated in the stratigraphy table. */
         BASEMENT_FORMATION  /**< A formation from the basement, not from the stratigraphy table. */
      };

      /// Indicates the type of surface. 
      enum SurfaceKind {
         SEDIMENT_SURFACE, /**< A surface indicated in the stratigraphy table. */
         BASEMENT_SURFACE  /**< A surface that is a part of the basement and not from the stratigraphy table. */
      };

      /// Enumeration datatype used in the definition of AttributeValues
      enum AttributeType {
         NoAttributeType = -1,
         Bool = 0, ///< For use with bool values.
         Int, ///< For use with int values.
         Long, ///< For use with long values.
         Float, ///< For use with float values.
         Double, ///< For use with double values.
         String ///< For use with string values.
      };

      /// Attributes for which a (GridMap) value can be requested via getMap ().
      enum SourceRockMapAttributeId {
         TocIni = 0, S1Ini, S2Ini, S3Ini, HcIni, OcIni, NcIni, CharLength, UpperBiot, LowerBiot,
         KerogenStartAct, KerogenEndAct, PreAsphaltStartAct, PreAsphaltEndAct, NettThickIni,
         NGenexTimeSteps, NGenexSlices
      };

      /// Attributes for which an AttributeValue be requested via getAttributeValue ().
      enum LithoTypeAttributeId {
         // Strings
         Lithotype = 0, Description, DefinedBy, DefinitionDate, LastChangedBy, LastChangedDate,
         FgColour, BgColour, Pixmap, HeatProdSource, PermMixModel, Porosity_Model,
         Multipoint_Porosity, Multipoint_Permeability,
         // Doubles
         Density, HeatProd, SurfacePorosity, CompacCoefES, CompacCoefSC, CompacCoefFM, StpThCond,
         ThCondAnisotropy, DepoPerm, PermDecrStressCoef, PermIncrRelaxCoef, PermAnisotropy,
         SeisVelocity, CapC1, CapC2, Compaction_Coefficient_SM, HydraulicFracturingPercent,
         ReferenceSolidViscosity, ActivationEnergy, MinimumPorosity,
         // Ints
         UserDefined, Number_Of_Data_Points
      };

      /// Attributes for which a (GridMap) value can be requested via getMap ().
      enum ReservoirMapAttributeId {
         DepthOffset = 0, ReservoirThickness, NetToGross, LayerFrequency
      } ;

      /// Attributes for which a (GridMap) value can be requested via getMap ().
      enum MobileLayerMapAttributeId {
         MobileLayerThickness = 0
      };

      /// Attributes for which a (GridMap) value can be requested via getMap ().
      enum PaleoPropertyMapAttributeId {
         MobileLayerThicknessMap,                    ///< Thickness of mobile layer.
         HeatFlowHistoryInstanceHeatFlowMap,         ///< Heat flow history.
         CrustThinningHistoryInstanceThicknessMap,   ///< Thickness of crust.
         MantleThicknessHistoryInstanceThicknessMap, ///< Thickness of mantle.
         SurfaceTemperatureHistoryInstanceMap,       ///< Temperature of top surface.
         SurfaceDepthHistoryInstanceMap//,             ///< Depth of top surface.
//          AllochthonousLithologyDistributionMap       ///< Distribution of allochthonous salt.
      };


      /// Attributes for which a (GridMap) value can be requested via getMap ().
      enum BottomBoundaryConditionsCrustHeatProductionAttributeId {
         // This value must be distinct from all other values used in Formations.
         CrustHeatProductionMap = 4,
         CrustThicknessMeltOnsetMap,
         BasaltThicknessMap
      };

      /// Attributes for which a (GridMap) value can be requested via getMap ().
      enum AllochthonousLithologyDistributionMapAttributeId {
         AllochthonousLithologyDistributionMap = 0
      };

      /// Attributes for which a (GridMap) value can be requested via getMap ().
      enum CTCMapAttributeId {
         T0Ini = 0, TRIni, HCuIni, HLMuIni, HBu, DeltaSL
      };

      /// Indicates the amount of output required for a particular property.
      enum PropertyOutputOption {
         NO_OUTPUT,                     ///< No output selected.
         SOURCE_ROCK_ONLY_OUTPUT,       ///< Output property only when formation is a source rock.
         SEDIMENTS_ONLY_OUTPUT,         ///< Output property only when formation is a sediment.
         SEDIMENTS_AND_BASEMENT_OUTPUT  ///< Output property when formation is a sediment or part of the basement.
      };

      /// Indicates which bottom boundary condition should be applied.
      enum BottomBoundaryConditions {
         MANTLE_HEAT_FLOW,               ///< Mantle heat flow boundary condition.
         FIXED_BASEMENT_TEMPERATURE,     ///< Fixed temperature boundary condition.
         ADVANCED_LITHOSPHERE_CALCULATOR ///< Initial lithosphere (mantle + crust) thickness
      };

      /// Used to distinguish between 1D/3D functionality.
      enum ModellingMode {MODE1D, MODE3D};

      /// Used to indicate the caldulation type of some property.
      ///
      /// E.g. The fluid density may be computed using all three types below.
      enum CalculationModel { CONSTANT_MODEL, CALCULATED_MODEL, TABLE_MODEL, STANDARD_MODEL, LOWCOND_MODEL, HIGHCOND_MODEL };

      /// The function describing the porosity.
      enum PorosityModel { EXPONENTIAL_POROSITY,
                           SOIL_MECHANICS_POROSITY };

      /// The model describing the permeability.
      enum PermeabilityModel { SANDSTONE_PERMEABILITY,
                               MUDSTONE_PERMEABILITY,
                               IMPERMEABLE_PERMEABILITY,
                               NONE_PERMEABILITY,
                               MULTIPOINT_PERMEABILITY };

      /// \brief The model used for the brine viscosity.
      enum ViscosityModel { BATTZLE_WANG_VISCOSITY, TEMIS_PACK_VISCOSITY };


      /// Enumeration of Pc - Kr models (capillary pressure, relative permeability models such as Brooks-Corey, Leverett..)
      enum PcKrModel {BROOKS_COREY_MODEL, TEMIS_PACK_MODEL};
    
      /// Enumeration of the fracture function type.
      enum FracturePressureFunctionType {
         ///< Do not apply any hydraulic fracturing.
         None,

         ///< Apply hydraulic fracturing, the function is defined w.r.t. depth from 0m.
         FunctionOfDepthWrtSeaLevelSurface,

         ///< Apply hydraulic fracturing, the function is defined w.r.t. depth from the top of the sediments.
         FunctionOfDepthWrtSedimentSurface,

         ///< Apply hydraulic fracturing, the function is defined as a fraction of the lithostatic-pressure
         ///< Pf = Ph + C * ( Pl - Ph )
         FunctionOfLithostaticPressure
      };

      /// Enumerations of the hydraulic fracture models.
      enum FracturePressureModel {
         ///< Totally non-conservative model.
         NON_CONSERVATIVE_TOTAL = 1,

         ///< Non-conservative model, but first attempt to restrict the pressure using the conservative method.
         NON_CONSERVATIVE_PARTIAL_1,

         ///< Non-conservative model, but first attempt (this time a bit harder)
         ///< to restrict the pressure using the conservative method.
         NON_CONSERVATIVE_PARTIAL_2,

         ///< Conservative model, try to restrict the pressure using the conservative method.
         CONSERVATIVE_1,

         ///< Conservative model, try (a bit harder) to restrict the pressure using the conservative method.
         CONSERVATIVE_2 };


      enum CoordinateAxis { X_COORD, Y_COORD };


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

      class InterpolateFunctor : public BinaryFunctor {

      public :
         InterpolateFunctor ( const double value1, const double value2, const double value3 ) :
            m_value1 ( value1 ), m_value2 ( value2 ), m_value3 ( value3 ) {};

         double operator ()( const double left, const double right ) const;

      private :

         const double m_value1;
         const double m_value2;
         const double m_value3;
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

      /// \brief Mutable array of ingneous-intrusion events.
      typedef vector<IgneousIntrusionEvent*> MutableIgneousIntrusionEventList;

      /// \brief Mutable array of permafrost events.
      typedef vector<PermafrostEvent*> MutablePermafrostEventList;

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
      enum PropertyStorage { SNAPSHOTIOTBL, THREEDTIMEIOTBL, TIMEIOTBL };

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

      /// \var IgneousIntrusionEventDuration
      /// \brief The time interval over which an igneous intrusion can be inflated.
      ///
      /// 1000 years.
      static const double IgneousIntrusionEventDuration = 0.001; // Ma

   }
}

inline double DataAccess::Interface::IdentityFunctor::operator ()( const double operand ) const {
   return operand;
}

inline double DataAccess::Interface::SubtractionFunctor::operator ()( const double op1, const double op2 ) const {
   return op1 - op2;
}

inline double DataAccess::Interface::InterpolateFunctor::operator ()( const double op1, const double op2 ) const {
   return op1 - (( op1 - op2 ) * ( m_value1 - m_value3 )) / ( m_value1 - m_value2 );
}

inline double DataAccess::Interface::AdditionFunctor::operator ()( const double op1, const double op2 ) const {
   return op1 + op2;
}

inline double DataAccess::Interface::SubtractConstant::operator ()( const double operand ) const {
   return operand - m_value;
}

inline double DataAccess::Interface::SubtractFromConstant::operator ()( const double operand ) const {
   return m_value - operand;
}

inline double DataAccess::Interface::AddConstant::operator ()( const double operand ) const {
   return operand + m_value;
}


#endif // _Interface_H_
