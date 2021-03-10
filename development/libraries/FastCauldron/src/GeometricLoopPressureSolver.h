//
// Copyright (C) 2015-2016 Shell International Exploration & Production.
// All rights reserved.
//
// Developed under license for Shell by PDS BV.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#ifndef _FASTCAULDRON__GEOMETRIC_LOOP_PRESSURE_SOLVER_H_
#define _FASTCAULDRON__GEOMETRIC_LOOP_PRESSURE_SOLVER_H_

#include "PressureSolver.h"

#include "layer.h"

// Forward declaration
class AppCtx;
class PETSC_3D_Array;
namespace GeoPhysics
{
   class CompoundLithology;
}

class GeometricLoopPressureSolver : public PressureSolver {

public :

   GeometricLoopPressureSolver ( AppCtx* appl );

   ~GeometricLoopPressureSolver();

   void adjustSolidThickness ( const double relativeThicknessTolerance,
                               const double absoluteThicknessTolerance,
                                     bool&  geometryHasConverged );

   void computeDependantProperties ( const double previousTime, 
                                     const double currentTime, 
                                     const bool   outputProperties );

   void initialisePressureProperties ( const double previousTime, 
                                       const double currentTime );


private :

  ///
  /// For a given overpressure and solid thickness there is a real thickness.
  /// This real thickness is determined by the following ODE:
  ///
  ///  \f[
  ///       \frac{dz}{ds}=\frac{1}{1-\phi}
  ///
  ///  \f]
  ///         dz        1       \n
  ///         --  = --------    \n
  ///         ds    1 - phi     \n
  ///
  /// This equation is solved using a simple method (Eulers) using 
  /// a series of n-steps (see implementation for value for n).
  /// Should this be removed, as it is not used?
  void computeRealThickness
     ( const LayerProps_Ptr Current_Layer,
       const CompoundLithology*    Current_Lithology,
       const bool           Include_Chemical_Compaction,
       const double         Solid_Thickness,
       const double         Temperature_Top,
       const double         Temperature_Bottom,
       const double         Overpressure_Top,
       const double         Overpressure_Bottom,
       const double         Intermediate_Max_VES_Top, 
       const double         Intermediate_Max_VES_Bottom,
             double&        VES,
             double&        Max_VES,
       const double         Porosity_Top,
       const double         Chemical_Compaction,
             double&        Porosity_Bottom,
             double&        Hydrostatic_Pressure,
             double&        Pore_Pressure_Top,
             double&        Pore_Pressure_Bottom,
             double&        Lithostatic_Pressure,
             double&        Fluid_Density,
             double&        Bulk_Density,
             double&        Real_Thickness ) const;


   void computeDependantPropertiesForLayer
      ( const LayerProps_Ptr  currentLayer, 
        const double          previousTime, 
        const double          currentTime, 
              PETSC_3D_Array& layerDepth,
              PETSC_3D_Array& hydrostaticPressure,
        const PETSC_3D_Array& Overpressure,
              PETSC_3D_Array& porePressure,
              PETSC_3D_Array& lithostaticPressure,
              PETSC_3D_Array& VES,
              PETSC_3D_Array& intermediateMaxVES,
        const PETSC_3D_Array& maxVES,
              PETSC_3D_Array& layerPorosity,
              PETSC_3D_Array& layerPermeabilityNormal,
              PETSC_3D_Array& layerPermeabilityPlane,
              PETSC_3D_Array& layerTemperature,
        const PETSC_3D_Array& layerChemicalCompaction );


};

#endif // _FASTCAULDRON__GEOMETRIC_LOOP_PRESSURE_SOLVER_H_
