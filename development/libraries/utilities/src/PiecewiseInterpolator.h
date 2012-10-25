#ifndef _UTILITIES__PIECEWISE_INTERPOLATOR_H_
#define _UTILITIES__PIECEWISE_INTERPOLATOR_H_

#include <iostream>
#include <vector>
#include <string>

namespace ibs {

   /// A simple piece-wise linear interpolator.
   /// The data must be added to the interpolator before calculation 
   /// can proceed.
   // Not currently working for cubic-spline, although all the code is
   // there, it has not been tested.
   class PiecewiseInterpolator {

   public :

      /// Which kind of interpolation, linear or cubic.
      enum InterpolationMethod { PIECEWISE_LINEAR, CUBIC_SPLINE };

      PiecewiseInterpolator ();

      ~PiecewiseInterpolator ();

      void setInterpolation
         ( const InterpolationMethod newInterpolationMethod,
           const int                 newNumberOfPoints,
           const double*             newPorosities,
           const double*             newPermeabilities );

      /// Compute the interpolation coefficients, based on the data set.
      void computeCoefficients ();

      /// Evaluate the interpolator at the point.
      double evaluate ( const double value ) const;

      /// Evaluate the derivative of the interpolator at the point.
      double evaluateDerivative ( const double value ) const;

      void print ( std::ostream& o ) const;

      /// Return a string representation of the interpolator.
      std::string image () const;

      PiecewiseInterpolator& operator=( const PiecewiseInterpolator& newInterpolator );

      friend bool operator== ( const PiecewiseInterpolator& interp1, 
                               const PiecewiseInterpolator& interp2 );


   private :

      void deleteCoefficients ();

      /// Find ni which panel, if any, the point lies.
      int findPanel ( const double value ) const;

      void computePiecewiseLinearCoefficients ();

      void computeCubicSplineCoefficients ();

      double evaluatePiecewiseLinear ( const double value ) const;

      double evaluateCubicSpline     ( const double value ) const;

      double evaluatePiecewiseLinearDerivative ( const double value ) const;

      double evaluateCubicSplineDerivative     ( const double value ) const;

      /// The number of points in the interpolant.
      int m_numberOfPoints;

      /// The interpolation method used in this interpolator.
      InterpolationMethod m_method;

      /// The coefficients of the interpolant.
      double* m_aCoeffs;
      double* m_bCoeffs;
      double* m_cCoeffs;
      double* m_dCoeffs;

      // Coefficients for the extrapolation method.
      double m_m0;
      double m_c0;

      double m_mN;
      double m_cN;

      /// The locally stored x-points.
      double* m_xs;

      /// The locally stored y-points.
      double* m_ys;

   }; 

}


#endif // _UTILITIES__PIECEWISE_INTERPOLATOR_H_
