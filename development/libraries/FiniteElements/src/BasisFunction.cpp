#include "BasisFunction.h"

void FiniteElementMethod::BasisFunction::operator ()
   ( const double         xi,
     const double         eta,
     const double         zeta,
           ElementVector& basis ) const {

  basis ( 1 ) = 0.125 * ( 1.0 - xi ) * ( 1.0 - eta ) * ( 1.0 - zeta );
  basis ( 2 ) = 0.125 * ( 1.0 + xi ) * ( 1.0 - eta ) * ( 1.0 - zeta );
  basis ( 3 ) = 0.125 * ( 1.0 + xi ) * ( 1.0 + eta ) * ( 1.0 - zeta );
  basis ( 4 ) = 0.125 * ( 1.0 - xi ) * ( 1.0 + eta ) * ( 1.0 - zeta );

  basis ( 5 ) = 0.125 * ( 1.0 - xi ) * ( 1.0 - eta ) * ( 1.0 + zeta );
  basis ( 6 ) = 0.125 * ( 1.0 + xi ) * ( 1.0 - eta ) * ( 1.0 + zeta );
  basis ( 7 ) = 0.125 * ( 1.0 + xi ) * ( 1.0 + eta ) * ( 1.0 + zeta );
  basis ( 8 ) = 0.125 * ( 1.0 - xi ) * ( 1.0 + eta ) * ( 1.0 + zeta );

}

//------------------------------------------------------------//

void FiniteElementMethod::BasisFunction::operator ()
   ( const double             xi,
     const double             eta,
     const double             zeta,
           GradElementVector& gradBasis ) const {



  gradBasis ( 1, 1 ) = -0.125 * ( 1.0 - eta ) * ( 1.0 - zeta );
  gradBasis ( 1, 2 ) = -0.125 * ( 1.0 - xi  ) * ( 1.0 - zeta );
  gradBasis ( 1, 3 ) = -0.125 * ( 1.0 - xi  ) * ( 1.0 - eta );

  gradBasis ( 2, 1 ) =  0.125 * ( 1.0 - eta ) * ( 1.0 - zeta );
  gradBasis ( 2, 2 ) = -0.125 * ( 1.0 + xi  ) * ( 1.0 - zeta );
  gradBasis ( 2, 3 ) = -0.125 * ( 1.0 + xi  ) * ( 1.0 - eta );

  gradBasis ( 3, 1 ) =  0.125 * ( 1.0 + eta ) * ( 1.0 - zeta );
  gradBasis ( 3, 2 ) =  0.125 * ( 1.0 + xi  ) * ( 1.0 - zeta );
  gradBasis ( 3, 3 ) = -0.125 * ( 1.0 + xi  ) * ( 1.0 + eta );

  gradBasis ( 4, 1 ) = -0.125 * ( 1.0 + eta ) * ( 1.0 - zeta );
  gradBasis ( 4, 2 ) =  0.125 * ( 1.0 - xi  ) * ( 1.0 - zeta );
  gradBasis ( 4, 3 ) = -0.125 * ( 1.0 - xi  ) * ( 1.0 + eta );


  gradBasis ( 5, 1 ) = -0.125 * ( 1.0 - eta ) * ( 1.0 + zeta );
  gradBasis ( 5, 2 ) = -0.125 * ( 1.0 - xi  ) * ( 1.0 + zeta );
  gradBasis ( 5, 3 ) =  0.125 * ( 1.0 - xi  ) * ( 1.0 - eta );

  gradBasis ( 6, 1 ) =  0.125 * ( 1.0 - eta ) * ( 1.0 + zeta );
  gradBasis ( 6, 2 ) = -0.125 * ( 1.0 + xi  ) * ( 1.0 + zeta );
  gradBasis ( 6, 3 ) =  0.125 * ( 1.0 + xi  ) * ( 1.0 - eta );

  gradBasis ( 7, 1 ) =  0.125 * ( 1.0 + eta ) * ( 1.0 + zeta );
  gradBasis ( 7, 2 ) =  0.125 * ( 1.0 + xi  ) * ( 1.0 + zeta );
  gradBasis ( 7, 3 ) =  0.125 * ( 1.0 + xi  ) * ( 1.0 + eta );

  gradBasis ( 8, 1 ) = -0.125 * ( 1.0 + eta ) * ( 1.0 + zeta );
  gradBasis ( 8, 2 ) =  0.125 * ( 1.0 - xi  ) * ( 1.0 + zeta );
  gradBasis ( 8, 3 ) =  0.125 * ( 1.0 - xi  ) * ( 1.0 + eta );

}

//------------------------------------------------------------//

void FiniteElementMethod::BasisFunction::operator ()
   ( const double             xi,
     const double             eta,
     const double             zeta,
           ElementVector&     basis,
           GradElementVector& gradBasis ) const {


  const double One_Minus_xi = 1.0 - xi;
  const double One_Plus_xi  = 1.0 + xi;

  const double One_Minus_eta = 1.0 - eta;
  const double One_Plus_eta  = 1.0 + eta;

  const double One_Minus_zeta = 1.0 - zeta;
  const double One_Plus_zeta  = 1.0 + zeta;


  basis ( 1 ) = 0.125 * One_Minus_xi * One_Minus_eta * One_Minus_zeta;
  basis ( 2 ) = 0.125 * One_Plus_xi  * One_Minus_eta * One_Minus_zeta;
  basis ( 3 ) = 0.125 * One_Plus_xi  * One_Plus_eta  * One_Minus_zeta;
  basis ( 4 ) = 0.125 * One_Minus_xi * One_Plus_eta  * One_Minus_zeta;

  basis ( 5 ) = 0.125 * One_Minus_xi * One_Minus_eta * One_Plus_zeta;
  basis ( 6 ) = 0.125 * One_Plus_xi  * One_Minus_eta * One_Plus_zeta;
  basis ( 7 ) = 0.125 * One_Plus_xi  * One_Plus_eta  * One_Plus_zeta;
  basis ( 8 ) = 0.125 * One_Minus_xi * One_Plus_eta  * One_Plus_zeta;


  gradBasis ( 1, 1 ) = -0.125 * One_Minus_eta * One_Minus_zeta;
  gradBasis ( 1, 2 ) = -0.125 * One_Minus_xi  * One_Minus_zeta;
  gradBasis ( 1, 3 ) = -0.125 * One_Minus_xi  * One_Minus_eta;

  gradBasis ( 2, 1 ) =  0.125 * One_Minus_eta * One_Minus_zeta;
  gradBasis ( 2, 2 ) = -0.125 * One_Plus_xi   * One_Minus_zeta;
  gradBasis ( 2, 3 ) = -0.125 * One_Plus_xi   * One_Minus_eta;

  gradBasis ( 3, 1 ) =  0.125 * One_Plus_eta * One_Minus_zeta;
  gradBasis ( 3, 2 ) =  0.125 * One_Plus_xi  * One_Minus_zeta;
  gradBasis ( 3, 3 ) = -0.125 * One_Plus_xi  * One_Plus_eta;

  gradBasis ( 4, 1 ) = -0.125 * One_Plus_eta * One_Minus_zeta;
  gradBasis ( 4, 2 ) =  0.125 * One_Minus_xi * One_Minus_zeta;
  gradBasis ( 4, 3 ) = -0.125 * One_Minus_xi * One_Plus_eta;


  gradBasis ( 5, 1 ) = -0.125 * One_Minus_eta * One_Plus_zeta;
  gradBasis ( 5, 2 ) = -0.125 * One_Minus_xi  * One_Plus_zeta;
  gradBasis ( 5, 3 ) =  0.125 * One_Minus_xi  * One_Minus_eta;

  gradBasis ( 6, 1 ) =  0.125 * One_Minus_eta * One_Plus_zeta;
  gradBasis ( 6, 2 ) = -0.125 * One_Plus_xi   * One_Plus_zeta;
  gradBasis ( 6, 3 ) =  0.125 * One_Plus_xi   * One_Minus_eta;

  gradBasis ( 7, 1 ) =  0.125 * One_Plus_eta * One_Plus_zeta;
  gradBasis ( 7, 2 ) =  0.125 * One_Plus_xi  * One_Plus_zeta;
  gradBasis ( 7, 3 ) =  0.125 * One_Plus_xi  * One_Plus_eta;

  gradBasis ( 8, 1 ) = -0.125 * One_Plus_eta * One_Plus_zeta;
  gradBasis ( 8, 2 ) =  0.125 * One_Minus_xi * One_Plus_zeta;
  gradBasis ( 8, 3 ) =  0.125 * One_Minus_xi * One_Plus_eta;

}

//------------------------------------------------------------//
