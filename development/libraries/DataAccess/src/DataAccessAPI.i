/* File : DotNetAPI.i */

%module DataAccessAPI

%{
// Interface for CBMGenerics library
#include "../../CBMGenerics/src/ComponentManager.h"
using namespace CBMGenerics;

// Interface to DataModel
#include "../../DataModel/src/PropertyAttribute.h"
#include "../../DataModel/src/AbstractCompoundLithology.h"
#include "../../DataModel/src/AbstractCompoundLithologyArray.h"
#include "../../DataModel/src/AbstractFormation.h"
#include "../../DataModel/src/AbstractSurface.h"
#include "../../DataModel/src/AbstractGrid.h"
#include "../../DataModel/src/AbstractProperty.h"
#include "../../DataModel/src/AbstractSnapshot.h"
#include "../../DataModel/src/AbstractValidator.h"

#include "Interface.h"
#include "daobject.h"

#include "AllochthonousLithology.h"
#include "AllochthonousLithologyDistribution.h"
#include "AllochthonousLithologyInterpolation.h"
#include "ApplicationGlobalOperations.h"
#include "AttributeValue.h"
#include "auxiliaryfaulttypes.h"
#include "BasementFormation.h"
#include "BasementSurface.h"
#include "bioconsts.h"
#include "biodegradationparameters.h"
#include "child.h"
#include "ConstrainedOverpressureInterval.h"
#include "CrustFormation.h"
#include "CrustalThicknessData.h"
#include "ContinentalCrustHistoryGenerator.h"
#include "DiffusionLeakageParameters.h"
#include "DynArray.h"
#include "FaultCollection.h"
#include "faultelementcalculator.h"
#include "faultfilereader.h"
#include "faultfilereaderfactory.h"
#include "Faulting.h"
#include "FluidDensitySample.h"
#include "FluidHeatCapacitySample.h"
#include "FluidThermalConductivitySample.h"
#include "FluidType.h"
#include "Formation.h"
#include "fracturepressurefunctionparameters.h"
#include "Grid.h"
#include "GridMap.h"
#include "hdf5funcs.h"
#include "ibsfaultfilereader.h"
#include "InputValue.h"
#include "Interface.h"
#include "IrreducibleWaterSaturationSample.h"
#include "landmarkfaultfilereader.h"
#include "LangmuirAdsorptionIsothermSample.h"
#include "LangmuirAdsorptionTOCEntry.h"
#include "LithologyHeatCapacitySample.h"
#include "LithologyThermalConductivitySample.h"
#include "LithoType.h"
#include "Local2DArray.h"
#include "MantleFormation.h"
#include "mapwriter.h"
#include "MessageHandler.h"
#include "Migration.h"
#include "MobileLayer.h"
#include "Objectfactory.h"
#include "OutputProperty.h"
#include "PaleoFormationProperty.h"
#include "PaleoProperty.h"
#include "PaleoSurfaceProperty.h"
#include "parent.h"
#include "PointAdsorptionHistory.h"
#include "ProjectData.h"
#include "Validator.h"
#include "ProjectHandle.h"
#include "Property.h"
#include "PropertyValue.h"
#include "RelatedProject.h"
#include "Reservoir.h"
#include "RunParameters.h"
#include "SGDensitySample.h"
#include "Snapshot.h"
#include "SourceRock.h"
#include "Surface.h"
#include "TableOceanicCrustThicknessHistory.h"
using namespace DataAccess::Interface;
#include "Trap.h"
#include "Trapper.h"
#include "zycorfaultfilereader.h"
#include "IgneousIntrusionEvent.h"

// Interface for SerialDataAccess library
#include "../../SerialDataAccess/src/SerialApplicationGlobalOperations.h"
#include "../../SerialDataAccess/src/SerialGrid.h"
#include "../../SerialDataAccess/src/SerialGridMap.h"
#include "../../SerialDataAccess/src/SerialMapWriter.h"
#include "../../SerialDataAccess/src/SerialMessageHandler.h"

// Interface for APIs library
#include "../../FileSystem/src/Path.h"
#include "../../FileSystem/src/FilePath.h"
#include "../../FileSystem/src/FolderPath.h"

%}

%include "../../swig-common/StdHelper.i"
%include "enums.swg"

%include <typemaps.i>
%include <arrays_csharp.i>

// HDF5::writeData2D
%apply bool & INOUT { bool & newDataset };
%apply int { hid_t };

%rename(PhaseId2) pvtFlash::PhaseId;

// Interface for CBMGenerics library
%include "../../CBMGenerics/src/ComponentManager.h"

// Interface to DataModel
%include "../../DataModel/src/PropertyAttribute.h"
%include "../../DataModel/src/AbstractCompoundLithology.h"
%include "../../DataModel/src/AbstractCompoundLithologyArray.h"
%include "../../DataModel/src/AbstractFormation.h"
%include "../../DataModel/src/AbstractSurface.h"
%include "../../DataModel/src/AbstractGrid.h"
%include "../../DataModel/src/AbstractProperty.h"
%include "../../DataModel/src/AbstractSnapshot.h"

%include "Interface.h"
%include "parent.h"
%include "daobject.h"
%include "child.h"
%include "auxiliaryfaulttypes.h"
%include "bioconsts.h"
%include "biodegradationparameters.h"
%include "Formation.h"
%include "Surface.h"
%include "PaleoProperty.h"
%include "PaleoFormationProperty.h"
%include "AllochthonousLithology.h"
%include "AllochthonousLithologyDistribution.h"
%include "AllochthonousLithologyInterpolation.h"
%include "ApplicationGlobalOperations.h"
%include "AttributeValue.h"
%include "BasementFormation.h"
%include "BasementSurface.h"
%include "ConstrainedOverpressureInterval.h"
%include "ContinentalCrustHistoryGenerator.h"
%include "CrustFormation.h"
%include "CrustalThicknessData.h"
%include "DiffusionLeakageParameters.h"
%include "DynArray.h"
%include "FaultCollection.h"
%include "faultelementcalculator.h"
%include "faultfilereader.h"
%include "faultfilereaderfactory.h"
%include "Faulting.h"
%include "FluidDensitySample.h"
%include "FluidHeatCapacitySample.h"
%include "FluidThermalConductivitySample.h"
%include "FluidType.h"
%include "fracturepressurefunctionparameters.h"
%include "Grid.h"
%include "GridMap.h"
%include "hdf5funcs.h"
%include "ibsfaultfilereader.h"
%include "InputValue.h"
%include "Interface.h"
%include "IrreducibleWaterSaturationSample.h"
%include "landmarkfaultfilereader.h"
%include "LangmuirAdsorptionIsothermSample.h"
%include "LangmuirAdsorptionTOCEntry.h"
%include "LithologyHeatCapacitySample.h"
%include "LithologyThermalConductivitySample.h"
%include "LithoType.h"
%include "Local2DArray.h"
%include "MantleFormation.h"
%include "mapwriter.h"
%include "MessageHandler.h"
%include "Migration.h"
%include "MobileLayer.h"
%include "Objectfactory.h"
%include "OutputProperty.h"
%include "PaleoSurfaceProperty.h"
%include "PointAdsorptionHistory.h"
%include "ProjectData.h"
%include "Validator.h"
%include "ProjectHandle.h"
%include "Property.h"
%include "PropertyValue.h"
%include "reconmodelfaultfilereader.h"
%include "RelatedProject.h"
%include "Reservoir.h"
%include "RunParameters.h"
%include "SGDensitySample.h"
%include "Snapshot.h"
%include "SourceRock.h"
%include "TableOceanicCrustThicknessHistory.h"
%include "Trap.h"
%include "Trapper.h"
%include "IgneousIntrusionEvent.h"
%include "zycorfaultfilereader.h"

// Interface for SerialDataAccess library
%include "../../SerialDataAccess/src/SerialApplicationGlobalOperations.h"
%include "../../SerialDataAccess/src/SerialGrid.h"
%include "../../SerialDataAccess/src/SerialGridMap.h"
%include "../../SerialDataAccess/src/SerialMapWriter.h"
%include "../../SerialDataAccess/src/SerialMessageHandler.h"

%include "../../utilities/src/FormattingException.h"

%include "../../FileSystem/src/Path.h"
%include "../../FileSystem/src/FilePath.h"
%include "../../FileSystem/src/FolderPath.h"

%template(SnapshotList)                std::vector<const DataAccess::Interface::Snapshot *>;               // list type for the snapshots
%template(LithoTypeList)               std::vector<const DataAccess::Interface::LithoType *>;              // list type for the lithotypes
%template(FormationList)               std::vector<const DataAccess::Interface::Formation *>;              // list type for the formations
%template(SurfaceList)                 std::vector<const DataAccess::Interface::Surface *>;                // list type for the surfaces
%template(ReservoirList)               std::vector<const DataAccess::Interface::Reservoir *>;              // list type for the reservoirs
%template(MobileLayerList)             std::vector<const DataAccess::Interface::MobileLayer *>;            // list type for the MobileLayers
%template(PaleoPropertyList)           std::vector<const DataAccess::Interface::PaleoProperty *>;          // list type for the PaleoProperties
%template(PaleoFormationPropertyList)  std::vector<const DataAccess::Interface::PaleoFormationProperty *>; // list type for the PaleoFormationProperties.
%template(PaleoSurfacePropertyList)    std::vector<const DataAccess::Interface::PaleoSurfaceProperty *>;   // list type for the PaleoSurfaceProperties
%template(FluidTypeList)               std::vector<const DataAccess::Interface::FluidType*>;               // list type of FluidTypes
%template(AllochthonousLithologyList)  std::vector<const DataAccess::Interface::AllochthonousLithology *>; // list type for the AllochthonousLithologies
%template(TrapList)                    std::vector<const DataAccess::Interface::Trap *>;                   // list type for the traps
%template(TrapperList)                 std::vector<const DataAccess::Interface::Trapper *>;                // list type for the trappers
%template(MigrationList)               std::vector<const DataAccess::Interface::Migration *>;              // list type for the trappers
%template(InputValueList)              std::vector<const DataAccess::Interface::InputValue *>;             // list type for the (Mapped)InputValues
%template(PropertyList)                std::vector<const DataAccess::Interface::Property *>;               // list type for the properties
%template(PropertyValueList)           std::vector<const DataAccess::Interface::PropertyValue *>;          // list type for the property values
%template(OutputPropertyList)          std::vector<const DataAccess::Interface::OutputProperty *>;         // list type for the output properties
%template(FluidDensitySampleList)      std::vector<const DataAccess::Interface::FluidDensitySample *>;     // list type for the fluid density  samples
%template(RelatedProjectList)          std::vector<const DataAccess::Interface::RelatedProject *>;         // list type for the related-projects
%template(FaultCollectionList)         std::vector<const DataAccess::Interface::FaultCollection *>;
%template(FaultList)                   std::vector<const DataAccess::Interface::Fault *>;
%template(PointList)                   std::vector<const DataAccess::Interface::Point *>;

%template(AllochthonousLithologyDistributionList)  std::vector<const DataAccess::Interface::AllochthonousLithologyDistribution *>; // list type for the AllochthonousLithologyDistributions
%template(AllochthonousLithologyInterpolationList) std::vector<const DataAccess::Interface::AllochthonousLithologyInterpolation *>;// list type for the AllochthonousLithologyInterpolations
%template(LithologyHeatCapacitySampleList)         std::vector<const DataAccess::Interface::LithologyHeatCapacitySample *>;        // list type for the lithology heat-capacity samples
%template(LithologyThermalConductivitySampleList)  std::vector<const DataAccess::Interface::LithologyThermalConductivitySample *>; // list type for the lithology thermal-conductivity samples
%template(FluidThermalConductivitySampleList)      std::vector<const DataAccess::Interface::FluidThermalConductivitySample *>;     // list type for the fluid thermal-conductivity samples
%template(FluidHeatCapacitySampleList)             std::vector<const DataAccess::Interface::FluidHeatCapacitySample *>;            // list type for the fluid heat-capacity samples
%template(ConstrainedOverpressureIntervalList)     std::vector<const DataAccess::Interface::ConstrainedOverpressureInterval*>;     // list type for constrained overpressure intervals
