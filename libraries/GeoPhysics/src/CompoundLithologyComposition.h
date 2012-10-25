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
                                     const std::string& lithoMixModel );

      void setComposition ( const std::string& lithoName1,
                            const std::string& lithoName2,
                            const std::string& lithoName3, 
                            const double       p1,
                            const double       p2,
                            const double       p3, 
                            const std::string& lithoMixModel );


      const std::string& lithologyName ( const int whichSimpleLithology ) const;

      double lithologyFraction ( const int whichSimpleLithology ) const;

      const std::string& mixingModel () const;

      const std::string& thermalModel () const;

      std::string returnKeyString () const;

      void setThermalModel(const std::string& thermalModel );

   private:

      std::string lythoType1;
      std::string lythoType2;
      std::string lythoType3;
      double      percent1;
      double      percent2;
      double      percent3;
      std::string mixModel;
      std::string thermModel;

   };

   bool operator< (const CompoundLithologyComposition& lhs, const CompoundLithologyComposition& rhs);

}

#endif // _GEOPHYSICS__COMPOUND_LITHOLOGY_COMPOSITION_H_
