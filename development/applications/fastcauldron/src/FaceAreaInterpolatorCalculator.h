#ifndef FASTCAULDRON__FACE_AREA_INTERPOLATOR_CALCULATOR_H
#define FASTCAULDRON__FACE_AREA_INTERPOLATOR_CALCULATOR_H

#include "SubdomainElement.h"
#include "VectorInterpolator.h"
#include "BoundaryId.h"


/// \brief Computes the interpolator for the area of the element face over time.
///
/// 
class FaceAreaInterpolatorCalculator {

public :

   /// \brief The number of faces an element has.
   static const unsigned int NumberOfEquations = 6;

   /// \brief The interpolator index for each of the element faces.
   static const unsigned int Gamma1Index = static_cast<unsigned int>(VolumeData::GAMMA_1);

   static const unsigned int Gamma2Index = static_cast<unsigned int>(VolumeData::GAMMA_2);

   static const unsigned int Gamma3Index = static_cast<unsigned int>(VolumeData::GAMMA_3);

   static const unsigned int Gamma4Index = static_cast<unsigned int>(VolumeData::GAMMA_4);

   static const unsigned int Gamma5Index = static_cast<unsigned int>(VolumeData::GAMMA_5);

   static const unsigned int Gamma6Index = static_cast<unsigned int>(VolumeData::GAMMA_6);


   typedef VectorInterpolator<NumberOfEquations> MultiInterpolator;

   typedef MultiInterpolator::CoefficientArray CoefficientArray;


   /// \brief Compute the interpolators for each face of the element.
   void compute ( const SubdomainElement&  element,
                  const CoefficientArray&  xs,
                        MultiInterpolator& interpolator ) const;


};

#endif // FASTCAULDRON__FACE_AREA_INTERPOLATOR_CALCULATOR_H
