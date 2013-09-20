#include "SourceTermTemporalInterpolator.h"

SourceTermTemporalInterpolator::SourceTermTemporalInterpolator ( const FormationSubdomainElementGrid& subdomainGrid ) : 
   m_interpolators ( subdomainGrid.lengthI (), subdomainGrid.lengthJ (), subdomainGrid.lengthK ())
{
   m_indexTransformation [ 0 ] = subdomainGrid.firstI ();
   m_indexTransformation [ 1 ] = subdomainGrid.firstJ ();
   m_indexTransformation [ 2 ] = subdomainGrid.firstK ();
}


SourceTermTemporalInterpolator::~SourceTermTemporalInterpolator () {
}

const typename SourceTermInterpolatorCalculator::MultiInterpolator& SourceTermTemporalInterpolator::access ( const LayerElement& element ) const {



   return m_interpolators ( element.getIPosition ()      - m_indexTransformation [ 0 ],
                            element.getJPosition ()      - m_indexTransformation [ 1 ],
                            element.getLocalKPosition () - m_indexTransformation [ 2 ] ).m_interpolator;
}

void SourceTermTemporalInterpolator::initialize ( const LayerElement& element ) const {

   if ( not m_interpolators ( element.getIPosition ()      - m_indexTransformation [ 0 ],
                              element.getJPosition ()      - m_indexTransformation [ 1 ],
                              element.getLocalKPosition () - m_indexTransformation [ 2 ]).m_isComputed ) {

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
                                   m_interpolators ( element.getIPosition ()      - m_indexTransformation [ 0 ],
                                                     element.getJPosition ()      - m_indexTransformation [ 1 ],
                                                     element.getLocalKPosition () - m_indexTransformation [ 2 ] ).m_interpolator );
      m_interpolators ( element.getIPosition ()      - m_indexTransformation [ 0 ],
                        element.getJPosition ()      - m_indexTransformation [ 1 ],
                        element.getLocalKPosition () - m_indexTransformation [ 2 ] ).m_isComputed = true;
   } 
}
