#ifndef _FASTCAULDRON__TEMPORAL_PROPERTY_INTERPOLATOR__H_
#define _FASTCAULDRON__TEMPORAL_PROPERTY_INTERPOLATOR__H_

#include "PetscBlockVector.h"
#include "property_manager.h"
#include "Subdomain.h"
#include "SubdomainElement.h"
#include "ElementVolumeGrid.h"

class ElementTemporalPropertyInterpolator {

public :

   static const int BlockSize = 2;

   typedef int IndexType;

   /// \brief Set the end points of the interpolant.
   void set ( const double startValue,
              const double endValue );

   /// \brief Get the start point of the interval.
   double getPropertyStart () const;

   /// \brief Get the end point of the interval.
   double getPropertyEnd () const;

   /// \brief Interpolate the property.
   ///
   /// p = ( 1 - lambda ) * p_start + lambda * p_end.
   double interpolate ( const double lambda ) const;

private :

   /// The start point of the interval.
   double m_propertyStart;

   /// The end point of the interval.
   double m_propertyEnd;

};


//------------------------------------------------------------//

class TemporalPropertyInterpolator {

   typedef PetscBlockVector <ElementTemporalPropertyInterpolator> InterpolatorArray;

public :

   TemporalPropertyInterpolator ( Subdomain& sub );

   TemporalPropertyInterpolator ( Subdomain& sub,
                                  const Basin_Modelling::Fundamental_Property property);

   ~TemporalPropertyInterpolator ();


   void setProperty ( const Basin_Modelling::Fundamental_Property property );

   double operator ()( const int i, const int j, const int k, const double lambda ) const;

   double operator ()( const SubdomainElement& element, const double lambda ) const;

   const ElementTemporalPropertyInterpolator&  operator ()( const int i, const int j, const int k ) const;

   const ElementTemporalPropertyInterpolator&  operator ()( const SubdomainElement& element ) const;


private :

   void computeTemporalProperty ( FormationSubdomainElementGrid&              formationGrid,
                                  const Basin_Modelling::Fundamental_Property property,
                                  InterpolatorArray&                          interpolator );


   Subdomain&         m_subdomain;
   ElementVolumeGrid& m_elementGrid;
   Vec                m_propertyVector;
   InterpolatorArray  m_propertyArray;
   bool               m_vectorRetrieved;
   
};

//------------------------------------------------------------//
//
// Inline functions.
//

inline double ElementTemporalPropertyInterpolator::getPropertyStart () const {
   return m_propertyStart;
}

inline double ElementTemporalPropertyInterpolator::getPropertyEnd () const {
   return m_propertyEnd;
}

inline double ElementTemporalPropertyInterpolator::interpolate ( const double lambda ) const {
   return ( 1.0 - lambda ) * m_propertyStart + lambda * m_propertyEnd;
}

inline double TemporalPropertyInterpolator::operator ()( const int i, const int j, const int k, const double lambda ) const {

#if 0
   assert ( m_vectorRetrieved );
#endif 

   return m_propertyArray ( k, j, i ).interpolate ( lambda );
}

inline double TemporalPropertyInterpolator::operator ()( const SubdomainElement& element, const double lambda ) const {

#if 0
   assert ( m_vectorRetrieved );
#endif 
   return operator ()( element.getI (), element.getJ (), element.getK (), lambda );
}

inline const ElementTemporalPropertyInterpolator& TemporalPropertyInterpolator::operator ()( const int i, const int j, const int k ) const {

#if 0
   assert ( m_vectorRetrieved );
#endif 

   return m_propertyArray ( k, j, i );
}


inline const ElementTemporalPropertyInterpolator& TemporalPropertyInterpolator::operator ()( const SubdomainElement& element ) const {

#if 0
   assert ( m_vectorRetrieved );
#endif 

   return operator ()( element.getI (), element.getJ (), element.getK ());
}


#endif // _FASTCAULDRON__TEMPORAL_PROPERTY_INTERPOLATOR__H_
