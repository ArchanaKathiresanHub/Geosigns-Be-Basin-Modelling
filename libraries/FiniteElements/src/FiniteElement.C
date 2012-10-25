#include "FiniteElement.h"


FiniteElementMethod::FiniteElement::FiniteElement () {
   m_geometryMatrix.zero ();
   m_jacobian.zero ();
   m_basis.zero ();
   m_referenceGradBasis.zero ();
}

void FiniteElementMethod::FiniteElement::setGeometryPoint ( const unsigned int position,
                                                            const double       x,
                                                            const double       y,
                                                            const double       z ) {

   m_geometryMatrix ( 1, position ) = x;
   m_geometryMatrix ( 2, position ) = y;
   m_geometryMatrix ( 3, position ) = z;
}

void FiniteElementMethod::FiniteElement::setGeometry ( const ElementGeometryMatrix& geometryMatrix ) {
   m_geometryMatrix = geometryMatrix;
}


void FiniteElementMethod::FiniteElement::setQuadraturePoint ( const double x,
                                                              const double y,
                                                              const double z,
                                                              const bool   computeJacobianInverse,
                                                              const bool   computeGradBasis ) {

   m_basisFunction ( x, y, z, m_basis, m_referenceGradBasis );
   matrixMatrixProduct ( m_geometryMatrix, m_referenceGradBasis, m_jacobian );

   if ( computeJacobianInverse or computeGradBasis ) {
      invert ( m_jacobian, m_jacobianInverse );
   }

   if ( computeGradBasis ) {
      matrixMatrixProduct ( m_referenceGradBasis, m_jacobianInverse, m_gradBasis );
   }

}

FiniteElementMethod::ThreeVector FiniteElementMethod::FiniteElement::interpolateGrad ( const ElementVector& coefficients ) const {

   ThreeVector referenceGradValue;
   ThreeVector gradValue;

   matrixTransposeVectorProduct ( m_referenceGradBasis, coefficients, referenceGradValue );
   matrixTransposeVectorProduct ( m_jacobianInverse, referenceGradValue, gradValue );

   return gradValue;
}

void FiniteElementMethod::FiniteElement::setTensor ( const double     verticalValue,
                                                     const double     horizontalValue,
                                                           Matrix3x3& tensor ) const {

   ThreeVector normal;
   double normalLength;

   double valueDiff = verticalValue - horizontalValue;

   getNormal ( normal, normalLength );

   tensor ( 1, 1 ) = valueDiff * normal ( 1 ) * normal ( 1 ) + horizontalValue;
   tensor ( 1, 2 ) = valueDiff * normal ( 1 ) * normal ( 2 );
   tensor ( 1, 3 ) = valueDiff * normal ( 1 ) * normal ( 3 );

   tensor ( 2, 1 ) = valueDiff * normal ( 2 ) * normal ( 1 );
   tensor ( 2, 2 ) = valueDiff * normal ( 2 ) * normal ( 2 ) + horizontalValue;
   tensor ( 2, 3 ) = valueDiff * normal ( 2 ) * normal ( 3 );

   tensor ( 3, 1 ) = valueDiff * normal ( 3 ) * normal ( 1 );
   tensor ( 3, 2 ) = valueDiff * normal ( 3 ) * normal ( 2 );
   tensor ( 3, 3 ) = valueDiff * normal ( 3 ) * normal ( 3 ) + horizontalValue;
}

void FiniteElementMethod::FiniteElement::getNormal ( ThreeVector& normal,
                                                     double&      normalLength ) const {

   ThreeVector row1;
   ThreeVector row2;

   // Two points about the following:
   //
   // 1. This may look strange but bcause we want the downward pointing normal,
   //    we swap the two rows in the cross product.
   //
   // 2. Because the Jacobian is computed (and used) as a transpose, the row
   //    and column indices must be transposed.
   //
   row1 ( 1 ) = m_jacobian ( 1, 2 );
   row1 ( 2 ) = m_jacobian ( 2, 2 );
   row1 ( 3 ) = m_jacobian ( 3, 2 );

   row2 ( 1 ) = m_jacobian ( 1, 1 );
   row2 ( 2 ) = m_jacobian ( 2, 1 );
   row2 ( 3 ) = m_jacobian ( 3, 1 );

   // Normal = R1 x R2 -- vector cross product
   normal ( 1 ) = row1 ( 2 ) * row2 ( 3 ) - row1 ( 3 ) * row2 ( 2 );
   normal ( 2 ) = row1 ( 3 ) * row2 ( 1 ) - row1 ( 1 ) * row2 ( 3 );
   normal ( 3 ) = row1 ( 1 ) * row2 ( 2 ) - row1 ( 2 ) * row2 ( 1 );

   normalLength  = length ( normal );

   scale ( normal, 1.0 / normalLength );

}
