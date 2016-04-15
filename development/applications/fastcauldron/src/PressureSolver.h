//
// Copyright (C) 2015-2016 Shell International Exploration & Production.
// All rights reserved.
//
// Developed under license for Shell by PDS BV.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#ifndef _FASTCAULDRON__PRESSURE_SOLVER_H_
#define _FASTCAULDRON__PRESSURE_SOLVER_H_


#include "petsc.h"
#include "petscts.h"
#include "petscdmda.h"

#include "BasisFunctionCache.h"

#include "propinterface.h"
#include "globaldefs.h"
#include "layer.h"

#include "ComputationalDomain.h"
#include "BoundaryConditions.h"
#include "GeneralElement.h"
#include "Saturation.h"

class PressureSolver {

public :

   static const double GeometricLoopThicknessTolerance;

   /// \brief The default value for the maximum number of linear solver iterations.
   static const int DefaultMaximumPressureLinearSolverIterations;


   /// \brief The maximum number of attempts that can be made when 
   static const int MaximumLinearSolveAttempts = 6;

   /// \brief The maximum number of GMRES restarts allowed
   static const int MaximumGMResRestartValue;

   /// \brief The amount by which the restart values is increased for each failure in the linear solver.
   static const int GMResRestartIncrementValue;

   /// \brief The default value for the gmres restart value.
   ///
   /// This is used only when the default solver fails 
   static const int DefaultGMResRestartValue;



   PressureSolver(AppCtx *appl);

   virtual ~PressureSolver();

   virtual void adjustSolidThickness ( const double relativeThicknessTolerance,
                                       const double absoluteThicknessTolerance,
                                             bool&  geometryHasConverged ) = 0;

   virtual void computeDependantProperties ( const double previousTime, 
                                             const double currentTime, 
                                             const bool   outputProperties ) = 0;

   virtual void initialisePressureProperties ( const double previousTime, 
                                               const double currentTime ) = 0;


   /// \brief Check the overpressure solution and update pore- and lithostatic pressures.
   void checkPressureSolution ();

   /// Assemble the Jacobian and residual for the Newton solver.
   void assembleSystem ( const ComputationalDomain& computationalDomain,
                         const double               previousTime,
                         const double               currentTime,
                         Mat&                       jacobian,
                         Vec&                       residual,
                         double&                    elementContributionsTime );


   ///
   /// Find the maximum difference between the overpressure solution at the 
   /// current and previous time steps.
   ///
   PetscScalar maximumPressureDifference ();
   PetscScalar maximumPressureDifference2 ();


   int getPlaneQuadratureDegree ( const int optimisationLevel ) const;

   int getDepthQuadratureDegree ( const int optimisationLevel ) const;

   /// Set the maximum number of Newton iterations that can be performed in the overpressure calculation.
   static void setMaximumNumberOfNonlinearIterations ( const int optimisationLevel,
                                                       const int maximumIterationCount );

   /// The maximum number of Newton iterations that can be performed in the overpressure calculation.
   static int getMaximumNumberOfNonlinearIterations ( const int optimisationLevel );

   /// The tolerance for the linear solver.
   double linearSolverTolerance ( const int Optimisation_Level ) const;

   /// Used in the geometric loop for overpressure and coupled calculations.
   ///
   /// At the end of each basin evolution (from basin age to present day)
   /// a real thickness is predicted, this is likely to differ from that 
   /// input in the strat table. How much of a difference can the user accept?
   /// It is determined by the optimisation level.
   double getRelativeThicknessTolerance ( const int optimisationLevel ) const;

   /// \brief Returns the absolute tolerance, in metres, for the layer.
   ///
   /// This is the special case of layers having a thickness that is less than 100 mts.
   double getAbsoluteThicknessTolerance ( const int optimisationLevel ) const;

   /// Returns the number of steps used in integrating the compaction equation.
   int numberOfStepsForCompactionEquation ( const int optimisationLevel ) const;

   static void setPlaneQuadratureDegree ( const int optimisationLevel,
                                          const int newDegree );

   static void setDepthQuadratureDegree ( const int optimisationLevel,
                                          const int newDegree );

   /// \brief The tolerance that the Newton solver for the overpressure should use.
   ///
   /// As we iterate in the geometric loop, the tolerance gets tighter.
   static double getNewtonSolverTolerance ( const int  optimisationLevel,
                                            const bool isGeometricLoop,
                                            const int  geometricLoopNumber );

   static void setNewtonSolverTolerance ( const int    optimisationLevel,
                                          const double newTolerance );

   static int getIterationsForIluFillLevelIncrease ( const int optimisationLevel,
                                                     const int currentIluFillLevel );

   static void setIterationsForIluFillLevelIncrease ( const int newIluFillLevelIterations );

   /// During a coupled calculation the depths of the basement will be different from 
   /// those calculated during a hydrostatic pressured temperature run. This is due to
   /// the different amount of solid material deposited (due to overpressure).
   void setBasementDepths ( const double           Current_Time,
                                  Double_Array_2D& Depth_Above,
                            const Boolean2DArray&  Valid_Needle );

protected :

   static int PlaneQuadratureDegrees [ NumberOfOptimisationLevels ];

   static int DepthQuadratureDegrees [ NumberOfOptimisationLevels ];

   static double NewtonSolverTolerances [ NumberOfOptimisationLevels ][ 3 ];

   static int s_iterationsForiluFillLevelIncrease [ NumberOfOptimisationLevels ];

   static int s_numberOfIterations [ NumberOfOptimisationLevels ];


   void initialiseFctCorrection ();

   /// \brief Get the boundary conditions for the pressure equation.
   void getBoundaryConditions ( const GeneralElement& element,
                                const double          currentTime,
                                const int             topIndex,
                                const bool            constrainedOverPressure,
                                const double          constrainedOverpressureValue,
                                const bool            isIceSheetLayer,
                                ElementVector&        fracturePressureExceeded,
                                BoundaryConditions&   bcs ) const;

   /// \brief Assemble the element Jacobian and residual for the pressure equation.
   void assembleElementSystem ( const GeneralElement&     element,
                                const double              currentTime,
                                const double              timeStep,
                                const BoundaryConditions& bcs,
                                const CompoundLithology*  elementLithology,
                                const ElementVector&      exceededFracturePressure,
                                const bool                includeChemicalCompaction,
                                const bool                includeInDarcySimulation,
                                const bool                includeWaterSaturation,
                                const bool                isIceSheetLayer,
                                const Saturation&         currentSaturation,
                                const Saturation&         previousSaturation,
                                ElementMatrix&            elementJacobian,
                                ElementVector&            elementResidual ) const;



   AppCtx* cauldron;
   FiniteElementMethod::BasisFunctionCache* basisFunctions;

};


#endif // _FASTCAULDRON__PRESSURE_SOLVER_H_
