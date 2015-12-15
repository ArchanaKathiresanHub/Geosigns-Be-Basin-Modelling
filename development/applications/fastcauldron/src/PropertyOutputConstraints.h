#ifndef _FASTCAULDRON_PROPERTY_OUTPUT_CONSTRAINTS_H_
#define _FASTCAULDRON_PROPERTY_OUTPUT_CONSTRAINTS_H_

#include <iostream>

#include "timefilter.h"
#include "globaldefs.h"

#include "Interface/Interface.h"
using namespace DataAccess;

namespace ApplicableOutputRegion {
   enum ApplicableRegion { SOURCE_ROCK_ONLY, SEDIMENTS_ONLY, SEDIMENTS_AND_BASEMENT };
}

const std::string& PropertyOutputOptionImage ( const Interface::PropertyOutputOption option );

class PropertyOutputConstraints {

public :

   PropertyOutputConstraints ();

   void initialise ( const CalculationMode          calculationMode,
                     const Interface::ModellingMode modellingMode );


   void print ( std::ostream& o = std::cout ) const;

   void constrain ( const PropertyList                     property,
                          Interface::PropertyOutputOption& option  ) const;

private :

   void applyOutputRegionConstraints ();

   void applyCalculationModeConstraints ( const CalculationMode calculationMode );

   void applyOutputPermittedConstraints ( const CalculationMode calculationMode );

   /// Ensure that the minimum option is not greater than the maximum.
   void ensureConsistency ();


   static const ApplicableOutputRegion::ApplicableRegion s_propertyOutputRegion [ PropertyListSize ];
   static const Interface::PropertyOutputOption          s_calculationModeMaxima [ NumberOfCalculationModes ];
   static const bool                                     s_outputPermitted [ PropertyListSize ][ NumberOfCalculationModes ];
   static const bool                                     s_outputRequired  [ PropertyListSize ][ NumberOfCalculationModes ];

   Interface::PropertyOutputOption m_minimumOutputOption [ PropertyListSize ];
   Interface::PropertyOutputOption m_maximumOutputOption [ PropertyListSize ];

};

#endif // _FASTCAULDRON_PROPERTY_OUTPUT_CONSTRAINTS_H_
