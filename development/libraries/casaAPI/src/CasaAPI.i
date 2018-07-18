/* File : BasinModeling.CasaAPI.i */
%module CasaAPI

%{
// Interface for APIs library
// CMB API
#include "../../utilities/src/FormattingException.h"
#include "../../cmbAPI/src/UndefinedValues.h"
#include "../../cmbAPI/src/ErrorHandler.h"
#include "../../cmbAPI/src/LithologyManager.h"
#include "../../cmbAPI/src/FluidManager.h"
#include "../../cmbAPI/src/ReservoirManager.h"
#include "../../cmbAPI/src/BiodegradeManager.h"
#include "../../cmbAPI/src/SourceRockManager.h"
#include "../../cmbAPI/src/StratigraphyManager.h"
#include "../../cmbAPI/src/FluidManagerImpl.h"
#include "../../cmbAPI/src/ReservoirManagerImpl.h"
#include "../../cmbAPI/src/BiodegradeManagerImpl.h"
#include "../../cmbAPI/src/LithologyManagerImpl.h"
#include "../../cmbAPI/src/SourceRockManagerImpl.h"
#include "../../cmbAPI/src/StratigraphyManagerImpl.h"
#include "../../cmbAPI/src/SnapshotManager.h"
#include "../../cmbAPI/src/SnapshotManagerImpl.h"
#include "../../cmbAPI/src/PropertyManager.h"
#include "../../cmbAPI/src/PropertyManagerImpl.h"
#include "../../cmbAPI/src/MapsManager.h"
#include "../../cmbAPI/src/MapsManagerImpl.h"
#include "../../cmbAPI/src/cmbAPI.h"
// CASA API
#include "CauldronApp.h"
#include "DataDigger.h"
#include "DataDiggerImpl.h"
#include "DoEGenerator.h"
#include "DoEGeneratorImpl.h"
#include "JobScheduler.h"
#include "JobSchedulerLocal.h"
#include "JobSchedulerLSF.h"
#include "MonteCarloSolver.h"
#include "MonteCarloSolverImpl.h"
#include "Observable.h"
#include "ObsValue.h"
#include "ObsGridPropertyXYZ.h"
#include "ObsGridPropertyWell.h"
#include "ObsValueDoubleArray.h"
#include "ObsValueDoubleScalar.h"
#include "ObsSpace.h"
#include "ObsSpaceImpl.h"
#include "Parameter.h"
#include "ParetoSensitivityInfo.h"
#include "PrmCrustThinning.h"
#include "PrmOneCrustThinningEvent.h"
#include "PrmTopCrustHeatProduction.h"
#include "PrmSourceRockProp.h"
#include "PrmSourceRockTOC.h"
#include "PrmSourceRockHI.h"
#include "PrmSourceRockHC.h"
#include "PrmSourceRockType.h"
#include "PrmSourceRockPreAsphaltStartAct.h"
#include "PrmPorosityModel.h"
#include "PrmSurfacePorosity.h"
#include "PrmPermeabilityModel.h"
#include "PrmLithoSTPThermalCond.h"
#include "RSProxy.h"
#include "RSProxyImpl.h"
#include "RSProxySet.h"
#include "RSProxySetImpl.h"
#include "RunManager.h"
#include "RunManagerImpl.h"
#include "RunCase.h"
#include "RunCaseImpl.h"
#include "RunCaseSet.h"
#include "RunCaseSetImpl.h"
#include "TornadoSensitivityInfo.h"
#include "SensitivityCalculator.h"
#include "SensitivityCalculatorImpl.h"
#include "ScenarioAnalysis.h"
#include "VarParameter.h"
#include "VarSpace.h"
#include "VarSpaceImpl.h"
#include "VarPrmContinuous.h"
#include "VarPrmCategorical.h"
#include "VarPrmDiscrete.h"
#include "VarPrmCrustThinning.h"
#include "VarPrmOneCrustThinningEvent.h"
#include "VarPrmTopCrustHeatProduction.h"
#include "VarPrmSourceRockProp.h"
#include "VarPrmSourceRockTOC.h"
#include "VarPrmSourceRockHI.h"
#include "VarPrmSourceRockHC.h"
#include "VarPrmSourceRockType.h"
#include "VarPrmSourceRockPreAsphaltStartAct.h"
#include "VarPrmPorosityModel.h"
#include "VarPrmSurfacePorosity.h"
#include "VarPrmPermeabilityModel.h"
#include "VarPrmLithoSTPThermalCond.h"

// #include "../../FileSystem/src/Path.h"
// #include "../../FileSystem/src/FilePath.h"
// #include "../../FileSystem/src/FolderPath.h"

#include "casaAPI.h"

using namespace casa;
%}

%include "../../swig-common/StdHelper.i"
%include <std_shared_ptr.i>
%include <typemaps.i>
%include <arrays_csharp.i>
%include <enums.swg>

//%csconst(1);

/// Shared pointer types wrapping
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
%csconstvalue("LithologyManager.PermeabilityModel.PermSandstone")   Sandstone;
%csconstvalue("LithologyManager.PermeabilityModel.PermMudstone")    Mudstone;
%csconstvalue("LithologyManager.PermeabilityModel.PermNone")        None;
%csconstvalue("LithologyManager.PermeabilityModel.PermImpermeable") Impermeable;
%csconstvalue("LithologyManager.PermeabilityModel.PermMultipoint")  Multipoint;
%csconstvalue("LithologyManager.PermeabilityModel.PermUnknown")     Unknown;

CSHARP_ARRAYS(char, byte)
 
// CASA Observable.h:newObsValueFromDoubles()
%apply int  &INOUT { int & off };

// CASA ScenarioAnalysis.h:loadScenario()
%apply char INPUT[] { const char * stateFileBuf }


// Interface for APIs library
// CMB API

%include "../../utilities/src/FormattingException.h"
%include "../../cmbAPI/src/ErrorHandler.h"
%include "../../cmbAPI/src/UndefinedValues.h"
%include "../../cmbAPI/src/LithologyManager.h"
%include "../../cmbAPI/src/FluidManager.h"
%include "../../cmbAPI/src/ReservoirManager.h"
%include "../../cmbAPI/src/BiodegradeManager.h"
%include "../../cmbAPI/src/SourceRockManager.h"
%include "../../cmbAPI/src/StratigraphyManager.h"
%include "../../cmbAPI/src/FluidManagerImpl.h"
%include "../../cmbAPI/src/ReservoirManagerImpl.h"
%include "../../cmbAPI/src/BiodegradeManagerImpl.h"
%include "../../cmbAPI/src/LithologyManagerImpl.h"
%include "../../cmbAPI/src/SourceRockManagerImpl.h"
%include "../../cmbAPI/src/StratigraphyManagerImpl.h"
%include "../../cmbAPI/src/SnapshotManager.h"
%include "../../cmbAPI/src/SnapshotManagerImpl.h"
%include "../../cmbAPI/src/PropertyManager.h"
%include "../../cmbAPI/src/PropertyManagerImpl.h"
%include "../../cmbAPI/src/MapsManager.h"
%include "../../cmbAPI/src/MapsManagerImpl.h"
%include "../../cmbAPI/src/cmbAPI.h"
// CASA API
%include "CauldronApp.h"
%include "ObsValue.h"
%include "Observable.h"
%include "ObsSpace.h"
%include "ObsSpaceImpl.h"
%include "ObsGridPropertyXYZ.h"
%include "ObsGridPropertyWell.h"
%include "ObsValueDoubleArray.h"
%include "ObsValueDoubleScalar.h"
%include "Parameter.h"
%include "PrmCrustThinning.h"
%include "PrmOneCrustThinningEvent.h"
%include "PrmTopCrustHeatProduction.h"
%include "PrmSourceRockProp.h"
%include "PrmSourceRockTOC.h"
%include "PrmSourceRockHI.h"
%include "PrmSourceRockHC.h"
%include "PrmSourceRockType.h"
%include "PrmSourceRockPreAsphaltStartAct.h"
%include "PrmPorosityModel.h"
%include "PrmSurfacePorosity.h"
%include "PrmPermeabilityModel.h"
%include "PrmLithoSTPThermalCond.h"
%include "VarParameter.h"
%include "VarPrmContinuous.h"
%include "VarPrmCategorical.h"
%include "VarPrmDiscrete.h"
%include "VarPrmCrustThinning.h"
%include "VarPrmOneCrustThinningEvent.h"
%include "VarPrmTopCrustHeatProduction.h"
%include "VarPrmSourceRockProp.h"
%include "VarPrmSourceRockTOC.h"
%include "VarPrmSourceRockHI.h"
%include "VarPrmSourceRockHC.h"
%include "VarPrmSourceRockType.h"
%include "VarPrmSourceRockPreAsphaltStartAct.h"
%include "VarPrmPorosityModel.h"
%include "VarPrmSurfacePorosity.h"
%include "VarPrmPermeabilityModel.h"
%include "VarPrmLithoSTPThermalCond.h"
%include "VarSpace.h"
%include "VarSpaceImpl.h"
%include "RunCase.h"
%include "RunCaseImpl.h"
%include "RunCaseSet.h"
%include "RunCaseSetImpl.h"
%include "DataDigger.h"
%include "DataDiggerImpl.h"
%include "DoEGenerator.h"
%include "DoEGeneratorImpl.h"
%include "JobScheduler.h"
%include "JobSchedulerLocal.h"
%include "JobSchedulerLSF.h"
%include "RSProxy.h"
%include "RSProxyImpl.h"
%include "RSProxySet.h"
%include "RSProxySetImpl.h"
%include "RunManager.h"
%include "RunManagerImpl.h"
%include "ParetoSensitivityInfo.h"
%include "TornadoSensitivityInfo.h"
%include "SensitivityCalculator.h"
%include "SensitivityCalculatorImpl.h"
%include "MonteCarloSolver.h"
%include "MonteCarloSolverImpl.h"
%include "ScenarioAnalysis.h"

%include "casaAPI.h"

%template(CasesList)              std::vector<           casa::RunCase*>;
%template(ConstCasesList)         std::vector<const      casa::RunCase*>;
%template(SharedCasesList)        std::vector<shared_ptr<casa::RunCase> >;
%template(CoefficientsMap)        std::map<std::vector< unsigned int >, std::pair<double,double> >;
%template(CoefficientsMapList)    std::vector<std::map< std::vector< unsigned int>, std::pair<double,double> > >;
%template(PolynomCoefficient)     std::pair<std::vector< unsigned int >, std::pair<double,double> >;
%template(MCResults)              std::vector< std::pair<double,casa::RunCase*> >;
%template(MCSamplingPoint)        std::pair<double,casa::RunCase*>;
%template(SharedParameterList)    std::vector< shared_ptr<casa::Parameter> >;

/// CASA API SensitivityCalculator data types
%template(TornadoSensitivityData) std::vector<casa::TornadoSensitivityInfo>;
%template(VarParametersList)      std::vector<const casa::VarParameter*>;
