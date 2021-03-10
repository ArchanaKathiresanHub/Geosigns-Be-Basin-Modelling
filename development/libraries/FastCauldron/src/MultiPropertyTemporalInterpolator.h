#ifndef FASTCAULDRON__MULTI_PROPERTY_TEMPORAL_INTERPOLATOR__H
#define FASTCAULDRON__MULTI_PROPERTY_TEMPORAL_INTERPOLATOR__H

#include "array.h"

#include "Subdomain.h"

/// \brief A container for property interpolators.
///
/// It will contain an interpolator for each element in the Darcy subdomain.
/// The interpolator will be computed only the first time that it is accessed.
template<class PropertyCalculator>
class MultiPropertyTemporalInterpolator {

public :

   typedef typename PropertyCalculator::MultiInterpolator MultiInterpolator;

   MultiPropertyTemporalInterpolator ( const Subdomain&          subdomain,
                                       const PropertyCalculator& calculator );

   ~MultiPropertyTemporalInterpolator ();


   /// \brief Get access to the interpolator associated with the element.
   ///
   /// The interpolator is only computed the first time that it is accessed.
   const MultiInterpolator& access ( const SubdomainElement& element ) const;


private :

   struct ElementInterpolator {
      MultiInterpolator m_interpolator;
      bool              m_isComputed;

      ElementInterpolator ();

   };

   typedef ibs::Array3D <ElementInterpolator> MultiInterpolator3DArray;

   /// \brief The calculator for the interpoaltor of the property.
   const PropertyCalculator& propertyCalculator;

   /// \brief An array containing the interpolators for every element in the Darcy subdomain.
   mutable MultiInterpolator3DArray m_interpolators;

   /// \brief Since the element will have global index values they must be "transformed" into local index values.
   ///
   /// This is acheived by subtracting the first global index of the subdomain from the element global index values.
   unsigned int m_indexTransformation [ 3 ];

}; 


template<class PropertyCalculator>
inline MultiPropertyTemporalInterpolator<PropertyCalculator>::ElementInterpolator::ElementInterpolator () {
   m_isComputed = false;
}



template<class PropertyCalculator>
MultiPropertyTemporalInterpolator<PropertyCalculator>::MultiPropertyTemporalInterpolator ( const Subdomain&                      subdomain,
                                                                                                      const PropertyCalculator& calculator ) : 
   propertyCalculator ( calculator ),
   m_interpolators ( subdomain.getVolumeGrid ().lengthI (), subdomain.getVolumeGrid ().lengthJ (), subdomain.getVolumeGrid ().lengthK ())
{
   m_indexTransformation [ 0 ] = subdomain.getVolumeGrid ().firstI ();
   m_indexTransformation [ 1 ] = subdomain.getVolumeGrid ().firstJ ();
   m_indexTransformation [ 2 ] = subdomain.getVolumeGrid ().firstK ();
}


template<class PropertyCalculator>
MultiPropertyTemporalInterpolator<PropertyCalculator>::~MultiPropertyTemporalInterpolator () {
}


template<class PropertyCalculator>
const typename PropertyCalculator::MultiInterpolator& MultiPropertyTemporalInterpolator<PropertyCalculator>::access ( const SubdomainElement& element ) const {


   if ( not m_interpolators ( element.getI () - m_indexTransformation [ 0 ],
                              element.getJ () - m_indexTransformation [ 1 ],
                              element.getK () - m_indexTransformation [ 2 ]).m_isComputed ) {

      typename MultiInterpolator::CoefficientArray allLambdas;

      double x = 0.0;
      // Check for Degree = 0.
      double h = 1.0 / static_cast<double>( MultiInterpolator::Degree );
      int i;

      // Compute a uniform distribution of x values.
      for ( i = 0; i <= MultiInterpolator::Degree; ++i, x += h ) {
         allLambdas [ i ] = x;
      }

      propertyCalculator.compute ( element, allLambdas,
                                   m_interpolators ( element.getI () - m_indexTransformation [ 0 ],
                                                     element.getJ () - m_indexTransformation [ 1 ],
                                                     element.getK () - m_indexTransformation [ 2 ] ).m_interpolator );
      m_interpolators ( element.getI () - m_indexTransformation [ 0 ],
                        element.getJ () - m_indexTransformation [ 1 ],
                        element.getK () - m_indexTransformation [ 2 ] ).m_isComputed = true;
   }

   return m_interpolators ( element.getI () - m_indexTransformation [ 0 ],
                            element.getJ () - m_indexTransformation [ 1 ],
                            element.getK () - m_indexTransformation [ 2 ] ).m_interpolator;
}


#endif // FASTCAULDRON__MULTI_PROPERTY_TEMPORAL_INTERPOLATOR__H
