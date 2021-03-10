#ifndef _TEMPERATURESOLVER_H_
#define _TEMPERATURESOLVER_H_

#include "propinterface.h"
#include "SmectiteIlliteCalculator.h"
#include "BiomarkersCalculator.h"
#include "FissionTrackCalculator.h"
#include "VitriniteReflectance.h"

#include "BoundaryConditions.h"
#include "ComputationalDomain.h"

class Temperature_Solver {

public:

  ~Temperature_Solver();
  Temperature_Solver(AppCtx* Application_Context );

  void Compute_Crust_Heat_Production ();

  /// \brief Computes the heat production for each node in the domain.
  void computeHeatProduction ( const double previousTime,
                               const double currentTime ) const;


  ///
  /// Assemble the Jacobian and residual for the temperature equation
  ///
  void assembleSystem ( const ComputationalDomain& computationalDomain,
                        const double               previousTime,
                        const double               currentTime,
                        Mat&                       jacobian,
                        Vec&                       residual,
                        double&                    elementContributionsTime );


   /// Assemble the only the residual for the temperature equation
   void assembleResidual ( const ComputationalDomain& computationalDomain,
                           const double               previousTime,
                           const double               currentTime,
                           Vec&                       residual,
                           double&                    elementContributionsTime );

  /// \brief Assemble the stiffness matrix for the linearised temperature equation
   void assembleStiffnessMatrix ( const ComputationalDomain& computationalDomain,
                                  const double previousTime,
                                  const double currentTime,
                                  Mat&         stiffnessMatrix,
                                  Vec&         loadVector,
                                  double&      elementContributionsTime );


   /// \brief This corrects the temperature after it has been stored.
   ///
   /// It applies the cut-off for temperature oscillations that can occur
   /// when there is a basalt emplacement.
   void correctTemperatureSolution ( const double Current_Time );

  ///
  /// Update the Smectite-Illite calculation state,
  ///
  void computeSmectiteIlliteIncrement ( const double Previous_Time, const double Current_Time );

  ///
  /// Compute the SmectiteIllite fraction at the snapshot time
  ///
  void computeSnapShotSmectiteIllite ( const double Current_Time,
                                       const Boolean2DArray& validNeedle );
  void deleteSmectiteIlliteVector ();

  ///
  /// Update the Biomarkers calculation state,
  ///
  void computeBiomarkersIncrement ( const double Previous_Time, const double Current_Time );

  ///
  /// Collect data to be used for fission track analysis
  ///
  void collectFissionTrackSampleData(const double time);

  ///
  /// Perform fission track analysis
  ///
  void computeFissionTracks(void);

  ///
  /// Clear the input history for fission track calculator
  ///
  void resetFissionTrackCalculator(void);


  ///
  /// Compute the Biomarkers  at the snapshot time
  ///
  void computeSnapShotBiomarkers ( const double Current_Time, const Boolean2DArray& validNeedle );
  void deleteBiomarkersVectors ( );
  void resetSmectiteIlliteStateVectors( );
  void resetBiomarkerStateVectors();

  ///
  /// Set the surface temperature for the temperature vector in the topmost layer.
  ///
  void setSurfaceTemperature ( AppCtx*      basinModel,
                               const double Current_Time );

  ///
  /// Estimates the temperature in the basement before the steady state calculation.
  ///
  void Estimate_Basement_Temperature ( );

  void Estimate_Temperature ( AppCtx*      basinModel,
                              const double Current_Time );

  void Create_Source_Rocks_History_Arrays ( const int Number_Of_X_Nodes,
					    const int Number_Of_Y_Nodes );

  void Store_Temperature_Solution ( const DM  Temperature_FEM_Grid,
                                    const Vec Temperature_DOF_Numbers,
                                    const Vec Temperature,
                                    const double Current_Time );

  PetscScalar Maximum_Temperature_Difference ();
  PetscScalar Maximum_Temperature_Difference_In_Source_Rocks ();


  /// \brief The maximum number of Newton iterations that can be performed in the non-linear temperature calculation.
  int getMaximumNumberOfNonlinearIterations ( const int optimisationLevel ) const;

  /// \brief Get the tolerance for the non-linear temperature solver.
  double getNewtonSolverTolerance ( const int optimisationLevel ) const;

  static void setNewtonSolverTolerance ( const int    optimisationLevel,
                                         const double newTolerance );

  /// \brief Get the tolerance for the linear solver.
  double getLinearSolverTolerance ( const int Optimisation_Level ) const;

  /// \brief Get the tolerance for the linear solver.
  static void setLinearSolverTolerance ( const int    optimisationLevel,
                                         const double newTolerance );


  int getPlaneQuadratureDegree ( const int optimisationLevel ) const;

  int getDepthQuadratureDegree ( const int optimisationLevel ) const;


  static void setPlaneQuadratureDegree ( const int optimisationLevel,
					 const int newDegree );

  static void setDepthQuadratureDegree ( const int optimisationLevel,
					 const int newDegree );


  void writeFissionTrackResultsToDatabase(void);
private:

  Temperature_Solver( const Temperature_Solver & ); // prohibit copying
  Temperature_Solver & operator=(const Temperature_Solver & ); // prohibit assignment


   /// \brief Assemble the element Jacobian and residual for the nonlinear temperature equation.
   void assembleElementNonLinearSystem ( const GeneralElement&     element,
                                         const PETSC_3D_Array&     bulkHeatProd,
                                         const int                 planeQuadratureDegree,
                                         const int                 depthQuadratureDegree,
                                         const double              currentTime,
                                         const double              timeStep,
                                         const bool                includeAdvectiveTerm,
                                         const BoundaryConditions& bcs,
                                         const CompoundLithology*  elementLithology,
                                         const bool                includeChemicalCompaction,
                                         ElementMatrix&            elementJacobian,
                                         ElementVector&            elementResidual ) const;

   /// \brief Assemble the element residual for the nonlinear temperature equation.
   void assembleElementNonLinearResidual ( const GeneralElement&     element,
                                           const PETSC_3D_Array&     bulkHeatProd,
                                           const int                 planeQuadratureDegree,
                                           const int                 depthQuadratureDegree,
                                           const double              currentTime,
                                           const double              timeStep,
                                           const bool                includeAdvectiveTerm,
                                           const BoundaryConditions& bcs,
                                           const CompoundLithology*  elementLithology,
                                           const bool                includeChemicalCompaction,
                                           ElementVector&            elementResidual ) const;


   /// \brief Get the additional boundary conditions and element lithology for ALC mode.
   void getAlcBcsAndLithology ( const GeneralElement&     element,
                                const double              previousTime,
                                const double              currentTime,
                                const CompoundLithology*& elementLithology,
                                PETSC_2D_Array&           topBasaltDepth,
                                PETSC_2D_Array&           bottomBasaltDepth,
                                BoundaryConditions&       bcs ) const;

   /// \brief Get the boundary conditions for the temperature equation.
   ///
   /// This is for both the linearised and non-linear.
   void getBoundaryConditions ( const GeneralElement& element,
                                const double          currentTime,
                                const int             topIndex,
                                BoundaryConditions&   bcs ) const;

  static int PlaneQuadratureDegrees [ NumberOfOptimisationLevels ];
  static int DepthQuadratureDegrees [ NumberOfOptimisationLevels ];

  static int    s_numberOfNonlinearIterations [ NumberOfOptimisationLevels ];
  static double s_nonlinearSolverTolerance    [ NumberOfOptimisationLevels ];
  static double s_linearSolverTolerances      [ NumberOfOptimisationLevels ];

  AppCtx*        Basin_Model;

  SmectiteIlliteCalculator m_SmectiteIlliteCalculator;
  BiomarkersCalculator m_BiomarkersCalculator;
  FissionTrackCalculator m_FissionTrackCalculator;

  Vec            Crust_Heat_Production;

};

#endif /* _TEMPERATURESOLVER_H_ */
