//
// Copyright (C) 2015-2016 Shell International Exploration & Production.
// All rights reserved.
//
// Developed under license for Shell by PDS BV.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//
#ifndef GEOPHYSICS__PERMEABILITY_MIXER__H
#define GEOPHYSICS__PERMEABILITY_MIXER__H

// Access to STL
#ifdef _MSC_VER
#include <array>
#else
#include <tr1/array>
#endif

#include <vector>

#include "AlignedWorkSpaceArrays.h"
#include "ArrayDefinitions.h"
#include "GeoPhysicalConstants.h"
#include "MultiCompoundProperty.h"

#include "Interface/Interface.h"

namespace GeoPhysics {

   /// \brief Used to mix permeability values and their derivatives.
   class PermeabilityMixer {

   public :

      typedef std::tr1::array<double,MaximumNumberOfLithologies> FixedSizeArray;

      /// \brief Create typedef for the workspace of vectors used in intermediate permeability calculation.
      typedef AlignedWorkSpaceArrays<MaximumNumberOfLithologies> PermeabilityWorkSpaceArrays;

      PermeabilityMixer ();

      /// \brief Reset the permeability mixer object.
      ///
      /// \param [in] percentage      A vector containing the percentages of the lithologies.
      /// \param [in] anisotropies    A vector of the anosotropies for each simple lithology.
      /// \param [in] legacyBehaviour Indicate that legay behaviour is required
      /// \param [in] layeringIndex   The layering index.
      /// \param [in] mixModel        How the mixing is to be done.
      /// \param [in] isFault         Whether or not this lithology is a fault lithology.
      void reset ( const std::vector<double>&                percentages,
                   const std::vector<double>&                anisotropies,
                   const bool                                legacyBehaviour,
                   const double                              layeringIndex,
                   const DataAccess::Interface::MixModelType mixModel,
                   const bool                                isFault );

      /// \brief Get the current value of the layering index.
      double getLayeringIndex () const;

      /// \brief Mix the permeability.
      void mixPermeability ( const FixedSizeArray& simplePermeabilityValues,
                             double&               permeabilityNormal,
                             double&               permeabilityPlane ) const;


      /// \brief Mix the permeability derivatives.
      void mixPermeabilityDerivatives ( const FixedSizeArray& permeabilities,
                                        const FixedSizeArray& derivativesWRTVes,
                                        double&               permeabilityDerivativeNormal,
                                        double&               permeabilityDerivativePlane) const;

      /// \brief Mix the permeability for an array of values.
      ///
      /// \param  [in] size                  The number of values to be mixed.
      /// \param  [in] simplePermeabilities  The permeability values of the simple lithologies
      /// \param [out] permeabilityNormal    The mixed permeability values for the vertical direction.
      /// \param [out] permeabilityPlane     The mixed permeability values for the horizontal direction.
      void mixPermeabilityArray ( const unsigned int           size,
                                  PermeabilityWorkSpaceArrays& simplePermeabilities,
                                  ArrayDefs::Real_ptr          permeabilityNormal,
                                  ArrayDefs::Real_ptr          permeabilityPlane ) const;

   private :


      /// \brief Set the permeability for an array of values for one lithologies.
      void mixPermeabilityArrayOneLitho ( const unsigned int           size,
                                          PermeabilityWorkSpaceArrays& simplePermeabilities,
                                          ArrayDefs::Real_ptr          permeabilityNormal,
                                          ArrayDefs::Real_ptr          permeabilityPlane ) const;

      /// \brief Mix the permeability for an array of values for two lithologies.
      ///
      /// An optimisation for mixing only two lithologies.
      void mixPermeabilityArrayTwoLithos ( const unsigned int           size,
                                           PermeabilityWorkSpaceArrays& simplePermeabilities,
                                           ArrayDefs::Real_ptr          permeabilityNormal,
                                           ArrayDefs::Real_ptr          permeabilityPlane ) const;

      /// \brief Mix the permeability for an array of values for three lithologies.
      ///
      /// An optimisation for mixing only three lithologies.
      void mixPermeabilityArrayThreeLithos ( const unsigned int           size,
                                             PermeabilityWorkSpaceArrays& simplePermeabilities,
                                             ArrayDefs::Real_ptr          permeabilityNormal,
                                             ArrayDefs::Real_ptr          permeabilityPlane ) const;

      /// \brief Mix the permeability for an array of values for two lithologies.
      void mixTwoHomogeneous ( const unsigned int           size,
                               PermeabilityWorkSpaceArrays& simplePermeabilities,
                               ArrayDefs::Real_ptr          permeabilityNormal,
                               ArrayDefs::Real_ptr          permeabilityPlane ) const;

      /// \brief Mix the permeability for an array of values for three lithologies.
      void mixThreeHomogeneous ( const unsigned int           size,
                                 PermeabilityWorkSpaceArrays& simplePermeabilities,
                                 ArrayDefs::Real_ptr          permeabilityNormal,
                                 ArrayDefs::Real_ptr          permeabilityPlane ) const;

      /// \brief Mix the permeability for an array of values for two lithologies.
      ///
      /// An optimisation when the layering index is zero.
      void mixTwoLayeringIndexZero ( const unsigned int           size,
                                     PermeabilityWorkSpaceArrays& simplePermeabilities,
                                     ArrayDefs::Real_ptr          permeabilityNormal,
                                     ArrayDefs::Real_ptr          permeabilityPlane ) const;

      /// \brief Mix the permeability for an array of values for three lithologies.
      ///
      /// An optimisation when the layering index is zero.
      void mixThreeLayeringIndexZero ( const unsigned int           size,
                                       PermeabilityWorkSpaceArrays& simplePermeabilities,
                                       ArrayDefs::Real_ptr          permeabilityNormal,
                                       ArrayDefs::Real_ptr          permeabilityPlane ) const;
      /// \brief Mix the permeability for an array of values for two lithologies.
      ///
      /// An optimisation when the layering index is one.
      void mixTwoLayeringIndexOne ( const unsigned int           size,
                                    PermeabilityWorkSpaceArrays& simplePermeabilities,
                                    ArrayDefs::Real_ptr          permeabilityNormal,
                                    ArrayDefs::Real_ptr          permeabilityPlane ) const;

      /// \brief Mix the permeability for an array of values for three lithologies.
      ///
      /// An optimisation when the layering index is one.
      void mixThreeLayeringIndexOne ( const unsigned int           size,
                                      PermeabilityWorkSpaceArrays& simplePermeabilities,
                                      ArrayDefs::Real_ptr          permeabilityNormal,
                                      ArrayDefs::Real_ptr          permeabilityPlane ) const;

      /// \brief Mix the permeability for an array of values for two lithologies.
      ///
      /// An optimisation when the layering index is quarter.
      void mixTwoLayeringIndexQuarter ( const unsigned int           size,
                                        PermeabilityWorkSpaceArrays& simplePermeabilities,
                                        ArrayDefs::Real_ptr          permeabilityNormal,
                                        ArrayDefs::Real_ptr          permeabilityPlane ) const;

      /// \brief Mix the permeability for an array of values for three lithologies.
      ///
      /// An optimisation when the layering index is quarter.
      void mixThreeLayeringIndexQuarter ( const unsigned int           size,
                                          PermeabilityWorkSpaceArrays& simplePermeabilities,
                                          ArrayDefs::Real_ptr          permeabilityNormal,
                                          ArrayDefs::Real_ptr          permeabilityPlane ) const;

      /// \brief Mix the permeability for an array of values for two lithologies.
      void mixTwoLayeringIndexGeneral ( const unsigned int           size,
                                        PermeabilityWorkSpaceArrays& simplePermeabilities,
                                        ArrayDefs::Real_ptr          permeabilityNormal,
                                        ArrayDefs::Real_ptr          permeabilityPlane ) const;

      /// \brief Mix the permeability for an array of values for three lithologies.
      void mixThreeLayeringIndexGeneral ( const unsigned int           size,
                                          PermeabilityWorkSpaceArrays& simplePermeabilities,
                                          ArrayDefs::Real_ptr          permeabilityNormal,
                                          ArrayDefs::Real_ptr          permeabilityPlane ) const;

      /// \brief The lithology fractions.
      ///
      /// Written as \f$ \{ w_1, w_2, w_3 \} \f$
      std::vector<double>                 m_weights;

      /// \brief The individial lithology anosotropies.
      ///
      /// Written as \f$ \{ \alpha_1, \alpha_2, \alpha_3 \} \f$
      std::vector<double>                 m_anisotropies;

      /// \brief The layering index value \f$ L_i \f$.
      double                              m_layeringIndex;

      /// \brief Indicates how the mixing is to occur.
      DataAccess::Interface::MixModelType m_mixModel;

      bool                                m_legacyBehaviour;

      /// \brief Indicate whether the lithology is a fault lithology or not.
      ///
      /// All fault lithologies are to be mixed using a homogeneous mixing rule
      bool                                m_isFault;

      /// \brief Permeability horizontal mixing exponent, \f$ P_h \f$.
      ///
      /// \f$ P_h = \frac{1-4 L_i}{3} = \frac{1-P_v}{2}\f$
      double                              m_mixHorizonExp;

      /// \brief The inverse of \f$ P_h \f$, i.e. \f$ \frac{1}{P_h} \f$ .
      double m_inverseMixHorizonExp;

      // \brief Permeability vertical mixing exponent, \f$ P_v \f$
      ///
      /// \f$ P_v = \frac{1+2 L_i}{3} = \frac{1-P_v}{2}\f$
      double                               m_mixVerticalExp;

      /// \brief The inverse of \f$ P_v \f$, i.e. \f$ \frac{1}{P_v} \f$ .
      double                               m_inverseMixVerticalExp;

      /// \brief Value saved to simplify the computation of permeability.
      ///
      /// \f$ R_2 = \frac{w_2}{w_1}\f$
      double                               m_percentRatio2;

      /// brief Value saved to simplify the computation of permeability, \f$ w_1^{P_h} \f$
      double                               m_percentPowerPlane;

      /// brief Value saved to simplify the computation of permeability, \f$ w_1^{P_v} \f$
      double                               m_percentPowerNormal;

      /// brief Value saved to simplify the computation of permeability, \f$ \frac{w_3}{w_1} \f$
      double                               m_percentRatio3;

      /// brief Value saved to simplify the computation of permeability, \f$ \frac{\alpha_2}{\alpha_1} \f$
      double                               m_anisoRatioExp2;

      /// brief Value saved to simplify the computation of permeability, \f$ \frac{\alpha_3}{\alpha_1} \f$
      double                               m_anisoRatioExp3;

   };

}

inline double GeoPhysics::PermeabilityMixer::getLayeringIndex () const {
   return m_layeringIndex;
}


#endif // GEOPHYSICS__PERMEABILITY_MIXER__H
