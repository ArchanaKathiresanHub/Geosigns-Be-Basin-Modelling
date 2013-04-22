#ifndef _FASTCAULDRON__PRESSURE_SOLVER_H_
#define _FASTCAULDRON__PRESSURE_SOLVER_H_


#include "petsc.h"
#include "petscts.h"
#include "petscdmda.h"

#include "CauldronCalculator.h"

#include "propinterface.h"
#include "globaldefs.h"
#include "layer.h"

#include "LayerElement.h"

class PressureSolver : public CauldronCalculator {

public :

   static const double GeometricLoopThicknessTolerance = 0.001;

   static const int DefaultMaximumNumberOfPressureLinearSolverIterations = 1000;


   PressureSolver(AppCtx *appl);

   virtual void adjustSolidThickness ( const double relativeThicknessTolerance,
                                       const double absoluteThicknessTolerance,
                                             bool&  geometryHasConverged ) = 0;

   virtual void computeDependantProperties ( const double previousTime, 
                                             const double currentTime, 
                                             const bool   outputProperties ) = 0;

   virtual void initialisePressureProperties ( const double previousTime, 
                                               const double currentTime ) = 0;

   /// Copies the pressure solution that is stored in the vectors in the layers
   /// to the Overpressure vector that is defined on the entire basin.
   void restorePressureSolution ( const DM  pressureFemGrid,
                                  const Vec pressureDofNumbers,
                                        Vec Overpressure );

   /// Copies the overpressure vector that is stored on the entire basin to
   /// the vectors in the layers.
   void storePressureSolution ( const DM  pressureFemGrid,
                                const Vec pressureDofNumbers,
                                const Vec overpressure );

   /// Assemble the Jacobian and residual for the Newton solver.
   void assembleSystem ( const double  previousTime,
                         const double  currentTime,
                         const DM&     pressureFEMGrid,
                         const Vec&    pressureDOFs,
                         const Vec&    pressureNodeIncluded,
                               Mat&    jacobian,
                               Vec&    residual,
                               double& elementContributionsTime );


#if 0
   /// Assemble just the residual for the Newton solver.
   void assembleResidual ( const double  previousTime,
                           const double  currentTime,
                           const DM&     pressureFEMGrid,
                           const Vec&    pressureDOFs,
                                 Vec&    Residual,
                                 double& elementContributionsTime );
#endif



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

   void setLayerElements ( const DM  femGrid,
                           const Vec dofNumbers,
                                 LayerElementReferenceArray& elementRefs );


protected :

   static int PlaneQuadratureDegrees [ NumberOfOptimisationLevels ];

   static int DepthQuadratureDegrees [ NumberOfOptimisationLevels ];

   static double NewtonSolverTolerances [ NumberOfOptimisationLevels ][ 3 ];

   static int s_iterationsForiluFillLevelIncrease [ NumberOfOptimisationLevels ];

   static int s_numberOfIterations [ NumberOfOptimisationLevels ];


   void initialiseFctCorrection ();


};


#endif // _FASTCAULDRON__PRESSURE_SOLVER_H_
