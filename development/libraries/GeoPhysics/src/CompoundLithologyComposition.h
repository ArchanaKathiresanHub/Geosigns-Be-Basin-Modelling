//
// Copyright (C) 2015-2016 Shell International Exploration & Production.
// All rights reserved.
//
// Developed under license for Shell by PDS BV.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//
#ifndef _GEOPHYSICS__COMPOUND_LITHOLOGY_COMPOSITION_H_
#define _GEOPHYSICS__COMPOUND_LITHOLOGY_COMPOSITION_H_


#include <string>

namespace GeoPhysics {

   /// Holds the names and fractions of the component simple-lithologies.
   class CompoundLithologyComposition {

   public :

      CompoundLithologyComposition ();

      CompoundLithologyComposition ( const std::string& lithoName1,
                                     const std::string& lithoName2,
                                     const std::string& lithoName3,
                                     const double       p1,
                                     const double       p2,
                                     const double       p3,
                                     const std::string& lithoMixModel,
                                     const double       lithoLayeringIndex);


      ///Set lithology names, percentages and compound lithology mixing model
      void setComposition ( const std::string& lithoName1,
                            const std::string& lithoName2,
                            const std::string& lithoName3,
                            const double       p1,
                            const double       p2,
                            const double       p3,
                            const std::string& lithoMixModel,
                            const double       lithoLayeringIndex);


      const std::string& lithologyName ( const int whichSimpleLithology ) const;

      double lithologyFraction ( const int whichSimpleLithology ) const;

      const std::string& mixingModel () const;

      double layeringIndex() const;

      const std::string& thermalModel () const;

      std::string returnKeyString () const;

      void setThermalModel(const std::string& thermalModel );

   private:

      std::string m_lythoType1;         //!< Name first lithology
      std::string m_lythoType2;         //!< Name second lithology
      std::string m_lythoType3;         //!< Name third lithology
      double      m_percent1;           //!< Percentage first lithology
      double      m_percent2;           //!< Percentage second lithology
      double      m_percent3;           //!< Percentage third lithology
      std::string m_mixModel;           //!< Mixing model
      double      m_mixLayeringIndex;   //!< Layering Index, used to mix permeabilities
      std::string m_thermModel;

   };

   bool operator< (const CompoundLithologyComposition& lhs, const CompoundLithologyComposition& rhs);

}

#endif // _GEOPHYSICS__COMPOUND_LITHOLOGY_COMPOSITION_H_
