#ifndef _TEMPERATURESOLVER_H_
#define _TEMPERATURESOLVER_H_

#include "propinterface.h"
#include "vrecalc.h"
#include "SmectiteIlliteCalculator.h"
#include "BiomarkersCalculator.h"
#include "FissionTrackCalculator.h"
class Temperature_Solver {

public:

  ~Temperature_Solver();
  Temperature_Solver(AppCtx* Application_Context );

  void Compute_Crust_Heat_Production ();

  void Compute_Heat_Flow_Boundary_Conditions ( const double Current_Time );

  ///
  /// Assemble the Jacobian and residual for the temperature equation
  ///
  void Assemble_System   ( const double  Previous_Time,
                           const double  Current_Time,
                           const DA&     Temperature_FEM_Grid,
                           const Vec&    Temperature_DOFs,
                                 Mat&    Jacobian,
                                 Vec&    Residual,
                                 double& Element_Contributions_Time );

  ///
  /// Assemble the only the residual for the temperature equation
  ///
  void Assemble_Residual ( const double  Previous_Time,
                           const double  Current_Time,
                           const DA&     Temperature_FEM_Grid,
                           const Vec&    Temperature_DOFs,
                                 Vec&    Residual,
                                 double& Element_Contributions_Time );

  ///
  /// Assemble the stiffness matrix for the linearised temperature equation
  ///
  void Assemble_Stiffness_Matrix   ( const double  Previous_Time,
                                     const double  Current_Time,
                                     const DA&     Temperature_FEM_Grid,
                                     const Vec&    Temperature_DOFs,
                                           Mat&    Stiffness_Matrix,
                                           Vec&    Load_Vector,
                                           double& Element_Contributions_Time );

  /// Zero the intermediate vre calculation vectors.
  void initialiseVReVectors ( AppCtx* basinModel );

  ///
  /// Compute the VRe, if required, over the time step
  ///
  void computeVReIncrement ( AppCtx*      basinModel,
                             const double Previous_Time,
                             const double Current_Time );

  ///
  /// Compute the VRe at the snapshot time
  ///
  void computeSnapShotVRe ( AppCtx*         basinModel,
                            const double    Current_Time );

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

  void deleteVReVectors ( AppCtx* Basin_Model );



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

  void Restore_Temperature_Solution ( const DA  Temperature_FEM_Grid,
				      const Vec Temperature_DOF_Numbers,
				            Vec Temperature );


  void Store_Temperature_Solution ( const DA  Temperature_FEM_Grid,
                                    const Vec Temperature_DOF_Numbers,
                                    const Vec Temperature,
                                    const double Current_Time );

  PetscScalar Maximum_Temperature_Difference ();
  PetscScalar Maximum_Temperature_Difference_In_Source_Rocks ();


  ///
  /// The maximum number of Newton iterations that can be performed in the 
  /// temperature calculation. It is dependant on the user selecting to do a 
  /// nonlinear temperature calculation and the optimisation level.
  ///
  int maximumNumberOfNonlinearIterations ( const int optimisationLevel ) const;

  double linearSolverTolerance ( const int Optimisation_Level ) const;


  int getPlaneQuadratureDegree ( const int optimisationLevel ) const;

  int getDepthQuadratureDegree ( const int optimisationLevel ) const;


  static void setPlaneQuadratureDegree ( const int optimisationLevel,
					 const int newDegree );

  static void setDepthQuadratureDegree ( const int optimisationLevel,
					 const int newDegree );


  void writeFissionTrackResultsToDatabase(void);
private:


  static int PlaneQuadratureDegrees [ NumberOfOptimisationLevels ];

  static int DepthQuadratureDegrees [ NumberOfOptimisationLevels ];



  AppCtx*        Basin_Model;

  VreCalc        Vitrinite_Calculator;

  SmectiteIlliteCalculator m_SmectiteIlliteCalculator;
  BiomarkersCalculator m_BiomarkersCalculator;
  FissionTrackCalculator m_FissionTrackCalculator;

  Vec            Mantle_Heat_Flow;
  Vec            Crust_Heat_Production;

};

#endif /* _TEMPERATURESOLVER_H_ */
