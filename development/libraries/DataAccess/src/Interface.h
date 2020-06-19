//
// Copyright (C) 2016-2018 Shell International Exploration & Production.
// All rights reserved.
//
// Developed under license for Shell by PDS BV.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#ifndef INTERFACE_H
#define INTERFACE_H

#include <map>
#include <memory>
#include <string>
#include <vector>


namespace ddd
{
   /// \brief Rank of process.
   ///
   /// Will return 0 for serial programmes and the actual rank for mpi-distributed programmes.
   int GetRank( void );

   /// \brief Number of processes.
   ///
   /// Will return 1 for serial programmes and the actual size for mpi-distributed programmes.
   int GetSize( void );
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
      class GridMap;
      class IgneousIntrusionEvent;
      class InputValue;
      class IrreducibleWaterSaturationSample;
      class LangmuirAdsorptionIsothermSample;
      class LangmuirAdsorptionTOCEntry;
      class LithoType;
      class LithologyHeatCapacitySample;
      class LithologyThermalConductivitySample;
      class MapWriter;
      class MantleFormation;
      class MobileLayer;
      class ObjectFactory;
      class OutputProperty;
      class PaleoFormationProperty;
      class PaleoProperty;
      class PaleoSurfaceProperty;
      class Parent;
      class PermafrostEvent;
      class Point;
      class PointAdsorptionHistory;
      class ProjectData;
      class ProjectHandle;
      class Property;
      class PropertyValue;
      class RelatedProject;
      class Reservoir;
      class ReservoirOptions;
      class RunParameters;
      class SGDensitySample;
      class SimulationDetails;
      class Snapshot;
      class SourceRock;
      class Surface;
      class SurfaceDepthHistoryInstance;
      class SurfaceTemperatureHistoryInstance;
      class Trap;
      class Trapper;
      class Migration;


      // Data classes, each of them corresponding to a complete IoTbl
      class CrustalThicknessData;               ///< Data from [CTCIoTbl                 ]
      class CrustalThicknessRiftingHistoryData; ///< Data from [CTCRiftingHistoryIoTbl   ]
      class OceanicCrustThicknessHistoryData;   ///< Data from [OceaCrustalThicknessIoTbl]



      /// list type for the snapshots
      typedef std::vector<const Snapshot *> SnapshotList;
      /// list type for the lithotypes
      typedef std::vector<const LithoType *> LithoTypeList;
      /// list type for the formations
      typedef std::vector<const Formation *> FormationList;
      /// list type for the surfaces
      typedef std::vector<const Surface *> SurfaceList;
      /// list type for the reservoirs
      typedef std::vector<const Reservoir *> ReservoirList;
      /// list type for the MobileLayers
      typedef std::vector<const MobileLayer *> MobileLayerList;
      /// list type for the PaleoProperties.
      typedef std::vector<const PaleoProperty *> PaleoPropertyList;
      /// list type for the PaleoFormationProperties.
      typedef std::vector<const PaleoFormationProperty *> PaleoFormationPropertyList;
      /// list type for the PaleoSurfaceProperties.
      typedef std::vector<const PaleoSurfaceProperty *> PaleoSurfacePropertyList;
      /// list type of FluidTypes.
      typedef std::vector<const FluidType*> FluidTypeList;
      /// list type of FaultCollection.
      typedef std::vector<const FaultCollection*> FaultCollectionList;
      /// list type for Faults
      typedef std::vector<const Fault *> FaultList;
      /// list type for the AllochthonousLithologies
      typedef std::vector<const Point *> PointList;
      /// list type for the AllochthonousLithologies
      typedef std::vector<const AllochthonousLithology *> AllochthonousLithologyList;
      /// list type for the AllochthonousLithologyDistributions
      typedef std::vector<const AllochthonousLithologyDistribution *> AllochthonousLithologyDistributionList;
      /// list type for the AllochthonousLithologyInterpolations
      typedef std::vector<const AllochthonousLithologyInterpolation *> AllochthonousLithologyInterpolationList;
      /// list type for the traps
      typedef std::vector<const Trap *> TrapList;
      /// list type for the trappers
      typedef std::vector<const Trapper *> TrapperList;
      /// list type for the Migrations
      typedef std::vector<const Migration *> MigrationList;
      /// list type for the (Mapped)InputValues
      typedef std::vector<const InputValue *> InputValueList;
      /// list type for the properties
      typedef std::vector<const Property *> PropertyList;
      /// list type for the property values
      typedef std::vector<const PropertyValue *> PropertyValueList;
      /// map type for the properties and property values
      typedef std::map<const Property*, PropertyValueList> PropertyPropertyValueListMap;
      /// list type for the output properties.
      typedef std::vector<const OutputProperty *> OutputPropertyList;
      /// list type for the lithology heat-capacity samples.
      typedef std::vector<const LithologyHeatCapacitySample *> LithologyHeatCapacitySampleList;
      /// list type for the lithology thermal-conductivity samples.
      typedef std::vector<const LithologyThermalConductivitySample *> LithologyThermalConductivitySampleList;
      /// list type for the fluid density  samples.
      typedef std::vector<const FluidDensitySample *> FluidDensitySampleList;
      /// list type for the fluid thermal-conductivity samples.
      typedef std::vector<const FluidThermalConductivitySample *> FluidThermalConductivitySampleList;
      /// list type for the fluid heat-capacity samples.
      typedef std::vector<const FluidHeatCapacitySample *> FluidHeatCapacitySampleList;
      /// list type for the related-projects.
      typedef std::vector<const RelatedProject *> RelatedProjectList;
      /// list type for constrained overpressure intervals.
      typedef std::vector<const ConstrainedOverpressureInterval*> ConstrainedOverpressureIntervalList;

      /// list type for the SimulationDetails
      typedef std::vector<const SimulationDetails*> SimulationDetailsList;

      /// \typedef SimulationDetailsListPtr
      /// \brief Smart pointer to std::vector of simulation-details.
      typedef std::shared_ptr<SimulationDetailsList> SimulationDetailsListPtr;

      /// \typedef PropertyListPtr
      /// \brief Smart pointer to std::vector of properties.
      typedef std::shared_ptr<PropertyList> PropertyListPtr;


      const std::string ALCBasalt = "ALC Basalt";

      // Shale-gas specific classes.

      /// \typedef IrreducibleWaterSaturationSampleList
      /// \brief List of all irreducible-water-saturation samples.
      typedef std::vector<const IrreducibleWaterSaturationSample*> IrreducibleWaterSaturationSampleList;

      /// \typedef LangmuirAdsorptionIsothermSampleList
      /// \brief List of all Langmuir-adsorption-isotherm samples.
      typedef std::vector<const LangmuirAdsorptionIsothermSample*> LangmuirAdsorptionIsothermSampleList;

      /// \typedef LangmuirAdsorptionTOCEntryList
      /// \brief List of all Langmuir-adsorption-toc entries.
      typedef std::vector<const LangmuirAdsorptionTOCEntry*> LangmuirAdsorptionTOCEntryList;

      /// \typedef PointAdsorptionHistoryList
      /// \brief List of all adsorption-history points.
      typedef std::vector<const PointAdsorptionHistory*> PointAdsorptionHistoryList;

      /// \typedef SGDensitySampleList
      /// \brief List of all sg-density samples.
      typedef std::vector<const SGDensitySample*> SGDensitySampleList;


      /// Used to select Surface PropertyValues whose Properties are continuous across Formation boundaries
      /// Used to indicate 2D PropertyValues
      const int SURFACE = 0x1;
      /// Used to select Formation PropertyValues
      const int FORMATION = 0x2;
      /// Used to select Reservoir PropertyValues
      const int RESERVOIR = 0x4;
      /// Used to select Surface PropertyValues whose Properties are discontinuous across Formation boundaries
      const int FORMATIONSURFACE = 0x8;

      /// Used to indicate 2D PropertyValues
      const int MAP = 0x1;
      /// Used to indicate 3D PropertyValues
      const int VOLUME = 0x2;

      /// Used to indicate Snapshots that are in the SnapshotIoTbl
      const int MAJOR = 0x1;
      const int MINOR = 0x2;

      enum InputValueType { PropertyMap, FaultMap };

      /// \brief Indicates the model method of mixing the lithologies in a layer.
      enum MixModelType { UNDEFINED, HOMOGENEOUS, LAYERED };

      /// Specifies whether a Property applies to formations, reservoirs or traps.
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
         Density, HeatProd, SurfacePorosity, CompacCoefES,CompacCoefESA, CompacCoefESB, StpThCond,
         ThCondAnisotropy, DepoPerm, PermDecrStressCoef, PermIncrRelaxCoef, PermAnisotropy,
         SeisVelocity, NExponentVelocity, CapC1, CapC2, Compaction_Coefficient_SM, HydraulicFracturingPercent,
         ReferenceSolidViscosity, ActivationEnergy, MinimumPorosity, CompacRatioES,
         // Ints
         UserDefined, Number_Of_Data_Points
      };

      /// Attributes for which a (GridMap) value can be requested via getMap ().
      enum ReservoirMapAttributeId {
         NetToGross = 0
      };

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
         SurfaceDepthHistoryInstanceMap              ///< Depth of top surface.
      };


      /// Attributes for which a (GridMap) value can be requested via getMap ().
      enum BottomBoundaryConditionsCrustHeatProductionAttributeId {
         // This value must be distinct from all other values used in Formations.
         CrustHeatProductionMap = 6,
         CrustThicknessMeltOnsetMap, ///< Used for upgrading legacy ALC projects only
         BasaltThicknessMap          ///< Used for upgrading legacy ALC projects only
      };

      /// Attributes for which a (GridMap) value can be requested via getMap ().
      enum AllochthonousLithologyDistributionMapAttributeId {
         AllochthonousLithologyDistributionMap = 0
      };

      /// @brief Attributes for which a (GridMap) value can be requested via getMap ()
      /// @details To be requested from CrustalThicknessData object
      enum CTCMapAttributeId {
         HCuIni, ///< The initial continental crust thickness map         [m]
         HLMuIni ///< The initial lithospheric mantle crust thickness map [m]
      };

      /// @brief Attributes for which a (GridMap) value can be requested via getMap ()
      /// @details To be requested from CrustalThicknessRiftingHistoryData object
      enum CTCRiftingHistoryMapAttributeId {
         HBu,     ///< The maximum oceanic crustal thickness map [m]
         DeltaSL  ///< The sea level adjustment map              [m]
      };

      /// @brief Indicates what is the tectonic contexct at a specific snapshot
      enum TectonicFlag {
         ACTIVE_RIFTING, ///< Syn rift deposition
         PASSIVE_MARGIN, ///< Post rift deposition
         FLEXURAL_BASIN  ///< Subduction (subsidence is no more driven by rifting)
      };

      /// Indicates the amount of output required for a particular property.
      enum PropertyOutputOption {
         NO_OUTPUT,                     ///< No output selected.
         SOURCE_ROCK_ONLY_OUTPUT,       ///< Output property only when formation is a source rock.
         SHALE_GAS_ONLY_OUTPUT,         ///< Output property only when formation is a source rock and adsorption is on.
         SEDIMENTS_ONLY_OUTPUT,         ///< Output property only when formation is a sediment.
         SEDIMENTS_AND_BASEMENT_OUTPUT  ///< Output property when formation is a sediment or part of the basement.
      };

      /// Indicates which bottom boundary condition should be applied.
      enum BottomBoundaryConditions {
         MANTLE_HEAT_FLOW,                 ///< Mantle heat flow boundary condition.
         FIXED_BASEMENT_TEMPERATURE,       ///< Fixed temperature boundary condition.
         ADVANCED_LITHOSPHERE_CALCULATOR,  ///< Initial lithosphere (mantle + crust) thickness
         IMPROVED_LITHOSPHERE_CALCULATOR_LINEAR_ELEMENT_MODE ///< Initial lithosphere (mantle + crust) thickness, linear element mode
      };

      /// Used to distinguish between 1D/3D functionality.
      enum ModellingMode { MODE1D, MODE3D };

      /// Used to indicate the caldulation type of some property.
      ///
      /// E.g. The fluid density may be computed using all three types below.
      enum CalculationModel { CONSTANT_MODEL, CALCULATED_MODEL, TABLE_MODEL, STANDARD_MODEL, LOWCOND_MODEL, HIGHCOND_MODEL };

      /// The function describing the porosity.
      enum PorosityModel {
         EXPONENTIAL_POROSITY,
         SOIL_MECHANICS_POROSITY,
         DOUBLE_EXPONENTIAL_POROSITY
      };

      /// The model describing the permeability.
      enum PermeabilityModel {
         SANDSTONE_PERMEABILITY,
         MUDSTONE_PERMEABILITY,
         IMPERMEABLE_PERMEABILITY,
         NONE_PERMEABILITY,
         MULTIPOINT_PERMEABILITY
      };

      /// \brief The model used for the brine viscosity.
      enum ViscosityModel { BATTZLE_WANG_VISCOSITY, TEMIS_PACK_VISCOSITY };


      /// Enumeration of Pc - Kr models (capillary pressure, relative permeability models such as Brooks-Corey, Leverett..)
      enum PcKrModel { BROOKS_COREY };

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
         CONSERVATIVE_2
      };

		/// Indicates which seismic velocity algorithm should be used.
		enum SeismicVelocityModel {
			GARDNERS_VELOCITY_ALGORITHM,
			KENNANS_VELOCITY_ALGORITHM,
			WYLLIES_VELOCITY_ALGORITHM
		};

			enum CoordinateAxis { X_COORD, Y_COORD, Z_COORD };

      /// operator type definition used in the computation of GridMaps from other GridMaps
      typedef double( *BinaryOperator ) ( double, double );
      /// operator type definition used in the computation of GridMaps from other GridMaps
      typedef double( *UnaryOperator ) ( double );

      class BinaryFunctor {
      public:

         virtual ~BinaryFunctor() {}

         virtual double operator ()( const double left, const double right ) const = 0;

      };

      class SubtractionFunctor : public BinaryFunctor {

      public:

         double operator ()( const double left, const double right ) const;

      };

      class AdditionFunctor : public BinaryFunctor {

      public:

         double operator ()( const double left, const double right ) const;

      };

      class InterpolateFunctor : public BinaryFunctor {

      public:
         InterpolateFunctor( const double value1, const double value2, const double value3 ) :
            m_value1( value1 ), m_value2( value2 ), m_value3( value3 ) {}

         double operator ()( const double left, const double right ) const;

      private:

         const double m_value1;
         const double m_value2;
         const double m_value3;
      };


      class UnaryFunctor {
      public:

         virtual ~UnaryFunctor() {}

         virtual double operator ()( const double operand ) const = 0;

      };


      class IdentityFunctor : public UnaryFunctor {
      public:

         double operator ()( const double operand ) const;

      };

      class IdentityMinusFunctor : public UnaryFunctor {
      public:

         double operator ()( const double operand ) const;

      };

      class SubtractConstant : public UnaryFunctor {
      public:

         explicit SubtractConstant( const double value ) : m_value( value ) {}

         double operator ()( const double operand ) const;

      private:

         const double m_value;

      };

      class SubtractFromConstant : public UnaryFunctor {
      public:

         explicit SubtractFromConstant( const double value ) : m_value( value ) {}

         double operator ()( const double operand ) const;

      private:

         const double m_value;

      };

      class AddConstant : public UnaryFunctor {
      public:

         explicit AddConstant( const double value ) : m_value( value ) {}

         double operator ()( const double operand ) const;

      private:

         const double m_value;

      };


      /// mutable list type for the snapshots
      typedef std::vector<Snapshot *> MutableSnapshotList;
      /// mutable list type for the lithotypes
      typedef std::vector<LithoType *> MutableLithoTypeList;
      /// mutable list type for the SourceRocks
      typedef std::vector<SourceRock *> MutableSourceRockList;
      /// mutable list type for the formations
      typedef std::vector<Formation *> MutableFormationList;
      /// mutable list type for the surfaces
      typedef std::vector<Surface *> MutableSurfaceList;
      /// mutable list type for the reservoirs
      typedef std::vector<Reservoir *> MutableReservoirList;
      /// mutable list type for the MobileLayers
      typedef std::vector<MobileLayer *> MutableMobileLayerList;
      ///  mutable list type for the FluidTypes.
      typedef std::vector<FluidType*> MutableFluidTypeList;
      /// mutable list type for the SurfaceTemperatureHistoryInstances.
      typedef std::vector<SurfaceTemperatureHistoryInstance *> MutableSurfaceTemperatureHistoryInstanceList;
      /// mutable list type for the SurfaceDepthHistoryInstances.
      typedef std::vector<SurfaceDepthHistoryInstance *> MutableSurfaceDepthHistoryInstanceList;
      /// mutable list type for the AllochthonousLithologies
      typedef std::vector<AllochthonousLithology *> MutableAllochthonousLithologyList;
      /// mutable list type for the AllochthonousLithologyDistributions
      typedef std::vector<AllochthonousLithologyDistribution *> MutableAllochthonousLithologyDistributionList;
      /// mutable list type for the AllochthonousLithologyInterpolations
      typedef std::vector<AllochthonousLithologyInterpolation *> MutableAllochthonousLithologyInterpolationList;
      /// mutable list type for the traps
      typedef std::vector<Trap *> MutableTrapList;
      /// mutable list type for the trappers
      typedef std::vector<Trapper *> MutableTrapperList;
      /// mutable list type for the Migrations
      typedef std::vector<Migration *> MutableMigrationList;
      /// mutable list type for the (Mapped)InputValues
      typedef std::vector<InputValue *> MutableInputValueList;
      /// mutable list type for the Properties
      typedef std::vector<Property *> MutablePropertyList;
      /// mutable list type for the PropertyValues
      typedef std::vector<PropertyValue *> MutablePropertyValueList;
      /// mutable list type for the output properties.
      typedef std::vector<OutputProperty *> MutableOutputPropertyList;
      /// mutable list type for the lithology heat-capacity samples.
      typedef std::vector<LithologyHeatCapacitySample *> MutableLithologyHeatCapacitySampleList;
      /// list type for the lithology thermal-conductivity samples.
      typedef std::vector<LithologyThermalConductivitySample *> MutableLithologyThermalConductivitySampleList;
      /// mutable list type for the fluid density  samples.
      typedef std::vector<FluidDensitySample *> MutableFluidDensitySampleList;
      /// mutable list type for the fluid thermal-conductivity samples.
      typedef std::vector<FluidThermalConductivitySample *> MutableFluidThermalConductivitySampleList;
      /// mutable list type for the fluid heat-capacity samples.
      typedef std::vector<FluidHeatCapacitySample *> MutableFluidHeatCapacitySampleList;
      /// list type for related-projects.
      typedef std::vector<RelatedProject *> MutableRelatedProjectList;
      /// list type for the PaleoProperties.
      typedef std::vector<PaleoProperty *> MutablePaleoPropertyList;
      /// list type for the PaleoFormationProperties.
      typedef std::vector<PaleoFormationProperty *> MutablePaleoFormationPropertyList;
      /// list type for the PaleoSurfaceProperties.
      typedef std::vector<PaleoSurfaceProperty *> MutablePaleoSurfacePropertyList;
      /// list type for constrained overpressure intervals.
      typedef std::vector<ConstrainedOverpressureInterval*> MutableConstrainedOverpressureIntervalList;

      /// mutable list type for FaultCollections
      typedef std::vector<FaultCollection *> MutableFaultCollectionList;

      /// mutable list type for the Crustal Thickness input data
      typedef std::vector< CrustalThicknessData *> MutableCrustalThicknessDataList;

      /// \brief Mutable array of ingneous-intrusion events.
      typedef std::vector<IgneousIntrusionEvent*> MutableIgneousIntrusionEventList;

      /// \brief Mutable array of permafrost events.
      typedef std::vector<PermafrostEvent*> MutablePermafrostEventList;

      // Shale-gas specific classes.

      /// \typedef MutableIrreducibleWaterSaturationSampleList
      /// \brief Mutable list of all irreducible-water-saturation samples.
      typedef std::vector<IrreducibleWaterSaturationSample*> MutableIrreducibleWaterSaturationSampleList;

      /// \typedef MutableLangmuirAdsorptionIsothermSampleList
      /// \brief Mutable list of all Langmuir-adsorption-isotherm samples.
      typedef std::vector<LangmuirAdsorptionIsothermSample*> MutableLangmuirAdsorptionIsothermSampleList;

      /// \typedef MutableLangmuirAdsorptionTOCEntryList
      /// \brief Mutable list of all Langmuir-adsorption-toc entries.
      typedef std::vector<LangmuirAdsorptionTOCEntry*> MutableLangmuirAdsorptionTOCEntryList;

      /// \typedef MutablePointAdsorptionHistoryList
      /// \brief Mutable list of all adsorption-history points.
      typedef std::vector<PointAdsorptionHistory*> MutablePointAdsorptionHistoryList;

      /// \typedef MutableSGDensitySampleList
      /// \brief Mutable list of all sg-density samples.
      typedef std::vector<SGDensitySample*> MutableSGDensitySampleList;


      /// list type for the SimulationDetails
      typedef std::vector<SimulationDetails*> MutableSimulationDetailsList;

      /// Value for undefined used in maps
      const double DefaultUndefinedMapValue = 99999;
      /// Value for undefined used in maps
      const int DefaultUndefinedMapValueInteger = 99999;
      /// Value for undefined used in project files
      const double DefaultUndefinedScalarValue = -9999;
      const double RecordValueUndefined = DefaultUndefinedScalarValue;
      const int    DefaultUndefinedScalarIntValue = -9999;


      /// Must correspond to the FaultStatus in Migration
      typedef enum {
         NoFault = 0, Seal, Pass, Waste, SealOil, PassOil, NumFaults
      } FaultStatus;

      /// Used to specify whether a PropertyValue is stored in the TimeIoTbl (2D PropertyValues) or
      /// in the SnapshotIoTbl (3D PropertyValues).
      enum PropertyStorage { SNAPSHOTIOTBL, THREEDTIMEIOTBL, TIMEIOTBL };

      /// \var CrustBottomSurfaceName
      /// The name of the surface at the bottom of the crust.
      static const std::string CrustBottomSurfaceName = "Bottom of Crust";

      /// \var MantleBottomSurfaceName
      /// The name of the surface at the bottom of the mantle.
      static const std::string MantleBottomSurfaceName = "Bottom of Lithospheric Mantle";

      /// \var CrustFormationName
      /// The name of the crust formation.
      static const std::string CrustFormationName = "Crust";

      /// \var MantleFormationName
      /// The name of the mantle formation.
      static const std::string MantleFormationName = "Mantle";

      /// \var NullString
      static const std::string NullString = "";

      /// \brief A vector of zero length.
      static const std::vector<std::string> NoTableNames = std::vector<std::string>();

      /// \brief A vector of names of the largest output tables in the project file.
      ///
      /// These are the candidates for being moved to the output table file.
      static const std::vector<std::string> DefaultOutputTableNames =
                            std::vector<std::string>({ "1DTimeIoTbl",    "3DTimeIoTbl",
                                                       "MigrationIoTbl", "TimeIoTbl",
                                                       "TrapIoTbl",      "TrapperIoTbl" });

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

inline double DataAccess::Interface::IdentityMinusFunctor::operator ()( const double operand ) const {
   return -operand;
}

inline double DataAccess::Interface::SubtractionFunctor::operator ()( const double op1, const double op2 ) const {
   return op1 - op2;
}

inline double DataAccess::Interface::InterpolateFunctor::operator ()( const double op1, const double op2 ) const {
   return op1 - ( ( op1 - op2 ) * ( m_value1 - m_value3 ) ) / ( m_value1 - m_value2 );
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


#endif // INTERFACE_H
