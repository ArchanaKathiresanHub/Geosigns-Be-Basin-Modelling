#ifndef FASTCAULDRON__FACE_PERMEABILITY_INTERPOALTOR_CALCULATOR__H
#define FASTCAULDRON__FACE_PERMEABILITY_INTERPOALTOR_CALCULATOR__H

#include "array.h"
#include "Subdomain.h"
#include "SubdomainElement.h"
#include "MatrixInterpolator.h"
#include "PetscBlockVector.h"
#include "SimpleBoundedArray.h"


/// \brief Computes the interpoaltor of the permeabilities for the face of each element.
///
/// During construction the permeabilities are averaged, using the harmonic average, 
/// across the face of rach element. Some communication of values between processes
/// may be required for this.
class FacePermeabilityInterpolatorCalculator {

public :

   /// \brief The number of faces an element has.
   static const unsigned int NumberOfFaces = 6;

   /// \brief The normal and plane direction for the permeability.
   ///
   /// These will differ only if the permeability anisotropy is not 1.
   static const unsigned int NumberOfDirections = 2;

   static const unsigned int TotalNumberOfEquations = NumberOfFaces * NumberOfDirections;

   /// \brief The interpolator used for the permeabilities.
   typedef MatrixInterpolator<NumberOfFaces, NumberOfDirections> MultiInterpolator;

   /// \brief The array of coeficients.
   typedef MultiInterpolator::CoefficientArray CoefficientArray;


   /// \brief During construction the permeabilities are averaged across the faces of the elements.
   ///
   /// The computation of the interpolant is delayed until it is required.
   FacePermeabilityInterpolatorCalculator ( const Subdomain& subdomain );


   /// \brief Compute the interpoaltor for the permeability.
   ///
   /// The permeabilities are for each face and in each, of the two, directions: normal and plane.
   /// The interpolator will be computed from the, already averaged, values that are stored
   /// locally on the processor.
   void compute ( const SubdomainElement&  element,
                  const CoefficientArray&  xs,
                        MultiInterpolator& interpolator ) const;

private :


   /// \brief The number of permeability values that need to be stored in order to compute the averaged permeability interpoaltor.
   static const unsigned int NumberOfPermeabilityValues = MultiInterpolator::NumberOfCoefficients * TotalNumberOfEquations;

   /// \brief A one dimension array containing all the erquired permeability values.
   ///
   /// It is stored this way so that it makes the use of the petsc vectors easier.
   /// The order will be:
   ///    1. faces; then
   ///    2. direction; finally
   ///    3. abscissae.
   typedef SimpleBoundedArray <NumberOfPermeabilityValues> PermeabilityValuesArray;

   /// \brief The petsc vector used for communication of values between processors.
   ///
   /// Used only a single time during the construction.
   typedef PetscBlockVector<PermeabilityValuesArray> PermeabilityValuesVector;

   /// \brief The array of the permeability values.
   typedef ibs::Array3D <PermeabilityValuesArray> PermeabilityValues3DArray;


   /// \brief Compute the permeability values that are local to the processor.
   void computeLocalPermeabilities ( const Subdomain& subdomain,
                                     const ElementVolumeGrid& elementGrid,
                                     PermeabilityValuesVector& permeabilityValues ) const;

   /// \brief Compute the average permeability across all faces of all active elements.
   ///
   /// This will involve element-premeability values that are not local to this process so
   /// the permeability values vector must be in the correct state.
   void averagePermeabilities ( const Subdomain& subdomain,
                                const ElementVolumeGrid& elementGrid,
                                const PermeabilityValuesVector& permeabilityValues );


   /// \brief Will contain the averaged permeability values for each face, durection and abscissa.
   PermeabilityValues3DArray m_averagedPermeability;

   /// \brief Since the element will have global index values they must be "transformed" into local index values.
   ///
   /// This is acheived by subtracting the first global index of the subdomain from the element global index values.
   unsigned int m_indexTransformation [ 3 ];

};


#endif // FASTCAULDRON__FACE_PERMEABILITY_INTERPOALTOR_CALCULATOR__H
