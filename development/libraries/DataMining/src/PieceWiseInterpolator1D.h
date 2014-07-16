#ifndef _NUMERICS__PIECEWISE_INTERPOLATOR_1D_H_
#define _NUMERICS__PIECEWISE_INTERPOLATOR_1D_H_

#include <vector>
#include <string>

namespace Numerics {

   /// Simple one-dimension piecewise interpolator.
   class PieceWiseInterpolator1D {

      typedef std::vector<double> DoubleArray;

   public :

      /// The null-value.
      static const double NullValue;

      /// \enum Which piecewise-interpolation scheme is to be used.
      enum InterpolationKind { PIECEWISE_LINEAR, CUBIC_SPLINE };

      /// \brief Return the string representation of the interpolation-kind.
      static const std::string& InterpolationKindImage ( const InterpolationKind kind );

      /// \brief Return the short string representation of the interpolation-kind.
      static const std::string& InterpolationKindShortImage ( const InterpolationKind kind );

      PieceWiseInterpolator1D ();

      ~PieceWiseInterpolator1D ();

      /// \brief Add a data point to the interpolator.
      void addPoint ( const double x,
                      const double y );

      /// \brief Compute the interpolator based on the add data points.
      ///
      /// Once frozen no more data can be added.
      void freeze ( const InterpolationKind kind = PIECEWISE_LINEAR,
                    const bool              allowExtrapolation = false );

      /// \brief Determine if the interpolator object is frozen.
      bool isFrozen () const;

      /// \brief Evaluate the interpolator.
      double operator ()( const double s ) const;

   private :

      /// \brief Find the panel in which the point lies.
      int findPanel ( const double s ) const;

      /// \brief Evaluate the linear interpolator.
      double evaluateLinear ( const double s ) const;

      /// \brief Evaluate the cubic-spline interpolator.
      double evaluateCubic ( const double s ) const;

      /// \brief Compute the linear interpolator.
      void computeLinear ();

      /// \brief Compute the cubic-spline interpolator.
      void computeCubic ();

      InterpolationKind m_kind;
      bool              m_frozen;
      bool              m_allowExtrapolation;

      DoubleArray m_xs;
      DoubleArray m_ys;

      DoubleArray m_as;
      DoubleArray m_bs;
      DoubleArray m_cs;
      DoubleArray m_ds;

   };

}

#endif // _NUMERICS__PIECEWISE_INTERPOLATOR_1D_H_
