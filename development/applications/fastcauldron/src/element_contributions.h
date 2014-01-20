//------------------------------------------------------------//

#ifndef __Element_Contributions_HH__
#define __Element_Contributions_HH__

//------------------------------------------------------------//

#include "FiniteElementTypes.h"
#include "BasisFunctionCache.h"
#include "globaldefs.h"
#include "Quadrature.h"
#include "HydraulicFracturingManager.h"

//------------------------------------------------------------//

#include "CompoundLithology.h"
#include "GeoPhysicsFluidType.h"

//------------------------------------------------------------//

#include "CompoundProperty.h"

//------------------------------------------------------------//

#include "Saturation.h"

//------------------------------------------------------------//

#include "Matrix8x8.h"

//------------------------------------------------------------//

using namespace GeoPhysics;
using namespace FiniteElementMethod;

namespace Basin_Modelling {


  enum Element_Boundary { Gamma_1, Gamma_2, Gamma_3, Gamma_4, Gamma_5, Gamma_6 };

  string Element_Boundary_Image ( const Element_Boundary Boundary );


  //------------------------------------------------------------//

  void Compute_Normal
     ( const Matrix3x3&              Jacobian,
             ThreeVector&            Normal );

  void Compute_Normal
     ( const Matrix3x3&              Jacobian,
             ThreeVector&            Normal,
             ThreeVector&            angles );

  void Set_Heat_Conductivity_Tensor
     ( const double                   Conductivity_Normal,
       const double                   Conductivity_Plane,
       const Matrix3x3&              Jacobian,
             Matrix3x3&              Conductivity_Tensor );

  void Set_Permeability_Tensor
     ( const PetscScalar              Permeability_Normal,
       const PetscScalar              Permeability_Plane,
       const Matrix3x3&              Jacobian,
             Matrix3x3&              Permeability_Tensor );


  double Compute_Solid_Thickness
     ( const ElementVector& Thickness,
       const ElementVector& Basis );


  double Maximum_Diameter 
     ( const ElementGeometryMatrix& geometryMatrix );

  double CFL_Value
     ( const CompoundLithology*            Lithology,
       const GeoPhysics::FluidType*        Fluid,
       const bool                          includeChemicalCompaction,
       const ElementGeometryMatrix&      geometryMatrix,
       const ElementVector&               Current_Ph,
       const ElementVector&               Current_Po,
       const ElementVector&               Current_Element_VES,
       const ElementVector&               Current_Element_Max_VES,
       const ElementVector&               Current_Element_Temperature,
       const ElementVector&               Current_Element_Chemical_Compaction,
       const ElementVector&               Fracture_Pressure_Exceeded );

  void computeFluidMobilityTerms
     ( const bool                debugParameter,
       const bool                Has_Fractured,
       const double              fractureScaling,
       const double              VES,
       const double              Max_VES,
       const CompoundProperty&   Porosity,
       const double              Fluid_Density,
       const double              Fluid_Viscosity,
       const double              relativePermeability,
       const Matrix3x3&          Jacobian,
       const ThreeVector&        Grad_Overpressure,
       const CompoundLithology*  Lithology,
             ThreeVector&        Fluid_Velocity,
             Matrix3x3&          Fluid_Mobility );


   void computePermGradPTerm ( const bool                debugParameter,
                               const bool                Has_Fractured,
                               const double              fractureScaling,
                               const double              VES,
                               const double              Max_VES,
                               const CompoundProperty&   Porosity,
                               const double              relativePermeability,
                               const Matrix3x3&          Jacobian,
                               const ThreeVector&        Grad_Overpressure,
                               const CompoundLithology*  lithology,
                                     ThreeVector&        permGradP );

   void computeFluidMobilityTerms ( const bool                debugParameter,
                                    const bool                Has_Fractured,
                                    const double              fractureScaling,
                                    const double              VES,
                                    const double              Max_VES,
                                    const CompoundProperty&   Porosity,
                                    const double              porosityDerivativeWrtVes,
                                    const double              fluidDensity,
                                    const double              fluidDensityDerivativeWrtPressure,
                                    const double              Fluid_Viscosity,
                                    const double              relativePermeability,
                                    const Matrix3x3&          Jacobian,
                                    const ThreeVector&        Grad_Overpressure,
                                    const CompoundLithology*  lithology,
                                          ThreeVector&        Fluid_Velocity,
                                          Matrix3x3&          Fluid_Mobility,
                                          ThreeVector&        fluidVelocityDerivative,
                                    const bool                isPermafrost = false );

  void computeFluidFlux
     ( const bool                imposeFluxLimit,
       const bool                hasFractured,
       const double              VES,
       const double              maxVES,
       const CompoundProperty&   porosity,
       const double              fluidViscosity,
       const Matrix3x3&          jacobian,
       const ThreeVector&        gradOverpressure,
       const double              relativePermeability,
       const CompoundLithology*  lithology,
             ThreeVector&        fluidFlux );

  /// Compute fluid velocity for output.
  ///
  /// Computes the fluid-velocity and not the fluid-flux.
  void computeFluidVelocity
     ( const CompoundLithology*       lithology,
       const GeoPhysics::FluidType*   fluid,
       const bool                     includeChemicalCompaction,
       const ElementGeometryMatrix& geometryMatrix,
       const ElementVector&          currentElementVES,
       const ElementVector&          currentElementMaxVES,
       const ElementVector&          currentElementPh,
       const ElementVector&          currentElementPo,
       const ElementVector&          currentElementTemperature,
       const ElementVector&          currentElementChemicalCompaction,
       const double                  currentElementRelPerm,
             ThreeVector&            fluidVelocity );

  /// Compute fluid velocity for output.
  ///
  /// Computes the fluid-velocity and not the fluid-flux.
  void computeFluidVelocity
     ( const double                   x,
       const double                   y,
       const double                   z,
       const CompoundLithology*       lithology,
       const GeoPhysics::FluidType*   fluid,
       const bool                     includeChemicalCompaction,
       const ElementGeometryMatrix& geometryMatrix,
       const ElementVector&          currentElementVES,
       const ElementVector&          currentElementMaxVES,
       const ElementVector&          currentElementPh,
       const ElementVector&          currentElementPo,
       const ElementVector&          currentElementTemperature,
       const ElementVector&          currentElementChemicalCompaction,
       const double                  currentElementRelPerm,
             ThreeVector&            fluidVelocity );

  bool Degenerate_Element ( const ElementGeometryMatrix& geometryMatrix );

  ///
  /// Applies the Neumann type boundar conditions to the temperature equations.
  ///
  void Apply_Heat_Flow_Boundary_Conditions 
     ( const ElementGeometryMatrix& geometryMatrix,
       const ElementVector&          Nodal_BC_Values,
             ElementVector&          ElementVector_Contributions );

  void applyPressureNeumannBoundaryConditions 
     ( const ElementGeometryMatrix& geometryMatrix,
       const CompoundLithology*       Lithology,
       const GeoPhysics::FluidType*   Fluid,
       const bool                     includeChemicalCompaction,
       const Boundary_Conditions*     BCs,
       const ElementVector&          Current_Ph,
       const ElementVector&          Current_Po,
       const ElementVector&          Current_Element_VES,
       const ElementVector&          Current_Element_Max_VES,
       const ElementVector&          Current_Element_Temperature,
       const ElementVector&          Current_Element_Chemical_Compaction,
             ElementVector&          ElementVector_Contributions );

  ///
  /// Apply Dirichlet type boundary conditions to both Jacobian and 
  /// residual for non-linear systems using the Newton solver.
  ///
  void Apply_Dirichlet_Boundary_Conditions_Newton
     ( const Boundary_Conditions*          BCs,
       const ElementVector&               Dirichlet_Boundary_Values,
       const double                        Dirichlet_Boundary_Scaling,
       const ElementVector&               Current_Property_Values,
             ElementMatrix&               Jacobian,
             ElementVector&               Residual );

  ///
  /// Apply Dirichlet type boundary conditions to residual
  /// only for non-linear systems using the Newton solver.
  ///
  void Apply_Dirichlet_Boundary_Conditions_Newton
     ( const Boundary_Conditions*          BCs,
       const ElementVector&               Dirichlet_Boundary_Values,
       const double                        Dirichlet_Boundary_Scaling,
       const ElementVector&               Current_Property_Values,
             ElementVector&               Residual );

  ///
  /// Apply Dirichlet type boundary conditions to both stiffness 
  /// matrix and load vector for linearised systems.
  ///
  void Apply_Dirichlet_Boundary_Conditions_Linear
     ( const Boundary_Conditions*          BCs,
       const ElementVector&               Dirichlet_Boundary_Values,
       const double                        Dirichlet_Boundary_Scaling,
             ElementMatrix&               Stiffness_Matrix,
             ElementVector&               Load_Vector );

  ///
  /// Assemble the Jacobian and residual for elements have a salt lithology 
  /// (in fact its for any lithology with zero porosity).
  ///
  void Assemble_Element_Pressure_Salt_System
     ( const double                   timeStep,
       const CompoundLithology*       Lithology,
       const GeoPhysics::FluidType*   Fluid,
       const ElementGeometryMatrix&   geometryMatrix,
             ElementMatrix&           Element_Jacobian,
             ElementVector&           Element_Residual );

  ///
  /// The overpressure equation:
  ///
  /// \f[
  ///      \frac{\partial (\phi \rho_f)}{\partial t}=
  ///                      \nabla \cdot \left( \frac{\rho_f k}{\mu} \nabla P_o \right) + S_p
  /// \f]
  ///
  /// The weak form:
  ///
  /// \f[
  ///      \int_{e} N \frac{(\phi \rho_f)_{n}}{\delta t} dx - \int_{e} N \frac{(\phi \rho_f)_{n-1}}{\delta t} dx =
  ///                       \int_{e} \nabla N \cdot \left( \frac{\rho_f k}{\mu} \nabla P_o \right)  dx +
  ///                       \int_{e} N S_p  dx
  /// \f]
  ///
  /// The Residual:
  ///
  /// \f[
  ///     R = -\int_{e} N \frac{(\phi \rho_f)_{n}}{\delta t} dx + \int_{e} N \frac{(\phi \rho_f)_{n-1}}{\delta t} dx
  ///          -\int_{e} \nabla N \cdot \left( \frac{\rho_f k}{\mu} \nabla P_o \right)  dx
  ///          +\int_{e} N S_p  dx
  /// \f]
  ///
  /// The overpressure is defined as:
  ///
  /// \f[
  ///      P_o(x) = \sum_{i=1}^n \alpha_i N_{i}(\^{x})
  /// \f]
  ///
  /// The Jacobian:
  ///
  /// \f[{eqnarray}
  ///     J &=& \frac{\partial R}{\partial \alpha_i} \newline
  ///       &=& -\int_{e} N \frac{\partial (\phi \rho_f)_{n}}{\partial P_o}\frac{1}{\delta t} N dx
  ///           -\int_{e} \nabla N \left( \frac{1}{\mu}\frac{\partial (k \rho_f)}{\partial P_o} \nabla P_o \right) N dx
  ///           -\int_{e} \nabla N \left(\frac{k \rho_f}{\mu} \right) \nabla N dx
  /// \f]
  ///
  ///
  ///
  void Assemble_Element_Pressure_System
     ( const BasisFunctionCache&           basisFunctions,
       const double                        currentTime,
       const double                        timeStep,
       const Boundary_Conditions*          Element_BCs,
       const Boundary_Conditions*          BCs,
       const ElementVector&                Dirichlet_Boundary_Values,
       const bool                          isIceSheetLayer,
       const CompoundLithology*            Lithology,
       const GeoPhysics::FluidType*        Fluid,
       const bool                          includeChemicalCompaction,
       const Interface::FracturePressureModel         fractureModel,

       const ElementGeometryMatrix&      previousGeometryMatrix,
       const ElementGeometryMatrix&      geometryMatrix,

       const ElementVector&               Previous_Element_Solid_Thickness,
       const ElementVector&               Current_Element_Solid_Thickness,
       const ElementVector&               Previous_Ph,
       const ElementVector&               Current_Ph,
       const ElementVector&               Previous_Po,
       const ElementVector&               Current_Po,
       const ElementVector&               Current_Pl,
       const ElementVector&               Previous_Element_VES,
       const ElementVector&               Current_Element_VES,
       const ElementVector&               Previous_Element_Max_VES,
       const ElementVector&               Current_Element_Max_VES,
       const ElementVector&               Previous_Element_Temperature,
       const ElementVector&               Current_Element_Temperature,

       const ElementVector&               Previous_Element_Chemical_Compaction,
       const ElementVector&               Current_Element_Chemical_Compaction,

       const ElementVector&               Fracture_Pressure_Exceeded,
       const ElementVector&               preFractureScaling,
       const BooleanVector&               Included_Nodes,

       const bool                         includeWaterSaturation,
       const Saturation&                  currentSaturation,
       const Saturation&                  previousSaturation,

             ElementMatrix&               Element_Jacobian,
             ElementVector&               Element_Residual );


  void computeHeatFlow
     ( const bool                     isBasementFormation,
	   const CompoundLithology*       lithology,
       const GeoPhysics::FluidType*   fluid,
       const bool                     includeChemicalCompaction,
       const bool                     includeAdvectionTerm,
       const ElementGeometryMatrix& geometryMatrix,
       const ElementVector&          currentElementVES,
       const ElementVector&          currentElementMaxVES,
       const ElementVector&          currentElementPo,
       const ElementVector&          currentElementPp,
       const ElementVector&          currentElementLp,
       const ElementVector&          currentElementTemperature,
       const ElementVector&          currentElementChemicalCompaction,
             ThreeVector&            heatFlow );

   // The location of evaluation (x, y, z) should be from the reference-element [-1,1]^3
  void computeHeatFlow
     ( const bool                     isBasementFormation,
	   const double                   x,
       const double                   y,
       const double                   z,
       const CompoundLithology*       lithology,
       const GeoPhysics::FluidType*   fluid,
       const bool                     includeChemicalCompaction,
       const bool                     includeAdvectionTerm,
       const ElementGeometryMatrix& geometryMatrix,
       const ElementVector&          currentElementVES,
       const ElementVector&          currentElementMaxVES,
       const ElementVector&          currentElementPo,
       const ElementVector&          currentElementPp,
       const ElementVector&          currentElementLp,
       const ElementVector&          currentElementTemperature,
       const ElementVector&          currentElementChemicalCompaction,
             ThreeVector&            heatFlow );


  void Compute_Heat_Flow
     ( const bool                   isBasementFormation,    
	   const CompoundLithology*     Lithology,
       const GeoPhysics::FluidType* Fluid,
       const ElementVector&         Temperature_Vector,
       const double                 Temperature_Value,
       const double                 Porosity,
       const double                 PorePressure,
       const double                 LithostaticPressure,
       const Matrix3x3&          Jacobian,
       const GradElementVector& Grad_Basis,
             ThreeVector&        Heat_Flow,
             Matrix3x3&          Conductivity_Tensor );


  void Assemble_Element_Temperature_System
     ( const bool                          isBasementFormation,
	   const int                           planeQuadratureDegree,
       const int                           depthQuadratureDegree,
       const double                        currentTime,
       const double                        timeStep,
       const bool                          Is_Steady_State,
       const bool                          Include_Advection_Term,
       const Boundary_Conditions*          BCs,
       const ElementVector&               Dirichlet_Boundary_Values,
       const CompoundLithology*            Lithology,
       const GeoPhysics::FluidType*        Fluid,
       const bool                          includeChemicalCompaction,

       const ElementGeometryMatrix&      previousGeometryMatrix,
       const ElementGeometryMatrix&      geometryMatrix,

       const ElementVector&               Element_Heat_Production,
       const ElementVector&               Previous_Ph,
       const ElementVector&               Current_Ph,
       const ElementVector&               Previous_Po,
       const ElementVector&               Current_Po,
       const ElementVector&               Current_Lp,
       const ElementVector&               Previous_Element_VES,
       const ElementVector&               Current_Element_VES,
       const ElementVector&               Previous_Element_Max_VES,
       const ElementVector&               Current_Element_Max_VES,
       const ElementVector&               Previous_Element_Temperature,
       const ElementVector&               Current_Element_Temperature,

       const ElementVector&               Previous_Element_Chemical_Compaction,
       const ElementVector&               Current_Element_Chemical_Compaction,

             ElementMatrix&               Element_Jacobian,
             ElementVector&               Element_Residual );


  void Assemble_Element_Temperature_Residual
     ( const bool                          isBasementFormation,
	   const int                           planeQuadratureDegree,
       const int                           depthQuadratureDegree,
       const double                        currentTime,
       const double                        timeStep,
       const bool                          Is_Steady_State,
       const bool                          Include_Advection_Term,
       const Boundary_Conditions*          BCs,
       const ElementVector&                Dirichlet_Boundary_Values,
       const CompoundLithology*            Lithology,
       const GeoPhysics::FluidType*        Fluid,
       const bool                          includeChemicalCompaction,

       const ElementGeometryMatrix&      previousGeometryMatrix,
       const ElementGeometryMatrix&      geometryMatrix,

       const ElementVector&               Element_Heat_Production,
       const ElementVector&               Previous_Ph,
       const ElementVector&               Current_Ph,
       const ElementVector&               Previous_Po,
       const ElementVector&               Current_Po,
       const ElementVector&               Current_Lp,
       const ElementVector&               Previous_Element_VES,
       const ElementVector&               Current_Element_VES,
       const ElementVector&               Previous_Element_Max_VES,
       const ElementVector&               Current_Element_Max_VES,
       const ElementVector&               Previous_Element_Temperature,
       const ElementVector&               Current_Element_Temperature,

       const ElementVector&               Previous_Element_Chemical_Compaction,
       const ElementVector&               Current_Element_Chemical_Compaction,

             ElementVector&               Element_Residual );



  void Assemble_Element_Temperature_Stiffness_Matrix
     ( const bool                          isBasementFormation,
	   const int                           planeQuadratureDegree,
       const int                           depthQuadratureDegree,
       const double                        currentTime,
       const double                        timeStep,
       const bool                          Is_Steady_State,
       const bool                          Include_Advection_Term,
       const Boundary_Conditions*          BCs,
       const ElementVector&               Nodal_BC_Values,
       const CompoundLithology*            Lithology,
       const GeoPhysics::FluidType*        Fluid,
       const bool                          includeChemicalCompaction,

       const ElementGeometryMatrix&       geometryMatrix,
       const ElementVector&               Element_Heat_Production,
       const ElementVector&               Current_Ph,
       const ElementVector&               Current_Po,

       const ElementVector&               Previous_Pp,
       const ElementVector&               Current_Pp,
       const ElementVector&               Previous_Lp,
       const ElementVector&               Current_Lp,

       const ElementVector&               Previous_Element_VES,
       const ElementVector&               Current_Element_VES,

       const ElementVector&               Previous_Element_Max_VES,
       const ElementVector&               Current_Element_Max_VES,

       const ElementVector&               Previous_Element_Temperature,
       const ElementVector&               Current_Element_Temperature,
       const ElementVector&               Previous_Element_Chemical_Compaction,
       const ElementVector&               Current_Element_Chemical_Compaction,

             ElementMatrix&               Element_Stiffness_Matrix,
             ElementVector&               Element_Load_Vector );


  void Assemble_Element_Pressure_System2
     ( const int                           planeQuadratureDegree,
       const int                           depthQuadratureDegree,
       const double                        currentTime,
       const double                        timeStep,
       const Boundary_Conditions*  Element_BCs,
       const Boundary_Conditions*          BCs,
       const ElementVector&               Dirichlet_Boundary_Values,
       const CompoundLithology*            Lithology,
       const GeoPhysics::FluidType*        Fluid,
       const bool                          includeChemicalCompaction,

       const ElementGeometryMatrix&      previousGeometryMatrix,
       const ElementGeometryMatrix&      geometryMatrix,

       const ElementVector&               Previous_Element_Solid_Thickness,
       const ElementVector&               Current_Element_Solid_Thickness,
       const ElementVector&               Previous_Ph,
       const ElementVector&               Current_Ph,
       const ElementVector&               Previous_Po,
       const ElementVector&               Current_Po,
       const ElementVector&               Current_Pl,
       const ElementVector&               Previous_Element_VES,
       const ElementVector&               Current_Element_VES,
       const ElementVector&               Previous_Element_Max_VES,
       const ElementVector&               Current_Element_Max_VES,
       const ElementVector&               Previous_Element_Temperature,
       const ElementVector&               Current_Element_Temperature,

       const ElementVector&               Previous_Element_Chemical_Compaction,
       const ElementVector&               Current_Element_Chemical_Compaction,

       const ElementVector&               Fracture_Pressure_Exceeded,
       const BooleanVector&               Included_Nodes,

             ElementMatrix&               Element_Jacobian,
             ElementVector&               Element_Residual );


  void Assemble_Element_Pressure_Residual2
     ( const int                           planeQuadratureDegree,
       const int                           depthQuadratureDegree,
       const double                        currentTime,
       const double                        timeStep,
       const Boundary_Conditions*  Element_BCs,
       const Boundary_Conditions*          BCs,
       const ElementVector&               Dirichlet_Boundary_Values,
       const CompoundLithology*            Lithology,
       const GeoPhysics::FluidType*        Fluid,
       const bool                          includeChemicalCompaction,
       const ElementGeometryMatrix&      previousGeometryMatrix,
       const ElementGeometryMatrix&      geometryMatrix,
       const ElementVector&               Previous_Element_Solid_Thickness,
       const ElementVector&               Current_Element_Solid_Thickness,
       const ElementVector&               Previous_Ph,
       const ElementVector&               Current_Ph,
       const ElementVector&               Previous_Po,
       const ElementVector&               Current_Po,
       const ElementVector&               Current_Pl,
       const ElementVector&               Previous_Element_VES,
       const ElementVector&               Current_Element_VES,
       const ElementVector&               Previous_Element_Max_VES,
       const ElementVector&               Current_Element_Max_VES,
       const ElementVector&               Previous_Element_Temperature,
       const ElementVector&               Current_Element_Temperature,

       const ElementVector&               Previous_Element_Chemical_Compaction,
       const ElementVector&               Current_Element_Chemical_Compaction,
       const ElementVector&               Fracture_Pressure_Exceeded,
             ElementVector&               Element_Residual );



  void computeGradSurfaceDepth ( const PetscScalar          xi,
                                 const PetscScalar          eta,
                                 const ElementVector&      surfaceDepth,
                                       ThreeVector&        referenceGradSurfaceDepth );


} // end namespace Basin_Modelling


//------------------------------------------------------------//

#endif // __Element_Contributions_HH__

//------------------------------------------------------------//
