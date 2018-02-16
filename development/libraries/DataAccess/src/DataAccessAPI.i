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

#include "Interface/Interface.h"
#include "Interface/daobject.h"

#include "Interface/AllochthonousLithology.h"
#include "Interface/AllochthonousLithologyDistribution.h"
#include "Interface/AllochthonousLithologyInterpolation.h"
#include "Interface/ApplicationGlobalOperations.h"
#include "Interface/AttributeValue.h"
#include "Interface/auxiliaryfaulttypes.h"
#include "Interface/BasementFormation.h"
#include "Interface/BasementSurface.h"
#include "Interface/bioconsts.h"
#include "Interface/biodegradationparameters.h"
#include "Interface/child.h"
#include "Interface/ConstrainedOverpressureInterval.h"
#include "Interface/CrustFormation.h"
#include "Interface/CrustalThicknessData.h"
#include "Interface/ContinentalCrustHistoryGenerator.h"
#include "Interface/DiffusionLeakageParameters.h"
#include "Interface/DynArray.h"
#include "Interface/FaultCollection.h"
#include "Interface/faultelementcalculator.h"
#include "Interface/faultfilereader.h"
#include "Interface/faultfilereaderfactory.h"
#include "Interface/Faulting.h"
#include "Interface/FluidDensitySample.h"
#include "Interface/FluidHeatCapacitySample.h"
#include "Interface/FluidThermalConductivitySample.h"
#include "Interface/FluidType.h"
#include "Interface/Formation.h"
#include "Interface/fracturepressurefunctionparameters.h"
#include "Interface/Grid.h"
#include "Interface/GridMap.h"
#include "hdf5funcs.h"
#include "Interface/ibsfaultfilereader.h"
#include "Interface/InputValue.h"
#include "Interface/Interface.h"
#include "Interface/IrreducibleWaterSaturationSample.h"
#include "Interface/landmarkfaultfilereader.h"
#include "Interface/LangmuirAdsorptionIsothermSample.h"
#include "Interface/LangmuirAdsorptionTOCEntry.h"
#include "Interface/LithologyHeatCapacitySample.h"
#include "Interface/LithologyThermalConductivitySample.h"
#include "Interface/LithoType.h"
#include "Interface/Local2DArray.h"
#include "Interface/MantleFormation.h"
#include "Interface/mapwriter.h"
#include "Interface/MessageHandler.h"
#include "Interface/Migration.h"
#include "Interface/MobileLayer.h"
#include "Interface/Objectfactory.h"
#include "Interface/OutputProperty.h"
#include "Interface/PaleoFormationProperty.h"
#include "Interface/PaleoProperty.h"
#include "Interface/PaleoSurfaceProperty.h"
#include "Interface/parent.h"
#include "Interface/PointAdsorptionHistory.h"
#include "Interface/ProjectData.h"
#include "Interface/Validator.h"
#include "Interface/ProjectHandle.h"
#include "Interface/Property.h"
#include "Interface/PropertyValue.h"
#include "Interface/reconmodelfaultfilereader.h"
#include "Interface/RelatedProject.h"
#include "Interface/Reservoir.h"
#include "Interface/RunParameters.h"
#include "Interface/SGDensitySample.h"
#include "Interface/Snapshot.h"
#include "Interface/SourceRock.h"
#include "Interface/Surface.h"
#include "Interface/TableOceanicCrustThicknessHistory.h"
using namespace DataAccess::Interface;
#include "Interface/TouchstoneMap.h"
#include "Interface/Trap.h"
#include "Interface/Trapper.h"
#include "Interface/zycorfaultfilereader.h"
#include "Interface/IgneousIntrusionEvent.h"

// Interface for SerialDataAccess library
#include "../../SerialDataAccess/src/Interface/SerialApplicationGlobalOperations.h"
#include "../../SerialDataAccess/src/Interface/SerialGrid.h"
#include "../../SerialDataAccess/src/Interface/SerialGridMap.h"
#include "../../SerialDataAccess/src/Interface/SerialMapWriter.h"
#include "../../SerialDataAccess/src/Interface/SerialMessageHandler.h"

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

%include "Interface/Interface.h"
%include "Interface/parent.h"
%include "Interface/daobject.h"
%include "Interface/child.h"
%include "Interface/auxiliaryfaulttypes.h"
%include "Interface/bioconsts.h"
%include "Interface/biodegradationparameters.h"
%include "Interface/Formation.h"
%include "Interface/Surface.h"
%include "Interface/PaleoProperty.h"
%include "Interface/PaleoFormationProperty.h"
%include "Interface/AllochthonousLithology.h"
%include "Interface/AllochthonousLithologyDistribution.h"
%include "Interface/AllochthonousLithologyInterpolation.h"
%include "Interface/ApplicationGlobalOperations.h"
%include "Interface/AttributeValue.h"
%include "Interface/BasementFormation.h"
%include "Interface/BasementSurface.h"
%include "Interface/ConstrainedOverpressureInterval.h"
%include "Interface/ContinentalCrustHistoryGenerator.h"
%include "Interface/CrustFormation.h"
%include "Interface/CrustalThicknessData.h"
%include "Interface/DiffusionLeakageParameters.h"
%include "Interface/DynArray.h"
%include "Interface/FaultCollection.h"
%include "Interface/faultelementcalculator.h"
%include "Interface/faultfilereader.h"
%include "Interface/faultfilereaderfactory.h"
%include "Interface/Faulting.h"
%include "Interface/FluidDensitySample.h"
%include "Interface/FluidHeatCapacitySample.h"
%include "Interface/FluidThermalConductivitySample.h"
%include "Interface/FluidType.h"
%include "Interface/fracturepressurefunctionparameters.h"
%include "Interface/Grid.h"
%include "Interface/GridMap.h"
%include "hdf5funcs.h"
%include "Interface/ibsfaultfilereader.h"
%include "Interface/InputValue.h"
%include "Interface/Interface.h"
%include "Interface/IrreducibleWaterSaturationSample.h"
%include "Interface/landmarkfaultfilereader.h"
%include "Interface/LangmuirAdsorptionIsothermSample.h"
%include "Interface/LangmuirAdsorptionTOCEntry.h"
%include "Interface/LithologyHeatCapacitySample.h"
%include "Interface/LithologyThermalConductivitySample.h"
%include "Interface/LithoType.h"
%include "Interface/Local2DArray.h"
%include "Interface/MantleFormation.h"
%include "Interface/mapwriter.h"
%include "Interface/MessageHandler.h"
%include "Interface/Migration.h"
%include "Interface/MobileLayer.h"
%include "Interface/Objectfactory.h"
%include "Interface/OutputProperty.h"
%include "Interface/PaleoSurfaceProperty.h"
%include "Interface/PointAdsorptionHistory.h"
%include "Interface/ProjectData.h"
%include "Interface/Validator.h"
%include "Interface/ProjectHandle.h"
%include "Interface/Property.h"
%include "Interface/PropertyValue.h"
%include "Interface/reconmodelfaultfilereader.h"
%include "Interface/RelatedProject.h"
%include "Interface/Reservoir.h"
%include "Interface/RunParameters.h"
%include "Interface/SGDensitySample.h"
%include "Interface/Snapshot.h"
%include "Interface/SourceRock.h"
%include "Interface/TableOceanicCrustThicknessHistory.h"
%include "Interface/TouchstoneMap.h"
%include "Interface/Trap.h"
%include "Interface/Trapper.h"
%include "Interface/IgneousIntrusionEvent.h"
%include "Interface/zycorfaultfilereader.h"

// Interface for SerialDataAccess library
%include "../../SerialDataAccess/src/Interface/SerialApplicationGlobalOperations.h"
%include "../../SerialDataAccess/src/Interface/SerialGrid.h"
%include "../../SerialDataAccess/src/Interface/SerialGridMap.h"
%include "../../SerialDataAccess/src/Interface/SerialMapWriter.h"
%include "../../SerialDataAccess/src/Interface/SerialMessageHandler.h"

%include "../../utilities/src/FormattingException.h"

%include "../../FileSystem/src/Path.h"
%include "../../FileSystem/src/FilePath.h"
%include "../../FileSystem/src/FolderPath.h"

%template(SnapshotList)                std::vector<const DataAccess::Interface::Snapshot *>;               // list type for the snapshots
%template(LithoTypeList)               std::vector<const DataAccess::Interface::LithoType *>;              // list type for the lithotypes
%template(TouchstoneMapList)           std::vector<const DataAccess::Interface::TouchstoneMap *>;          // list type for the TouchstoneMaps
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
