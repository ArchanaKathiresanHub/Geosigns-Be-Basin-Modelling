#ifndef _FASTCAULDRON__NON_GEOMETRIC_LOOP_PRESSURE_SOLVER_H_
#define _FASTCAULDRON__NON_GEOMETRIC_LOOP_PRESSURE_SOLVER_H_

#include "PressureSolver.h"

#include "propinterface.h"
#include "layer.h"

class NonGeometricLoopPressureSolver : public PressureSolver {

public :

   NonGeometricLoopPressureSolver ( AppCtx* appl );

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
   /// Compute the solid thickness for the non-geometric loop method for the overpressure.
   /// For a given overpressure and real thickness there is a solid thickness.
   /// This solid thickness is determined by the following ODE:
   ///
   ///  \f[
   ///       \frac{dz}{ds}=1-\phi
   ///
   ///  \f]
   ///         ds               \n
   ///         -- = 1 - phi     \n
   ///         dz               \n
   ///
   /// This equation is solved using a simple single step method (Eulers)
   /// in a predictor-corrector type way.
   ///
   void computeSolidThickness
      ( const LayerProps_Ptr Current_Layer,
        const CompoundLithology*    Current_Lithology,
        const bool           Include_Chemical_Compaction,
        const double         Real_Thickness,
        const double         lateralStressFactor,
        const double         Temperature_Bottom,
        const double         Overpressure_Bottom,
        const double         Intermediate_Max_VES_Bottom,
              double&        VES,
              double&        Max_VES,
        const double         Porosity_Top,
        const double         Chemical_Compaction_Bottom,
              double&        Porosity_Bottom,
              double&        Hydrostatic_Pressure,
              double&        Pore_Pressure,
              double&        Lithostatic_Pressure,
              double&        Fluid_Density,
              double&        Bulk_Density,
              double&        Solid_Thickness ) const;

   void computeDependantPropertiesForLayer
      ( const LayerProps_Ptr  currentLayer, 
        const double          previousTime, 
        const double          currentTime, 
              PETSC_3D_Array& previousLayerDepth,
              PETSC_3D_Array& currentlayerDepth,
              PETSC_3D_Array& layerFCT,
              PETSC_3D_Array& layerHydrostaticPressure,
              PETSC_3D_Array& layerOverpressure,
              PETSC_3D_Array& layerPorePressure,
              PETSC_3D_Array& layerLithostaticPressure,
              PETSC_3D_Array& layerVES,
              PETSC_3D_Array& layerIntermediateMaxVES,
              PETSC_3D_Array& layerMaxVES,
              PETSC_3D_Array& layerPorosity,
              PETSC_3D_Array& layerPermeabilityNormal,
              PETSC_3D_Array& layerPermeabilityPlane,
              PETSC_3D_Array& layerTemperature,
              PETSC_3D_Array& layerChemicalCompaction );


};

#endif // _FASTCAULDRON__NON_GEOMETRIC_LOOP_PRESSURE_SOLVER_H_
