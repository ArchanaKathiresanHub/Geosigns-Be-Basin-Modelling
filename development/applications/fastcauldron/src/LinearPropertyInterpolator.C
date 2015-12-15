#include "LinearPropertyInterpolator.h"

LinearPropertyInterpolator::LinearInterpolator::LinearInterpolator () {
   coeffA = 0.0;
   coeffB = 0.0;
}


void LinearPropertyInterpolator::LinearInterpolator::add ( const double startTime, 
                                                           const double startProperty,
                                                           const double endTime, 
                                                           const double endProperty ) {

   double timeDifference = endTime - startTime;

   coeffA = ( endTime * startProperty - startTime * endProperty ) / timeDifference;
   coeffB = ( endProperty - startProperty ) / timeDifference;
}



void LinearPropertyInterpolator::LinearInterpolator::add ( const double time, 
                                                           const double property ) {

   coeffA = property;
   coeffB = 0.0;
}


double LinearPropertyInterpolator::LinearInterpolator::compute ( const double time ) const {
   return coeffA + time * coeffB;
}


LinearPropertyInterpolator::LinearPropertyInterpolator () {
}


void LinearPropertyInterpolator::initialise ( DM da ) {
   m_interpolators.create ( da );
}

void LinearPropertyInterpolator::compute ( DM                                             da,
                                           const Basin_Modelling::Fundamental_Property    propertyTointerpolate,
                                           const double                                   startTime,
                                           Basin_Modelling::Fundamental_Property_Manager& startProperties,
                                           const double                                   endTime,
                                           Basin_Modelling::Fundamental_Property_Manager& endProperties ) {

   DMDALocalInfo daInfo;

   int i;
   int j;
   int kLast; 

   DMDAGetLocalInfo ( da, &daInfo );
   initialise ( da );

   kLast = daInfo.gzm - 1;

   // for ( i = m_interpolators.First ( 1, Exclude_Ghost_Values ); i <= m_interpolators.Last ( 1, Exclude_Ghost_Values ); ++i ) {

   //    for ( j = m_interpolators.First ( 2, Exclude_Ghost_Values ); j <= m_interpolators.Last ( 2, Exclude_Ghost_Values ); ++j ) {
   for ( i = m_interpolators.firstI (); i <= m_interpolators.lastI (); ++i ) {

      for ( j = m_interpolators.firstJ (); j <= m_interpolators.lastJ (); ++j ) {
         double property1 = startProperties ( propertyTointerpolate, kLast, j, i );
         double property2 = endProperties   ( propertyTointerpolate, kLast, j, i );

         m_interpolators ( i, j ).add ( startTime, property1, 
                                        endTime,   property2 );
      }

   }

}

void LinearPropertyInterpolator::compute ( DM                                  da,
                                           const double                        startTime,
                                           const PETSc_Local_2D_Array<double>& startProperty,
                                           const double                        endTime,
                                           const PETSc_Local_2D_Array<double>& endProperty ) {

   int i;
   int j;

   initialise ( da );

   for ( i = m_interpolators.firstI (); i <= m_interpolators.lastI (); ++i ) {

      for ( j = m_interpolators.firstJ (); j <= m_interpolators.lastJ (); ++j ) {
         m_interpolators ( i, j ).add ( startTime, startProperty ( i, j ), endTime, endProperty ( i, j ) );
      }

   }

}


void LinearPropertyInterpolator::compute ( DM                                             da,
                                           const Basin_Modelling::Fundamental_Property    propertyTointerpolate,
                                           const double                                   time,
                                           Basin_Modelling::Fundamental_Property_Manager& properties ) {

   DMDALocalInfo daInfo;
   int i;
   int j;
   int kLast;

   initialise ( da );

   DMDAGetLocalInfo ( da, &daInfo );
   kLast = daInfo.gzm - 1;

   // for ( i = m_interpolators.First ( 1, Exclude_Ghost_Values ); i <= m_interpolators.Last ( 1, Exclude_Ghost_Values ); ++i ) {

   //    for ( j = m_interpolators.First ( 2, Exclude_Ghost_Values ); j <= m_interpolators.Last ( 2, Exclude_Ghost_Values ); ++j ) {
   for ( i = m_interpolators.firstI (); i <= m_interpolators.lastI (); ++i ) {

      for ( j = m_interpolators.firstJ (); j <= m_interpolators.lastJ (); ++j ) {
         m_interpolators ( i, j ).add ( time, properties ( propertyTointerpolate, kLast, j, i ));
      }

   }

}

void LinearPropertyInterpolator::compute ( const int i,
                                           const int j,
                                           const double time,
                                           const double property ) {

   m_interpolators ( i, j ).add ( time, property );
}


void LinearPropertyInterpolator::compute ( const int i,
                                           const int j,
                                           const double startTime,
                                           const double startProperty,
                                           const double endTime,
                                           const double endProperty ) {

   m_interpolators ( i, j ).add ( startTime, startProperty, endTime, endProperty );
}


// void LinearPropertyInterpolator::compute ( DA                    da,
//                                            const double          startTime,
//                                            const PETSC_3D_Array& startProperty,
//                                            const double          endTime,
//                                            const PETSC_3D_Array& endProperty ) {

//    int i;
//    int j;

//    m_interpolators.create ( da );

//    for ( i = m_interpolators.First ( 1 ); i <= m_interpolators.Last ( 1 ); ++i ) {

//       for ( j = m_interpolators.First ( 2 ); j <= m_interpolators.Last ( 2 ); ++j ) {
//          m_interpolators ( i, j ).add ( startTime, startProperty ( i, j ), endTime, endProperty ( i, j ));
//       }

//    }

// }


double LinearPropertyInterpolator::evaluateProperty(const int& i, const int& j, const double& t ) const {
   return m_interpolators ( i, j ).compute ( t );
}
