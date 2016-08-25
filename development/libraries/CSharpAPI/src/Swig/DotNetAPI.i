/* File : DotNetAPI.i */
 
%module CauldronAPI

%include "Base/Common.i"
%include "std_vector.i"
%include "std_string.i"
%include "std_map.i"
%include "std_pair.i"

%include "enums.swg"
%csconst(1);

/// Shared pointer types wrapping
// CASA API
%include "std_shared_ptr.i"
// CASA API
%shared_ptr(casa::Parameter)
%shared_ptr(casa::PrmCompactionCoefficient)
%shared_ptr(casa::PrmLithoSTPThermalCond)
%shared_ptr(casa::PrmSourceRockHC)
%shared_ptr(casa::PrmSourceRockTOC)
%shared_ptr(casa::PrmWindow)
%shared_ptr(casa::PrmCrustThinning)
%shared_ptr(casa::PrmOneCrustThinningEvent)
%shared_ptr(casa::PrmSourceRockHI)
%shared_ptr(casa::PrmSourceRockType)
%shared_ptr(casa::PrmLithoFraction)
%shared_ptr(casa::PrmPermeabilityModel)
%shared_ptr(casa::PrmSourceRockPreAsphaltStartAct)
%shared_ptr(casa::PrmSurfacePorosity)
%shared_ptr(casa::PrmLithologyProp)
%shared_ptr(casa::PrmPorosityModel)
%shared_ptr(casa::PrmSourceRockProp)
%shared_ptr(casa::PrmTopCrustHeatProduction)
%shared_ptr(casa::RunCase)
%shared_ptr(casa::RunCaseImpl)

// CASA API enums constants
%csconstvalue("Cauldron.PermeabilityModel.SANDSTONE_PERMEABILITY")   PermSandstone;
%csconstvalue("Cauldron.PermeabilityModel.MUDSTONE_PERMEABILITY")    PermMudstone;
%csconstvalue("Cauldron.PermeabilityModel.NONE_PERMEABILITY")        PermNone;  
%csconstvalue("Cauldron.PermeabilityModel.IMPERMEABLE_PERMEABILITY") PermImpermeable;
%csconstvalue("Cauldron.PermeabilityModel.MULTIPOINT_PERMEABILITY")  PermMultipoint;

%csconstvalue("Cauldron.PorosityModel.EXPONENTIAL_POROSITY")        PorExponential;
%csconstvalue("Cauldron.PorosityModel.SOIL_MECHANICS_POROSITY")     PorSoilMechanics;
%csconstvalue("Cauldron.PorosityModel.DOUBLE_EXPONENTIAL_POROSITY") PorDoubleExponential;

%csconstvalue("Cauldron.LithologyManager.PermeabilityModel.PermSandstone")   Sandstone;
%csconstvalue("Cauldron.LithologyManager.PermeabilityModel.PermMudstone")    Mudstone;
%csconstvalue("Cauldron.LithologyManager.PermeabilityModel.PermNone")        None;
%csconstvalue("Cauldron.LithologyManager.PermeabilityModel.PermImpermeable") Impermeable;
%csconstvalue("Cauldron.LithologyManager.PermeabilityModel.PermMultipoint")  Multipoint;
%csconstvalue("Cauldron.LithologyManager.PermeabilityModel.PermUnknown")     Unknown;


%{
// Interface to DataModel
#include "../../../DataModel/src/PropertyAttribute.h"
#include "../../../DataModel/src/AbstractFormation.h"
#include "../../../DataModel/src/AbstractSurface.h"
#include "../../../DataModel/src/AbstractGrid.h"
#include "../../../DataModel/src/AbstractProperty.h"
#include "../../../DataModel/src/AbstractSnapshot.h"

#include "../../../DataAccess/src/Interface/Interface.h"
#include "../../../DataAccess/src/Interface/daobject.h"

#include "../../../DataAccess/src/Interface/AllochthonousLithology.h"
#include "../../../DataAccess/src/Interface/AllochthonousLithologyDistribution.h"
#include "../../../DataAccess/src/Interface/AllochthonousLithologyInterpolation.h"
#include "../../../DataAccess/src/Interface/ApplicationGlobalOperations.h"
#include "../../../DataAccess/src/Interface/AttributeValue.h"
#include "../../../DataAccess/src/Interface/auxiliaryfaulttypes.h"
#include "../../../DataAccess/src/Interface/BasementFormation.h"
#include "../../../DataAccess/src/Interface/BasementSurface.h"
#include "../../../DataAccess/src/Interface/bioconsts.h"
#include "../../../DataAccess/src/Interface/biodegradationparameters.h"
#include "../../../DataAccess/src/Interface/child.h"
#include "../../../DataAccess/src/Interface/ConstrainedOverpressureInterval.h"
#include "../../../DataAccess/src/Interface/CrustFormation.h"
#include "../../../DataAccess/src/Interface/CrustalThicknessData.h"
#include "../../../DataAccess/src/Interface/DiffusionLeakageParameters.h"
#include "../../../DataAccess/src/Interface/DynArray.h"
#include "../../../DataAccess/src/Interface/FaultCollection.h"
#include "../../../DataAccess/src/Interface/faultelementcalculator.h"
#include "../../../DataAccess/src/Interface/faultfilereader.h"
#include "../../../DataAccess/src/Interface/faultfilereaderfactory.h"
#include "../../../DataAccess/src/Interface/Faulting.h"
#include "../../../DataAccess/src/Interface/FluidDensitySample.h"
#include "../../../DataAccess/src/Interface/FluidHeatCapacitySample.h"
#include "../../../DataAccess/src/Interface/FluidThermalConductivitySample.h"
#include "../../../DataAccess/src/Interface/FluidType.h"
#include "../../../DataAccess/src/Interface/Formation.h"
#include "../../../DataAccess/src/Interface/fracturepressurefunctionparameters.h"
#include "../../../DataAccess/src/Interface/Grid.h"
#include "../../../DataAccess/src/Interface/GridMap.h"
#include "../../../DataAccess/src/Interface/ibsfaultfilereader.h"
//#include "../../../DataAccess/src/Interface/Implementation.h"
#include "../../../DataAccess/src/Interface/InputValue.h"
#include "../../../DataAccess/src/Interface/Interface.h"
#include "../../../DataAccess/src/Interface/IrreducibleWaterSaturationSample.h"
#include "../../../DataAccess/src/Interface/landmarkfaultfilereader.h"
#include "../../../DataAccess/src/Interface/LangmuirAdsorptionIsothermSample.h"
#include "../../../DataAccess/src/Interface/LangmuirAdsorptionTOCEntry.h"
#include "../../../DataAccess/src/Interface/LithologyHeatCapacitySample.h"
#include "../../../DataAccess/src/Interface/LithologyThermalConductivitySample.h"
#include "../../../DataAccess/src/Interface/LithoType.h"
#include "../../../DataAccess/src/Interface/MantleFormation.h"
#include "../../../DataAccess/src/Interface/mapwriter.h"
#include "../../../DataAccess/src/Interface/MessageHandler.h"
#include "../../../DataAccess/src/Interface/Migration.h"
#include "../../../DataAccess/src/Interface/MobileLayer.h"
#include "../../../DataAccess/src/Interface/Objectfactory.h"
#include "../../../DataAccess/src/Interface/OutputProperty.h"
#include "../../../DataAccess/src/Interface/PaleoFormationProperty.h"
#include "../../../DataAccess/src/Interface/PaleoProperty.h"
//#include "../../../DataAccess/src/Interface/PaleoSurface.h"
#include "../../../DataAccess/src/Interface/PaleoSurfaceProperty.h"
#include "../../../DataAccess/src/Interface/parent.h"
#include "../../../DataAccess/src/Interface/PointAdsorptionHistory.h"
#include "../../../DataAccess/src/Interface/ProjectData.h"
#include "../../../DataAccess/src/Interface/ProjectHandle.h"
#include "../../../DataAccess/src/Interface/Property.h"
#include "../../../DataAccess/src/Interface/PropertyValue.h"
#include "../../../DataAccess/src/Interface/reconmodelfaultfilereader.h"
#include "../../../DataAccess/src/Interface/RelatedProject.h"
#include "../../../DataAccess/src/Interface/Reservoir.h"
#include "../../../DataAccess/src/Interface/RunParameters.h"
#include "../../../DataAccess/src/Interface/SGDensitySample.h"
#include "../../../DataAccess/src/Interface/Snapshot.h"
#include "../../../DataAccess/src/Interface/SourceRock.h"
#include "../../../DataAccess/src/Interface/Surface.h"
#include "../../../DataAccess/src/Interface/TouchstoneMap.h"
#include "../../../DataAccess/src/Interface/Trap.h"
#include "../../../DataAccess/src/Interface/Trapper.h"
#include "../../../DataAccess/src/Interface/zycorfaultfilereader.h"
#include "../../../DataAccess/src/Interface/IgneousIntrusionEvent.h"

// Interface for SerialDataAccess library
#include "../../../SerialDataAccess/src/Interface/SerialApplicationGlobalOperations.h"
#include "../../../SerialDataAccess/src/Interface/SerialGrid.h"
#include "../../../SerialDataAccess/src/Interface/SerialGridMap.h"
#include "../../../SerialDataAccess/src/Interface/SerialMapWriter.h"
#include "../../../SerialDataAccess/src/Interface/SerialMessageHandler.h"

// Interface for EosPack library
#include "../../../EosPack/src/EosPackCAPI.h"
#include "../../../EosPack/src/EosPack.h"
#include "../../../EosPack/src/PTDiagramCalculator.h"

// Interface for APIs library
// CMB API
#include "../../../utilities/src/FormattingException.h"
#include "../../../cmbAPI/src/UndefinedValues.h"
#include "../../../cmbAPI/src/ErrorHandler.h"
#include "../../../cmbAPI/src/LithologyManager.h"
#include "../../../cmbAPI/src/FluidManager.h"
#include "../../../cmbAPI/src/SourceRockManager.h"
#include "../../../cmbAPI/src/StratigraphyManager.h"
#include "../../../cmbAPI/src/FluidManagerImpl.h"
#include "../../../cmbAPI/src/LithologyManagerImpl.h"
#include "../../../cmbAPI/src/SourceRockManagerImpl.h"
#include "../../../cmbAPI/src/StratigraphyManagerImpl.h"
#include "../../../cmbAPI/src/SnapshotManager.h"
#include "../../../cmbAPI/src/SnapshotManagerImpl.h"
#include "../../../cmbAPI/src/PropertyManager.h"
#include "../../../cmbAPI/src/PropertyManagerImpl.h"
#include "../../../cmbAPI/src/MapsManager.h"
#include "../../../cmbAPI/src/MapsManagerImpl.h"
#include "../../../cmbAPI/src/cmbAPI.h"
// CASA API
#include "../../../casaAPI/src/CauldronApp.h"
#include "../../../casaAPI/src/DataDigger.h"
#include "../../../casaAPI/src/DataDiggerImpl.h"
#include "../../../casaAPI/src/DoEGenerator.h"
#include "../../../casaAPI/src/DoEGeneratorImpl.h"
#include "../../../casaAPI/src/JobScheduler.h"
#include "../../../casaAPI/src/JobSchedulerLocal.h"
#include "../../../casaAPI/src/JobSchedulerLSF.h"
#include "../../../casaAPI/src/MonteCarloSolver.h"
#include "../../../casaAPI/src/MonteCarloSolverImpl.h"
#include "../../../casaAPI/src/Observable.h"
#include "../../../casaAPI/src/ObsValue.h"
#include "../../../casaAPI/src/ObsGridPropertyXYZ.h"
#include "../../../casaAPI/src/ObsGridPropertyWell.h"
#include "../../../casaAPI/src/ObsValueDoubleArray.h"
#include "../../../casaAPI/src/ObsValueDoubleScalar.h"
#include "../../../casaAPI/src/ObsSpace.h"
#include "../../../casaAPI/src/ObsSpaceImpl.h"
#include "../../../casaAPI/src/Parameter.h"
#include "../../../casaAPI/src/PrmCrustThinning.h"
#include "../../../casaAPI/src/PrmOneCrustThinningEvent.h"
#include "../../../casaAPI/src/PrmTopCrustHeatProduction.h"
#include "../../../casaAPI/src/PrmSourceRockProp.h"
#include "../../../casaAPI/src/PrmSourceRockTOC.h"
#include "../../../casaAPI/src/PrmSourceRockHI.h"
#include "../../../casaAPI/src/PrmSourceRockHC.h"
#include "../../../casaAPI/src/PrmSourceRockType.h"
#include "../../../casaAPI/src/PrmSourceRockPreAsphaltStartAct.h"
#include "../../../casaAPI/src/PrmPorosityModel.h"
#include "../../../casaAPI/src/PrmSurfacePorosity.h"
#include "../../../casaAPI/src/PrmPermeabilityModel.h"
#include "../../../casaAPI/src/PrmLithoSTPThermalCond.h"
#include "../../../casaAPI/src/RSProxy.h"
#include "../../../casaAPI/src/RSProxyImpl.h"
#include "../../../casaAPI/src/RSProxySet.h"
#include "../../../casaAPI/src/RSProxySetImpl.h"
#include "../../../casaAPI/src/RunManager.h"
#include "../../../casaAPI/src/RunManagerImpl.h"
#include "../../../casaAPI/src/RunCase.h"
#include "../../../casaAPI/src/RunCaseImpl.h"
#include "../../../casaAPI/src/RunCaseSet.h"
#include "../../../casaAPI/src/RunCaseSetImpl.h"
#include "../../../casaAPI/src/TornadoSensitivityInfo.h"
#include "../../../casaAPI/src/SensitivityCalculator.h"
#include "../../../casaAPI/src/SensitivityCalculatorImpl.h"
#include "../../../casaAPI/src/ScenarioAnalysis.h"
#include "../../../casaAPI/src/VarParameter.h"
#include "../../../casaAPI/src/VarSpace.h"
#include "../../../casaAPI/src/VarSpaceImpl.h"
#include "../../../casaAPI/src/VarPrmContinuous.h"
#include "../../../casaAPI/src/VarPrmOneCrustThinningEvent.h"
#include "../../../casaAPI/src/VarPrmTopCrustHeatProduction.h"
#include "../../../casaAPI/src/VarPrmSourceRockProp.h"
#include "../../../casaAPI/src/VarPrmSourceRockTOC.h"
#include "../../../casaAPI/src/VarPrmSourceRockHI.h"
#include "../../../casaAPI/src/VarPrmSourceRockHC.h"
#include "../../../casaAPI/src/VarPrmSourceRockType.h"
#include "../../../casaAPI/src/VarPrmSourceRockPreAsphaltStartAct.h"
#include "../../../casaAPI/src/VarPrmPorosityModel.h"
#include "../../../casaAPI/src/VarPrmSurfacePorosity.h"
#include "../../../casaAPI/src/VarPrmPermeabilityModel.h"
#include "../../../casaAPI/src/VarPrmLithoSTPThermalCond.h"

#include "../../../FileSystem/src/Path.h"
#include "../../../FileSystem/src/FilePath.h"
#include "../../../FileSystem/src/FolderPath.h"

#include "../../../casaAPI/src/casaAPI.h"

using namespace casa;
%}


%include <typemaps.i>
%include <arrays_csharp.i>

// EosPackCAPI.h:EosPackComputeWithLumpingArr()
%apply double INPUT[]  { double * compMasses }
%apply double OUTPUT[] { double * phaseCompMasses }
%apply double OUTPUT[] { double * phaseDensity }
%apply double OUTPUT[] { double * phaseViscosity }

// EosPackCAPI.h:BuildPTDiagram()
%apply double INPUT[]  { double * comp }
%apply double OUTPUT[] { double * points }
%apply int    INOUT[]  { int * szIso }
%apply double OUTPUT[] { double * isolines }
%apply double OUTPUT[] { double * critPt }

// CASA Observable.h:newObsValueFromDoubles()
%apply int  &INOUT { int & off };

CSHARP_ARRAYS(char, byte)

// CASA ScenarioAnalysis.h:loadScenario()
%apply char INPUT[] { const char * stateFileBuf }

%rename(ComponentId2) DataAccess::Interface::ComponentId;
%rename(PhaseId2) pvtFlash::PhaseId;

// Interface to DataModel
%include "../../../DataModel/src/PropertyAttribute.h"
%include "../../../DataModel/src/AbstractFormation.h"
%include "../../../DataModel/src/AbstractSurface.h"
%include "../../../DataModel/src/AbstractGrid.h"
%include "../../../DataModel/src/AbstractProperty.h"
%include "../../../DataModel/src/AbstractSnapshot.h"

//%include "../../../DataAccess/src/Interface/Implementation.h"
%include "../../../DataAccess/src/Interface/Interface.h"
%include "../../../DataAccess/src/Interface/parent.h"
%include "../../../DataAccess/src/Interface/daobject.h"
%include "../../../DataAccess/src/Interface/child.h"
%include "../../../DataAccess/src/Interface/auxiliaryfaulttypes.h"
%include "../../../DataAccess/src/Interface/bioconsts.h"
%include "../../../DataAccess/src/Interface/biodegradationparameters.h"
%include "../../../DataAccess/src/Interface/Formation.h"
%include "../../../DataAccess/src/Interface/Surface.h"
%include "../../../DataAccess/src/Interface/PaleoProperty.h"
%include "../../../DataAccess/src/Interface/PaleoFormationProperty.h"
%include "../../../DataAccess/src/Interface/AllochthonousLithology.h"
%include "../../../DataAccess/src/Interface/AllochthonousLithologyDistribution.h"
%include "../../../DataAccess/src/Interface/AllochthonousLithologyInterpolation.h"
%include "../../../DataAccess/src/Interface/ApplicationGlobalOperations.h"
%include "../../../DataAccess/src/Interface/AttributeValue.h"
%include "../../../DataAccess/src/Interface/BasementFormation.h"
%include "../../../DataAccess/src/Interface/BasementSurface.h"
%include "../../../DataAccess/src/Interface/ConstrainedOverpressureInterval.h"
%include "../../../DataAccess/src/Interface/CrustFormation.h"
%include "../../../DataAccess/src/Interface/CrustalThicknessData.h"
%include "../../../DataAccess/src/Interface/DiffusionLeakageParameters.h"
%include "../../../DataAccess/src/Interface/DynArray.h"
%include "../../../DataAccess/src/Interface/FaultCollection.h"
%include "../../../DataAccess/src/Interface/faultelementcalculator.h"
%include "../../../DataAccess/src/Interface/faultfilereader.h"
%include "../../../DataAccess/src/Interface/faultfilereaderfactory.h"
%include "../../../DataAccess/src/Interface/Faulting.h"
%include "../../../DataAccess/src/Interface/FluidDensitySample.h"
%include "../../../DataAccess/src/Interface/FluidHeatCapacitySample.h"
%include "../../../DataAccess/src/Interface/FluidThermalConductivitySample.h"
%include "../../../DataAccess/src/Interface/FluidType.h"
%include "../../../DataAccess/src/Interface/fracturepressurefunctionparameters.h"
%include "../../../DataAccess/src/Interface/Grid.h"
%include "../../../DataAccess/src/Interface/GridMap.h"
%include "../../../DataAccess/src/Interface/ibsfaultfilereader.h"
%include "../../../DataAccess/src/Interface/InputValue.h"
%include "../../../DataAccess/src/Interface/Interface.h"
%include "../../../DataAccess/src/Interface/IrreducibleWaterSaturationSample.h"
%include "../../../DataAccess/src/Interface/landmarkfaultfilereader.h"
%include "../../../DataAccess/src/Interface/LangmuirAdsorptionIsothermSample.h"
%include "../../../DataAccess/src/Interface/LangmuirAdsorptionTOCEntry.h"
%include "../../../DataAccess/src/Interface/LithologyHeatCapacitySample.h"
%include "../../../DataAccess/src/Interface/LithologyThermalConductivitySample.h"
%include "../../../DataAccess/src/Interface/LithoType.h"
%include "../../../DataAccess/src/Interface/MantleFormation.h"
%include "../../../DataAccess/src/Interface/mapwriter.h"
%include "../../../DataAccess/src/Interface/MessageHandler.h"
%include "../../../DataAccess/src/Interface/Migration.h"
%include "../../../DataAccess/src/Interface/MobileLayer.h"
%include "../../../DataAccess/src/Interface/Objectfactory.h"
%include "../../../DataAccess/src/Interface/OutputProperty.h"
//%include "../../../DataAccess/src/Interface/PaleoSurface.h"
%include "../../../DataAccess/src/Interface/PaleoSurfaceProperty.h"
%include "../../../DataAccess/src/Interface/PointAdsorptionHistory.h"
%include "../../../DataAccess/src/Interface/ProjectData.h"
%include "../../../DataAccess/src/Interface/ProjectHandle.h"
%include "../../../DataAccess/src/Interface/Property.h"
%include "../../../DataAccess/src/Interface/PropertyValue.h"
%include "../../../DataAccess/src/Interface/reconmodelfaultfilereader.h"
%include "../../../DataAccess/src/Interface/RelatedProject.h"
%include "../../../DataAccess/src/Interface/Reservoir.h"
%include "../../../DataAccess/src/Interface/RunParameters.h"
%include "../../../DataAccess/src/Interface/SGDensitySample.h"
%include "../../../DataAccess/src/Interface/Snapshot.h"
%include "../../../DataAccess/src/Interface/SourceRock.h"
%include "../../../DataAccess/src/Interface/TouchstoneMap.h"
%include "../../../DataAccess/src/Interface/Trap.h"
%include "../../../DataAccess/src/Interface/Trapper.h"
%include "../../../DataAccess/src/Interface/IgneousIntrusionEvent.h"
%include "../../../DataAccess/src/Interface/zycorfaultfilereader.h"

// Interface for SerialDataAccess library
%include "../../../SerialDataAccess/src/Interface/SerialApplicationGlobalOperations.h"
%include "../../../SerialDataAccess/src/Interface/SerialGrid.h"
%include "../../../SerialDataAccess/src/Interface/SerialGridMap.h"
%include "../../../SerialDataAccess/src/Interface/SerialMapWriter.h"
%include "../../../SerialDataAccess/src/Interface/SerialMessageHandler.h"

// Interface for EosPack library
%include "../../../EosPack/src/EosPack.h"

// Interface for APIs library
// CMB API
%csconstvalue( "Cauldron.PermeabilityModel.IMPERMEABLE_PERMEABILITY" ) DataAccess::Interface::SANDSTONE_PERMEABILITY;
%csconstvalue( "Cauldron.PermeabilityModel.MUDSTONE_PERMEABILITY"    ) DataAccess::Interface::MUDSTONE_PERMEABILITY;
%csconstvalue( "Cauldron.PermeabilityModel.NONE_PERMEABILITY"        ) DataAccess::Interface::NONE_PERMEABILITY;
%csconstvalue( "Cauldron.PermeabilityModel.IMPERMEABLE_PERMEABILITY" ) DataAccess::Interface::IMPERMEABLE_PERMEABILITY;
%csconstvalue( "Cauldron.PermeabilityModel.MULTIPOINT_PERMEABILITY"  ) DataAccess::Interface::MULTIPOINT_PERMEABILITY;

%include "../../../utilities/src/FormattingException.h"
%include "../../../cmbAPI/src/ErrorHandler.h"
%include "../../../cmbAPI/src/UndefinedValues.h"
%include "../../../cmbAPI/src/LithologyManager.h"
%include "../../../cmbAPI/src/FluidManager.h"
%include "../../../cmbAPI/src/SourceRockManager.h"
%include "../../../cmbAPI/src/StratigraphyManager.h"
%include "../../../cmbAPI/src/FluidManagerImpl.h"
%include "../../../cmbAPI/src/LithologyManagerImpl.h"
%include "../../../cmbAPI/src/SourceRockManagerImpl.h"
%include "../../../cmbAPI/src/StratigraphyManagerImpl.h"
%include "../../../cmbAPI/src/SnapshotManager.h"
%include "../../../cmbAPI/src/SnapshotManagerImpl.h"
%include "../../../cmbAPI/src/PropertyManager.h"
%include "../../../cmbAPI/src/PropertyManagerImpl.h"
%include "../../../cmbAPI/src/MapsManager.h"
%include "../../../cmbAPI/src/MapsManagerImpl.h"
%include "../../../cmbAPI/src/cmbAPI.h"
// CASA API
%include "../../../casaAPI/src/CauldronApp.h"
%include "../../../casaAPI/src/DataDigger.h"
%include "../../../casaAPI/src/DataDiggerImpl.h"
%include "../../../casaAPI/src/DoEGenerator.h"
%include "../../../casaAPI/src/DoEGeneratorImpl.h"
%include "../../../casaAPI/src/JobScheduler.h"
%include "../../../casaAPI/src/JobSchedulerLocal.h"
%include "../../../casaAPI/src/JobSchedulerLSF.h"
%include "../../../casaAPI/src/MonteCarloSolver.h"
%include "../../../casaAPI/src/MonteCarloSolverImpl.h"
%include "../../../casaAPI/src/Observable.h"
%include "../../../casaAPI/src/ObsValue.h"
%include "../../../casaAPI/src/ObsGridPropertyXYZ.h"
%include "../../../casaAPI/src/ObsGridPropertyWell.h"
%include "../../../casaAPI/src/ObsValueDoubleArray.h"
%include "../../../casaAPI/src/ObsValueDoubleScalar.h"
%include "../../../casaAPI/src/ObsSpace.h"
%include "../../../casaAPI/src/ObsSpaceImpl.h"
%include "../../../casaAPI/src/Parameter.h"
%include "../../../casaAPI/src/PrmCrustThinning.h"
%include "../../../casaAPI/src/PrmOneCrustThinningEvent.h"
%include "../../../casaAPI/src/PrmTopCrustHeatProduction.h"
%include "../../../casaAPI/src/PrmSourceRockProp.h"
%include "../../../casaAPI/src/PrmSourceRockTOC.h"
%include "../../../casaAPI/src/PrmSourceRockHI.h"
%include "../../../casaAPI/src/PrmSourceRockHC.h"
%include "../../../casaAPI/src/PrmSourceRockType.h"
%include "../../../casaAPI/src/PrmSourceRockPreAsphaltStartAct.h"
%include "../../../casaAPI/src/PrmPorosityModel.h"
%include "../../../casaAPI/src/PrmSurfacePorosity.h"
%include "../../../casaAPI/src/PrmPermeabilityModel.h"
%include "../../../casaAPI/src/PrmLithoSTPThermalCond.h"
%include "../../../casaAPI/src/RSProxy.h"
%include "../../../casaAPI/src/RSProxyImpl.h"
%include "../../../casaAPI/src/RSProxySet.h"
%include "../../../casaAPI/src/RSProxySetImpl.h"
%include "../../../casaAPI/src/RunManager.h"
%include "../../../casaAPI/src/RunManagerImpl.h"
%include "../../../casaAPI/src/RunCase.h"
%include "../../../casaAPI/src/RunCaseImpl.h"
%include "../../../casaAPI/src/RunCaseSet.h"
%include "../../../casaAPI/src/RunCaseSetImpl.h"
%include "../../../casaAPI/src/TornadoSensitivityInfo.h"
%include "../../../casaAPI/src/SensitivityCalculator.h"
%include "../../../casaAPI/src/SensitivityCalculatorImpl.h"
%include "../../../casaAPI/src/ScenarioAnalysis.h"
%include "../../../casaAPI/src/VarParameter.h"
%include "../../../casaAPI/src/VarSpace.h"
%include "../../../casaAPI/src/VarSpaceImpl.h"
%include "../../../casaAPI/src/VarPrmContinuous.h"
%include "../../../casaAPI/src/VarPrmOneCrustThinningEvent.h"
%include "../../../casaAPI/src/VarPrmTopCrustHeatProduction.h"
%include "../../../casaAPI/src/VarPrmSourceRockProp.h"
%include "../../../casaAPI/src/VarPrmSourceRockTOC.h"
%include "../../../casaAPI/src/VarPrmSourceRockHI.h"
%include "../../../casaAPI/src/VarPrmSourceRockHC.h"
%include "../../../casaAPI/src/VarPrmSourceRockType.h"
%include "../../../casaAPI/src/VarPrmSourceRockPreAsphaltStartAct.h"
%include "../../../casaAPI/src/VarPrmPorosityModel.h"
%include "../../../casaAPI/src/VarPrmSurfacePorosity.h"
%include "../../../casaAPI/src/VarPrmPermeabilityModel.h"
%include "../../../casaAPI/src/VarPrmLithoSTPThermalCond.h"

%include "../../../FileSystem/src/Path.h"
%include "../../../FileSystem/src/FilePath.h"
%include "../../../FileSystem/src/FolderPath.h"

%include "../../../casaAPI/src/casaAPI.h"


%include <carrays.i>
%array_functions(double, doubleArray);


%include "../../../EosPack/src/EosPackCAPI.h"

// double array size of 1 for getting buble pressure for given temperature
%apply double OUTPUT[] { double * bubbleP }
%include "../../../EosPack/src/PTDiagramCalculator.h"

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

/// CASA API
%template(CasesList)          std::vector<      casa::RunCase*>;
%template(ConstCasesList)     std::vector<const casa::RunCase*>;

%template(CoefficientsMap)     std::map< std::vector< unsigned int >, std::pair<double,double> >;
%template(CoefficientsMapList) std::vector< std::map< std::vector< unsigned int>, std::pair<double,double> > >;
%template(PolynomCoefficient)  std::pair< std::vector< unsigned int >, std::pair<double,double> >;
%template(MCResults)           std::vector< std::pair<double,casa::RunCase*> >;
%template(MCSamplingPoint)     std::pair<double,RunCase*>;

/// CASA API SensitivityCalculator data types
%template(TornadoSensitivityData) std::vector<casa::TornadoSensitivityInfo>;
%template(VarParametersList)      std::vector<const casa::VarParameter*>;
%template(IntVector)              std::vector<int>;
