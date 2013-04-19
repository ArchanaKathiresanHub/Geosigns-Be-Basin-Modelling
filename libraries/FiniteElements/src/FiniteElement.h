#ifndef _FINITE_ELEMENT_METHOD__FINITE_ELEMENT_H_
#define _FINITE_ELEMENT_METHOD__FINITE_ELEMENT_H_

#include "FiniteElementTypes.h"
#include "BasisFunction.h"

namespace FiniteElementMethod {


   class FiniteElement {

   public :

      FiniteElement ();

      /// Assign a point in the geoemtry-matrix.
      void setGeometryPoint ( const unsigned int,
                              const double   x,
                              const double   y,
                              const double   z );

      /// Assign the geometry-matrix.
      void setGeometry ( const ElementGeometryMatrix& geometryMatrix );


      /// Evaluate the basis and grad-basis functions and compute the Jacobian at the quadrature point.
      void setQuadraturePoint ( const double x,
                                const double y,
                                const double z,
                                const bool   computeJacobianInverse = true,
                                const bool   computeGradBasis = true );

      /// Return a constant reference to the Jacobian matrix.
      const Matrix3x3& getJacobian () const;

      /// Return a constant reference to the inverse of the Jacobian matrix.
      const Matrix3x3& getJacobianInverse () const;

      /// Return a constant reference to the basis functions.
      ///
      /// The basis functions have been evaluated at the quadrature point.
      const ElementVector& getBasis () const;

      /// \brief Return a constant reference to the reference grad-basis functions.
      ///
      /// The grad-basis functions have been evaluated at the quadrature point.
      const GradElementVector& getReferenceGradBasis () const;

      /// \brief Return a constant reference to the grad-basis functions.
      ///
      /// The grad-basis functions have been evaluated at the quadrature point.
      const GradElementVector& getGradBasis () const;

      /// \brief Return a constant reference to the geometry-matrix.
      const ElementGeometryMatrix& getGeometryMatrix () const;


      /// Interpolate the values at the quadrature-point.
      double interpolate ( const ElementVector& coefficients ) const;

      /// Interpolate the grad-values at the quadrature-point.
      ThreeVector interpolateGrad ( const ElementVector& coefficients ) const;

      /// Computes the permeability or temperature-conductivity tensor.
      ///
      /// Given  (permN, permH) or (tempCondN, tempCondH) pairs.
      void setTensor ( const double     verticalValue,
                       const double     horizontalValue,
                             Matrix3x3& tensor ) const;

      /// Compute the upwards facing normal.
      ///
      /// The domain has a positive depth, so upwards here is in the negative depth direction.
      void getNormal ( ThreeVector& normal,
                       double&      normalLength ) const;

   private :

      BasisFunction         m_basisFunction;
      Matrix3x3             m_jacobian;
      Matrix3x3             m_jacobianInverse;
      ElementVector         m_basis;
      GradElementVector     m_referenceGradBasis;
      GradElementVector     m_gradBasis;
      ElementGeometryMatrix m_geometryMatrix;

   };

}

//------------------------------------------------------------//
//   Inline functions.
//------------------------------------------------------------//

inline const FiniteElementMethod::Matrix3x3& FiniteElementMethod::FiniteElement::getJacobian () const {
   return m_jacobian;
}

inline const FiniteElementMethod::Matrix3x3& FiniteElementMethod::FiniteElement::getJacobianInverse () const {
   return m_jacobianInverse;
}

inline const FiniteElementMethod::ElementVector& FiniteElementMethod::FiniteElement::getBasis () const {
   return m_basis;
}

inline const FiniteElementMethod::GradElementVector& FiniteElementMethod::FiniteElement::getReferenceGradBasis () const {
   return m_referenceGradBasis;
}

inline const FiniteElementMethod::GradElementVector& FiniteElementMethod::FiniteElement::getGradBasis () const {
   return m_gradBasis;
}

inline const FiniteElementMethod::ElementGeometryMatrix& FiniteElementMethod::FiniteElement::getGeometryMatrix () const {
   return m_geometryMatrix;
}

inline double FiniteElementMethod::FiniteElement::interpolate ( const ElementVector& coefficients ) const {
   return FiniteElementMethod::innerProduct ( m_basis, coefficients );
}

#endif // _FINITE_ELEMENT_METHOD__FINITE_ELEMENT_H_
