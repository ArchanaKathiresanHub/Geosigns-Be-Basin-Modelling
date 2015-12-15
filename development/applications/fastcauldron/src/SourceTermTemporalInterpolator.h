#ifndef FASTCAULDRON__SR_PROPERTY_TEMPORAL_INTERPOLATOR__H
#define FASTCAULDRON__SR_PROPERTY_TEMPORAL_INTERPOLATOR__H

#include "array.h"

#include "Subdomain.h"
#include "SourceTermInterpolatorCalculator.h"

/// \brief A container for property interpolators in Source rocks.
///
/// It will contain an interpolator for each element in the Source Rocks in Darcy subdomain.
/// The interpolator will be computed only the first time that it is accessed.

class SourceTermTemporalInterpolator {

public :

   typedef SourceTermInterpolatorCalculator::MultiInterpolator MultiInterpolator;

   SourceTermTemporalInterpolator ( const FormationSubdomainElementGrid&  subdomainGrid );

   ~SourceTermTemporalInterpolator ();


   /// \brief Get access to the interpolator associated with the element.
   ///
   const MultiInterpolator& access ( const LayerElement& element ) const;

   /// \brief Compute the interpolator
   void initialize ( const LayerElement& element ) const;
private :

   struct ElementInterpolator {
      MultiInterpolator m_interpolator;
      bool              m_isComputed;

      ElementInterpolator ();

   };

   typedef ibs::Array3D <ElementInterpolator> MultiInterpolator3DArray;

   /// \brief The calculator for the interpoaltor of the property.
   const SourceTermInterpolatorCalculator propertyCalculator;

   /// \brief An array containing the interpolators for every element in the Darcy subdomain.
   mutable MultiInterpolator3DArray m_interpolators;

   /// \brief Since the element will have global index values they must be "transformed" into local index values.
   ///
   /// This is acheived by subtracting the first global index of the subdomain from the element global index values.
   unsigned int m_indexTransformation [ 3 ];

}; 


inline SourceTermTemporalInterpolator::ElementInterpolator::ElementInterpolator () {
   m_isComputed = false;
}




#endif // FASTCAULDRON__SR_PROPERTY_TEMPORAL_INTERPOLATOR__H
