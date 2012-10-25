#ifndef _FASTCAULDRON__LINEAR_PROPERTY_INTERPOLATOR_H_
#define _FASTCAULDRON__LINEAR_PROPERTY_INTERPOLATOR_H_


#include "LocalGridInterpolator.h"

#include "property_manager.h"
#include "ghost_array.h"


class LinearPropertyInterpolator : public Genex6::LocalGridInterpolator {

   class LinearInterpolator {

   public :

      LinearInterpolator ();

      void add ( const double startTime, 
                 const double startProperty,
                 const double endTime, 
                 const double endProperty );

      void add ( const double time, 
                 const double property );

      double compute ( const double time ) const;

   private :

      double coeffA;
      double coeffB;

   };

public :

   LinearPropertyInterpolator ();

   void compute ( DA                                             da,
                  const Basin_Modelling::Fundamental_Property    propertyTointerpolate,
                  const double                                   startTime,
                  Basin_Modelling::Fundamental_Property_Manager& startProperties,
                  const double                                   endTime,
                  Basin_Modelling::Fundamental_Property_Manager& endProperties );

   // Compute a constant interpolator
   void compute ( DA                                             da,
                  const Basin_Modelling::Fundamental_Property    propertyTointerpolate,
                  const double                                   time,
                  Basin_Modelling::Fundamental_Property_Manager& properties );

   void compute ( DA                    da,
                  const double          startTime,
                  const PETSC_3D_Array& startProperty,
                  const double          endTime,
                  const PETSC_3D_Array& endProperty );

   void compute ( DA                                  da,
                  const double                        startTime,
                  const PETSc_Local_2D_Array<double>& startProperty,
                  const double                        endTime,
                  const PETSc_Local_2D_Array<double>& endProperty );

   void initialise ( DA da );

   /// Add constant interpolation at single location
   void compute ( const int i,
                  const int j,
                  const double time,
                  const double property );

   void compute ( const int i,
                  const int j,
                  const double startTime,
                  const double startProperty,
                  const double endTime,
                  const double endProperty );


   double evaluateProperty ( const int& i, const int& j, const double& t ) const;


private :

   PETSc_Local_2D_Array<LinearInterpolator> m_interpolators;
   

};


#endif // _FASTCAULDRON__LINEAR_PROPERTY_INTERPOLATOR_H_
