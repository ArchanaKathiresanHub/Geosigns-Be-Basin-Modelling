//
// Copyright (C) 2015-2016 Shell International Exploration & Production.
// All rights reserved.
//
// Developed under license for Shell by PDS BV.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//
#include <sstream>
#include <stdlib.h>

#include "propinterface.h"
#include "readproperties.h"
#include "ghost_array.h"
#include "utils.h"
#include "AllochthonousLithologyManager.h"
#include "temperature_solver.h"
#include "HydraulicFracturingManager.h"
#include "database.h"
#include "cauldronschema.h"
#include "cauldronschemafuncs.h"
#include "FastcauldronSimulator.h"
#include "FastcauldronFactory.h"
#include "PressureSolver.h"
#include "LogHandler.h"

using namespace database;

#include "layer_iterators.h"
#include "GeoPhysicalFunctions.h"
#include "NumericFunctions.h"
#include "Quadrature.h"

using namespace Basin_Modelling;

// Data access library
#include "GridMap.h"
#include "Grid.h"
#include "Snapshot.h"
#include "Surface.h"
#include "LithoType.h"
#include "Formation.h"
#include "CrustFormation.h"
#include "MantleFormation.h"
#include "PaleoProperty.h"
#include "PropertyValue.h"
#include "Property.h"
#include "PaleoSurfaceProperty.h"
#include "PaleoFormationProperty.h"
#include "RunParameters.h"
#include "Interface.h"

#include "GeoPhysicsCrustFormation.h"
#include "GeoPhysicsMantleFormation.h"

#include "CrustFormation.h"
#include "MantleFormation.h"
#include "Lithology.h"

#include "Subdomain.h"
#include "MultiComponentFlowHandler.h"
#include "FastcauldronSimulator.h"
#include "FolderPath.h"

// utilities library
#include "ConstantsNumerical.h"
using Utilities::Numerical::CauldronNoDataValue;
using Utilities::Numerical::IbsNoDataValue;
using Utilities::Numerical::MaxLineSize;
#include "ConstantsMathematics.h"
using Utilities::Maths::PaToMegaPa;
using Utilities::Maths::MegaPaToPa;
using Utilities::Maths::Zero;
#include "ConstantsPhysics.h"
using Utilities::Physics::AccelerationDueToGravity;

using namespace std;
#ifdef _MSC_VER
#define sleep(x) Sleep(1000 * x) // convert from to milliseconds
#endif /** _MSC_VER */
//------------------------------------------------------------//


//------------------------------------------------------------//

AppCtx::AppCtx(int argc, char** argv) : filterwizard(&timefilter)
{
   m_saveOnDarcyError = false;
   m_no2Doutput = false;
   m_onlyPrimaryOutput = false;

   Reference_DA_For_Io_Maps = 0;

   currentCalculationMode = NO_CALCULATION_MODE;

   // SetGridWindow (OFF);
   mapDA = &Reference_DA_For_Io_Maps;

   CheckForStartInDebugger (&argc, &argv);

   m_exitAtAgeSet = false;
   m_exitAtAge = CauldronNoDataValue;


#if 0
   PetscLogEventRegister (&Vec_Allocate, "Vec_Allocate", 0);
#endif

   m_useTemisRelPerm = PETSC_FALSE;

   m_doOutputAtAge = PETSC_FALSE;
   m_ageToOutput = CauldronNoDataValue;

   xCoarseGridPartitioning = 0;
   yCoarseGridPartitioning = 0;

   m_minimumTimeStep = Minimum_Pressure_Time_Step;
   m_maximumTimeStep = DefaultMaximumTimeStep;
   m_fixedTimeStep   = 0.0; // used from command-line to overwrite other settings
   m_cflTimeStepping = false;
   m_burialRateTimeStepping = false;
   m_elementFraction = DefaultElementBurialFraction;
   m_elementErosionFraction = DefaultElementErosionFraction;
   m_elementHiatusFraction = DefaultElementHiatusFraction;
   m_vesScaling = DefaultVesScalingForFctInitialisation;

   m_fixedPermafrostTimeStep = 0.0;
   m_permafrostTimeStep = 0.0;
   m_permafrost = false;
   m_permafrostCurrentInd = 0;

}

bool AppCtx::readProjectName () {

   bool projectFileReadOkay = true;

   char fname[MaxLineSize];
   fname[0] = '\0';

   char fname1[MaxLineSize];
   fname1[0] = '\0';

   char dirExtension[MaxLineSize];
   dirExtension[0] = '\0';

   PetscBool hasProject   = PETSC_FALSE;
   PetscOptionsGetString (PETSC_IGNORE, PETSC_IGNORE, "-project", fname, MaxLineSize, 0);
   PetscOptionsHasName(PETSC_IGNORE, PETSC_IGNORE, "-project", &hasProject );

   projectFileReadOkay = projectFileReadOkay && setProjectFileName (fname);

   if ( ! projectFileReadOkay ) {
     return false;
   }

   PetscOptionsGetString (PETSC_IGNORE, PETSC_IGNORE, "-save", fname1, MaxLineSize, 0);
   setFastCauldronProjectFileName (fname1);

   return projectFileReadOkay;
}

bool AppCtx::readProjectFile () {

   bool projectFileReadOkay = true;

   // Read Project File
   projectFileReadOkay = projectFileReadOkay && ReadProjectFile::Read (this);

   // Synchronise 'projectFileReadOkay' with all other processes.
   projectFileReadOkay = successfulExecution ( projectFileReadOkay );

   if ( !projectFileReadOkay ) {
     return false;
   }

   setLayerData ();

   projectSnapshots.setMinorSnapshotsPrescribed ( IsCalculationCoupled && DoTemperature );

   if (( DoOverPressure || Do_Iteratively_Coupled || ( !IsCalculationCoupled && DoTemperature ) || DoDecompaction )) {

     // What can be deleted here?
     // Only if we are starting a new run (overpressure, hydrostatic-temperature,
     // pt-coupled or decompaction) can the minorsnapshot file be deleted.

     // Since we are starting a new calculation we can delete any minor snapshots from a previous overpressure or coupled run.
     projectSnapshots.deleteMinorSnapshotFiles ( getOutputDirectory ());
     projectSnapshots.deleteMajorSnapshotFiles ( getOutputDirectory ());
   }

   // If the calculation mode is HIGH-RES decompaction then the minor snapshot times must not be changed.
   if ( DoDecompaction ) {

     // If we are doing decompaction or non-coupled hydrostatic temperature, then there are no minor snapshots
     // The snapshot table SHOULD NOT be updated here, since these may still be required for any possible re-runs of GenEx.
     projectSnapshots.clearMinorSnapshots ();
   } else if ( !DoHighResDecompaction ) {

     // Now that the set of major snapshot's has been assigned if there are no
     // existing minorsnapshots then they must be computed.
      projectSnapshots.initialiseMinorSnapshotVector ( FastcauldronSimulator::getInstance ().getMcfHandler ().solveFlowEquations ());
   }

   // If we are not doing an iteratively coupled model then there is no
   // need for the advective/convective term, as the overpressure is zero.
   if ( includeAdvectiveTerm && ( !Do_Iteratively_Coupled && !IsCalculationCoupled )) {
     includeAdvectiveTerm = false;
   }

   // If we are incorporating the advective/convective term, then we cannot use
   // the CG solver, as the term adds a non-symmetric part to the matrix. CG is
   // only guaranteed for sym +ve def systems.
   if ( includeAdvectiveTerm ) {
     Temperature_Linear_Solver_Type = KSPGMRES;
   } else {
     Temperature_Linear_Solver_Type = KSPCG;
   }

   // The restart value may be needed for both when the linear solver fails
   // and when the advective term is enabled.
   Temperature_GMRes_Restart      = 120;

   m_burialRateTimeStepping = FastcauldronSimulator::getInstance ().getRunParameters ()->useBurialRateTimeStepping ();

   if ( m_burialRateTimeStepping ) {
      timestepincr = 1.0;
      timestepdecr = 1.0;
      m_elementFraction = 1.0 / FastcauldronSimulator::getInstance ().getRunParameters ()->getSegmentFractionToBury ();

      if ( debug1 || verbose) {
         PetscPrintf ( PETSC_COMM_WORLD, " Using burial rate for time-stepping, element fraction: %f.\n", m_elementFraction );
      }

   }

   return projectFileReadOkay;
}


void AppCtx::CheckForStartInDebugger(int *argc, char ***args)
{

  PetscBool Start_In_CVD      = PETSC_FALSE;
  PetscBool Start_In_WORKSHOP = PETSC_FALSE;
  PetscBool Start_In_DDD      = PETSC_FALSE;

  std::string Debug_Command;
  std::string Process_Id = IntegerToString( Utilities::CheckMemory::getProcPID() );

  int  left    = *argc - 1;
  char **eargs = *args + 1;

   while (left)
   {

     PetscStrcmp( eargs[ 0 ], "-cvd", &Start_In_CVD );
     PetscStrcmp( eargs[ 0 ], "-workshop", &Start_In_WORKSHOP );
     PetscStrcmp( eargs[ 0 ], "-ddd", &Start_In_DDD );

     if ( Start_In_CVD )
      {
       cout << "Attaching cvd to " << *args[0] << " (pid: " << Process_Id << ") Please wait..." << endl;
       Debug_Command = "cvd -pid " + Process_Id + "&";

       system( Debug_Command.c_str() );
       sleep( 10 );
     }
     else if ( Start_In_WORKSHOP )
     {
       cout << "Attaching cvd to " << *args[ 0 ] << " (pid: " << Process_Id << ") Please wait..." << endl;
       Debug_Command = "workshop -D " + std::string( *args[ 0 ] ) + " " + Process_Id + "&";

       system( Debug_Command.c_str() );
      sleep(10);
    }
     else if ( Start_In_DDD )
     {

        char *MY_DEBUGGERDIR = getenv("MY_DEBUGGERDIR");

        if ( MY_DEBUGGERDIR != 0 && !File_Exists ( MY_DEBUGGERDIR )) {
           PetscPrintf ( PETSC_COMM_WORLD, " Environment variable MY_DEBUGGERDIR (=%s) does not point to anything, using system default.\n", MY_DEBUGGERDIR );
        }

        PetscSynchronizedPrintf ( PETSC_COMM_WORLD,"Attaching ddd to %s, with pid: %s. Please wait ...\n",
                                  *args[ 0 ], Process_Id.c_str ());

        // cout << "Attaching ddd to " << *args[ 0 ] << " (pid: " << Process_Id << ") Please wait..." << endl;

        if ( MY_DEBUGGERDIR == 0 || strlen (MY_DEBUGGERDIR) == 0 || !File_Exists ( MY_DEBUGGERDIR )) {
           Debug_Command = "ddd " + std::string( *args[ 0 ] ) + " " + Process_Id + "&";
        } else {
           Debug_Command = std::string ( MY_DEBUGGERDIR ) + ' ' + std::string( *args[ 0 ] ) + " " + Process_Id + "&";
        }

       system( Debug_Command.c_str() );
       sleep(10);
     }

      eargs++;
      left--;
  }

}

//------------------------------------------------------------//

const Boolean2DArray& AppCtx::getValidNeedles () const {
   return m_nodeIsDefined;
}

//------------------------------------------------------------//

double AppCtx::minimumTimeStep () const {
   return m_minimumTimeStep;
}

//------------------------------------------------------------//

double AppCtx::maximumTimeStep () const {
   return m_maximumTimeStep;
}
//------------------------------------------------------------//

double AppCtx::fixedTimeStep () const {
   return m_fixedTimeStep;
}

//------------------------------------------------------------//

double AppCtx::permafrostTimeStep () const {
   return m_permafrostTimeStep;
}

//------------------------------------------------------------//

bool AppCtx::cflTimeStepping () const {
   return m_cflTimeStepping;
}

//------------------------------------------------------------//

bool AppCtx::useBurialRateTimeStepping () const {
   return m_burialRateTimeStepping;
}

//------------------------------------------------------------//

bool AppCtx::permafrost () const {

   return m_permafrost;
}

//------------------------------------------------------------//

void AppCtx::setPermafrost () {

   m_permafrost = true;
   restartPermafrost ();
}

//------------------------------------------------------------//
void AppCtx::restartPermafrost () {

  m_permafrostCurrentInd = ( m_permafrostAges.size() != 0 ? m_permafrostAges.size() - 1 : 0 );
  m_permafrostTimeStep = 0.0;
}

//------------------------------------------------------------//

bool AppCtx::switchPermafrostTimeStep ( const double Current_Time ) {

  if( Current_Time == 0.0 ) {
    // Restart permafrost index
    restartPermafrost();
    return true;
  }
   // for every permafrost age the correspondent time step is being activated
   if( m_permafrostAges.size() != 0 ) {
      if( Current_Time > m_permafrostAges[m_permafrostCurrentInd] && m_permafrostTimeStep == 0 ) {
         // permafrost is not activated yet
         return false;
      }
      if( Current_Time <= m_permafrostAges[m_permafrostCurrentInd] ) {
         // the next (or the first) permafrost age is reached => activate the next time step
         m_permafrostTimeStep = ( m_fixedPermafrostTimeStep > 0 ? m_fixedPermafrostTimeStep : m_permafrostTimeSteps[m_permafrostCurrentInd] );
         -- m_permafrostCurrentInd;
      }
   }
   // if all permafrost ages have been reached (m_permafrostAges[m_permafrostCurrentInd] = 0 (present day)),
   // then continue with the last calculated time step = m_permafrostTimeStep until the present day
   return true;
}
//------------------------------------------------------------//

void AppCtx::adjustTimeStepToPermafrost ( const double Previous_Time, double & Current_Time ) {

  if( m_permafrost && fixedTimeStep() == 0.0 ) {
     // we want to start permafrost time stepping when the first permafrost event occurs, so adjust (step back) the current_time if it reaches the permafrost age
     // in order to start permafrost time-stepping exactly at the permafrost start age.
     if( Current_Time <= getNextPermafrostAge()) {
        const double nextPermafrostTimeStep = getNextPermafrostTimeStep();
        const double timeStepBack = ( Previous_Time - Current_Time ) - nextPermafrostTimeStep;
        if( timeStepBack > 0.0001 ) {
           Current_Time = Current_Time + NumericFunctions::Minimum ( timeStepBack, nextPermafrostTimeStep );
       }
     }
  }
}

//------------------------------------------------------------//

double AppCtx::getNextPermafrostTimeStep () const {

   return ( m_fixedPermafrostTimeStep > 0 ? m_fixedPermafrostTimeStep :
            ( m_permafrostTimeSteps.size() != 0 ? m_permafrostTimeSteps[m_permafrostCurrentInd] : 0.0 ));

}
//------------------------------------------------------------//

double AppCtx::getNextPermafrostAge () const {

   return ( m_permafrostAges.size() != 0 ? m_permafrostAges[m_permafrostCurrentInd] : 0.0 );

}

//------------------------------------------------------------//
double AppCtx::getPreviousIgneousIntrusionTime( const double Current_Time ) const {

   return FastcauldronSimulator::getInstance().getPreviousIgneousIntrusionTime( Current_Time );

}

//------------------------------------------------------------//

void AppCtx::Set_Crust_Layer ( CauldronCrustFormation* Pointer )
{
   Crust_Layer = Pointer;
}

void AppCtx::Set_Mantle_Layer ( CauldronMantleFormation* Pointer )
{
   Mantle_Layer = Pointer;
}

//------------------------------------------------------------//

void AppCtx::printHelp () const {

  std::stringstream helpBuffer;

  helpBuffer << endl;
  helpBuffer << endl;
  helpBuffer << "Usage: mpirun -np <procs> fastcauldron [-help] [-options]" << endl;
  helpBuffer << "The command line options include:" << endl << endl;
  helpBuffer << "    -help                  Shows this help message" << endl;
  helpBuffer << endl;
  helpBuffer << "  Basic solvers:" << endl;
  helpBuffer << "    -decompaction          Runs decompaction." << endl;
  helpBuffer << "    -temperature           Solve for the temperature using a hydrostatic pressure." << endl;
  helpBuffer << "    -coupled               To run a loosely coupled calculation, this must be either temperature or decompaction." << endl;
  helpBuffer << "    -nonlinear             For use in temperature or coupled calculation. Uses non-linear temperature calculator." << endl;
  helpBuffer << "    -overpressure          Solve for the overpressure using a linear temperature gradient." << endl;
  helpBuffer << "    -itcoupled             Solve the coupled pressure-temperature." << endl;
  helpBuffer << "    -genex                 Include GenEx5 calculation." << endl;
  // helpBuffer << "    -maps                  Saves output files in Cauldron Map Format (HDF)." << endl;
  // helpBuffer << "    -volumes               Saves output files in FastCauldron Volume Format (HDF)." << endl;
  helpBuffer << "    -project <filename>    Name of project file." << endl;
  helpBuffer << "    -save <filename>       Name of project file in which to save input and results." << endl;
  helpBuffer << endl;
  helpBuffer << endl;

  // The following are 'expert' options.
  helpBuffer << "  General options:" << endl;
  helpBuffer << endl;
  helpBuffer << "    -printcl                    Print the command line that was used when running the simulation." << endl;
  helpBuffer << "    -readfct                    Before running an overperssure calculation read-in the fct-correction factors from a previous overpressure" << endl
             << "                                run (not working)." << endl;
  helpBuffer << "    -numberminorss <n>          Number of minor-snapshots, n >= 0." << endl;
  helpBuffer << "    -verbosity <level>          Verbosity level of the log file(s): minimal|normal|detailed|diagnostic. Default value is 'normal'." << endl;
  helpBuffer << endl;
  helpBuffer << endl;

  // The following are 'expert' options.
  helpBuffer << "  Expert user options:" << endl;
  helpBuffer << endl;
  helpBuffer << "    -nohdfoutput                Do not output any data at snapshot times." << endl;
  helpBuffer << "    -nonlintol <tol>            Over-ride the tolerance of the pressure Newton solver, tol > 0.0." << endl;
  helpBuffer << "    -nonlinits <n>              Over-ride the maximum number of iterations used in the pressure Newton solver, n > 0." << endl;
  helpBuffer << "    -newtolerances              Use tighter tolerances for linear and non-linear solver for both pressure and temperature and maximum number of Newton iterations for the pressure." << endl;

  helpBuffer << "    -glfctweight <lambda>       The weighting of the current and previous fct-correction factors, lambda \\in (0,1], default: 1." << endl;

  helpBuffer << "    -fracmodel <n>              Over-ride the fracture pressure model, n = 1, 2, ..., 5." << endl;
  helpBuffer << "    -allowsolverchange          Allow the solver to be changed during the pressure solving process." << endl;
  helpBuffer << "    -allowilufillinc            Allow the ilu fill level to increase during Newton iteration if conditions arise." << endl;
  helpBuffer << "    -its2changeilufill <n>      Over-ride the iterations for changing the ilu fill level, n >= 1 (typically 80 < n < 1000)(Default n = 80)." << endl;
  helpBuffer << "    -recompjacevery <n>         Allow reuse of the Jacobian in the pressure calculation." << endl;
  helpBuffer << "    -relperm <method>           Use the <method> relative permeability function, where method \\in { none, temis, annette }, " << endl
             << "                                Default = annette." << endl
             << "                                If none is selected then rel-perm = 1.0" << endl;
  helpBuffer << "    -minhcsat <val>             The minimum hc-saturation value, for use in Temis type rel-perm (satex), val \\ in (0.0,1.0), " << endl
             << "                                default: " << DefaultMinimumHcSaturation << endl;
  helpBuffer << "    -minwatersat <val>          The minimum water-saturation value, for use in Temis type rel-perm (1-satir), val \\ in (0.0,1.0), " << endl
             << "                                default: " << DefaultMinimumWaterSaturation << endl;
  helpBuffer << "    -hcexpo <val>               The curve exponent of the rel-perm for both liquid and vapour, for use in Temis type rel-perm (pow), default: " << DefaultHcCurveExponent << endl;
  helpBuffer << "    -hcliqexpo <val>            The curve exponent of the rel-perm of liquid, for use in Temis type rel-perm (pow), default: " << DefaultHcCurveExponent << endl;
  helpBuffer << "    -hcvapexpo <val>            The curve exponent of the rel-perm of vapour, for use in Temis type rel-perm (pow), default: " << DefaultHcCurveExponent << endl;
  helpBuffer << "    -waterexpo <val>            The curve exponent of the rel-perm of water, for use in Temis type rel-perm (pwo), dfault: " << DefaultWaterCurveExponent << endl;

  helpBuffer << "    -temisviscosity             Use the TemisPack brine-viscosity function." << endl;
  helpBuffer << "    -refinenamedforms <form1,ref1,form2,ref2,...>" << endl
             << "                                Refine a selected set of the formations, formations are indicated by name, ref_i > 0." << endl;
  helpBuffer << "    -refinenumberedforms <form1,ref1,form2,ref2,...>" << endl
             << "                                Refine a selected set of the formations, formations are indicated by position from top, ref_i > 0." << endl;
  helpBuffer << "    -fcvesscale <val>           VES scaling factor when initialising the solid-thicknesses in the geometric loop pressure calculation, default value is " << DefaultVesScalingForFctInitialisation << endl;
  helpBuffer << "    -minor                      Runs high resolution decompaction at major and minor snapshot times." << endl;

  helpBuffer << endl;
  helpBuffer << endl;
  helpBuffer << "  Element quadrature:" << endl;
  helpBuffer << endl;
  helpBuffer << "    -pressplanequadrature <n>   Over-ride Gauss Legendre quadrature degree in plane for pressure solver, 1 <= n <= "
             << NumericFunctions::Quadrature::MaximumQuadratureDegree << "." << endl;
  helpBuffer << "    -pressdepthquadrature <n>   Over-ride Gauss Legendre quadrature degree in depth for pressure solver, 1 <= n <= "
             << NumericFunctions::Quadrature::MaximumQuadratureDegree << "." << endl;
  helpBuffer << "    -tempplanequadrature  <n>   Over-ride Gauss Legendre quadrature degree in plane for temperature solver, 1 <= n <= "
             << NumericFunctions::Quadrature::MaximumQuadratureDegree << "." << endl;
  helpBuffer << "    -tempdepthquadrature  <n>   Over-ride Gauss Legendre quadrature degree in depth for temperature solver, 1 <= n <= "
             << NumericFunctions::Quadrature::MaximumQuadratureDegree << "." << endl;

  // helpBuffer << "    -readfct                  Before running an overperssure calculation read-in the fct-correction factors from a previous overpressure run" << endl;
  // helpBuffer << "                              0 < x < 1 => maximum element height is smaller than user defined mantle-element-height." << endl;
  // helpBuffer << "                              1 < x     => elements larger than user defined mantle-element-height." << endl;
  // helpBuffer << "                              x = 1     => elements larger than user defined mantle-element-height." << endl;
  helpBuffer << endl;
  helpBuffer << endl;
  helpBuffer << "  Time step control:" << endl;
  helpBuffer << endl;
  helpBuffer << "    -mints <ts>                 Over-ride the default minimum time-step, ts > 0.0 (default minimum time-step = " << Minimum_Pressure_Time_Step << " Ma)." << endl;
  helpBuffer << "    -maxts <ts>                 Over-ride the default maximum time-step, ts > 0.0 (default maximum time-step = " << DefaultMaximumTimeStep << " Ma)." << endl;
  helpBuffer << "    -fixts <ts>                 Fix the time-step size, ts > 0.0." << endl;
  helpBuffer << "    -cflts                      Use the CFL condition for the time-stepping." << endl;
  helpBuffer << "    -brts                       Use the burial rate to control the time-stepping." << endl;
  helpBuffer << "    -brfrac <frac>              Fraction of element to bury to control the time-stepping, default is "
             << DefaultElementBurialFraction << ", must be use in combination with -brts." << endl;
  helpBuffer << "    -erfrac <frac>              Fraction of element to erode to control the time-stepping, default is "
             << DefaultElementBurialFraction << ", must be use in combination with -brts." << endl;
  helpBuffer << "                                If -brfrac is specified and not -erfrac, the -brfrac value will be used." << endl;

  helpBuffer << endl;
  helpBuffer << endl;
  helpBuffer << "  Lateral stress:" << endl;
  helpBuffer << endl;
  helpBuffer << "    -lateralstress <filename>   Include lateral stresses in calculation." << endl;
  helpBuffer << "                                The file must consist of only 2 columns {age stress-factor}, separated by a space, e.g." << endl;
  helpBuffer << endl;
  helpBuffer << "                                             100.0 0.0" << endl;
  helpBuffer << "                                              75.0 1.0" << endl;
  helpBuffer << "                                              50.0 1.0" << endl;
  helpBuffer << "                                              49.9 0.0" << endl;
  helpBuffer << "                                               0.0 0.0" << endl;
  helpBuffer << endl;
  helpBuffer << endl;
  helpBuffer << endl;
  helpBuffer << endl;
  helpBuffer << "  Multi-component multi-phase flow solver options:" << endl;
  helpBuffer << endl;
  helpBuffer << MultiComponentFlowHandler::getCommandLineOptions ();

  helpBuffer << "  Permafrost modelling options:" << endl;
  helpBuffer << "           -permafrost <ts>            Enable permafrost modelling and set a time-step size to be used (if defined), ts > 0.0." << endl;

  helpBuffer << endl;

  helpBuffer << "  Parallel I/O options:" << endl;
  helpBuffer << "           -primaryPod <dir>           Use dir to store imtermediate output files. Dir should be a shared dir on the cluster (or local)" << endl;
  helpBuffer << "           -primaryDouble              Output only primary properties in double precision." << endl;
  helpBuffer << "           -allproperties              Output all properties (selected in FilterTimeIoTbl) and not just primary." << endl;

  helpBuffer << endl;

  helpBuffer << "  Matrix and RHS save to file:" << endl;
  helpBuffer << "           -saveMatrix <timeStep>      At provided time step (or the next closest one) the FEM matrix and RHS is saved to file." << endl;
  helpBuffer << "           -matlab                     [optional] Output files in matlab format (default is binary)." << endl;

  helpBuffer << endl;

  helpBuffer << "  Linear solver settings:" << endl;
  helpBuffer << "           -disableHypre               Disable HYPRE BoomerAMG preconditioner as default fallback in case of linear solver divergence" << endl;
  helpBuffer << "                                       due to maximum number of iterations reached." << endl;

  helpBuffer << endl;
  helpBuffer << endl;

  PetscPrintf ( PETSC_COMM_WORLD, helpBuffer.str ().c_str ());

}

#undef __FUNCT__
#define __FUNCT__ "AppCtx::getCommandLineOptions"

bool AppCtx::getCommandLineOptions() {

  char Output_Level_Str [MaxLineSize];

  PetscBool Use_Non_Geometric_Loop = PETSC_FALSE;

  PetscBool ShowHelp   = PETSC_FALSE;
  PetscBool MapType    = PETSC_FALSE;
  PetscBool VolumeType = PETSC_FALSE;
  PetscBool Dummy      = PETSC_FALSE;
  PetscBool outputNotRequired = PETSC_FALSE;
  PetscBool outputAgeChanged = PETSC_FALSE;
  PetscBool exitAgeChanged = PETSC_FALSE;
  PetscBool bbtemp = PETSC_FALSE;
  PetscBool Found;
  double outputAge;
  double exitAge;
  PetscBool saveResultsIfDarcyError = PETSC_FALSE;
  int ierr;

  IsCalculationCoupled = PETSC_FALSE;

  PetscFunctionBegin;

  ierr = PetscOptionsHasName(PETSC_IGNORE, PETSC_IGNORE,"-help",&ShowHelp); CHKERRQ(ierr);
  if (ShowHelp) {
    printHelp ();
    ierr = PetscFinalize(); CHKERRQ(ierr);
    exit(0);
  }

  ierr = PetscOptionsHasName(PETSC_IGNORE, PETSC_IGNORE, "-cvd",&Dummy); CHKERRQ(ierr);
  ierr = PetscOptionsHasName(PETSC_IGNORE, PETSC_IGNORE, "-debug1",&debug1); CHKERRQ(ierr);
  ierr = PetscOptionsHasName(PETSC_IGNORE, PETSC_IGNORE, "-debug2",&debug2); CHKERRQ(ierr);
  ierr = PetscOptionsHasName(PETSC_IGNORE, PETSC_IGNORE, "-debug3",&debug3); CHKERRQ(ierr);

  ierr = PetscOptionsHasName(PETSC_IGNORE, PETSC_IGNORE, "-verbose", &verbose); CHKERRQ(ierr);

  ierr = PetscOptionsHasName(PETSC_IGNORE, PETSC_IGNORE, "-decompaction",&DoDecompaction); CHKERRQ(ierr);
  ierr = PetscOptionsHasName(PETSC_IGNORE, PETSC_IGNORE, "-hrdecompaction", &DoHighResDecompaction); CHKERRQ (ierr);
  ierr = PetscOptionsHasName(PETSC_IGNORE, PETSC_IGNORE, "-temperature",&DoTemperature); CHKERRQ(ierr);

  ierr = PetscOptionsHasName(PETSC_IGNORE, PETSC_IGNORE, "-genex",&doGenex); CHKERRQ(ierr);

  ierr = PetscOptionsHasName(PETSC_IGNORE, PETSC_IGNORE, "-itcoupled", &Do_Iteratively_Coupled ); CHKERRQ(ierr);
  ierr = PetscOptionsHasName(PETSC_IGNORE, PETSC_IGNORE, "-overpressure",&DoOverPressure); CHKERRQ(ierr);

  ierr = PetscOptionsHasName (PETSC_IGNORE, PETSC_IGNORE, "-nonlinear", &Nonlinear_Temperature ); CHKERRQ(ierr);
  ierr = PetscOptionsHasName (PETSC_IGNORE, PETSC_IGNORE, "-readfct",&readFCTCorrectionFactor ); CHKERRQ(ierr);

  ierr = PetscOptionsHasName(PETSC_IGNORE, PETSC_IGNORE, "-bbtemp", &bbtemp ); CHKERRQ(ierr);

  ierr = PetscOptionsHasName(PETSC_IGNORE, PETSC_IGNORE, "-nohdfoutput",&outputNotRequired ); CHKERRQ(ierr);
  PetscOptionsGetReal (PETSC_IGNORE, PETSC_IGNORE, "-onlyat", &outputAge, &outputAgeChanged );

  PetscOptionsGetReal (PETSC_IGNORE, PETSC_IGNORE, "-exitat", &exitAge, &exitAgeChanged );

  ierr = PetscOptionsHasName(PETSC_IGNORE, PETSC_IGNORE, "-saveonerror", &saveResultsIfDarcyError ); CHKERRQ(ierr);

  if ( saveResultsIfDarcyError ) {
     m_saveOnDarcyError = true;
  } else {
     m_saveOnDarcyError = false;
  }

  if ( exitAgeChanged == PETSC_TRUE ) {
     m_exitAtAgeSet = true;
     m_exitAtAge = exitAge;
  }

  if(outputAgeChanged)
  {
     m_doOutputAtAge = PETSC_TRUE;
     m_ageToOutput = outputAge;
  }

  bottomBasaltTemp = PetscBool ( bbtemp );

  if( bottomBasaltTemp ) {
     PetscPrintf ( PETSC_COMM_WORLD, "Turning on basalt temperature in the lower elements only!" );
  }

  DoHDFOutput = PetscBool ( ! outputNotRequired );

  if ( DoDecompaction )
  {
    currentCalculationMode = HYDROSTATIC_DECOMPACTION_MODE;
  }
  else if ( DoHighResDecompaction )
  {

     IsCalculationCoupled = FastcauldronSimulator::getInstance ().getLastPTWasCoupled ();

     if ( !IsCalculationCoupled ) {
        currentCalculationMode = HYDROSTATIC_HIGH_RES_DECOMPACTION_MODE;
     } else {
        currentCalculationMode = COUPLED_HIGH_RES_DECOMPACTION_MODE;
     }

  }
  else if ( DoTemperature )
  {
     PetscBool coupledParameter;
     ierr = PetscOptionsHasName(PETSC_IGNORE, PETSC_IGNORE, "-coupled",&coupledParameter ); CHKERRQ(ierr);

     IsCalculationCoupled = ( coupledParameter == PETSC_TRUE );

     if ( !IsCalculationCoupled ) {
        currentCalculationMode = HYDROSTATIC_TEMPERATURE_MODE;
     } else {
        currentCalculationMode = OVERPRESSURED_TEMPERATURE_MODE;
     }

  }
  else if ( DoOverPressure )
  {
    currentCalculationMode = OVERPRESSURE_MODE;
    IsCalculationCoupled = PETSC_TRUE;
  }
  else if ( Do_Iteratively_Coupled )
  {
    currentCalculationMode = PRESSURE_AND_TEMPERATURE_MODE;
    IsCalculationCoupled = PETSC_TRUE;
  }
  else
  {
    currentCalculationMode = NO_CALCULATION_MODE;
  }


  //
  //
  // Using geometric loop will be the default, so a -ngl argument must be
  // passed to fastcauldron for running pressure using the non geometric loop
  // option. To make things slightly easier to read (and understand) we use
  // the "positive" Use_Geometric_Loop variable.
  //
  ierr = PetscOptionsHasName(PETSC_IGNORE, PETSC_IGNORE,"-ngl",&Use_Non_Geometric_Loop); CHKERRQ(ierr);
  Use_Geometric_Loop = ! Use_Non_Geometric_Loop;

  if ( debug1 || verbose) {
    PetscPrintf( PETSC_COMM_WORLD, " Read FCT scaling factors %i \n", int ( readFCTCorrectionFactor ));
  }

  //
  //
  // Is it worth having parameters to control the Newton solver parameters: tolerance, minimum number of iterations,
  // initial jacobian iterations, compute-jacobian-every-n iterations, maximum number of iterations. And the thickness
  // tolerance.
  //
  //  -fc_newt_tol
  //  -fc_newt_min_its
  //  -fc_newt_max_its
  //  -fc_newt_init_jac_its
  //  -fc_newt_jac_every_nits
  //  -fc_thick_tol
  //

  Output_Level_Str [0] = '\0';

  PetscOptionsGetString (PETSC_IGNORE, PETSC_IGNORE, "-outputlevel", Output_Level_Str, MaxLineSize, &Found );

  if ( Found ) {
    Output_Level = atoi ( Output_Level_Str );
  } else {
    Output_Level = 0;
  }

  PetscFunctionReturn(NO_ERROR);
}

//------------------------------------------------------------//

void AppCtx::setAdditionalCommandLineParameters () {

   PetscBool pressurePlaneDegreeChanged;
   int        pressurePlaneDegree;

   PetscBool pressureDepthDegreeChanged;
   int        pressureDepthDegree;

   PetscBool temperaturePlaneDegreeChanged;
   int        temperaturePlaneDegree;

   PetscBool temperatureDepthDegreeChanged;
   int        temperatureDepthDegree;

   PetscBool newtonToleranceChanged;
   double     newtonTolerance;

   PetscBool newtonIterationsChanged;
   int        newtonIterations;

   PetscBool fractureModelChanged;
   int        fractureModel;

   PetscBool iterationsForIluFillLevelUpdateChanged;
   int        iterationsForIluFillLevelUpdate;

   PetscBool minimumTimeStepChanged;
   double newMinimumTimeStep;

   PetscBool maximumTimeStepChanged;
   double newMaximumTimeStep;

   PetscBool fixedTimeStepChanged;
   double newFixedTimeStep;

   PetscBool petscBurialRateTimeStepping = PETSC_FALSE;
   PetscBool petscCflTimeStepping = PETSC_FALSE;

   PetscBool petscPermafrost = PETSC_FALSE;
   PetscBool petscPermafrostTimeStep = PETSC_FALSE;

   PetscBool petscBurialRateFraction = PETSC_FALSE;
   double elementFraction;

   PetscBool petscErosionRateFraction = PETSC_FALSE;
   double elementErosionFraction;

   PetscBool vesScaleChanged = PETSC_FALSE;
   double vesScale;

   PetscBool hasLateralStressFile = PETSC_FALSE;
   char lateralStressFileName [ MaxLineSize ];

   PetscBool setNewTolerances = PETSC_FALSE;

   PetscOptionsHasName (PETSC_IGNORE, PETSC_IGNORE, "-newtolerances", &setNewTolerances );

   if ( setNewTolerances ) {
      PressureSolver::setMaximumNumberOfNonlinearIterations ( Optimisation_Level, DefaultMaximumNumberOfNonlinearIterations );
      PressureSolver::setNewtonSolverTolerance ( Optimisation_Level, 1, DefaultPressureNewtonSolverToleranceFirstIteration );
      PressureSolver::setNewtonSolverTolerance ( Optimisation_Level, 2, DefaultPressureNewtonSolverToleranceSecondIteration );
      PressureSolver::setNewtonSolverTolerance ( Optimisation_Level, 3, DefaultPressureNewtonSolverToleranceThirdIteration );
      PressureSolver::setLinearSolverTolerance ( Optimisation_Level, DefaultLinearSolverTolerance );

      Temperature_Solver::setNewtonSolverTolerance ( Optimisation_Level, DefaultTemperatureNewtonSolverTolerance );
      Temperature_Solver::setLinearSolverTolerance ( Optimisation_Level, DefaultLinearSolverTolerance );
   }

   PetscOptionsGetString (PETSC_IGNORE, PETSC_IGNORE, "-lateralstress", lateralStressFileName, MaxLineSize, &hasLateralStressFile );
   PetscOptionsGetInt  (PETSC_IGNORE, PETSC_IGNORE, "-pressplanequadrature", &pressurePlaneDegree, &pressurePlaneDegreeChanged );
   PetscOptionsGetInt  (PETSC_IGNORE, PETSC_IGNORE, "-pressdepthquadrature", &pressureDepthDegree, &pressureDepthDegreeChanged );
   PetscOptionsGetInt  (PETSC_IGNORE, PETSC_IGNORE, "-tempplanequadrature",  &temperaturePlaneDegree, &temperaturePlaneDegreeChanged );
   PetscOptionsGetInt  (PETSC_IGNORE, PETSC_IGNORE, "-tempdepthquadrature",  &temperatureDepthDegree, &temperatureDepthDegreeChanged );
   PetscOptionsGetReal (PETSC_IGNORE, PETSC_IGNORE, "-nonlintol", &newtonTolerance, &newtonToleranceChanged );
   PetscOptionsGetInt  (PETSC_IGNORE, PETSC_IGNORE, "-nonlinits", &newtonIterations, &newtonIterationsChanged );
   PetscOptionsGetInt  (PETSC_IGNORE, PETSC_IGNORE, "-fracmodel", &fractureModel, &fractureModelChanged );
   PetscOptionsGetInt  (PETSC_IGNORE, PETSC_IGNORE, "-its2changeilufill", &iterationsForIluFillLevelUpdate, &iterationsForIluFillLevelUpdateChanged );
   PetscOptionsGetInt  (PETSC_IGNORE, PETSC_IGNORE, "-recompjacevery", &pressureJacobianReuseCount, &allowPressureJacobianReuse );
   PetscOptionsHasName (PETSC_IGNORE, PETSC_IGNORE, "-allowilufillinc", &allowIluFillIncrease );
   PetscOptionsHasName (PETSC_IGNORE, PETSC_IGNORE, "-allowsolverchange", &allowSolverChange );
   PetscOptionsGetReal (PETSC_IGNORE, PETSC_IGNORE, "-mints", &newMinimumTimeStep, &minimumTimeStepChanged );
   PetscOptionsGetReal (PETSC_IGNORE, PETSC_IGNORE, "-maxts", &newMaximumTimeStep, &maximumTimeStepChanged );
   PetscOptionsGetReal (PETSC_IGNORE, PETSC_IGNORE, "-fixts", &newFixedTimeStep, &fixedTimeStepChanged );
   PetscOptionsHasName (PETSC_IGNORE, PETSC_IGNORE, "-cflts", &petscCflTimeStepping );
   PetscOptionsHasName (PETSC_IGNORE, PETSC_IGNORE, "-brts", &petscBurialRateTimeStepping );
   PetscOptionsGetReal (PETSC_IGNORE, PETSC_IGNORE, "-brfrac", &elementFraction, &petscBurialRateFraction );
   PetscOptionsGetReal (PETSC_IGNORE, PETSC_IGNORE, "-erfrac", &elementErosionFraction, &petscErosionRateFraction );
   PetscOptionsGetReal (PETSC_IGNORE, PETSC_IGNORE, "-fcvesscale", &vesScale, &vesScaleChanged );

   PetscOptionsHasName (PETSC_IGNORE, PETSC_IGNORE, "-permafrost", &petscPermafrost );
   PetscOptionsGetReal (PETSC_IGNORE, PETSC_IGNORE, "-permafrost", &m_fixedPermafrostTimeStep, &petscPermafrostTimeStep );

  if ( petscBurialRateTimeStepping ) {
     m_burialRateTimeStepping = bool ( petscBurialRateTimeStepping );

     timestepincr = 1.0;
     timestepdecr = 1.0;

     if ( petscBurialRateFraction ) {
        m_elementFraction = elementFraction;
     } else {
        m_elementFraction = DefaultElementBurialFraction;
     }

     if ( petscErosionRateFraction ) {
        m_elementErosionFraction = elementErosionFraction;
     } else {
        m_elementErosionFraction = DefaultElementErosionFraction;
     }

     if ( debug1 || verbose ) {
        PetscPrintf ( PETSC_COMM_WORLD, " Using burial rate for time-stepping, element fraction: burial = %f, erosion = %f.\n", m_elementFraction, m_elementErosionFraction );
     }


  }

  m_cflTimeStepping = bool ( petscCflTimeStepping );

  if ( m_cflTimeStepping && ( debug1 || verbose ) ) {
     PetscPrintf ( PETSC_COMM_WORLD, " Using the CFL condition to help determine the pressure time-step.\n" );
  }

  if ( allowPressureJacobianReuse && ( debug1 || verbose ) ) {
     PetscPrintf ( PETSC_COMM_WORLD, " Allowing the pressure Jacobian to be re-used during the Newton solve.\n" );
  }

  if ( vesScaleChanged ) {

     if ( !NumericFunctions::inRange ( vesScale, 0.0, 1.0 )) {
        PetscPrintf ( PETSC_COMM_WORLD, " Basin_Warning: ves scaling factor, %f, is not in interval [ 0.0 .. 1.0 ], some clipping will occur.\n", vesScale );
     }

     m_vesScaling = NumericFunctions::clipValueToRange<double> ( vesScale, 0.0, 1.0 );
     // m_vesScaling = NumericFunctions::Maximum ( 0.0, NumericFunctions::Minimum ( 1.0, vesScale ));
  }

  if ( pressurePlaneDegreeChanged ) {
    pressurePlaneDegree = NumericFunctions::Maximum ( 1, NumericFunctions::Minimum<int> ( pressurePlaneDegree, NumericFunctions::Quadrature::MaximumQuadratureDegree ));
    PressureSolver::setPlaneQuadratureDegree ( Optimisation_Level, pressurePlaneDegree );

    if ( debug1 || verbose ) {
      PetscPrintf ( PETSC_COMM_WORLD, " Setting pressure plane quadrature degree: %i\n", pressurePlaneDegree );
    }

  }

  if ( iterationsForIluFillLevelUpdateChanged ) {
    PressureSolver::setIterationsForIluFillLevelIncrease ( iterationsForIluFillLevelUpdate );

    if ( debug1 || verbose ) {
      PetscPrintf ( PETSC_COMM_WORLD, " Setting iterations to change the ilu fill-level: %i\n", iterationsForIluFillLevelUpdate );
    }

  }


  if ( pressureDepthDegreeChanged ) {
    pressureDepthDegree = NumericFunctions::Maximum ( 1, NumericFunctions::Minimum<int> ( pressureDepthDegree, NumericFunctions::Quadrature::MaximumQuadratureDegree ));
    PressureSolver::setDepthQuadratureDegree ( Optimisation_Level, pressureDepthDegree );

    if ( debug1 || verbose ) {
      PetscPrintf ( PETSC_COMM_WORLD, " Setting pressure depth quadrature degree: %i\n", pressureDepthDegree );
    }

  }

  if ( temperaturePlaneDegreeChanged ) {
    temperaturePlaneDegree = NumericFunctions::Maximum ( 1, NumericFunctions::Minimum<int> ( temperaturePlaneDegree, NumericFunctions::Quadrature::MaximumQuadratureDegree ));
    Temperature_Solver::setPlaneQuadratureDegree ( Optimisation_Level, temperaturePlaneDegree );

    if ( debug1 || verbose ) {
      PetscPrintf ( PETSC_COMM_WORLD, " Setting temperature plane quadrature degree: %i\n", temperaturePlaneDegree );
    }

  }

  if ( temperatureDepthDegreeChanged ) {
    temperatureDepthDegree = NumericFunctions::Maximum ( 1, NumericFunctions::Minimum<int> ( temperatureDepthDegree, NumericFunctions::Quadrature::MaximumQuadratureDegree ));
    Temperature_Solver::setDepthQuadratureDegree ( Optimisation_Level, temperatureDepthDegree );

    if ( debug1 || verbose ) {
      PetscPrintf ( PETSC_COMM_WORLD, " Setting temperature depth quadrature degree: %i\n", temperatureDepthDegree );
    }

  }

  if ( newtonToleranceChanged ) {
    PressureSolver::setNewtonSolverTolerance ( Optimisation_Level, newtonTolerance );

    if ( debug1 || verbose ) {
      PetscPrintf ( PETSC_COMM_WORLD, " Setting Newton solver tolerance: %e\n", newtonTolerance );
    }

  }

  if ( newtonIterationsChanged ) {
    PressureSolver::setMaximumNumberOfNonlinearIterations ( Optimisation_Level, newtonIterations );

    if ( debug1 || verbose ) {
      PetscPrintf ( PETSC_COMM_WORLD, " Setting maximum number of iterations in Newton solver : %i\n", newtonIterations );
    }

  }


  if ( m_burialRateTimeStepping && !newtonToleranceChanged ) {
     PressureSolver::setNewtonSolverTolerance ( Optimisation_Level, 0.1 * PressureSolver::getNewtonSolverTolerance ( Optimisation_Level, false, 3 ));

#if 0
    if ( debug1 ) {
      PetscPrintf ( PETSC_COMM_WORLD, " Setting Newton solver tolerance: %e\n", newtonTolerance );
    }
#endif

  }

  // Set a new minimum time-step.
  if ( minimumTimeStepChanged ) {

     if ( newMinimumTimeStep > 0.0 ) {
        m_minimumTimeStep = newMinimumTimeStep;

        if ( debug1 || verbose ) {
           PetscPrintf ( PETSC_COMM_WORLD, " Overriding minimum time-step: %f\n", newMinimumTimeStep );
        }

     } else {
        PetscPrintf ( PETSC_COMM_WORLD, " ERROR  Minimum time-step is not a positive value: %f\n", newMinimumTimeStep );
     }


  }

  if ( maximumTimeStepChanged ) {

     if ( newMaximumTimeStep > 0.0 ) {
        m_maximumTimeStep = newMaximumTimeStep;

        if ( debug1 || verbose ) {
           PetscPrintf ( PETSC_COMM_WORLD, " Overriding maximum time-step: %f\n", newMaximumTimeStep );
        }

     } else {
        PetscPrintf ( PETSC_COMM_WORLD, " ERROR  Maximum time-step is not a positive value: %f\n", newMaximumTimeStep );
     }


  }

  if ( fixedTimeStepChanged ) {

     if ( newFixedTimeStep > 0.0 ) {
        m_maximumTimeStep = newFixedTimeStep;
        m_minimumTimeStep = newFixedTimeStep;
        m_fixedTimeStep   = newFixedTimeStep;

        if ( debug1 || verbose ) {
           PetscPrintf ( PETSC_COMM_WORLD, " Fixied time-step size: %f\n", newFixedTimeStep );
        }

     } else {
        PetscPrintf ( PETSC_COMM_WORLD, " ERROR  Fixed time-step size is not positive: %f\n", newFixedTimeStep );
     }


  }

  // Set a new fracture-pressure model.

  if ( fractureModelChanged ) {
    HydraulicFracturingManager::getInstance ().setModel ( fractureModel );
  }

  if ( debug1 || verbose ) {
    PetscPrintf ( PETSC_COMM_WORLD, " Fracture model: %s \n",
                  fracturePressureModelStr ( HydraulicFracturingManager::getInstance ().getModel ()).c_str ());
  }

  if ( hasLateralStressFile ) {

     if ( File_Exists ( lateralStressFileName )) {
        loadLateralStressFile ( lateralStressFileName );
     } else {
        PetscPrintf ( PETSC_COMM_WORLD, " Lateral stress file %s does not exist.\n", lateralStressFileName );
     }

  }

  if( petscPermafrost ) {
     if( ! FastcauldronSimulator::getInstance ().getPermafrost() ) {
        FastcauldronSimulator::getInstance ().setPermafrost ( bool( petscPermafrost ));


        if( FastcauldronSimulator::getInstance ().determinePermafrost(  m_permafrostTimeSteps, m_permafrostAges )) {

           setPermafrost();
           PetscPrintf ( PETSC_COMM_WORLD, "Permafrost is on.\n");
        }
     }
     if( petscPermafrostTimeStep && FastcauldronSimulator::getInstance ().getPermafrost() ) {
        if( m_fixedPermafrostTimeStep > 0 ) {
           PetscPrintf ( PETSC_COMM_WORLD, "Overriding permafrost fixed time-step: %lf\n", m_fixedPermafrostTimeStep );
        }
     }
  }
}

//------------------------------------------------------------//

void AppCtx::computeLowPermeabilitySubdomains ( const double lowPermeability ) {

   unsigned int i;
   double perm;

   for ( i = 0; i < layers.size () - 2; ++i ) {
      perm = layers [ i ]->estimateStandardPermeability ();
      cout << " Layer " << layers [ i ]->layername << " has perm " << perm << endl;
   }

}

//------------------------------------------------------------//

void AppCtx::loadLateralStressFile ( const std::string& lateralStressFileName ) {

   std::ifstream stressFile ( lateralStressFileName.c_str ());

   ibs::Interpolator* interp = new ibs::Interpolator;
   double time;
   double lateralStressFactor;
   bool   pointAdded = false;

   while ( stressFile.good ()) {
      stressFile >> time;

      if ( stressFile.good ()) {
         stressFile >> lateralStressFactor;
         interp->addPoint ( time, lateralStressFactor );
         pointAdded = true;

         // if ( debug1 )
         cout << " lateral stres factors: " << time << "  " << lateralStressFactor << endl;
      }

   }

   if ( pointAdded ) {
      FastcauldronSimulator::getInstance ().addLateralStressInterpolator ( interp );
   } else {
      delete interp;
   }

   stressFile.close ();
}

//------------------------------------------------------------//

void AppCtx::setParametersFromEnvironment () {

  char* chemicalCompactionVesValue = getenv ( "CHEMICAL_COMPACTION_VES_VALUE" );
  char* strEnd;
  size_t i;
  double vesValue;
  bool chemicalCompactionVesValueWellDefined;

  if ( chemicalCompactionVesValue != 0 ) {
    vesValue = strtod ( chemicalCompactionVesValue, &strEnd );

    if ( strEnd == chemicalCompactionVesValue ) {
      PetscPrintf ( PETSC_COMM_WORLD,
                    " Basin_Warning:  CHEMICAL_COMPACTION_VES_VALUE environment variable is not set correctly.\n" );
      PetscPrintf ( PETSC_COMM_WORLD,
                    " Basin_Warning:  Cannot convert value '%s' to a double.\n",
                    chemicalCompactionVesValue );
      PetscPrintf ( PETSC_COMM_WORLD,
                    " Basin_Warning:  Chemical compaction will not be affected by the environment variable.\n" );
    } else {
      chemicalCompactionVesValueWellDefined = (( strEnd == 0 ) || ( strlen ( strEnd ) == 0 ));

      if ( chemicalCompactionVesValueWellDefined ) {
        PetscPrintf ( PETSC_COMM_WORLD,
                      " Basin_Warning:  CHEMICAL_COMPACTION_VES_VALUE environment variable is set correctly: %s.\n",
                      chemicalCompactionVesValue );
        PetscPrintf ( PETSC_COMM_WORLD,
                      " Basin_Warning:  Setting ves to value: %5.5f.\n",
                      vesValue );
        PetscPrintf ( PETSC_COMM_WORLD,
                      " Basin_Warning:  These warning messages serve only as a reminder that the environment variable is set to a valid value.\n" );
      } else {
        PetscPrintf ( PETSC_COMM_WORLD,
                      " Basin_Warning:  CHEMICAL_COMPACTION_VES_VALUE environment variable is set incorrectly: %s.\n",
                      chemicalCompactionVesValue );
        PetscPrintf ( PETSC_COMM_WORLD,
                      " Basin_Warning:  However setting ves to value: %5.5f. This was extracted from the leading substring of the environment variable.\n",
                      vesValue );
        PetscPrintf ( PETSC_COMM_WORLD,
                      " Basin_Warning:  The trailing substring of the environment variable is: %s. For correct execution please remove this trailing substring.\n",
                      strEnd );
        PetscPrintf ( PETSC_COMM_WORLD,
                      " Basin_Warning:  These warning messages serve only as a reminder that the environment variable is set to a valid value.\n" );
      }

      for ( i = 0; i < layers.size() - 2; ++i ) {
        layers [ i ]->setChemicalCompactionVesValue ( vesValue );
      }

    }

  }

}

//------------------------------------------------------------//

#undef __FUNCT__
#define __FUNCT__ "AppCtx::setProjectFileName"

bool AppCtx::setProjectFileName(const std::string& projectFileName) {

  bool returnStatus;

  m_ProjectFileName = projectFileName;

  if ( m_ProjectFileName.length() == 0 ) {
    PetscPrintf( PETSC_COMM_WORLD, " Basin_Error: project file name is empty. \n");
    returnStatus = false;
  } else if ( ! File_Exists ( projectFileName )) {
    PetscPrintf( PETSC_COMM_WORLD, " Basin_Error: project file '%s' does not exist. \n",
                 projectFileName.c_str ());
    returnStatus = false;
  } else {
    returnStatus = true;
  }

  return returnStatus;
}

#undef __FUNCT__
#define __FUNCT__ "AppCtx::setFastCauldronProjectFileName"

void AppCtx::setFastCauldronProjectFileName(std::string projectName)
{
   if (projectName.length () == 0)
   {
      m_FastCauldronProjectFileName = getProjectFileName ();
   }
   else
   {
      m_FastCauldronProjectFileName = projectName;
   }
}

string AppCtx::Get_Project_Filename_Base ( )
{

  std::string Project_Filename_Base = m_ProjectFileName;

  removeExtension( Project_Filename_Base );

  return Project_Filename_Base;
}


#undef __FUNCT__
#define __FUNCT__ "AppCtx::getOutputDirectory"

string AppCtx::getOutputDirectory ()
{
   return ibs::FolderPath( FastcauldronSimulator::getInstance ().getFullOutputDir () ).path();
}

#undef __FUNCT__
#define __FUNCT__ "AppCtx::getOutputPath"

string AppCtx::getOutputPath ()
{
   return ibs::FolderPath( FastcauldronSimulator::getInstance ().getOutputDir () ).path();
}

#undef __FUNCT__
#define __FUNCT__ "AppCtx::makeOutputDirectory"

bool AppCtx::makeOutputDirectory ()
{
   return FastcauldronSimulator::getInstance ().makeOutputDir ();
}

void AppCtx::setRelatedProject ( const int indexI, const int indexJ ) {

   const CauldronGridDescription& grid = FastcauldronSimulator::getInstance ().getCauldronGridDescription ();

   if ( indexI >= 0 && indexJ >= 0 ) {
      Related_Project_Ptr project1D = new Related_Project;


      project1D->X_Coord = grid.originI + indexI * grid.deltaI;
      project1D->Y_Coord = grid.originJ + indexJ * grid.deltaJ;
      project1D->Name = "from command line index_"  + IntegerToString ( indexI ) + "  " + IntegerToString ( indexJ );
      Related_Projects.push_back ( project1D );
   }

}

void AppCtx::setRelatedProject ( const double locationX, const double locationY ) {

   const CauldronGridDescription& grid = FastcauldronSimulator::getInstance ().getCauldronGridDescription ();

   Related_Project_Ptr project1D = new Related_Project;

   project1D->X_Coord = locationX;
   project1D->Y_Coord = locationY;
   project1D->Name = "from command line point_"  + IntegerToString ( int ( project1D->X_Coord )) + "  " + IntegerToString ( int ( project1D->Y_Coord ));
   Related_Projects.push_back ( project1D );

}

bool AppCtx::setRelatedProject ( const Interface::RelatedProject* relatedProject ) {
   if ( relatedProject == 0 ) {
      return false;
   } else {
      Related_Project_Ptr project1D = new Related_Project;

      project1D->X_Coord = relatedProject->getEast ();
      project1D->Y_Coord = relatedProject->getNorth ();

      if ( relatedProject->getName ().find ( "*Point" ) != std::string::npos ) {
         project1D->Name = "Point_" + IntegerToString( int ( project1D -> X_Coord ))
            + "_east__" + IntegerToString ( int ( project1D -> Y_Coord )) + "_north";
      } else {
         std::string projectName = relatedProject->getName ();

         removeExtension ( projectName );
         project1D->Name = projectName;
      }

      Related_Projects.push_back ( project1D );
   }
   return true;
}

void AppCtx::addUndefinedAreas ( const Interface::GridMap* theMap ) {

   if ( theMap == 0 ) {
      return;
   }

   unsigned int i;
   unsigned int j;

   bool dataIsRetrieved = theMap->retrieved ();

   if ( !dataIsRetrieved ) {
      theMap->retrieveGhostedData ();
   }

   for ( i = (unsigned int)m_nodeIsDefined.firstI ( true ); i <= (unsigned int)m_nodeIsDefined.lastI ( true ); ++i ) {

      for ( j = (unsigned int)m_nodeIsDefined.firstJ ( true ); j <= (unsigned int)m_nodeIsDefined.lastJ ( true ); ++j ) {

         if ( theMap->getValue ( i, j ) == theMap->getUndefinedValue ()) {
            m_nodeIsDefined ( i, j ) = false;
         }

      }

   }

   if ( !dataIsRetrieved ) {
      // If the data was not retrived then restore the map back to its original state.
      theMap->restoreData ( false, true );
   }


}

void AppCtx::setValidNodeArray () {

   if ( debug1 ) {
      PetscPrintf( PETSC_COMM_WORLD, "o Setting-up value node array and element array..." );
   }

   Layer_Iterator basinLayers;

   m_nodeIsDefined.create ( *mapDA );

   unsigned int i;
   unsigned int j;

   // Copy the contents of the valid-node array from the project-handle. This should be temporary.
   for ( i = FastcauldronSimulator::getInstance ().firstI ( true ); i <= FastcauldronSimulator::getInstance ().lastI ( true ); ++i ) {

      for ( j = FastcauldronSimulator::getInstance ().firstJ ( true ); j <= FastcauldronSimulator::getInstance ().lastJ ( true ); ++j ) {
         m_nodeIsDefined ( i, j ) = FastcauldronSimulator::getInstance ().getNodeIsValid ( i, j );
      }

   }

   if ( currentCalculationMode == HYDROSTATIC_DECOMPACTION_MODE or
        currentCalculationMode == NO_CALCULATION_MODE ) {
      return;
   } else if ( currentCalculationMode == COUPLED_HIGH_RES_DECOMPACTION_MODE ) {

      // Any of the output properties that appear in the snapshot files could be used here,
      // since all of them will have the same undefined areas.
      const Interface::Property* fctCorrection = FastcauldronSimulator::getInstance ().findProperty ( "FCTCorrection" );

      const Interface::Snapshot* presentDay = FastcauldronSimulator::getInstance ().findSnapshot ( 0.0, Interface::MAJOR );

      // Here any formation will do, since all will have the same undefnied areas.
      // So pick the top one, since there is always a top formation.
      Interface::PropertyValueList* fctValueList = FastcauldronSimulator::getInstance ().getPropertyValues ( Interface::FORMATION,
                                                                                                             fctCorrection,
                                                                                                             presentDay,
                                                                                                             0,
                                                                                                             layers [ 0 ],
                                                                                                             0,
                                                                                                             Interface::MAP );

      const Interface::PropertyValue* fct = *fctValueList->begin ();

      assert ( fctValueList->size () == 1 );

      if ( fct != 0 && fct->getGridMap () != 0 ) {
         addUndefinedAreas ( fct->getGridMap ());
      }

      delete fctValueList;

   }

   shareValidNeedleData ();

   // Set up the element-index-exists set.
   // Can this be moved to below the if statement.
   setUp2dEltMapping ();

   m_nodeIsDefined.fill ( false );

   vector < Elt2dIndices >::const_iterator elementIter;
   int node;

   for ( elementIter = mapElementList.begin (); elementIter != mapElementList.end (); ++elementIter ) {
      const Elt2dIndices& element = *elementIter;

      if ( element.exists ) {

         for ( node = 0; node < 4; ++node ) {
            int iPos = element.i[ node ];
            int jPos = element.j[ node ];
            m_nodeIsDefined ( iPos, jPos ) = true;
         }

      }

   }

   if ( debug1 ) {
      PetscPrintf(PETSC_COMM_WORLD, " DONE\n");
   }

   shareValidNeedleData ();
}

void AppCtx::shareValidNeedleData () {

   bool IncludeGhostValues = true;

   int i;
   int j;

   Vec definedNodeVec;
   DMDALocalInfo dainfo;

   DMDAGetLocalInfo ( *mapDA, &dainfo );
   DMCreateGlobalVector ( *mapDA, &definedNodeVec );
   VecSet ( definedNodeVec, Zero );

   PETSC_2D_Array definedNode ( *mapDA, definedNodeVec, INSERT_VALUES, IncludeGhostValues );


   for ( i = dainfo.gxs; i < dainfo.gxs + dainfo.gxm; ++i ) {

      for ( j = dainfo.gys; j < dainfo.gys + dainfo.gym; ++j ) {
         definedNode ( j, i ) = ( m_nodeIsDefined ( i, j ) ? 1.0 : 0.0 );
      }

   }

   definedNode.Restore_Global_Array ( Update_Including_Ghosts );

   definedNode.Set_Global_Array ( *mapDA, definedNodeVec, INSERT_VALUES, IncludeGhostValues );


   for ( i = dainfo.gxs; i < dainfo.gxs + dainfo.gxm; ++i ) {

      for ( j = dainfo.gys; j < dainfo.gys + dainfo.gym; ++j ) {
         m_nodeIsDefined ( i, j ) = definedNode ( j, i ) > 0.0;
      }

   }

   VecDestroy( &definedNodeVec );
}

void AppCtx::printValidNeedles () const {

   const bool includeGhostNodes = false;

   int p;
   int i;
   int j;

   for ( p = 0; p < FastcauldronSimulator::getInstance ().getSize (); ++p ) {

      if ( p == FastcauldronSimulator::getInstance ().getRank () ) {
         std::stringstream buffer;
         buffer << " Distribution for process: " << p << endl;

         buffer << "        ";

         for ( i = m_nodeIsDefined.firstI ( includeGhostNodes ); i <= m_nodeIsDefined.lastI ( includeGhostNodes ); ++i ) {
            buffer << " " << i;
         }

         buffer << endl;

         for ( j = m_nodeIsDefined.lastJ ( includeGhostNodes ); j >= m_nodeIsDefined.firstJ ( includeGhostNodes ); --j ) {
            buffer << " row " << j << "  ";

            for ( i = m_nodeIsDefined.firstI ( includeGhostNodes ); i <= m_nodeIsDefined.lastI ( includeGhostNodes ); ++i ) {
               buffer << " " << ( m_nodeIsDefined ( i, j ) ? "x" : "." );
            }

            buffer << endl;
         }

         buffer << endl;
         cout << buffer.str () << endl;
      }

      MPI_Barrier(PETSC_COMM_WORLD);
   }

}

#undef __FUNCT__
#define __FUNCT__ "AppCtx::Get_FCT_Correction"
bool AppCtx::Get_FCT_Correction () {

   if ( ! IsCalculationCoupled && ! readFCTCorrectionFactor ) {
      return true;
   }

   if ( currentCalculationMode != OVERPRESSURED_TEMPERATURE_MODE and
        currentCalculationMode != COUPLED_HIGH_RES_DECOMPACTION_MODE and
        !readFCTCorrectionFactor ) {
      return true;
   }

   FastcauldronSimulator::getInstance ().applyFctCorrections ();
   return true;
}

//------------------------------------------------------------//

void AppCtx::setSnapshotInterval ( const SnapshotInterval& interval ) {

  Layer_Iterator basinLayers;

  basinLayers.Initialise_Iterator ( layers, Ascending, Sediments_Only, Active_Layers_Only );

  while ( ! basinLayers.Iteration_Is_Done () ) {
    basinLayers.Current_Layer ()->setSnapshotInterval ( interval, this );
    basinLayers++;
  }

}


void AppCtx::Print_Nodes_Value_From_Polyfunction( ) {

  int NX, NY, NZ;
  int IX, IY, IZ;
  int XS, YS;

  DMDAGetCorners( *mapDA, &XS, &YS, PETSC_IGNORE, &NX, &NY, PETSC_IGNORE );

  LayerProps_Ptr Current_Layer;
  Layer_Iterator Layers;

  Layers.Initialise_Iterator ( layers, Ascending, Sediments_Only, Active_And_Inactive_Layers );

  while ( ! Layers.Iteration_Is_Done () ) {

    Current_Layer = Layers.Current_Layer ();

    NZ = Current_Layer -> getMaximumNumberOfElements ();

    cout << endl;
    cout << Current_Layer->layername << endl;
    cout << endl;

    for ( IX = XS; IX < XS + NX; IX++ )
    {
      for ( IY = YS; IY < YS + NY; IY++ )
      {
   for ( IZ = 0; IZ < NZ; IZ++ )
   {
     cout << endl << "RT [" << IX << "," << IY << "," << IZ << "]" << endl;
     Current_Layer -> getRealThickness ( IX, IY, IZ ).printPoints();
   }
      }
    }
    Layers++;
  }

}


//------------------------------------------------------------//

void AppCtx::initialiseTimeIOTable ( const std::string& currentOperation ) {

  database::Table* localTimeIoTbl   = FastcauldronSimulator::getInstance ().getTable ("TimeIoTbl");
  database::Table* local3DTimeIoTbl = FastcauldronSimulator::getInstance ().getTable ("3DTimeIoTbl");

  if ( currentOperation == DecompactionRunStatusStr ||
       currentOperation == HydrostaticTemperatureRunStatusStr ||
       currentOperation == OverpressureRunStatusStr ||
       currentOperation == CoupledPressureTemperatureRunStatusStr ) {

    localTimeIoTbl->clear ();
    local3DTimeIoTbl->clear ();

  } else if ( currentOperation == HighResDecompactionRunStatusStr ||
              currentOperation == OverpressuredTemperatureRunStatusStr ) {

    clearOperationFromTimeIOTable ( localTimeIoTbl, currentOperation + "_Result.HDF" );
  }

}

//------------------------------------------------------------//

void AppCtx::clearOperationFromTimeIOTable ( database::Table* table, const std::string& mapFileName ) {

  std::vector < Record* > recordsForDeletion;
  std::vector < Record* >::iterator recordIterator;

  Table::iterator tblIter;

  for ( tblIter = table->begin (); tblIter != table->end (); ++tblIter ) {

    if ( getMapFileName ( *tblIter ) == mapFileName  ) {
      recordsForDeletion.push_back ( *tblIter );
    }

  }

  for ( recordIterator = recordsForDeletion.begin (); recordIterator != recordsForDeletion.end (); ++recordIterator ) {
    table->deleteRecord ( *recordIterator );
  }


}

//------------------------------------------------------------//

void AppCtx::deleteTimeIORecord ( const double  age )
{

  database::Table* timeIoTbl = FastcauldronSimulator::getInstance ().getTable ("TimeIoTbl");

   std::vector < Record * >recordsForDeletion;
   std::vector < Record * >::iterator recordIterator;

   Table::iterator tblIter;

   for (tblIter = timeIoTbl->begin (); tblIter != timeIoTbl->end (); ++tblIter)
   {
      if (getTime (*tblIter) == age)
      {
         recordsForDeletion.push_back (*tblIter);
      }
   }

   for (recordIterator = recordsForDeletion.begin (); recordIterator != recordsForDeletion.end (); ++recordIterator)
   {
      timeIoTbl->deleteRecord (*recordIterator);
   }
}

//------------------------------------------------------------//

void AppCtx::deleteTimeIORecord ( const std::string& propertyName,
                                  const double  age ) {


  database::Table* timeIoTbl = FastcauldronSimulator::getInstance ().getTable ("TimeIoTbl");
  std::vector < Record* > recordsForDeletion;
  std::vector < Record* >::iterator recordIterator;

  Table::iterator tblIter;

  for ( tblIter = timeIoTbl->begin (); tblIter != timeIoTbl->end (); ++tblIter ) {

    if ( getTime ( *tblIter ) == age && getPropertyName ( *tblIter ) == propertyName  ) {
      recordsForDeletion.push_back ( *tblIter );
    }

  }

  for ( recordIterator = recordsForDeletion.begin (); recordIterator != recordsForDeletion.end (); ++recordIterator ) {
    timeIoTbl->deleteRecord ( *recordIterator );
  }

}

//------------------------------------------------------------//

void AppCtx::deleteTimeIORecord ( const std::string& propertyName,
                                  const double  age,
                                  const std::string& surfaceName,
                                  const std::string& formationName ) {


  database::Table* timeIoTbl = FastcauldronSimulator::getInstance ().getTable ("TimeIoTbl");
  std::vector < Record* > recordsForDeletion;
  std::vector < Record* >::iterator recordIterator;

  Table::iterator tblIter;

  for ( tblIter = timeIoTbl->begin (); tblIter != timeIoTbl->end (); ++tblIter ) {

    if ( getTime ( *tblIter ) == age &&
         getPropertyName  ( *tblIter ) == propertyName &&
         getSurfaceName   ( *tblIter ) == surfaceName &&
         getFormationName ( *tblIter ) == formationName ) {
      recordsForDeletion.push_back ( *tblIter );
    }

  }

  for ( recordIterator = recordsForDeletion.begin (); recordIterator != recordsForDeletion.end (); ++recordIterator ) {
    timeIoTbl->deleteRecord ( *recordIterator );
  }

}

//------------------------------------------------------------//

//delete ALL properties defined on minor snaps
void AppCtx::deleteMinorSnapshotsFromTimeIOTable ( const snapshottimeContainer& savedMinorSnapshotTimes,
                                                   const PropListVec&           properties )
{
   SnapshotConstIterator ssIter;

   for (ssIter = savedMinorSnapshotTimes.rbegin (); ssIter != savedMinorSnapshotTimes.rend (); ++ssIter)
   {
      deleteTimeIORecord (*ssIter);
   }

}

//------------------------------------------------------------//


#undef __FUNCT__
#define __FUNCT__ "AppCtx::interpolateProperty"

void AppCtx::interpolateProperty (const double       currentTime,
                                   const PropertyIdentifier& property ) {

   Layer_Iterator basinLayers;
   basinLayers.Initialise_Iterator ( layers, Ascending, Sediments_Only, Active_Layers_Only );

   while ( ! basinLayers.Iteration_Is_Done ()) {
      basinLayers.Current_Layer ()->interpolateProperty ( this, currentTime, bool ( DoHighResDecompaction ), property );
      basinLayers++;
   }

}

//------------------------------------------------------------//


#undef __FUNCT__
#define __FUNCT__ "AppCtx::calcNodeVes"

bool AppCtx::calcNodeVes( const double time ) {

  /* local variables */
  int            i, j, k;
  int            ix, iy;
  int            xs, ys, zs;
  int            xm, ym, zm;
  bool           firstpass = true;
  double**       prev = NULL;
  LayerProps_Ptr Current_Layer;
  Layer_Iterator Layers;


  if ( IsCalculationCoupled )
  {
    interpolateProperty ( time, VES );
    return true;
  }

  Layers.Initialise_Iterator ( layers, Descending, Sediments_Only, Active_Layers_Only );

  while ( ! Layers.Iteration_Is_Done () ) {

    Current_Layer = Layers.Current_Layer ();

    DMDAGetCorners( Current_Layer -> layerDA, &xs, &ys, &zs, &xm, &ym, &zm );

    int Top_Z_Index = zm - 1;

    PETSC_3D_Array ves ( Current_Layer->layerDA, Current_Layer->Current_Properties ( Basin_Modelling::VES_FP ));

    if ( firstpass )
    {
      prev = Array<double>::create2d(xm,ym);
    }

    for ( i = xs; i < ( xs + xm ); i++)
    {
      for ( j = ys; j < ( ys + ym ); j++)
      {

         if ( !nodeIsDefined ( i, j )) continue;
         ix = i - xs; iy = j - ys;

        // Set the Top Surface Ves to Zero ( FirstPass = true )
        //or Copy Ves from Bottom Surface of Above Layer
        ves( Top_Z_Index,j,i ) = firstpass ? 0.0 : prev[ix][iy];

      }
    }

    firstpass = false;
    // determine if the switch permafrost is activated (imposing a constant hydrostatic pressure in the layer)
    const GeoPhysics::FluidType * fluid = dynamic_cast<const GeoPhysics::FluidType*>( Current_Layer->getFluidType() );
    bool switchPermaFrost = fluid->isPermafrostEnabled();

    // Set the rest of the values
    for ( i = xs; i < ( xs + xm ); i++ )
    {
       for ( j = ys; j < ( ys + ym ); j++ )
       {

          if ( !nodeIsDefined( i, j ) ) continue;

          ix = i - xs; iy = j - ys;

          double Density_Difference = Current_Layer->calcDiffDensity(i, j);

          for ( k = Top_Z_Index - 1; k >= zs; k--)
          {
             double solidThickness = Current_Layer->getSolidThickness(i, j, k, time);
             // Fluid is denser than rock and the permafrost switch is on
             if ( Density_Difference <= 0.0 && switchPermaFrost && solidThickness >= 0 )
             {    
                 double solidDensity = Current_Layer->getLithology(i, j, k)->density();
                 double surfacePorosity = Current_Layer->getLithology(i, j, k)->surfacePorosity();
                 ves( k, j, i ) = ves( k + 1, j, i ) +
                   AccelerationDueToGravity * solidDensity * solidThickness *( 1.0 - surfacePorosity );
             }
             else if (solidThickness >= 0 )
             {
                ves( k,j,i ) = ves( k+1,j,i ) + AccelerationDueToGravity * Density_Difference * solidThickness;
             }
             else
             {
                ves( k,j,i ) = ves( k+1,j,i );
             }

          }
       }
    }

    // Store bottom values
    for ( i = xs; i < ( xs + xm ); i++)
    {
      for ( j = ys; j < ( ys + ym ); j++)
      {
          if ( !nodeIsDefined ( i, j )) continue;
          ix = i - xs; iy = j - ys;
          prev[ix][iy] = ves( 0,j,i );
      }
    }

    Layers++;
  }

  if ( prev != NULL ) Array<double>::delete2d(prev);

  return true;

}

bool AppCtx::calcNodeMaxVes( const double time ) {

  /* local variables */
  int            i, j, k;
  int            xs, ys, zs;
  int            xm, ym, zm;
  double         dVes, Max_Ves;
  double         Density_Difference;
  bool           Segment_May_Be_Eroding;
  LayerProps_Ptr Current_Layer;
  Layer_Iterator Layers;

  if ( IsCalculationCoupled )
  {
    interpolateProperty ( time, MAXVES );
    return true;
  }

  Layers.Initialise_Iterator ( layers, Ascending, Sediments_Only, Active_Layers_Only );

  while ( ! Layers.Iteration_Is_Done () ) {

     Current_Layer = Layers.Current_Layer ();

     DMDAGetCorners( Current_Layer -> layerDA, &xs, &ys, &zs, &xm, &ym, &zm );

     PETSC_3D_Array ves     ( Current_Layer->layerDA, Current_Layer->Current_Properties ( Basin_Modelling::VES_FP ));
     PETSC_3D_Array max_ves ( Current_Layer->layerDA, Current_Layer->Current_Properties ( Basin_Modelling::Max_VES ));

     // Set the rest of the values
     for ( i = xs; i < ( xs + xm ); i++)
     {
        for ( j = ys; j < ( ys + ym ); j++)
        {

           if ( !nodeIsDefined ( i, j )) continue;

           Density_Difference = Current_Layer -> calcDiffDensity( i,j );

           for ( k = 0; k < ( zs + zm ); k++)
           {
              if ( k == 0 )
              {
                 Max_Ves = PetscMax( ves( 0,j,i ) , max_ves( 0,j,i ) );
                 max_ves( 0,j,i ) = Max_Ves;
                 continue;
              }

              Segment_May_Be_Eroding = Current_Layer->getSolidThickness ( i, j, k - 1 ).descending ( time );
              double thickness = Current_Layer->getSolidThickness ( i, j, k - 1, time );

              if ( ves( k,j,i ) < 50.0 && ( thickness == 0.0 || Segment_May_Be_Eroding ))
              {
                 dVes = AccelerationDueToGravity * Density_Difference * thickness;
                 Max_Ves = max_ves( k-1,j,i ) - dVes;
                 Max_Ves = PetscMax( Max_Ves , ves( k,j,i ) );
                 Max_Ves = PetscMax( Max_Ves , max_ves( k,j,i ) );
              }
              else
              {
                 Max_Ves = PetscMax( ves( k,j,i ) , max_ves( k,j,i ) );
              }

              max_ves( k,j,i ) = Max_Ves;


           }
        }
     }

     Layers++;
  }

  return true;
}

bool AppCtx::calcPorosities( const double time ) {

  /* local variables */
  int            i, j, k;
  int            xs, ys, zs;
  int            xm, ym, zm;


  bool Include_Chemical_Compaction;

  LayerProps_Ptr Current_Layer;
  Layer_Iterator Layers;

  Layers.Initialise_Iterator ( layers, Descending, Sediments_Only, Active_Layers_Only );

  while ( ! Layers.Iteration_Is_Done () ) {

    Current_Layer = Layers.Current_Layer ();

    Include_Chemical_Compaction = Do_Chemical_Compaction && ( Current_Layer -> Get_Chemical_Compaction_Mode ());

    DMDAGetCorners( Current_Layer -> layerDA, &xs, &ys, &zs, &xm, &ym, &zm );

    PETSC_3D_Array porosity( Current_Layer -> layerDA, Current_Layer -> Porosity );

    PETSC_3D_Array ves                 ( Current_Layer->layerDA, Current_Layer->Current_Properties ( Basin_Modelling::VES_FP ));
    PETSC_3D_Array max_ves             ( Current_Layer->layerDA, Current_Layer->Current_Properties ( Basin_Modelling::Max_VES ));
    PETSC_3D_Array Chemical_Compaction ( Current_Layer->layerDA, Current_Layer->Current_Properties ( Basin_Modelling::Chemical_Compaction ));

    for ( i = xs; i < ( xs + xm ); i++)
    {
      for ( j = ys; j < ( ys + ym ); j++)
      {
          for ( k = zs; k < ( zs + zm ); k++)
          {
              if ( !nodeIsDefined ( i, j )){
                  porosity( k,j,i ) = CauldronNoDataValue;
                  continue;
              }

              if ( Do_Iteratively_Coupled || IsCalculationCoupled || DoOverPressure ) {
                const CompoundLithology* currentLithology;

                currentLithology = Current_Layer->getLithology ( i,j );
                porosity( k,j,i ) = currentLithology->porosity( ves(k,j,i), max_ves(k,j,i), Include_Chemical_Compaction, Chemical_Compaction ( k, j, i ));
              } else {
                porosity( k,j,i ) = Current_Layer->getLithology ( i, j )->porosity( max_ves(k,j,i), max_ves(k,j,i), false, 0.0 );


                //LogHandler with DEBUG_SEVERITY prints the info with "-verbosity diagnostic"
                LogHandler(LogHandler::DEBUG_SEVERITY)
                    << "calcBottomPorosity for " << Current_Layer->getName() << " at time "<< time <<" Ma : "<< porosity(k, j, i);
              }
          }
      }
    }

    Layers++;
  }

  return true;
}

bool AppCtx::Calculate_Pressure( const double time ) {

  /* local variables */
  int i, j, k, k_top;
  int ix, iy;
  int xs, ys, zs;
  int xm, ym, zm;

  int Top_Z_Node_Index;

  double seaBottomDepth;

  double Temperature;

  double Hydrostatic_Pressure;
  double Lithostatic_Pressure;
  double Fluid_Pressure;
  double Excess_Pressure;

  double Solid_Density;
  double Fluid_Density;

  double Solid_Thickness;
  double Segment_Thickness;

  bool   firstpass   = true;
  double **surface_sea_bottom_depth = NULL;
  double **surface_temperature      = NULL;
  double **previous_hydro_pressure  = NULL;
  double **previous_litho_pressure  = NULL;
  double **previous_fluid_pressure  = NULL;
  double **previous_excess_pressure = NULL;

  LayerProps_Ptr Current_Layer;
  Layer_Iterator Layers;

  Layers.Initialise_Iterator ( layers, Descending, Sediments_Only, Active_Layers_Only );

  while ( ! Layers.Iteration_Is_Done () ) {

    Current_Layer = Layers.Current_Layer ();

    if ( Current_Layer -> fluid == 0 )
    {
       cout << "Basin_Warning: Lithology has no fluid..." << Current_Layer->layername << endl;
    }

    DMDAGetCorners( Current_Layer -> layerDA, &xs, &ys, &zs, &xm, &ym, &zm );

    PETSC_3D_Array depth           ( Current_Layer->layerDA, Current_Layer->Current_Properties ( Basin_Modelling::Depth ));
    PETSC_3D_Array temperature     ( Current_Layer->layerDA, Current_Layer->Current_Properties ( Basin_Modelling::Temperature ));
    PETSC_3D_Array ves             ( Current_Layer->layerDA, Current_Layer->Current_Properties ( Basin_Modelling::VES_FP ));
    PETSC_3D_Array hydro_pressure  ( Current_Layer->layerDA, Current_Layer->Current_Properties ( Basin_Modelling::Hydrostatic_Pressure ));
    PETSC_3D_Array litho_pressure  ( Current_Layer->layerDA, Current_Layer->Current_Properties ( Basin_Modelling::Lithostatic_Pressure ));
    PETSC_3D_Array fluid_pressure  ( Current_Layer->layerDA, Current_Layer->Current_Properties ( Basin_Modelling::Pore_Pressure ));
    PETSC_3D_Array excess_pressure ( Current_Layer->layerDA, Current_Layer->Current_Properties ( Basin_Modelling::Overpressure ));

    Top_Z_Node_Index = zm - 1;

    if ( firstpass )
    {
      surface_sea_bottom_depth = Array<double>::create2d( xm,ym );
      surface_temperature      = Array<double>::create2d( xm,ym );

      previous_hydro_pressure  = Array<double>::create2d( xm,ym );
      previous_litho_pressure  = Array<double>::create2d( xm,ym );
      previous_fluid_pressure  = Array<double>::create2d( xm,ym );
      previous_excess_pressure = Array<double>::create2d( xm,ym );

      for ( i = xs; i < ( xs + xm ); i++ )
      {
          for ( j = ys; j < ( ys + ym ); j++ )
          {
              if ( !nodeIsDefined ( i, j )) continue;
              ix = i - xs; iy = j - ys;

              seaBottomDepth = depth( Top_Z_Node_Index,j,i );
              surface_sea_bottom_depth[ ix ][ iy ] = seaBottomDepth;
              surface_temperature[ ix ][ iy ] = FastcauldronSimulator::getInstance ().getSeaBottomTemperature ( i, j, time );
              computeHydrostaticPressure ( Current_Layer -> fluid, surface_temperature[ ix ][ iy ], seaBottomDepth, Hydrostatic_Pressure );
              hydro_pressure( Top_Z_Node_Index,j,i ) = Hydrostatic_Pressure;
              litho_pressure( Top_Z_Node_Index,j,i ) = Hydrostatic_Pressure;
              fluid_pressure( Top_Z_Node_Index,j,i ) = Hydrostatic_Pressure;
              excess_pressure( Top_Z_Node_Index,j,i ) = 0.0;
          }
      }

      firstpass = false;

    }
    else
    {
      for ( i = xs; i < ( xs + xm ); i++ )
      {
           for ( j = ys; j < ( ys + ym ); j++ )
           {
                  if ( !nodeIsDefined ( i, j )) continue;

             ix = i - xs; iy = j - ys;

             hydro_pressure( Top_Z_Node_Index,j,i ) = previous_hydro_pressure[ ix ][ iy ];
             litho_pressure( Top_Z_Node_Index,j,i ) = previous_litho_pressure[ ix ][ iy ];
             fluid_pressure( Top_Z_Node_Index,j,i ) = previous_fluid_pressure[ ix ][ iy ];
             excess_pressure( Top_Z_Node_Index,j,i ) = previous_excess_pressure[ ix ][ iy ];

           }
      }
    }

    for ( i = xs; i < ( xs + xm ); i++)
    {
      for ( j = ys; j < ( ys + ym ); j++)
      {

         if ( !nodeIsDefined ( i, j )) continue;
         ix = i - xs; iy = j - ys;
         Solid_Density = Current_Layer->getLithology ( i,j )->density();
         // Determine if the permafrost switch is on
         bool switchPermafrost = Current_Layer->fluid->isPermafrostEnabled();
         
         for ( k = ( Top_Z_Node_Index - 1 ); k >= 0; k-- ){
             k_top = k + 1;
             Solid_Thickness   = Current_Layer->getSolidThickness ( i, j, k, time );
             
             if ( Solid_Thickness < DepositingThicknessTolerance ){
                 hydro_pressure( k,j,i )  = hydro_pressure( k_top,j,i );
                 litho_pressure( k,j,i )  = litho_pressure( k_top,j,i );
                 fluid_pressure( k,j,i )  = fluid_pressure( k_top,j,i );
                 excess_pressure( k,j,i ) = excess_pressure( k_top,j,i );
                 continue;
             }
             Segment_Thickness = depth( k,j,i ) - depth( k_top,j,i );
             Temperature = temperature ( k, j, i );
             
             if ( Temperature == CauldronNoDataValue || Temperature == IbsNoDataValue ) {
                 Temperature = Estimate_Temperature_At_Depth( depth( k_top,j,i ),
                                                         surface_temperature[ix][iy],
                                                         surface_sea_bottom_depth[ix][iy] );
             }
             
             Fluid_Density = Current_Layer -> fluid -> density( Temperature, fluid_pressure( k_top,j,i ) );
             
             // Fluid is denser than rock and the permafrost switch is on
             if ( switchPermafrost && Solid_Density <= Fluid_Density )
             {
                 Hydrostatic_Pressure = hydro_pressure( k_top, j, i );
                 double surfacePorosity = Current_Layer->getLithology( i, j, k )->surfacePorosity( );
                 Lithostatic_Pressure = litho_pressure( k_top, j, i ) +
                     Solid_Thickness * Solid_Density *( 1.0 - surfacePorosity ) * AccelerationDueToGravity * PaToMegaPa;
             }
             else
             {
                 Hydrostatic_Pressure = hydro_pressure( k_top,j,i )+ Fluid_Density * AccelerationDueToGravity * Segment_Thickness * PaToMegaPa;
                 Lithostatic_Pressure = litho_pressure( k_top,j,i ) + ( ( Segment_Thickness - Solid_Thickness ) * Fluid_Density +
                                        Solid_Thickness * Solid_Density ) * AccelerationDueToGravity * PaToMegaPa;
             }
             
             if ( IsCalculationCoupled ) {
                 Fluid_Pressure  = Lithostatic_Pressure - ( ves( k,j,i ) * PaToMegaPa );
             } else {
                 Fluid_Pressure  = Hydrostatic_Pressure;
             }
             
             //
             // // Must take max ( overpressure, 0 ) here because errors in the
             // // interpolation may give rise to negative overpressures.
             //
             
             Excess_Pressure = NumericFunctions::Maximum ( Fluid_Pressure - Hydrostatic_Pressure, 0.0 );
             if (std::isnan( Hydrostatic_Pressure )){
                 cout << "Basin_Error: Error hydrostatic... " << endl << flush;
             }
             
             if (std::isnan( Lithostatic_Pressure )){
                 cout << "Basin_Error: Error lithostatic ... "
                  << Lithostatic_Pressure << "  "
                  << litho_pressure( k_top,j,i ) << "  "
                  << Segment_Thickness << "  "
                  << Solid_Thickness << "  "
                  << Fluid_Density << "  "
                  << Solid_Density
                  << endl << flush;
             }
             
             if (std::isnan( Fluid_Pressure )){
                 cout << "Basin_Error: Error pore-pressure... " << endl << flush;
             }
             
             if (std::isnan( Excess_Pressure )){
                 cout << "Basin_Error: Error overpressure... " << endl << flush;
             }
             
             hydro_pressure( k,j,i )  = Hydrostatic_Pressure;
             litho_pressure( k,j,i )  = Lithostatic_Pressure;
             fluid_pressure( k,j,i )  = Fluid_Pressure;
             excess_pressure( k,j,i ) = Excess_Pressure;
         }

      }
    }

    for ( i = xs; i < ( xs + xm ); i++ )
    {
      for ( j = ys; j < ( ys + ym ); j++ )
      {
          ix = i - xs; iy = j - ys;
          
          previous_hydro_pressure[ ix ][ iy ] = hydro_pressure( 0,j,i );
          previous_litho_pressure[ ix ][ iy ] = litho_pressure( 0,j,i );
          previous_fluid_pressure[ ix ][ iy ] = fluid_pressure( 0,j,i );
          previous_excess_pressure[ ix ][ iy ] = excess_pressure( 0,j,i );
      }
    }

    Layers++;
  }

  if ( surface_sea_bottom_depth != NULL ) {
    Array<double>::delete2d( surface_sea_bottom_depth );
  }

  if ( surface_temperature != NULL ) {
    Array<double>::delete2d( surface_temperature );
  }

  if ( previous_hydro_pressure != NULL ) {
    Array<double>::delete2d( previous_hydro_pressure );
  }

  if ( previous_litho_pressure != NULL ) {
    Array<double>::delete2d( previous_litho_pressure );
  }

  if ( previous_fluid_pressure != NULL ) {
    Array<double>::delete2d( previous_fluid_pressure );
  }

  if ( previous_excess_pressure != NULL ) {
    Array<double>::delete2d( previous_excess_pressure );
  }

  return true;

}



double AppCtx::Estimate_Temperature_At_Depth( const double Node_Depth,
                     const double Surface_Temperature,
                     const double Surface_Sea_Bottom_Depth ) {

  /* local variables */
  double Estimated_Temperature;
  double Real_Depth;

  Real_Depth = Node_Depth - Surface_Sea_Bottom_Depth;

  if ( Real_Depth <= 0.0 )
  {
    Estimated_Temperature = Surface_Temperature;
  }
  else
  {
    Estimated_Temperature = Surface_Temperature + Real_Depth * Temperature_Gradient;
  }

#if 0
  if ( Estimated_Temperature > 512.0 )
  {
    cout << "Error...." << Node_Depth << "  " << Surface_Temperature << "  " << Surface_Sea_Bottom_Depth << endl;
  }
#endif

  return Estimated_Temperature;
}


//------------------------------------------------------------//

void AppCtx::setInitialTimeStep () {

#define INITIAL_PRESSURE_TIMESTEP_REQUIRED 1

#ifdef INITIAL_PRESSURE_TIMESTEP_REQUIRED
   size_t i;
   double layerDepositionInterval;

   const double maximumElementHeight = FastcauldronSimulator::getInstance ().getRunParameters ()->getBrickHeightSediment ();

   double layerTimeStepSize;

   // // The minimum time it takes for a single element to be fully deposited in the mesh
   // double minimumElementDepositionPeriod = 1000.0;

   // The rate at which ELEMENTS are deposited in the layer. The number of elements
   // in the layer is fixed (depends only on present day thickness). The number of
   // elements is independant of the porosity change as the layer is deposited.
   double layerElementDepositionPeriod;

   double layerElementErosionPeriod;

   int standardElementsEroded;

   const int rank = FastcauldronSimulator::getInstance ().getRank ()  ;
   if ( (debug1 || verbose) && rank == 0) {
      cout << setw ( 20 ) << "Layer name";
      cout << setw ( 15 ) << "Start";
      cout << setw ( 15 ) << "End";
      cout << setw (  8 ) << "Element";
      cout << setw (  8 ) << "Steps";
      cout << setw ( 15 ) << "time-step";
      cout << setw ( 15 ) << "#-time-steps";
      cout << endl;
   }


    for ( i = 0; i < layers.size() - 2; i++ ) {
      layerDepositionInterval = layers [ i + 1 ]->depoage - layers [ i ]->depoage;

      LayerProps* formation = layers [ i ];

      // The time it takes for a single element in the layer to be fully deposited
      if ( layers [ i ]->getMaximumNumberOfElements () > 0 ) {

         if ( formation->getMinimumThickness () >= 0.0 && formation->getMaximumThickness () > 0.0 ) {
            // Only deposition.

            layerElementDepositionPeriod = layerDepositionInterval / double ( layers [ i ]->getMaximumNumberOfElements ());

            // // Take the minimum of the ElementDepositionTimes for all layers
            // minimumElementDepositionPeriod = NumericFunctions::Minimum ( minimumElementDepositionPeriod, layerElementDepositionPeriod );
            layerTimeStepSize = m_elementFraction * layerElementDepositionPeriod;

         } else if ( formation->getMinimumThickness () < 0.0 && formation->getMaximumThickness () > 0.0 ) {
            // Deposition and erosion.

            standardElementsEroded = NumericFunctions::Maximum ( 1, static_cast<int>(std::ceil ( std::fabs ( formation->getMinimumThickness () ) / maximumElementHeight )));
            layerElementDepositionPeriod = layerDepositionInterval / double ( layers [ i ]->getMaximumNumberOfElements ());
            layerElementErosionPeriod = m_elementErosionFraction * layerDepositionInterval / standardElementsEroded;
            layerTimeStepSize = m_elementFraction * NumericFunctions::Minimum ( layerElementDepositionPeriod, layerElementErosionPeriod );

         } else if ( formation->getMaximumThickness () < 0.0 ) {
            // Only erosion.

            standardElementsEroded = NumericFunctions::Maximum ( 1, static_cast<int>(std::ceil ( std::fabs ( formation->getMinimumThickness ()) / maximumElementHeight )));
            layerTimeStepSize = m_elementFraction * m_elementErosionFraction * layerDepositionInterval / standardElementsEroded;

         } else { // if ( formation->Minimum_Thickness_Value = 0.0 and formation->getMaximumThickness () = 0.0 ) {
            // Hiatus.

            layerTimeStepSize = m_elementFraction * m_elementHiatusFraction * layerDepositionInterval;

         }

         if ( (debug1 || verbose) && rank == 0 ) {
            cout << std::setw ( 20 ) << layers [ i ]->layername
                 << std::setw ( 15 ) << layers [ i + 1 ]->depoage
                 << std::setw ( 15 ) << layers [ i ]->depoage
                 << std::setw (  8 ) << layers [ i ]->getMaximumNumberOfElements ()
                 << std::setw (  8 ) << layers [ i ]->getMinimumThickness ()
                 << std::setw ( 15 ) << layers [ i ]->getMaximumThickness ()
                 << std::setw ( 15 ) << layerTimeStepSize
                 << endl;
         }

         m_layerPreferredTimeStepSize [ layers [ i ]] = layerTimeStepSize;
      }

   }

  // // Minimum of the ElementDepositionTime and the user defined initial time-step (from the project file)
  // m_computedInitialPressureTimeStep = NumericFunctions::Minimum ( timestepsize, MaximumInitialElementDeposition * minimumElementDepositionPeriod );

  m_computedInitialPressureTimeStep = timestepsize;
#else
  m_computedInitialPressureTimeStep = timestepsize;
#endif

  if ( debug1 || verbose) {
    PetscPrintf(PETSC_COMM_WORLD, " Initial time step %f\n", m_computedInitialPressureTimeStep );
  }

}


//------------------------------------------------------------//

const LayerProps* AppCtx::findDepositingLayer ( const double time ) const {

   size_t i = 0;

   for ( i = 0; i <= layers.size () - 2; ++i ) {

      if ( layers [ i ]->depoage <= time && time <= layers [ i + 1 ]->depoage ) {
         return layers [ i ];
      }

   }

   return 0;
}

//------------------------------------------------------------//

double AppCtx::getInitialTimeStep ( const double currentTime ) const {

   LayerPreferredTimeStepSizeMap::const_iterator layerTs = m_layerPreferredTimeStepSize.find ( findDepositingLayer ( currentTime ));
   double deltaT;

   if ( (debug1 || verbose) and FastcauldronSimulator::getInstance ().getRank () == 0 ) {
      cout << " At time " << currentTime << " the layer " << layerTs->first->layername << " is being deposited, default time-step size: " << layerTs->second << endl;
   }

   if ( m_burialRateTimeStepping && layerTs != m_layerPreferredTimeStepSize.end ()) {
      deltaT = layerTs->second;
   } else {
      deltaT = timestepsize;
   }

#if 0
   deltaT = NumericFunctions::Maximum ( deltaT, minimumTimeStep ());
   deltaT = NumericFunctions::Minimum ( deltaT, maximumTimeStep ());
#endif

   return deltaT;
}

//------------------------------------------------------------//


double AppCtx::initialPressureTimeStep () const {
  return m_computedInitialPressureTimeStep;
}


//------------------------------------------------------------//

bool AppCtx::setNodeDepths ( const double time ) {

  int            i, j, k;
  int            xs, ys, zs;
  int            xm, ym, zm;

  double**       topDepths = 0;

  const double presentDayMantleThickness     = FastcauldronSimulator::getInstance ().getMantleFormation ()->getPresentDayThickness ();
  const double mantleMaximumElementThickness = FastcauldronSimulator::getInstance ().getRunParameters ()->getBrickHeightMantle ();
  double basementThickness = 0.0;
  double mantleThickness = 0.0;

  LayerProps_Ptr currentLayer;
  Layer_Iterator basinLayers;

  basinLayers.Initialise_Iterator ( layers, Descending, Basement_And_Sediments, Active_Layers_Only );

  while ( ! basinLayers.Iteration_Is_Done () ) {

    currentLayer = basinLayers.Current_Layer ();

    if ( currentLayer -> isMantle ()) {
       if ( isALC() ) {
          basementThickness = FastcauldronSimulator::getInstance ().getMantleFormation ()->getInitialLithosphericMantleThickness () +
             FastcauldronSimulator::getInstance ().getCrustFormation()->getInitialCrustalThickness();
       } else {
          mantleThickness = FastcauldronSimulator::getInstance ().getMantleFormation ()->getPresentDayThickness ();
       }
    }

    DMDAGetCorners ( currentLayer->layerDA, &xs, &ys, &zs, &xm, &ym, &zm);
    int numberOfNodes = zs + zm - 1;

    PETSC_3D_Array depth   ( currentLayer->layerDA, currentLayer->Current_Properties ( Basin_Modelling::Depth ));

    // Initialise top depths of layer.
    if ( topDepths == 0 ) {
        topDepths = Array<double>::create2d( xm,ym, 0.0 );
        
        for ( i = xs; i < ( xs + xm ); i++ ) {
            for ( j = ys; j < ( ys + ym ); j++ ) {
               if ( nodeIsDefined ( i, j )) {
                   depth( numberOfNodes,j,i ) = FastcauldronSimulator::getInstance ().getSeaBottomDepth ( i, j, time );
               } else {
               depth( numberOfNodes,j,i ) = CauldronNoDataValue;
               }
           }
      }

    } else {
        for ( i = xs; i < ( xs + xm ); i++ ) {
            for ( j = ys; j < ( ys + ym ); j++ ) {
                if ( nodeIsDefined ( i, j )) {
                    depth( numberOfNodes,j,i ) = topDepths [i-xs][j-ys];
                } else {
                    depth( numberOfNodes,j,i ) = CauldronNoDataValue;
                }
            }
        }

    }

    // Now set all other nodes in the layer.
    for ( i = xs; i < ( xs + xm ); i++ ) {

      for ( j = ys; j < ( ys + ym ); j++ ) {

         if ( nodeIsDefined ( i, j )) {

            if ( currentLayer -> isMantle ()) {
               if ( !isALC() ) {
                  basementThickness = mantleThickness +  FastcauldronSimulator::getInstance ().getCrustThickness ( i, j, 0.0 );
               }
               setMantleDepth ( basementThickness, i, j, time, numberOfNodes, depth );
            } else {

               for ( k = numberOfNodes - 1; k >= 0; --k ) {
                  double thickness = currentLayer->getRealThickness ( i, j, k ).F(time);

                  if ( thickness == IbsNoDataValue || thickness == CauldronNoDataValue ) {
                     depth ( k, j, i ) = depth ( k + 1, j, i );
                  } else {
                     depth ( k, j, i ) = depth ( k + 1 ,j, i ) + thickness;
                  }

               }

            }

         } else {

            for ( k = numberOfNodes; k >= 0; --k ) {
               depth ( k, j, i ) = CauldronNoDataValue;
            }

         }

      }

    }

    // The bottom depths of this layer are the top depths of the layer below!
    for ( i = xs; i < ( xs + xm ); i++ ) {
        for ( j = ys; j < ( ys + ym ); j++ ) {
            topDepths[i-xs][j-ys] = depth( 0,j,i );
        }
    }

    basinLayers++;
  }

  if ( topDepths != 0 ) {
    Array<double>::delete2d( topDepths );
  }

  return true;
}

//------------------------------------------------------------//

void AppCtx::setMantleDepth ( const double basementThickness, //presentDayMantleThickness,
                              const int    i,
                              const int    j,
                              const double time,
                              const int    numberOfNodes,
                                    PETSC_3D_Array& depth ) const {

   int k;

   int maximumMantleElementHeight = FastcauldronSimulator::getInstance ().getRunParameters ()->getBrickHeightMantle ();

  // double basementThickness = presentDayMantleThickness + FastcauldronSimulator::getInstance ().getCrustThickness ( i, j, 0.0 );
   double currentCrustThickness = FastcauldronSimulator::getInstance ().getCrustThickness ( i, j, time );
   double crustThinningRatio;
   double currentMantleThickness = basementThickness - currentCrustThickness;
   double maximumDepth = depth ( numberOfNodes, j, i ) + currentMantleThickness;

   if ( currentCrustThickness > 0.0 ) {
      crustThinningRatio = Crust ()->getCrustMaximumThickness ( i, j ) / currentCrustThickness;
   } else {
      crustThinningRatio = -1.0;
   }

   if ( isALC() )
   {
      assert( currentCrustThickness ); // Effective Crustal thickness can't be 0

      const double maxCrustThinningRatio = Crust()->getCrustMaximumThickness( i, j )/Crust()->getCrustMinimumThickness( i, j );
      for ( k = numberOfNodes - 1; k >= 0; --k )
      {
         const double mantleSegmentHeight = Mantle() -> getMantleElementHeight0( maxCrustThinningRatio, k, numberOfNodes-1 ) / crustThinningRatio;
         depth( k, j, i ) = NumericFunctions::Minimum ( maximumDepth, depth ( k + 1, j, i ) + mantleSegmentHeight );
      }
   }
   else
   {
      double mantleSegmentHeight = maximumMantleElementHeight / crustThinningRatio;

      for ( k = numberOfNodes - 1; k >= 0; --k ) {
         depth( k, j, i ) = NumericFunctions::Minimum ( maximumDepth, depth ( k + 1, j, i ) + mantleSegmentHeight );
      }

   }

}

//------------------------------------------------------------//


bool AppCtx::calcNodeDepths( const double time ) {

  const double presentDayMantleThickness = FastcauldronSimulator::getInstance ().getMantleFormation ()->getPresentDayThickness ();
  const double mantleMaximumElementThickness = FastcauldronSimulator::getInstance ().getRunParameters ()->getBrickHeightMantle ();

  /* local variables */
  int            i, j, k;
  int            xs, ys, zs;
  int            xm, ym, zm;

  double**       prev = NULL;
  double         densityDifference;
  double         segmentThickness;
  double         solidThickness;

  bool           firstpass = true;

  const CompoundLithology*  currentLithology;
  LayerProps_Ptr currentLayer;
  Layer_Iterator activeLayers;

  double basementThickness = 0.0;
  double mantleThickness = 0.0;

  activeLayers.Initialise_Iterator ( layers, Descending, Basement_And_Sediments, Active_Layers_Only );

  while ( ! activeLayers.Iteration_Is_Done () ) {

    currentLayer = activeLayers.Current_Layer ();
    if ( currentLayer->isMantle ()) {
       if ( isALC() ) {
          basementThickness = FastcauldronSimulator::getInstance ().getMantleFormation ()->getInitialLithosphericMantleThickness() +
             FastcauldronSimulator::getInstance ().getCrustFormation()->getInitialCrustalThickness();
       } else {
          mantleThickness = FastcauldronSimulator::getInstance ().getMantleFormation ()->getPresentDayThickness ();
       }
    }

    DMDAGetCorners(currentLayer->layerDA, &xs, &ys, &zs, &xm, &ym, &zm);


    int numberOfNodes = zs + zm - 1;


    PETSC_3D_Array depth  ( currentLayer->layerDA, currentLayer->Current_Properties ( Basin_Modelling::Depth ));
    PETSC_3D_Array ves    ( currentLayer->layerDA, currentLayer->Current_Properties ( Basin_Modelling::VES_FP ));
    PETSC_3D_Array maxVes ( currentLayer->layerDA, currentLayer->Current_Properties ( Basin_Modelling::Max_VES ));

    // Set top of depth array.
    if ( firstpass ) {
       prev = Array<double>::create2d( xm,ym );

       for ( i = xs; i < ( xs + xm ); i++ )
       {
          for ( j = ys; j < ( ys + ym ); j++ )
          {
             if ( nodeIsDefined ( i, j ))
             {
                depth ( numberOfNodes, j, i ) = FastcauldronSimulator::getInstance ().getSeaBottomDepth ( i, j, time );
             }

          }
       }

       firstpass = false;
    } else {

       for ( i = xs; i < ( xs + xm ); i++ ) {

          for ( j = ys; j < ( ys + ym ); j++ ) {

             if ( nodeIsDefined ( i, j )) {
                depth ( numberOfNodes, j, i ) = prev[i-xs][j-ys];
             }

          }

       }

    }

    // Now fill in rest of depth array.
    for ( i = xs; i < ( xs + xm ); i++ ) {

       for ( j = ys; j < ( ys + ym ); j++ ) {

          if ( nodeIsDefined ( i, j )) {

             if ( currentLayer->isMantle ()) {
                  if ( !isALC() ) {
                     basementThickness = mantleThickness + FastcauldronSimulator::getInstance ().getCrustThickness ( i, j, 0.0 );
                }
                setMantleDepth ( basementThickness, i, j, time, numberOfNodes, depth );
             } else {

                currentLithology = currentLayer->getLithology ( i, j );
                densityDifference = currentLayer->calcDiffDensity(i, j);

                for ( k = numberOfNodes - 1; k >= 0; k-- ) {
                   solidThickness = currentLayer->getSolidThickness ( i, j, k ).F(time);

                   if ( currentLithology->isIncompressible ()) {

                      if ( solidThickness > 0 ) {
                         depth ( k, j, i ) = depth ( k + 1, j, i ) + solidThickness / ( 1.0 - currentLithology->surfacePorosity ());
                      } else {
                         depth ( k, j, i ) = depth ( k + 1, j, i );
                      }

                   } else if ( currentLayer->isMobileLayer ()) {
                      depth( k,j,i ) = depth( k+1,j,i ) + solidThickness;
                   } else {

                      if ( solidThickness > DepositingThicknessTolerance ) {

                         if ( IsCalculationCoupled ) {
                            segmentThickness = currentLithology->computeSegmentThickness ( maxVes ( k + 1, j, i ), maxVes ( k, j, i ),
                                                                                           ves ( k + 1, j, i ), ves ( k, j, i ),
                                                                                           densityDifference,
                                                                                           solidThickness );
                         } else {
                            // Should the ves be used here too?
                            segmentThickness = currentLithology->computeSegmentThickness ( maxVes ( k + 1, j, i ), maxVes ( k, j, i ),
                                                                                           densityDifference,
                                                                                           solidThickness );
                         }

                         depth ( k, j, i ) = depth ( k + 1, j, i ) + segmentThickness;
                      } else {
                         depth ( k, j, i ) = depth ( k + 1, j, i );
                      }

                   }

                }

             }

          } else {

             for ( k = numberOfNodes; k >= 0; --k ) {
                depth ( k, j, i ) = CauldronNoDataValue;
             }

          }

       }

    }

    // Store bottom values
    for ( i = xs; i < ( xs + xm ); i++ )
    {
        for ( j = ys; j < ( ys + ym ); j++ )
        {
            prev[i-xs][j-ys] = depth( 0,j,i );
        }
    }

    activeLayers++;
  }

  if ( prev != NULL ) Array<double>::delete2d( prev );

  return true;
}

void AppCtx::Retrieve_Lithology_ID ()
{

  int            i, j, k;
  int            xs, ys, zs;
  int            xm, ym, zm;

  LayerProps_Ptr Current_Layer;
  Layer_Iterator Layers;

  Layers.Initialise_Iterator ( layers, Descending, Basement_And_Sediments, Active_Layers_Only );

  while ( ! Layers.Iteration_Is_Done () ) {

    Current_Layer = Layers.Current_Layer ();

    DMDAGetCorners( Current_Layer -> layerDA,&xs,&ys,&zs,&xm,&ym,&zm );

    DMCreateGlobalVector( Current_Layer -> layerDA, &Current_Layer -> Lithology_ID );

    VecSet( Current_Layer -> Lithology_ID, CauldronNoDataValue );

    PETSC_3D_Array lithology_id( Current_Layer -> layerDA, Current_Layer -> Lithology_ID );

    for ( i = xs; i < ( xs + xm ); i++ )
    {
        for ( j = ys; j < ( ys + ym ); j++ )
        {
            if ( !nodeIsDefined ( i, j )) continue;
            for ( k = zs; k < zs+zm; k++ )
            {
                lithology_id( k,j,i ) = (float) ((Lithology*)(Current_Layer->getLithology( i, j )))->getLithologyID ();
            }
        }
    }

    Layers++;
  }

}


void AppCtx::deleteLithologyIDs () {

  PetscBool validVector;
  LayerProps_Ptr currentLayer;
  Layer_Iterator Layers;

  Layers.Initialise_Iterator ( layers, Descending, Basement_And_Sediments, Active_Layers_Only );

  while ( ! Layers.Iteration_Is_Done () ) {

    currentLayer = Layers.Current_Layer ();

    VecValid ( currentLayer->Lithology_ID, &validVector );

    if ( validVector ) {
       Destroy_Petsc_Vector ( currentLayer->Lithology_ID );
    }

    Layers++;
  }

}

void AppCtx::setLayerBottSurfaceName() {

  size_t I;

  for ( I = 0; I <= layers.size()-2; I++ ) {

    layers[ I ]->BottSurfaceName = layers[ I + 1 ]->TopSurfaceName;
    layers[ I ]->BottSurface_DepoSeq = layers[ I + 1 ]->TopSurface_DepoSeq;

  }

}

bool AppCtx::createFormationLithologies ( const bool canRunSaltModelling ) {

  if (debug1 && FastcauldronSimulator::getInstance ().getRank () == 0 ) cout << "o Creating Lithologies...";

  bool Created_Lithologies = true;
  int Global_Number_Of_Lithologies;
  int Max_Number_Of_Lithologies_On_Single_Proc;

  // If running Darcy simulator then disable halokinesis salt modelling.
  bool enableHalokinesis = canRunSaltModelling && !FastcauldronSimulator::getInstance ().getMcfHandler ().solveFlowEquations ();

  Layer_Iterator Layers;
  Layers.Initialise_Iterator ( layers, Descending, Basement_And_Sediments,
                Active_And_Inactive_Layers );

  Created_Lithologies = FastcauldronSimulator::getInstance ().setFormationLithologies ( enableHalokinesis, true );

  if ( !successfulExecution ( Created_Lithologies )) {
     cout << "Basin_Error: Could not create lithologies " << endl;
     return false;
  }

  while ( ! Layers.Iteration_Is_Done () ) {

    LayerProps_Ptr Current_Layer = Layers.Current_Layer ();

    // Skip Erosion or Hiatus Layers
    if ( Current_Layer -> getMaximumNumberOfElements() <= 0 )
    {
      Layers++;
      continue;
    }

    Current_Layer -> Previous_Topmost_Segments.reallocate ( FastcauldronSimulator::getInstance ().getActivityOutputGrid ());
    Current_Layer -> Previous_Topmost_Segments.fill ( -1 );

    Current_Layer -> Current_Topmost_Segments.reallocate ( FastcauldronSimulator::getInstance ().getActivityOutputGrid ());
    Current_Layer -> Current_Topmost_Segments.fill ( -1 );

    Layers++;
  }


  int Local_Number_Of_Lithologies = FastcauldronSimulator::getInstance ().getLithologyManager ().size();

  /* Find Maximum Number of Lithologies (excluding Crust & Mantle) on a single processor */
  MPI_Allreduce( &Local_Number_Of_Lithologies, &Max_Number_Of_Lithologies_On_Single_Proc, 1,
       MPI_INT, MPI_MAX, PETSC_COMM_WORLD );

  Generate_Lithology_Identifier_List ( Max_Number_Of_Lithologies_On_Single_Proc,
                                       Global_Number_Of_Lithologies );

  if (debug1 && FastcauldronSimulator::getInstance ().getRank () == 0 ) cout << " DONE" << endl;

  return Created_Lithologies;

}

//------------------------------------------------------------//


int AppCtx::Layer_Position ( const string& Formation_Name ) const {

  size_t I;
  int Position = -1;

  for ( I = 0; I < layers.size (); I++ ) {

    if ( layers [ I ] -> layername == Formation_Name ){
      Position = int ( I );
      break;
    }

  }

  return Position;
}


//------------------------------------------------------------//


bool AppCtx::inProcessorRange ( const int  globalIIndex,
                                const int  globalJIndex,
                                const bool includeLowerIGhostNodes,
                                const bool includeUpperIGhostNodes,
                                const bool includeLowerJGhostNodes,
                                const bool includeUpperJGhostNodes ) const {

  DMDALocalInfo dainfo;
  int xMin;
  int xMax;

  int yMin;
  int yMax;


  DMDAGetLocalInfo( *mapDA, &dainfo );

  if ( includeLowerIGhostNodes ) {
    xMin = dainfo.gxs;
  } else {
    xMin = dainfo.xs;
  }

  if ( includeUpperIGhostNodes ) {
    xMax = dainfo.gxs + ( dainfo.gxm - 1 );
  } else {
    xMax = dainfo.xs + ( dainfo.xm - 1 );
  }

  if ( includeLowerJGhostNodes ) {
    yMin = dainfo.gys;
  } else {
    yMin = dainfo.ys;
  }

  if ( includeUpperJGhostNodes ) {
    yMax = dainfo.gys + ( dainfo.gym - 1 );
  } else {
    yMax = dainfo.ys + ( dainfo.ym - 1 );
  }

  //check that current node is within this processor range
  return (( xMin <= globalIIndex ) && ( globalIIndex <= xMax ) &&
          ( yMin <= globalJIndex ) && ( globalJIndex <= yMax ));

}


bool AppCtx::In_Processor_Range ( const int globalIIndex, const int globalJIndex )
{

  DMDALocalInfo dainfo;
  int xMin;
  int xMax;

  int yMin;
  int yMax;

  DMDAGetLocalInfo( *mapDA, &dainfo );

  xMin = dainfo.xs;
  xMax = dainfo.xs + ( dainfo.xm - 1 );

  yMin = dainfo.ys;
  yMax = dainfo.ys + ( dainfo.ym - 1 );

  //check that current node is within this processor range
  return (( xMin <= globalIIndex ) && ( globalIIndex <= xMax ) &&
          ( yMin <= globalJIndex ) && ( globalJIndex <= yMax ));

}

void AppCtx::Generate_Lithology_Identifier_List( const int Max_Nb_Lithology_Single_Proc,
                   int &Total_Number_Of_Lithologies )
{

  /* Local Variables */
  const int root = 0;

  struct Key_List {
    char Key_Name [ MaxLineSize ];
  };

  int*         Master_Litho_ID_List = 0;
  int*         Litho_ID_List = 0;
  Key_List*    Master_Litho_Key_List = 0;
  Key_List*    Litho_Key_List = 0;

  MPI_Datatype Key_List_type;
  MPI_Datatype type[1] = {MPI_CHAR};
  int          block_length[1] = {MaxLineSize};
  MPI_Aint     displacement[1] = {0};

  int Litho_counter = 0;
  string Key_String = "";
  int my_rank;
  int mpi_size;
  int I;

  String_Container Litho_List_Key_String;
  String_Container::iterator str_it;

  /* Allocate Memory for local ID and Key lists (all Processors) */
  Litho_ID_List  = new int[ Max_Nb_Lithology_Single_Proc ];
  Litho_Key_List = new Key_List[ Max_Nb_Lithology_Single_Proc ];

  /* Create an MPI datatype called Key_List_type allowing the of passing complex strings */
  MPI_Type_struct( 1, block_length, displacement, type, &Key_List_type );
  MPI_Type_commit( &Key_List_type );

  /* Each Processor populates its local key list */
  LithologyManager::CompoundLithologyIterator lm_iter;

  for ( lm_iter = FastcauldronSimulator::getInstance ().getLithologyManager ().begin();
        lm_iter != FastcauldronSimulator::getInstance ().getLithologyManager ().end();
        lm_iter++ ) {

    CompoundLithologyComposition lc = lm_iter->first;
    Key_String = lc.returnKeyString ();
    Copy_String_To_Char(Key_String, Litho_Key_List[Litho_counter++].Key_Name, MaxLineSize);
  }

  /* Find processor rank */
  MPI_Comm_rank(  PETSC_COMM_WORLD, &my_rank );

  /* if processor is root allocate and initialize master key list */
  if ( my_rank == root ) {

    /* Find number of processors */
    MPI_Comm_size( PETSC_COMM_WORLD, &mpi_size );

    Master_Litho_Key_List = new Key_List[ mpi_size * Max_Nb_Lithology_Single_Proc ];

    for ( I = 0; I < ( mpi_size * Max_Nb_Lithology_Single_Proc ); I++ ) {
      Master_Litho_Key_List[ I ].Key_Name[0] = '\0';
    }

  }

  /* Each process (root process included) sends the contents of its Litho_Key_List
     to the root process, store into Master_Litho_Key_List */
  MPI_Gather( Litho_Key_List, Max_Nb_Lithology_Single_Proc, Key_List_type,
         Master_Litho_Key_List, Max_Nb_Lithology_Single_Proc, Key_List_type,
         0, PETSC_COMM_WORLD );

  /* Root processes the Master_Litho_Key_List and attributes Litho Identifier (ID) */
  if ( my_rank == root ) {

    Total_Number_Of_Lithologies = 0;

    /* Allocate Memory for Global ID list (root only) */
    Master_Litho_ID_List = new int[ mpi_size * Max_Nb_Lithology_Single_Proc ];

    for ( I = 0; I < ( mpi_size * Max_Nb_Lithology_Single_Proc ); I++ ) {

      /* Initialize value to -1 */
      Master_Litho_ID_List[ I ] = -1;

      Key_String = string ( Master_Litho_Key_List[ I ].Key_Name );

      if ( Key_String == "" ) continue;

      str_it = Litho_List_Key_String.find( Key_String );

      if ( str_it == Litho_List_Key_String.end() ) {
          Master_Litho_ID_List[ I ] = Total_Number_Of_Lithologies;
          Litho_List_Key_String[ Key_String ] = Total_Number_Of_Lithologies++;
      } else {
          Master_Litho_ID_List[ I ] = str_it->second;
      }

    }


  }

  /* root process sends parts of the content of its Master_Litho_ID_List to all process,
     and store them into Litho_ID_List */
  MPI_Scatter( Master_Litho_ID_List, Max_Nb_Lithology_Single_Proc, MPI_INT,
          Litho_ID_List, Max_Nb_Lithology_Single_Proc, MPI_INT,
          0, PETSC_COMM_WORLD );


  // Tell all processes how many lithologies there are in total.
  MPI_Bcast( &Total_Number_Of_Lithologies, 1, MPI_INT, 0, PETSC_COMM_WORLD );

  /* Assign the Lithology Identifier (ID) to Lithology objects */
  Litho_counter = 0;
  for ( lm_iter = FastcauldronSimulator::getInstance ().getLithologyManager ().begin();
        lm_iter != FastcauldronSimulator::getInstance ().getLithologyManager ().end();
        lm_iter++ ) {

     Lithology* lithology = (Lithology*)(lm_iter->second);

     lithology->setLithologyID ( Litho_ID_List[ Litho_counter++ ] );
  }

  /* Deallocate arrays */
  MPI_Type_free( &Key_List_type );
  delete[] Litho_Key_List;
  delete[] Litho_ID_List;

  if ( my_rank == root ) {
    delete[] Master_Litho_Key_List;
    delete[] Master_Litho_ID_List;
  }

}

bool AppCtx::findActiveElements(const double time)
{
  // get the number of layers as we are going to need this a lot
  size_t nlayers = layers.size();

  // Set nr of active elements in mantle, crust & sea
  // to total amount of elements
  Crust() -> setNrOfActiveElements( Crust() -> getMaximumNumberOfElements() );
  Mantle() -> setNrOfActiveElements( Mantle() -> getMaximumNumberOfElements() );

  // Reset the other layers to 0 active elements
  int i;

  for (i=0; i<int(nlayers-2); i++) {
    layers[i]->setNrOfActiveElements(0);
  }

  // Now figure out where we are
  // If we are before the end of deposition of the crust
  if ( time > Crust() -> depoage ) {
    // our time is totally out of range
    return true;
  }

  // loop backwards through the layers i.e. upwards in the deposition sequence excluding the Mantle, Crust and Surface
  for (i = int(nlayers-3); i>=0; i--) {

    if (0 == layers[i]->getMaximumNumberOfElements()) continue;

    // if we have already deposited this layer then all elements are active
    if (time <= layers[i]->depoage) {
        layers[i]->setNrOfActiveElements(layers[i]->getMaximumNumberOfElements());
    } else {

      //Time is Greater (or Equal) than this layer's start of Deposition i.e. the end of the deposition of the layer above -> exit loop
      if (time > layers[i+1]->depoage || ( fabs(time-layers[i+1]->depoage) < time*1e-9 ) )
      {
         break;
      }

      //Set the number of active elements according to the proportion of layer deposited
      layers[i]->setNrOfActiveElements(layers[i]->getMaximumNumberOfElements());

      // terminate the processing as this is the last layer being deposited
      break;
    }
  }

  return true;
}

int AppCtx::getLayerIndex ( const string& name ) const {

   size_t i = 0;

   for ( i = 0; i < layers.size (); ++i ) {

      if (layers [ i ]->layername == name ) {
         return static_cast<int>(i);
      }

   }

   return -1;
}


LayerProps* AppCtx::findLayer(const string& LayerName) const
{
  size_t nr = 0;

  for (nr = 0; nr < layers.size(); nr++) {
    if (layers[nr]->layername == LayerName) {
      return layers[nr];
    }
  }
  cout << "Basin_Warning: Layer [" << LayerName << "] not found" << endl;
  return (LayerProps*)0;
}

int AppCtx::Find_Layer_Index(const double Deposition_Age,
                             const double tolerance ) const
{
  size_t Count;

  for ( Count = 0; Count < layers.size(); Count++ ) {

     if (( Deposition_Age != 0.0 && ( std::fabs ( Deposition_Age - layers [ Count ]->depoage ) / Deposition_Age ) < tolerance ) or
         ( Deposition_Age == 0.0 && ( std::fabs ( Deposition_Age - layers [ Count ]->depoage )) < tolerance )) {
        return static_cast<int>( Count );
     }

  }

  return -1;
}

LayerProps* AppCtx::Find_Layer_From_Surface(const string& SurfaceName)
{
  size_t nr = 0;

  for (nr = 0; nr < layers.size(); nr++) {
    if (layers[nr]->TopSurfaceName == SurfaceName) {
      return layers[nr];
    }
  }
  cout << "Basin_Warning: Layer associated with Top Surface[" << SurfaceName << "] not found" << endl;
  return (LayerProps*)0;
}

int AppCtx::findSurfacePosition ( const string& surfaceName ) const {

  size_t position = 0;

  for ( position = 0; position < layers.size(); position++ ) {

    if ( layers[position]->TopSurfaceName == surfaceName ) {
      return int ( position );
    }

  }

  cerr << "Layer associated with Top Surface name[" << surfaceName << "] not found" << endl;
  return -1;
}

int AppCtx::findFormationPosition ( const string& formationName ) const {

  size_t position = 0;

  for ( position = 0; position < layers.size(); position++ ) {

    if ( layers[position]->layername == formationName ) {
      return int ( position );
    }

  }

  cerr << "Layer associated with Formation name [" << formationName << "] not found" << endl;
  return -1;
}

//------------------------------------------------------------//

void AppCtx::selectSourceRockLayers ( LayerStack& selectedLayers  ) {

  size_t nr = 0;

  selectedLayers.clear ();

  for (nr = 0; nr < layers.size(); nr++) {

    if (layers[nr]->isActive() && layers[nr]->isSourceRock()) {
      selectedLayers.push_back(nr);
    }

  }

}

//------------------------------------------------------------//

const GeoPhysics::FluidType* AppCtx::findFluid(const string& FluidName)
{
   return (const GeoPhysics::FluidType*)(FastcauldronSimulator::getInstance ().findFluid ( FluidName ));
}

void AppCtx::print() {

  const CauldronGridDescription& grid = FastcauldronSimulator::getInstance ().getCauldronGridDescription ();

  cout << "Simple litho list size  = " << FastcauldronSimulator::getInstance ().getLithologyManager ().numberOfSimpleLithologies () << endl;
  cout << "Layer list size  = " << layers.size() << endl;
  cout << "Delta X          = " << grid.deltaI << endl;
  cout << "Delta Y          = " << grid.deltaJ << endl;
  cout << "Time step size   = " << timestepsize << endl;
  cout << "Time step incr   = " << timestepincr << endl;
  cout << "Time step decr   = " << timestepdecr << endl;

  FastcauldronSimulator::getInstance ().getLithologyManager ().printSimpleLithologies ();

  LayerList::iterator layerIter = layers.begin();
  while (layerIter != layers.end()) {
    (*layerIter)->print();
    layerIter++;
  }
}

void AppCtx::printFCT() {

  int xs, ys, xm, ym;
  size_t layer;
  DMDAGetCorners(*mapDA,
                 &xs, &ys, PETSC_IGNORE,
                 &xm, &ym, PETSC_IGNORE);


  for (layer=0; layer<layers.size(); layer++) {

    cout << "FCT points for layer " << layer << " (layer has ";
    cout << layers[layer]->getMaximumNumberOfElements() << " elements)" << endl;

    unsigned int element;

    for (element=0; element < layers[layer]->getMaximumNumberOfElements(); element++) {
      cout << "Element " << element << endl;
      int i, j;
      for (i=0; i < xm; i++) {
   for (j=0; j < ym; j++) {
     cout << i << "," << j << " : ";
     layers[layer]->getSolidThickness ( i, j, 0 ).printPoints();
   }
      }
    }
  }

}


void AppCtx::setUp2dEltMapping()
{


   int QuadIndices[4][2] = {{0,0},{1,0},{1,1},{0,1}};

   int xs,ys,xm,ym,xdim,ydim;

   DMDAGetInfo(*mapDA,PETSC_IGNORE,&xdim,&ydim,PETSC_IGNORE,PETSC_IGNORE,
               PETSC_IGNORE,PETSC_IGNORE,PETSC_IGNORE,PETSC_IGNORE,PETSC_IGNORE,PETSC_IGNORE,PETSC_IGNORE,PETSC_IGNORE);
   DMDAGetCorners(*mapDA,&xs,&ys,PETSC_IGNORE,&xm,&ym,PETSC_IGNORE);

   int i, j;

   for (j=ys; j<ys+ym; j++) {

      for (i=xs; i<xs+xm; i++) {

         if (i == xdim-1 || j == ydim-1 ) {
            continue;
         } else {
            Elt2dIndices EltIndices;
            int Inode;
            bool ElementExists = true;

            for (Inode = 0; Inode<4; Inode++) {
               int idxX = QuadIndices[Inode][0];
               int idxY = QuadIndices[Inode][1];

               if ( !nodeIsDefined ( i + idxX, j + idxY )) {
                  EltIndices.nodeDefined[Inode] = false;
                  ElementExists = false;
               } else {
                  EltIndices.nodeDefined[Inode] = true;
               }

               EltIndices.i[Inode] = i+idxX;
               EltIndices.j[Inode] = j+idxY;
               EltIndices.exists = ElementExists;
            }

            mapElementList.push_back(EltIndices);
         }

      }

   }

}

void AppCtx::Examine_Load_Balancing() {

  int xs,ys,xm,ym;
  DMDAGetCorners(*mapDA, &xs, &ys, PETSC_IGNORE, &xm, &ym, PETSC_IGNORE);

  int Number_Of_Valid_Nodes = xm*ym;

  int NNodeX,NNodeY;
  DMDAGetInfo( *mapDA, PETSC_IGNORE, &NNodeX, &NNodeY, PETSC_IGNORE, PETSC_IGNORE, PETSC_IGNORE,
               PETSC_IGNORE, PETSC_IGNORE, PETSC_IGNORE, PETSC_IGNORE, PETSC_IGNORE, PETSC_IGNORE, PETSC_IGNORE );

  float Percentage_Of_Total_Number_Of_Nodes = float(xm*ym) / float(NNodeX*NNodeY) * 100.0;

  int i,j;
  for ( i = xs; i < xs+xm; i++ ) {
    for ( j = ys; j < ys+ym; j++ ) {
      if ( !nodeIsDefined ( i, j )) Number_Of_Valid_Nodes--;
    }
  }

  float Percentage_Of_Active_Nodes = float(Number_Of_Valid_Nodes) / float(xm*ym) * 100.0;

  PetscPrintf(PETSC_COMM_WORLD,
         "\n------------ Load Balancing Information ------------\n");
  PetscPrintf(PETSC_COMM_WORLD,
         " CPU  Nb Nodes   Percent. Total      Percent. Activity\n");

  PetscSynchronizedPrintf(PETSC_COMM_WORLD," %3d %8d %13.1f %19.1f\n",
                          FastcauldronSimulator::getInstance ().getRank (),xm*ym,
           Percentage_Of_Total_Number_Of_Nodes,
           Percentage_Of_Active_Nodes);

  PetscSynchronizedFlush(PETSC_COMM_WORLD, PETSC_STDOUT);

  PetscPrintf(PETSC_COMM_WORLD,
         "\n---------------------------------------------------------\n");

}

void AppCtx::Output_Number_Of_Geological_Events() {

   database::Table * snapshotIoTbl;
   snapshotIoTbl = FastcauldronSimulator::getInstance ().getTable ("SnapshotIoTbl");
   PETSC_ASSERT(snapshotIoTbl);
   if (snapshotIoTbl->size() > 0)
   {
     PetscPrintf ( PETSC_COMM_WORLD, " Number of Geological Events : %d\n", projectSnapshots.numberOfMajorSnapshots ());
   }

}

//------------------------------------------------------------//


void AppCtx::Display_Grid_Description() {

  DMDALocalInfo dainfo;
  DMDAGetLocalInfo( *mapDA, &dainfo );

  PetscSynchronizedPrintf(PETSC_COMM_WORLD,"rank            %3d \n",FastcauldronSimulator::getInstance ().getRank ());
  PetscSynchronizedPrintf(PETSC_COMM_WORLD,"mx , my, mz     %3d %3d %3d\n",dainfo.mx,dainfo.my,dainfo.mz);
  PetscSynchronizedPrintf(PETSC_COMM_WORLD,"xs , ys, zs     %3d %3d %3d\n",dainfo.xs,dainfo.ys,dainfo.zs);
  PetscSynchronizedPrintf(PETSC_COMM_WORLD,"xm , ym, zm     %3d %3d %3d\n",dainfo.xm,dainfo.ym,dainfo.zm);
  PetscSynchronizedPrintf(PETSC_COMM_WORLD,"gxs , gys, gzs  %3d %3d %3d\n",dainfo.gxs,dainfo.gys,dainfo.gzs);
  PetscSynchronizedPrintf(PETSC_COMM_WORLD,"gxm , gym, gzm  %3d %3d %3d\n",dainfo.gxm,dainfo.gym,dainfo.gzm);

}

#undef __FUNCT__
#define __FUNCT__ "Create_Reference_DA_for_Io_Maps"

bool AppCtx::Create_Reference_DA_for_Io_Maps () {

  int ierr;

  ierr = FastcauldronSimulator::DACreate2D ( Reference_DA_For_Io_Maps );

  CHKERRQ(ierr);

  return NO_ERROR;

}


AppCtx::~AppCtx(){

  size_t indx;

  layers.clear ();

  for ( indx = 0; indx < Related_Projects.size (); indx++ ) {
    delete Related_Projects [ indx ];
  }

  Related_Projects.clear ();

  if ( Reference_DA_For_Io_Maps != NULL ) DMDestroy( &Reference_DA_For_Io_Maps );

  if ( xCoarseGridPartitioning == 0 ) {
    delete [] xCoarseGridPartitioning;
    delete [] yCoarseGridPartitioning;
  }

}

void AppCtx::Locate_Related_Project( )
{

   const CauldronGridDescription& grid = FastcauldronSimulator::getInstance ().getCauldronGridDescription ();

  const double Origin_X = grid.originI;
  const double Origin_Y = grid.originJ;
  const int    Number_Of_X_Nodes = grid.nrI;
  const int    Number_Of_Y_Nodes = grid.nrJ;
  const double Delta_X = grid.deltaI;
  const double Delta_Y = grid.deltaJ;

  double X_Coord;
  double Y_Coord;

  size_t indx;

  for ( indx = 0; indx < Related_Projects.size (); indx++ )
  {

    Related_Project_Ptr WellLoc = Related_Projects [ indx ];

    WellLoc -> Exists = false;

    X_Coord = WellLoc -> X_Coord;
    Y_Coord = WellLoc -> Y_Coord;

    if ( X_Coord <= Origin_X )
    {
      WellLoc -> In_Range = false;
    }
    else if ( X_Coord > Origin_X + double ( Number_Of_X_Nodes ) * Delta_X )
    {
      WellLoc -> In_Range = false;
    }
    else
    {
      WellLoc -> In_Range = true;
      WellLoc -> X_Position = int (( X_Coord - Origin_X ) / Delta_X );
    }

    if ( ! WellLoc -> In_Range ) continue;

    if ( Y_Coord <= Origin_Y )
    {
      WellLoc -> In_Range = false;
    }
    else if ( Y_Coord > Origin_Y + double ( Number_Of_Y_Nodes ) * Delta_Y )
    {
      WellLoc -> In_Range = false;
    }
    else
    {
      WellLoc -> In_Range = true;
      WellLoc -> Y_Position = int (( Y_Coord - Origin_Y ) / Delta_Y );
    }

    if ( ! WellLoc -> In_Range ) continue;

    WellLoc -> In_Range = In_Processor_Range ( WellLoc -> X_Position, WellLoc -> Y_Position );

    if ( WellLoc -> In_Range )
    {

       if ( nodeIsDefined ( WellLoc -> X_Position, WellLoc -> Y_Position ))
      {
   WellLoc -> Exists = true;
   cout << FastcauldronSimulator::getInstance ().getRank () << " " << WellLoc -> Name << " "
        << WellLoc -> X_Position << " " << WellLoc -> Y_Position << endl;
      }
    }

  }

}

//------------------------------------------------------------//


// What is this used for?
double AppCtx::Minimum_Permeability ( const LayerProps* Current_Layer ) const {

   cout << " AppCtx::Minimum_Permeability "  << endl;

  const double Porosity = 0.03;
  //
  //
  // The approximate VES at 10km depth in a hydrostatically pressure basin
  //
  const double VES      = 120.0e6;

  double Permeability = 100.0;
  double Lithology_Permeability_Normal;
  double Lithology_Permeability_Plane;

  int X_Start;
  int Y_Start;
  int Z_Start;

  int X_Count;
  int Y_Count;
  int Z_Count;

  int I, J;

  DMDAGetCorners ( *mapDA, &X_Start, &Y_Start, &Z_Start, &X_Count, &Y_Count, &Z_Count );

  for ( I = X_Start; I < X_Start + X_Count; I++ ) {

    for ( J = Y_Start; J < Y_Start + Y_Count; J++ ) {

       if ( m_nodeIsDefined ( I, J )) {
        Current_Layer->getLithology( I, J )->calcBulkPermeabilityNP ( VES, VES, Porosity,
                                                                      Lithology_Permeability_Normal,
                                                                      Lithology_Permeability_Plane );

        Permeability = NumericFunctions::Minimum ( Permeability, NumericFunctions::Minimum ( Lithology_Permeability_Normal, Lithology_Permeability_Plane ));
      }

    }

  }

  return Permeability;
}


//------------------------------------------------------------//


void AppCtx::Fill_Topmost_Segment_Arrays ( const double Previous_Time,
                                           const double Current_Time ) {


  Layer_Iterator All_Layers ( layers,
                              Basin_Modelling::Descending,
                              Basin_Modelling::Sediments_Only,
                              Basin_Modelling::Active_Layers_Only );

  while ( ! All_Layers.Iteration_Is_Done () ) {
    All_Layers.Current_Layer () -> Fill_Topmost_Segment_Arrays ( Previous_Time, Current_Time, Use_Geometric_Loop, m_nodeIsDefined );
    All_Layers++;
  }

}

//------------------------------------------------------------//

void AppCtx::setLayerData () {

  size_t i;

  for ( i = 0; i < layers.size() - 2; i++ ) {
    layers [ i ]->depositionStartAge = layers [ i + 1 ] -> depoage;
  }

  // Sometime before the burial of the sediments.
  Crust_Layer->depositionStartAge  = 4500.0;
  Mantle_Layer->depoage = 4500.0;
  Mantle_Layer->depositionStartAge = 5000.0;

}

bool AppCtx::calcBasementProperties ( const double Current_Time ) {

   int Layer_K;
   size_t layer;
   int xs, ys, zs, xm, ym, zm;
   int layerMx, layerMy, layerMz;

   ElementGeometryMatrix Geometry_Matrix;
   const CompoundLithology *  Element_Lithology;

   DMDAGetCorners ( *mapDA, &xs, &ys, PETSC_IGNORE, &xm, &ym, PETSC_IGNORE );

   CauldronCrustFormation * crust = dynamic_cast<CauldronCrustFormation *>(layers [layers.size () - 2]);
   assert( crust != 0 );

   CauldronMantleFormation * mantle = dynamic_cast<CauldronMantleFormation *>(layers [layers.size () - 1]);
   assert( mantle != 0 );

   PETSC_2D_Array basaltThicknessALC    ( *mapDA, crust->ThicknessBasaltALC );
   PETSC_2D_Array topBasaltDepth        ( *mapDA, crust->TopBasaltDepth );
   PETSC_2D_Array bottomBasaltDepth     ( *mapDA, crust->BottomBasaltDepth );
   PETSC_2D_Array ccrustThickness       ( *mapDA, crust->ThicknessCCrustALC );
   PETSC_2D_Array smoothBasaltThickness ( *mapDA, crust->BasaltThickness );
   PETSC_2D_Array smCrustThickness      ( *mapDA, crust->SmCCrustThickness );
   PETSC_2D_Array smTopBasaltDepth      ( *mapDA, crust->SmTopBasaltDepth );
   PETSC_2D_Array smBottomBasaltDepth   ( *mapDA, crust->SmBottomBasaltDepth );
   PETSC_2D_Array origMantleDepth       ( *mapDA, mantle->UpliftedOrigMantleDepth );
   PETSC_2D_Array hlmod                 ( *mapDA, mantle->LithosphereThicknessMod );

   DMDAGetCorners ( *mapDA, &xs, &ys, PETSC_IGNORE, &xm, &ym, PETSC_IGNORE );

   PETSC_3D_Array depth ( crust->layerDA, crust->Current_Properties ( Basin_Modelling::Depth ));
   PETSC_3D_Array mantleDepth ( mantle->layerDA, mantle->Current_Properties ( Basin_Modelling::Depth ));

   int m_kIndexCrust  = crust->getMaximumNumberOfElements ();
   int m_kIndexMantle = mantle->getMaximumNumberOfElements ();

   // cout << "Time = " <<  Current_Time << ", Num of elements crust = " << m_kIndexCrust << ", Num of elements mantle = " << mantle-> getNrOfElements () << endl;
   unsigned int i, j;
   double crustThinningRatio;
   const double initCrustThickness  = FastcauldronSimulator::getInstance ().getCrustFormation()->getInitialCrustalThickness();
   const double initMantleThickness = FastcauldronSimulator::getInstance ().getMantleFormation ()->getInitialLithosphericMantleThickness ();

   for ( i = xs; i < xs + xm; ++i ) {
       for ( j = ys; j < ys + ym; ++j ) {
          if(FastcauldronSimulator::getInstance ().nodeIsDefined ( i, j )) {

             if( bottomBasaltDepth( j, i ) != CauldronNoDataValue && topBasaltDepth( j, i ) != CauldronNoDataValue ) {
                basaltThicknessALC( j, i ) = bottomBasaltDepth( j, i ) - topBasaltDepth( j, i );
             }

             if( topBasaltDepth( j, i ) == CauldronNoDataValue ) {
                topBasaltDepth( j, i ) = depth( 0, j, i ); //bottomBasaltDepth( j, i );
             }

             smTopBasaltDepth( j, i ) = depth( m_kIndexCrust, j, i ) + FastcauldronSimulator::getInstance ().getContCrustThickness(i, j, Current_Time);
             ccrustThickness( j, i )  = topBasaltDepth( j, i ) - depth( m_kIndexCrust, j, i );
             smBottomBasaltDepth( j, i ) = smTopBasaltDepth( j, i ) + FastcauldronSimulator::getInstance ().getBasaltThickness(i, j, Current_Time);

             if( bottomBasaltDepth( j, i ) == CauldronNoDataValue ) {
                 bottomBasaltDepth( j, i ) = depth( 0, j, i );
             }

             smoothBasaltThickness( j, i ) = FastcauldronSimulator::getInstance ().getBasaltThickness(i, j, Current_Time);
             hlmod( j, i ) =  depth( m_kIndexCrust, j, i ) + FastcauldronSimulator::getInstance ().getLithosphereThicknessMod(i, j, Current_Time);
             smCrustThickness( j, i ) = FastcauldronSimulator::getInstance ().getContCrustThickness(i, j, Current_Time);

             crustThinningRatio = FastcauldronSimulator::getInstance ().getCrustThickness( i, j, Current_Time ) / initCrustThickness; // Effective Crustal Thickness / initial Crustal Thickness

             origMantleDepth( j, i ) = mantleDepth( m_kIndexMantle, j, i ) + initMantleThickness * crustThinningRatio;
          }
       }
   }
   return true;
}

//------------------------------------------------------------//
