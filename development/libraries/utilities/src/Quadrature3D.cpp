#include "Quadrature3D.h"
#include <climits>

NumericFunctions::Quadrature3D* NumericFunctions::Quadrature3D::s_quadratureInstance = 0;

void NumericFunctions::Quadrature3D::finalise () {
   Quadrature::finaliseQuadrature ();
}


void NumericFunctions::Quadrature3D::get ( const int       degree,
                                                 Iterator& iter ) {

   get ( degree, degree, degree, iter );
}

void NumericFunctions::Quadrature3D::get ( const int       xDegree,
                                           const int       yDegree,
                                           const int       zDegree,
                                                 Iterator& iter ) {

   iter.m_degree [ 0 ] = xDegree;
   iter.m_degree [ 1 ] = yDegree;
   iter.m_degree [ 2 ] = zDegree;

   Quadrature::getInstance ().getGaussLegendreQuadrature ( iter.m_degree [ 0 ],
                                                           iter.m_points [ 0 ],
                                                           iter.m_weights [ 0 ]);

   Quadrature::getInstance ().getGaussLegendreQuadrature ( iter.m_degree [ 1 ],
                                                           iter.m_points [ 1 ],
                                                           iter.m_weights [ 1 ]);

   Quadrature::getInstance ().getGaussLegendreQuadrature ( iter.m_degree [ 2 ],
                                                           iter.m_points [ 2 ],
                                                           iter.m_weights [ 2 ]);

   iter.initialise ();

}

void NumericFunctions::Quadrature3D::get ( const int       degree,
                                           const int       face,
                                                 Iterator& iter ) {

   unsigned int free1 = UINT_MAX;
   unsigned int free2 = UINT_MAX;
   unsigned int fixed = UINT_MAX;

   if ( face == 0 or face == 5 ) {
      free1 = 0;
      free2 = 1;
      fixed = 2;
   } else if ( face == 1 or face == 3 ) {
      free1 = 0;
      fixed = 1;
      free2 = 2;
   } else if ( face == 2 or face == 4 ) {
      fixed = 0;
      free1 = 1;
      free2 = 2;
   }

   iter.m_degree [ free1 ] = degree;
   iter.m_degree [ free2 ] = degree;
   iter.m_degree [ fixed ] = 1;

   Quadrature::getInstance ().getGaussLegendreQuadrature ( iter.m_degree [ free1 ],
                                                           iter.m_points [ free1 ],
                                                           iter.m_weights [ free1 ]);

   Quadrature::getInstance ().getGaussLegendreQuadrature ( iter.m_degree [ free2 ],
                                                           iter.m_points [ free2 ],
                                                           iter.m_weights [ free2 ]);

   Quadrature::getInstance ().getFixedQuadrature ( face,
                                                   iter.m_points [ fixed ],
                                                   iter.m_weights [ fixed ]);

   iter.initialise ();

}



NumericFunctions::Quadrature3D::Iterator::Iterator () {

   m_degree   [ 0 ] = 0;
   m_degree   [ 1 ] = 0;
   m_degree   [ 2 ] = 0;

   m_points   [ 0 ] = 0;
   m_points   [ 1 ] = 0;
   m_points   [ 2 ] = 0;

   m_weights  [ 0 ] = 0;
   m_weights  [ 1 ] = 0;
   m_weights  [ 2 ] = 0;

   m_position [ 0 ] = 0;
   m_position [ 1 ] = 0;
   m_position [ 2 ] = 0;

   m_iterationComplete = true;
}

void NumericFunctions::Quadrature3D::Iterator::initialise () {

   if ( m_points [ 0 ] != 0 ) {
      // If the iterator has been set then just reset the position and complete flag.
      m_position [ 0 ] = 0;
      m_position [ 1 ] = 0;
      m_position [ 2 ] = 0;
      m_iterationComplete = false;
   } else {
      // Otherwise set the complete flag to true so no iteration can occur.
      m_iterationComplete = true;
   }

}


NumericFunctions::Quadrature3D::Iterator& NumericFunctions::Quadrature3D::Iterator::operator++() {

   int i;

   for ( i = 2; i >= 0; --i ) {

      if ( m_position [ i ] + 1 == m_degree [ i ]) {
         m_position [ i ] = 0;
      } else {
         ++m_position [ i ];
         break;
      }

   }

   m_iterationComplete = ( i == -1 );

   return *this;
}

