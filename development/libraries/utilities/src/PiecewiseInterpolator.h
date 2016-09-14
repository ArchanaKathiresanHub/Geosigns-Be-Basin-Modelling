#ifndef _UTILITIES__PIECEWISE_INTERPOLATOR_H_
#define _UTILITIES__PIECEWISE_INTERPOLATOR_H_

#include <iostream>
#include <vector>
#include <string>

namespace ibs {

   /// \brief A simple piece-wise linear interpolator.
   ///
   /// The data must be added to the interpolator before calculation
   /// can proceed.
   class PiecewiseInterpolator {

      /// \brief Class used to sort porosit and permeability values.
      ///
      /// It does not sort the values directly but is used to sort an index pointer array.
      class PointerSort {

      public :

         /// Constructor with the array to be sorted.
         PointerSort ( const double* xVals );

         bool operator ()( const int p1,
                           const int p2 ) const;

      private :

         const double* m_xs;

      };

   public :


      PiecewiseInterpolator ();

      ~PiecewiseInterpolator ();

      /// \brief Set the values for the interpolator.
      ///
      /// The data will copied and sorted into ascending order of porosities.
      void setInterpolation ( const int                 newNumberOfPoints,
                              const double*             newPorosities,
                              const double*             newPermeabilities );

      /// \brief Evaluate the interpolator at the point.
      double evaluate ( const double value ) const;

      /// \brief Evaluate the interpolator for an array of values.
      ///
      /// The values array is over written by the interpolated values.
      void evaluate ( const int size, double* values ) const;

      /// \brief Evaluate the derivative of the interpolator at the point.
      double evaluateDerivative ( const double value ) const;

      void print ( std::ostream& o ) const;

      /// \brief Return a string representation of the interpolator.
      std::string image () const;

      /// \brief Assignment operator
      PiecewiseInterpolator& operator=( const PiecewiseInterpolator& newInterpolator );


   private :

      /// \brief Compute the interpolation coefficients, based on the data set.
      ///
      /// All member varibales must be set before calling this function.
      void computeCoefficients ();

      /// \brief Deallocate the arrays used in interpolator and set them to null.
      void deleteCoefficients ();

      /// \brief Find in which panel, if any, the point lies.
      int findPanel ( const double value ) const;

      /// The number of points in the interpolant.
      int m_numberOfPoints;

      /// The coefficients of the interpolant.
      double* m_aCoeffs;
      double* m_bCoeffs;

      /// The locally stored x-points.
      double* m_xs;

      /// The locally stored y-points.
      double* m_ys;

   };

}


inline bool ibs::PiecewiseInterpolator::PointerSort::operator ()( const int p1,
                                                                  const int p2 ) const {
   return m_xs [ p1 ] < m_xs [ p2 ];
}


#endif // _UTILITIES__PIECEWISE_INTERPOLATOR_H_
