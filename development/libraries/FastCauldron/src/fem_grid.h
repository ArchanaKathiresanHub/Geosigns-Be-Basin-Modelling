//
// Copyright (C) 2015-2016 Shell International Exploration & Production.
// All rights reserved.
//
// Developed under license for Shell by PDS BV.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

//------------------------------------------------------------//

#ifndef __FEM_Grid_HH__
#define __FEM_Grid_HH__

//------------------------------------------------------------//

#include "GridMap.h"

//------------------------------------------------------------//

#include "ConstantsFastcauldron.h"
#include "history.h"
#include "propinterface.h"
#include "snapshotdata.h"
#include "temperature_solver.h"

#include "PressureSolver.h"

#include "LayerElement.h"
#include "BoundaryId.h"
#include "Subdomain.h"

#include "VreOutputGrid.h"
#include "ChemicalCompactionGrid.h"

#include "ComputationalDomain.h"

class TemperatureForVreInputGrid;
class VitriniteReflectance;

//------------------------------------------------------------//

namespace Basin_Modelling {


  class FEM_Grid {

  public :

    explicit FEM_Grid ( AppCtx* Application_Context );
    ~FEM_Grid ();

    //----------------------------//

     /// \brief Solve for the over-pressure only.
     ///
     /// Solves the pressure equation for either geometric or non-geometric loop
     /// over the entire history of the basin specified in the project file.
     void solvePressure ( bool& solverHasConverged,
                          bool& errorInDarcy,
                          bool& geometryHasConverged );

     /// \brief Solve for the temperature only.
     ///
     /// Solves the temperature equation using a hydrostatic pressure function
     /// over the entire history of the basin specified in the project file.
     void solveTemperature ( bool& solverHasConverged,
                             bool& errorInDarcy );

     /// \brief Solve for both the over-pressure and the temperature.
     ///
     /// Solves the coupled pressure-temperature equations over the
     /// entire history of the basin specified in the project file.
     /// The pressure can be either geometric or non-geometric loop.
     void solveCoupled ( bool& solverHasConverged,
                         bool& errorInDarcy,
                         bool& geometryHasConverged );

     AppCtx* getAppCtx( ) const;
     //----------------------------//

  private :

     /// \brief Solve the pressure equation from the begining of deposition to present day.
     ///
     /// If the user has selected the geometric loop, then this will be one geometric iteration.
     void Evolve_Pressure_Basin ( const int   Number_Of_Geometric_Iterations,
                                        bool& overpressureHasDiverged,
                                        bool& errorInDarcy );

     /// \brief Solve the temperature equation from the begining of deposition to present day.
     void Evolve_Temperature_Basin ( bool& temperatureHasDiverged,
                                     bool& errorInDarcy );

     /// \brief Solve the coupled pressure and temperature equations.
     ///
     /// Solve the coupled pressure and temperature equations from the begining of
     /// deposition to present day, if the user has selected the geometric loop,
     /// then this will be one geometric iteration.
     void Evolve_Coupled_Basin ( const int   Number_Of_Geometric_Iterations,
                                       bool& hasDiverged,
                                       bool& errorInDarcy );


     /// \brief Advance the currentTime by the timeStep.
     bool Step_Forward ( double& previousTime,
                         double& currentTime,
                         double& Time_Step,
                         bool&   majorSnapshotTimesUpdated );

     /// \brief Compute the next time step for overpressure calculations.
     ///
     /// It is dependant on:
     ///
     ///        1) The current time step;
     ///        2) The increase/decrease factors (defined in the project file);
     ///        3) The maximum difference between the overpressure in the current
     ///           and previous time steps;
     ///        4) How many Newton iterations the current time step took to converge.
     ///
     void Determine_Next_Pressure_Time_Step ( const double  currentTime,
                                                    double& Time_Step,
                                              const int     Number_Of_Newton_Iterations );

     /// \brief Compute the next time step for temperature calculations.
     ///
     /// It is dependant on:
     ///
     ///        1) The current time step;
     ///        2) The increase/decrease factors (defined in the project file);
     ///        3) The maximum difference between the temperature in the current
     ///           and previous time steps;
     ///        4) The maximum difference in the temperature of the source rock
     ///           layers in the current and previous time steps;
     ///
     void Determine_Next_Temperature_Time_Step ( const double  currentTime,
                                                       double& Time_Step );

     /// \brief Compute the next time step for overpressure calculations.
     ///
     /// It is dependant on:
     ///
     ///        1) The current time step;
     ///        2) The increase/decrease factors (defined in the project file);
     ///        3) The maximum difference between the overpressure and temperature in the
     ///           current and previous time steps;
     ///        4) The maximum difference in the temperature of the source rock
     ///           layers in the current and previous time steps;
     ///        5) How many Newton iterations the overpressure took to converge for the
     ///           current time step.
     ///
     void Determine_Next_Coupled_Time_Step  ( const double  currentTime,
                                                    double& Time_Step );

     /// \brief Compute the next time step for permafrost calculations.
     void Determine_Permafrost_Time_Step ( const double  currentTime, double & Time_Step );

     /// \ brief Compute the next time step for igneous intrusion
     void determineIgneousIntrusionTimeStep ( const double currentTime, const double previousTimeStep, double & Time_Step );

     /// \brief Allocate the Finite element grids for overpressure and temperature.
     ///
     /// The grids are different for both calculations so 2 are needed.
     void Construct_FEM_Grid ( const double                    previousTime,
                               const double                    currentTime,
                               const SnapshotEntrySetIterator& majorSnapshots,
                               const bool                      majorSnapshotTimesUpdated );


     /// \brief Set all vector used in transient calculation to zero.
     void clearLayerVectors ();

     /// \brief Set properties that are not computed when solver for hydrostatic temperature.
     ///
     /// During the temperature calculation there are properties that need to be
     /// computed (porosity, ...) that depend on the pressures (hydrostatic, ...).
     /// There properties are computed here.
     void Set_Pressure_Dependent_Properties ( const double currentTime );

     /// \brief Solve the pressure for the current time time, using a Newton solver for the nonlinear equation.
     void Solve_Pressure_For_Time_Step    ( const double  previousTime,
                                            const double  currentTime,
                                            const int     Maximum_Number_Of_Nonlinear_Iterations,
                                                  bool&   overpressureHasDiverged,
                                                  int&    Number_Of_Nonlinear_Iterations,
                                                  double& Po_Norm,
                                                  bool&   fracturingOccurred );

     /// \brief Solve the temperature for the current time, which solver is used will depend on the user input.
     ///
     /// The options are nonlinear and linear.
     void Solve_Temperature_For_Time_Step ( const double  previousTime,
                                            const double  currentTime,
                                            const int     Maximum_Number_Of_Nonlinear_Iterations,
                                                  bool&   temperatureHasDiverged,
                                                  int&    Number_Of_Nonlinear_Iterations,
                                                  double& T_Norm );

     /// \brief Solve the temperature for the current time step using a Newton solver.
     void Solve_Nonlinear_Temperature_For_Time_Step ( const double  previousTime,
                                                      const double  currentTime,
                                                      const int     Maximum_Number_Of_Nonlinear_Iterations,
                                                      const bool    isSteadyStateCalculation,
                                                            bool&   temperatureHasDiverged,
                                                            int&    Number_Of_Nonlinear_Iterations,
                                                            double& T_Norm );

     /// \brief Solve the linearised temperature for the current time step.
     void Solve_Linear_Temperature_For_Time_Step ( const double  previousTime,
                                                   const double  currentTime,
                                                         bool&   temperatureHasDiverged,
                                                         double& T_Norm );

     /// \brief Solve the coupled pressure-temperature equations.
     ///
     /// The pressure equation is solved first followed immediately by the temperature.
     /// It is assumed that the overpressure affects the temperature more than vice-versa.
     void Solve_Coupled_For_Time_Step     ( const double  previousTime,
                                            const double  currentTime,
                                            const int     Maximum_Number_Of_Nonlinear_Pressure_Iterations,
                                            const int     Maximum_Number_Of_Nonlinear_Temperature_Iterations,
                                                  bool&   hasDiverged,
                                                  int&    Number_Of_Nonlinear_Iterations,
                                                  double& Po_Norm,
                                                  double& T_Norm,
                                                  bool&   fracturingOccurred );

     /// The properties that are computed as a part of the calculation are copied
     /// from the current time step properties to the previous time step properties.
     void Copy_Current_Properties ();

    ///
    /// If a needle, or a layer, is entirely eroded then at the end of an overpressure
    /// calculation there is no reference to which it can be judged to have converged
    /// to the correct thickness. Therefore the values used to determine convergence are
    /// the calculated thickness at end of deposition of the layer and the deposition
    /// thickness input in the stratigraphy table. The value that is stored is this
    /// thickness at deposition.
    ///
    void Store_Computed_Deposition_Thickness ( const double currentTime );



    /// \brief Print a fixed set of properties for a related project.
    ///
    /// Outputs the properties at the needle that is closest to the point specified
    /// by the related project. If no related project is specified then the (0,0)
    /// needle is taken.
    void printRelatedProjects ( const double currentAge ) const;

    /// \brief Save properties to disk.
    void Save_Properties ( const double currentTime );

     /// \brief Solves the steady state temperature equation for the basement.
     ///
     /// Basically this pre-heats the basement.
     void Initialise_Basin_Temperature ( bool& temperatureHasDiverged );

     ///
     ///
     ///
     void Print_Needle ( const double currentAge, const int I, const int J ) const;

     void printElementNeedle ( const int i, const int j ) const;

     /// \brief Integrate the chemical-compaction equation.
     void integrateChemicalCompaction ( const double previousTime,
                                        const double currentTime );

     /// \brief Integrate the Genex5 equations over the time interval.
     void integrateGenex ( const double previousTime,
                           const double currentTime );

     /// \brief Perform any operations that are required after the time-step.
     ///
     /// Zeroing vectors, deleting vectors, ...
     void postTimeStepOperations ( const double currentTime );

     /// \brief Clear the source-rock-node-output objects from the source-rock nodes.
     void clearGenexOutput ();

     /// \brief Reset the transported mass vectors to zero.
     void zeroTransportedMass ( const double currentTime );

     /// \brief Compute temperature dependant properties when solving for over-pressure only.
     void Compute_Temperature_Dependant_Properties ( const double currentTime );


     /// Create the DA that is used for the entire pressure calculation (all sediments),
     /// then, using this, create the Vec's for the Depths and DOFs for the pressure grid.
     void Construct_Pressure_FEM_Grid    ( const double previousTime,
                                           const double currentTime );

     /// Create the DA that is used for the entire temperature calculation (all sediments and basement)
     /// then, using this, create the Vec's for the Depths and DOFs for the temperature grid.
     void Construct_Temperature_FEM_Grid ( const double                    previousTime,
                                           const double                    currentTime,
                                           const SnapshotEntrySetIterator& majorSnapshots,
                                           const bool                      majorSnapshotTimesUpdated );

     /// \brief Assign current values to layer-elements.
     void setLayerElements ( const double age );

     /// \brief Log max residual locations
     void logMaxDeltaOverPressureLocations( const std::vector<int>& locationMaxValue, const double maxResidual,
                                   const int MaximumNumberOfNonlinearIterations ) const;

     ///------------------------------------------------------------//


     AppCtx*     basinModel;
     int Number_Of_X_Processors;
     int Number_Of_Y_Processors;

     PetscScalar Pressure_Newton_Solver_Tolerance;
     PetscScalar Temperature_Newton_Solver_Tolerance;

     int Local_Number_Of_X_Nodes;
     int Local_Number_Of_Y_Nodes;

     int Number_Of_X_Nodes;
     int Number_Of_Y_Nodes;
     int Number_Of_Pressure_Z_Nodes;
     int Number_Of_Temperature_Z_Nodes;
     int Total_Number_Of_Pressure_Nodes;
     int Total_Number_Of_Temperature_Nodes;

     int pressureStencilWidth;
     int temperatureStencilWidth;


     double Origin_X, Origin_Y, Delta_X, Delta_Y;

     bool Include_Ghost_Values;
     bool Update_Ghost_Values;

     SnapshotEntrySetIterator majorSnapshots;
     SnapshotEntrySetIterator minorSnapshots;
     snapshottimeContainer    savedMinorSnapshotTimes;

     //*{
     ///
     /// Time taken to perform various tasks for a single geometric iteration
     ///
     PetscLogDouble System_Assembly_Time;
     PetscLogDouble System_Solve_Time;
     PetscLogDouble Element_Assembly_Time;
     PetscLogDouble Property_Calculation_Time;
     PetscLogDouble Property_Saving_Time;

     PetscLogDouble m_temperatureDomainResetTime;
     PetscLogDouble m_temperatureMatrixAllocationTime;
     PetscLogDouble m_temperatureSolutionMappingTime;

     //*}

     //*{
     ///
     /// Time taken to perform various tasks for the entire calculation.
     ///
     PetscLogDouble Accumulated_System_Assembly_Time;
     PetscLogDouble Accumulated_System_Solve_Time;
     PetscLogDouble Accumulated_Element_Assembly_Time;
     PetscLogDouble Accumulated_Property_Calculation_Time;
     PetscLogDouble Accumulated_Property_Saving_Time;
     //*}

     /// Wrapper grid that saves computed Vre data to the grid
     VreOutputGrid                              m_vreOutputGrid;

     /// The vitrinte reflectance algorithm
     std::unique_ptr<GeoPhysics::VitriniteReflectance> m_vreAlgorithm;

     /// Grid that links computation of chemical compaction and grid
     std::unique_ptr<ChemicalCompactionGrid>  m_chemicalCompactionGrid;
     ///and corresponding chemical compaction calculator
     std::unique_ptr<ChemicalCompactionCalculator>  m_chemicalCompactionCalculator;

     Temperature_Solver Temperature_Calculator;
     PressureSolver*    pressureSolver;


     PropListVec mapOutputProperties;
     PropListVec m_volumeOutputProperties;
     PropListVec genexOutputProperties;
     PropListVec shaleGasOutputProperties;
     PropListVec looselyCoupledOutputProperties;
     PropListVec looselyCoupledOutputMapProperties;

     /// List of map properties which are only output at present day, t=0Ma
     PropListVec m_concludingMapOutputProperties;
     /// List of volume properties which are only output at present day, t=0Ma
     PropListVec m_concludingVolumeOutputProperties;
     PropListVec m_combinedVolumeOutputProperties;

     ///
     /// Records a fixed set of properties at a node on a surface.
     /// These properties are recorded for each time step as soon as the node comes into existence.
     ///
     History     m_surfaceNodeHistory;

     ComputationalDomain m_temperatureComputationalDomain;
     ComputationalDomain m_pressureComputationalDomain;

     PetscBool m_saveMatrixToFile;     // Boolean flag for saving matrix and rhs to file
     PetscBool m_saveInMatlabFormat;   // TRUE: matlab format, FALSE: binary
     double    m_saveTimeStep;         // Time step for matrix and rhs save
  }; // end class FEM_Grid


   typedef FEM_Grid* FEM_Grid_Ptr;


} // end namespace Basin_Modelling


//------------------------------------------------------------//

#endif // __FEM_Grid_HH__

//------------------------------------------------------------//
