#ifndef FASTCAULDRON__PORE_VOLUME_ROCK_COMPRESSION_INTERPOLATOR_CALCULATOR_H
#define FASTCAULDRON__PORE_VOLUME_ROCK_COMPRESSION_INTERPOLATOR_CALCULATOR_H

#include "SubdomainElement.h"
#include "VectorInterpolator.h"


/// \brief Computes the interpolator for the pore-volume and the rock compression term over time.
///
/// Every values required to calculate the interpoaltors are stored locally.
class PoreVolumeInterpolatorCalculator {

public :

   /// \brief The number of interpolation required.
   ///
   /// The pore-volume and the rock compression term
   static const unsigned int NumberOfEquations = 2;

   /// \brief The equation number of the pore-volume interpolator.
   static const unsigned int PoreVolumeIndex = 0;

   /// \brief The equation number of the rock-compression term interpolator.
   static const unsigned int RockCompressionIndex = 1;


   typedef VectorInterpolator<NumberOfEquations> MultiInterpolator;

   typedef MultiInterpolator::CoefficientArray CoefficientArray;


   void compute ( const SubdomainElement&  element,
                  const CoefficientArray&  xs,
                        MultiInterpolator& interpolator ) const;


};

#endif // FASTCAULDRON__PORE_VOLUME_ROCK_COMPRESSION_INTERPOLATOR_CALCULATOR_H
