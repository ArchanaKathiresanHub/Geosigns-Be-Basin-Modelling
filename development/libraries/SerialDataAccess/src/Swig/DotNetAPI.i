/* File : DotNetAPI.i */

%module SerialDataAccess

#define USEINTERFACE 
 
%include "Base/Common.i"

%{

#include "../../../DataAccess/src/interface/Interface.h"
#include "../../../DataAccess/src/interface/daobject.h"

#include "../../../DataAccess/src/interface/AllochthonousLithology.h"
#include "../../../DataAccess/src/interface/AllochthonousLithologyDistribution.h"
#include "../../../DataAccess/src/interface/AllochthonousLithologyInterpolation.h"
#include "../../../DataAccess/src/interface/ApplicationGlobalOperations.h"
#include "../../../DataAccess/src/interface/AttributeValue.h"
#include "../../../DataAccess/src/interface/auxiliaryfaulttypes.h"
#include "../../../DataAccess/src/interface/BasementFormation.h"
#include "../../../DataAccess/src/interface/BasementSurface.h"
#include "../../../DataAccess/src/interface/bioconsts.h"
#include "../../../DataAccess/src/interface/biodegradationparameters.h"
#include "../../../DataAccess/src/interface/child.h"
#include "../../../DataAccess/src/interface/ConstrainedOverpressureInterval.h"
#include "../../../DataAccess/src/interface/CrustFormation.h"
#include "../../../DataAccess/src/interface/CrustalThicknessData.h"
#include "../../../DataAccess/src/interface/DiffusionLeakageParameters.h"
#include "../../../DataAccess/src/interface/DynArray.h"
#include "../../../DataAccess/src/interface/FaultCollection.h"
#include "../../../DataAccess/src/interface/faultelementcalculator.h"
#include "../../../DataAccess/src/interface/faultfilereader.h"
#include "../../../DataAccess/src/interface/faultfilereaderfactory.h"
#include "../../../DataAccess/src/interface/Faulting.h"
#include "../../../DataAccess/src/interface/FluidDensitySample.h"
#include "../../../DataAccess/src/interface/FluidHeatCapacitySample.h"
#include "../../../DataAccess/src/interface/FluidThermalConductivitySample.h"
#include "../../../DataAccess/src/interface/FluidType.h"
#include "../../../DataAccess/src/interface/Formation.h"
#include "../../../DataAccess/src/interface/fracturepressurefunctionparameters.h"
#include "../../../DataAccess/src/interface/Grid.h"
#include "../../../DataAccess/src/interface/GridMap.h"
#include "../../../DataAccess/src/interface/ibsfaultfilereader.h"
//#include "../../../DataAccess/src/interface/Implementation.h"
#include "../../../DataAccess/src/interface/InputValue.h"
#include "../../../DataAccess/src/interface/Interface.h"
#include "../../../DataAccess/src/interface/IrreducibleWaterSaturationSample.h"
#include "../../../DataAccess/src/interface/landmarkfaultfilereader.h"
#include "../../../DataAccess/src/interface/LangmuirAdsorptionIsothermSample.h"
#include "../../../DataAccess/src/interface/LangmuirAdsorptionTOCEntry.h"
#include "../../../DataAccess/src/interface/Lead.h"
#include "../../../DataAccess/src/interface/LeadTrap.h"
#include "../../../DataAccess/src/interface/LithologyHeatCapacitySample.h"
#include "../../../DataAccess/src/interface/LithologyThermalConductivitySample.h"
#include "../../../DataAccess/src/interface/LithoType.h"
#include "../../../DataAccess/src/interface/MantleFormation.h"
#include "../../../DataAccess/src/interface/mapwriter.h"
#include "../../../DataAccess/src/interface/MessageHandler.h"
#include "../../../DataAccess/src/interface/Migration.h"
#include "../../../DataAccess/src/interface/MobileLayer.h"
#include "../../../DataAccess/src/interface/Objectfactory.h"
#include "../../../DataAccess/src/interface/OutputProperty.h"
#include "../../../DataAccess/src/interface/PaleoFormationProperty.h"
#include "../../../DataAccess/src/interface/PaleoProperty.h"
//#include "../../../DataAccess/src/interface/PaleoSurface.h"
#include "../../../DataAccess/src/interface/PaleoSurfaceProperty.h"
#include "../../../DataAccess/src/interface/parent.h"
#include "../../../DataAccess/src/interface/PointAdsorptionHistory.h"
#include "../../../DataAccess/src/interface/ProjectData.h"
#include "../../../DataAccess/src/interface/ProjectHandle.h"
#include "../../../DataAccess/src/interface/Property.h"
#include "../../../DataAccess/src/interface/PropertyValue.h"
#include "../../../DataAccess/src/interface/reconmodelfaultfilereader.h"
#include "../../../DataAccess/src/interface/RelatedProject.h"
#include "../../../DataAccess/src/interface/Reservoir.h"
#include "../../../DataAccess/src/interface/RunParameters.h"
#include "../../../DataAccess/src/interface/SGDensitySample.h"
#include "../../../DataAccess/src/interface/Snapshot.h"
#include "../../../DataAccess/src/interface/SourceRock.h"
#include "../../../DataAccess/src/interface/Surface.h"
#include "../../../DataAccess/src/interface/TouchstoneMap.h"
#include "../../../DataAccess/src/interface/Trap.h"
#include "../../../DataAccess/src/interface/Trapper.h"
#include "../../../DataAccess/src/interface/TrapPhase.h"
#include "../../../DataAccess/src/interface/zycorfaultfilereader.h"

%}


//%include "../../../DataAccess/src/interface/Implementation.h"
%include "../../../DataAccess/src/interface/Interface.h"
%include "../../../DataAccess/src/interface/parent.h"
%include "../../../DataAccess/src/interface/daobject.h"
%include "../../../DataAccess/src/interface/child.h"
%include "../../../DataAccess/src/interface/auxiliaryfaulttypes.h"
%include "../../../DataAccess/src/interface/bioconsts.h"
%include "../../../DataAccess/src/interface/biodegradationparameters.h"
%include "../../../DataAccess/src/interface/Formation.h"
%include "../../../DataAccess/src/interface/Surface.h"
%include "../../../DataAccess/src/interface/PaleoProperty.h"
%include "../../../DataAccess/src/interface/PaleoFormationProperty.h"
%include "../../../DataAccess/src/interface/AllochthonousLithology.h"
%include "../../../DataAccess/src/interface/AllochthonousLithologyDistribution.h"
%include "../../../DataAccess/src/interface/AllochthonousLithologyInterpolation.h"
//%include "../../../DataAccess/src/interface/ApplicationGlobalOperations.h"
%include "../../../DataAccess/src/interface/AttributeValue.h"
%include "../../../DataAccess/src/interface/BasementFormation.h"
%include "../../../DataAccess/src/interface/BasementSurface.h"
%include "../../../DataAccess/src/interface/ConstrainedOverpressureInterval.h"
%include "../../../DataAccess/src/interface/CrustFormation.h"
%include "../../../DataAccess/src/interface/CrustalThicknessData.h"
%include "../../../DataAccess/src/interface/DiffusionLeakageParameters.h"
%include "../../../DataAccess/src/interface/DynArray.h"
%include "../../../DataAccess/src/interface/FaultCollection.h"
%include "../../../DataAccess/src/interface/faultelementcalculator.h"
%include "../../../DataAccess/src/interface/faultfilereader.h"
%include "../../../DataAccess/src/interface/faultfilereaderfactory.h"
%include "../../../DataAccess/src/interface/Faulting.h"
%include "../../../DataAccess/src/interface/FluidDensitySample.h"
%include "../../../DataAccess/src/interface/FluidHeatCapacitySample.h"
%include "../../../DataAccess/src/interface/FluidThermalConductivitySample.h"
%include "../../../DataAccess/src/interface/FluidType.h"
%include "../../../DataAccess/src/interface/fracturepressurefunctionparameters.h"
%include "../../../DataAccess/src/interface/Grid.h"
%include "../../../DataAccess/src/interface/GridMap.h"
%include "../../../DataAccess/src/interface/ibsfaultfilereader.h"
%include "../../../DataAccess/src/interface/InputValue.h"
%include "../../../DataAccess/src/interface/Interface.h"
%include "../../../DataAccess/src/interface/IrreducibleWaterSaturationSample.h"
%include "../../../DataAccess/src/interface/landmarkfaultfilereader.h"
%include "../../../DataAccess/src/interface/LangmuirAdsorptionIsothermSample.h"
%include "../../../DataAccess/src/interface/LangmuirAdsorptionTOCEntry.h"
%include "../../../DataAccess/src/interface/Lead.h"
%include "../../../DataAccess/src/interface/LeadTrap.h"
%include "../../../DataAccess/src/interface/LithologyHeatCapacitySample.h"
%include "../../../DataAccess/src/interface/LithologyThermalConductivitySample.h"
%include "../../../DataAccess/src/interface/LithoType.h"
%include "../../../DataAccess/src/interface/MantleFormation.h"
%include "../../../DataAccess/src/interface/mapwriter.h"
//%include "../../../DataAccess/src/interface/MessageHandler.h"
%include "../../../DataAccess/src/interface/Migration.h"
%include "../../../DataAccess/src/interface/MobileLayer.h"
%include "../../../DataAccess/src/interface/Objectfactory.h"
%include "../../../DataAccess/src/interface/OutputProperty.h"
//%include "../../../DataAccess/src/interface/PaleoSurface.h"
%include "../../../DataAccess/src/interface/PaleoSurfaceProperty.h"
%include "../../../DataAccess/src/interface/PointAdsorptionHistory.h"
%include "../../../DataAccess/src/interface/ProjectData.h"
%include "../../../DataAccess/src/interface/ProjectHandle.h"
%include "../../../DataAccess/src/interface/Property.h"
%include "../../../DataAccess/src/interface/PropertyValue.h"
%include "../../../DataAccess/src/interface/reconmodelfaultfilereader.h"
%include "../../../DataAccess/src/interface/RelatedProject.h"
%include "../../../DataAccess/src/interface/Reservoir.h"
%include "../../../DataAccess/src/interface/RunParameters.h"
%include "../../../DataAccess/src/interface/SGDensitySample.h"
%include "../../../DataAccess/src/interface/Snapshot.h"
%include "../../../DataAccess/src/interface/SourceRock.h"
%include "../../../DataAccess/src/interface/TouchstoneMap.h"
%include "../../../DataAccess/src/interface/Trap.h"
%include "../../../DataAccess/src/interface/Trapper.h"
%include "../../../DataAccess/src/interface/TrapPhase.h"
%include "../../../DataAccess/src/interface/zycorfaultfilereader.h"




/// list type for the snapshots
%template(SnapshotList)      std::vector<const DataAccess::Interface::Snapshot *>;
/// list type for the lithotypes
%template(LithoTypeList)      std::vector<const DataAccess::Interface::LithoType *>;
/// list type for the TouchstoneMaps
%template(TouchstoneMapList)      std::vector<const DataAccess::Interface::TouchstoneMap *>;
/// list type for the formations
%template(FormationList)      std::vector<const DataAccess::Interface::Formation *>;
/// list type for the surfaces
%template(SurfaceList)      std::vector<const DataAccess::Interface::Surface *>;
/// list type for the reservoirs
%template(ReservoirList)      std::vector<const DataAccess::Interface::Reservoir *>;
/// list type for the MobileLayers
%template(MobileLayerList)      std::vector<const DataAccess::Interface::MobileLayer *>;
/// list type for the PaleoProperties.
%template(PaleoPropertyList)      std::vector<const DataAccess::Interface::PaleoProperty *>;
/// list type for the PaleoFormationProperties.
%template(PaleoFormationPropertyList)      std::vector<const DataAccess::Interface::PaleoFormationProperty *>;
/// list type for the PaleoSurfaceProperties.
%template(PaleoSurfacePropertyList)      std::vector<const DataAccess::Interface::PaleoSurfaceProperty *>;
/// list type of FluidTypes.
%template(FluidTypeList)      std::vector<const DataAccess::Interface::FluidType*>;
/// list type for the AllochthonousLithologies
%template(AllochthonousLithologyList)      std::vector<const DataAccess::Interface::AllochthonousLithology *>;
/// list type for the AllochthonousLithologyDistributions
%template(AllochthonousLithologyDistributionList)      std::vector<const DataAccess::Interface::AllochthonousLithologyDistribution *>;
/// list type for the AllochthonousLithologyInterpolations
%template(AllochthonousLithologyInterpolationList)      std::vector<const DataAccess::Interface::AllochthonousLithologyInterpolation *>;
/// list type for the traps
%template(TrapList)      std::vector<const DataAccess::Interface::Trap *>;
/// list type for the trappers
%template(TrapperList)      std::vector<const DataAccess::Interface::Trapper *>;
/// list type for the trappers
%template(MigrationList)      std::vector<const DataAccess::Interface::Migration *>;
/// list type for the Leads
%template(LeadList)      std::vector<const DataAccess::Interface::Lead *>;
/// list type for the LeadTraps
%template(LeadTrapList)      std::vector<const DataAccess::Interface::LeadTrap *>;
/// list type for the (Mapped)InputValues
%template(InputValueList)      std::vector<const DataAccess::Interface::InputValue *>;
/// list type for the properties
%template(PropertyList)      std::vector<const DataAccess::Interface::Property *>;
/// list type for the property values
%template(PropertyValueList)      std::vector<const DataAccess::Interface::PropertyValue *>;
/// list type for the output properties.
%template(OutputPropertyList)      std::vector<const DataAccess::Interface::OutputProperty *>;
/// list type for the lithology heat-capacity samples.
%template(LithologyHeatCapacitySampleList)      std::vector<const DataAccess::Interface::LithologyHeatCapacitySample *>;
/// list type for the lithology thermal-conductivity samples.
%template(LithologyThermalConductivitySampleList)      std::vector<const DataAccess::Interface::LithologyThermalConductivitySample *>;
/// list type for the fluid density  samples.
%template(FluidDensitySampleList)      std::vector<const DataAccess::Interface::FluidDensitySample *>;
/// list type for the fluid thermal-conductivity samples.
%template(FluidThermalConductivitySampleList)      std::vector<const DataAccess::Interface::FluidThermalConductivitySample *>;
/// list type for the fluid heat-capacity samples.
%template(FluidHeatCapacitySampleList)      std::vector<const DataAccess::Interface::FluidHeatCapacitySample *>;
/// list type for the related-projects.
%template(RelatedProjectList)      std::vector<const DataAccess::Interface::RelatedProject *>;
/// list type for constrained overpressure intervals.
%template(ConstrainedOverpressureIntervalList)      std::vector<const DataAccess::Interface::ConstrainedOverpressureInterval*>;


%template(FaultCollectionList)      std::vector<const DataAccess::Interface::FaultCollection *>;
%template(FaultList)      std::vector<const DataAccess::Interface::Fault *>;
%template(PointList)      std::vector<const DataAccess::Interface::Point *>;

