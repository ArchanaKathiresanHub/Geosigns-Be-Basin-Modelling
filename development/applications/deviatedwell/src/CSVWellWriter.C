#include "CSVWellWriter.h"

#include <iostream>
#include <iomanip>

#include "Interface/GridMap.h"
#include "Interface/Formation.h"
#include "Interface/LithoType.h"
#include "Interface/Property.h"

#include "GlobalDefs.h"

using namespace DataAccess;
using namespace Mining;

const std::string CSVWellWriter::identifier = "csv";
const std::string CSVWellWriter::extension = "csv";


CSVWellWriter::CSVWellWriter () : WellWriter ( extension ) {
}

CSVWellWriter::~CSVWellWriter () {
} 

void CSVWellWriter::write ( const std::string&                                 name,
                            const DataAccess::Mining::ElementPositionSequence& elements,
                            const DataAccess::Mining::CauldronWell&            well,
                            const DataAccess::Mining::DataMiner::ResultValues& results,
                            const DataAccess::Mining::DataMiner::PropertySet&  properties,
                            const DistanceUnit                                 outputDistanceUnit ) {

   std::ofstream outputFile ( name.c_str ());
   size_t defaultWidth = 27;

   size_t i;
   size_t j;

   double value;
   // The depth of the water surface, Kelly-bushing of any other possible elevation.
   double additionalDepth;

   const Interface::GridMap* map;

   additionalDepth = 0.0;

   if ( well.getWaterDepth () != well.getNullValue ()) {
      additionalDepth = well.getWaterDepth ();
   }

   if ( well.getKellyBushingDepth () != well.getNullValue ()) {
      additionalDepth += well.getKellyBushingDepth ();
   }

   if ( well.getElevation () != well.getNullValue ()) {
      additionalDepth += well.getElevation ();
   }

   // Output the well-name.
   outputFile << well.getName () << std::endl;

   // output the header.
   outputFile << std::setw ( defaultWidth ) << "X (" + unitShortIdentifier ( outputDistanceUnit ) + ")" << ",";
   outputFile << std::setw ( defaultWidth ) << "Y (" + unitShortIdentifier ( outputDistanceUnit ) + ")" << ",";

#if 0
   outputFile << std::setw ( defaultWidth ) << "Z (" + unitShortIdentifier ( outputDistanceUnit ) + ")" << ",";
#endif

   outputFile << std::setw ( defaultWidth ) << "TVD (" + unitShortIdentifier ( outputDistanceUnit ) + ")" << ",";
   outputFile << std::setw ( defaultWidth ) << "DAH (" + unitShortIdentifier ( outputDistanceUnit ) + ")" << ",";
   outputFile << std::setw ( defaultWidth ) << "Formation" << ',';
   outputFile << std::setw ( defaultWidth ) << "Lithology1" << ',';
   outputFile << std::setw ( defaultWidth ) << "Lithology2" << ',';
   outputFile << std::setw ( defaultWidth ) << "Lithology3" << ',';
   outputFile << std::setw ( defaultWidth ) << "Fraction1 (%)" << ',';
   outputFile << std::setw ( defaultWidth ) << "Fraction2 (%)" << ',';
   outputFile << std::setw ( defaultWidth ) << "Fraction3 (%)" << ',';

   for ( i = 0; i < properties.size (); ++i ) {
      outputFile << std::setw ( defaultWidth ) << properties [ i ]->getName () + " (" + properties [ i ]->getUnit () + ")";

      if ( i < properties.size () - 1 ) {
         outputFile << ",";
      }

   }

   outputFile << std::endl;
   // END of header output.

   // Loop over all data values.
   for ( i = 0; i < results.size (); ++i ) {
      const InterpolatedPropertyValues& sample = results [ i ];
      const ElementPosition& element = sample.getElement ();

      // output the x value.
      value = element.getActualPoint ()( 0 );

      if ( value == DataAccess::Interface::DefaultUndefinedScalarValue or value == DataAccess::Interface::DefaultUndefinedMapValue ) {
         value = well.getNullValue ();
      } else {
         value = convertUnitValue ( value, METRE, outputDistanceUnit );
      }

      outputFile << std::setw ( defaultWidth ) << value << ",";

      // output the y value.
      value = element.getActualPoint ()( 1 );

      if ( value == DataAccess::Interface::DefaultUndefinedScalarValue or value == DataAccess::Interface::DefaultUndefinedMapValue ) {
         value = well.getNullValue ();
      } else {
         value = convertUnitValue ( value, METRE, outputDistanceUnit );
      }

      outputFile << std::setw ( defaultWidth ) << value << ",";

#if 0
      // output the z value.
      value = element.getActualPoint ()( 2 );

      if ( value == DataAccess::Interface::DefaultUndefinedScalarValue or value == DataAccess::Interface::DefaultUndefinedMapValue ) {
         value = well.getNullValue ();
      } else {
         value = convertUnitValue ( value, METRE, outputDistanceUnit );
      }

      outputFile << std::setw ( defaultWidth ) << value << ",";
#endif

      // output the tvd value.
      value = element.getActualPoint ()( 2 );

      if ( value == DataAccess::Interface::DefaultUndefinedScalarValue or value == DataAccess::Interface::DefaultUndefinedMapValue ) {
         value = well.getNullValue ();
      } else {
         value = convertUnitValue ( value + additionalDepth, METRE, outputDistanceUnit );
      }

      outputFile << std::setw ( defaultWidth ) << value << ",";

      // output the dah value.
      value = element.getDepthAlongHole ();

      if ( value == DataAccess::Interface::DefaultUndefinedScalarValue or value == DataAccess::Interface::DefaultUndefinedMapValue ) {
         value = well.getNullValue ();
      } else {
         value = convertUnitValue ( value, METRE, outputDistanceUnit );
      }

      outputFile << std::setw ( defaultWidth ) << value << ",";

      // Output formation and lithology information.
      if ( element.getFormation () != 0 ) {
         outputFile << std::setw ( defaultWidth ) << addQuotes ( element.getFormation ()->getName ()) << ",";

         if ( element.getFormation ()->getLithoType1 () != 0 ) {
            outputFile << std::setw ( defaultWidth ) << addQuotes ( element.getFormation ()->getLithoType1 ()->getName ()) << ",";
         } else {
            outputFile << std::setw ( defaultWidth ) << "\"\"" << ",";
         }

         if ( element.getFormation ()->getLithoType2 () != 0 ) {
            outputFile << std::setw ( defaultWidth ) << addQuotes ( element.getFormation ()->getLithoType2 ()->getName ()) << ",";
         } else {
            outputFile << std::setw ( defaultWidth ) << "\"\"" << ",";
         }

         if ( element.getFormation ()->getLithoType3 () != 0 ) {
            outputFile << std::setw ( defaultWidth ) << addQuotes ( element.getFormation ()->getLithoType3 ()->getName ()) << ",";
         } else {
            outputFile << std::setw ( defaultWidth ) << "\"\"" << ",";
         }

         map = element.getFormation ()->getLithoType1PercentageMap ();

         if ( map != 0 ) {
            outputFile << std::setw ( defaultWidth ) << map->getValue ( element.getI (), element.getJ ()) << ",";
         } else {
            outputFile << std::setw ( defaultWidth ) << 0.0 << ",";
         } 

         map = element.getFormation ()->getLithoType2PercentageMap ();

         if ( map != 0 ) {
            outputFile << std::setw ( defaultWidth ) << map->getValue ( element.getI (), element.getJ ()) << ",";
         } else {
            outputFile << std::setw ( defaultWidth ) << 0.0 << ",";
         } 

         map = element.getFormation ()->getLithoType3PercentageMap ();

         if ( map != 0 ) {
            outputFile << std::setw ( defaultWidth ) << map->getValue ( element.getI (), element.getJ ()) << ",";
         } else {
            outputFile << std::setw ( defaultWidth ) << 0.0 << ",";
         } 

      } else {
         outputFile << std::setw ( defaultWidth ) << "\"\"" << ",";
         outputFile << std::setw ( defaultWidth ) << "\"\"" << ",";
         outputFile << std::setw ( defaultWidth ) << "\"\"" << ",";
         outputFile << std::setw ( defaultWidth ) << "\"\"" << ",";
         outputFile << std::setw ( defaultWidth ) << 0.0 << ",";
         outputFile << std::setw ( defaultWidth ) << 0.0 << ",";
         outputFile << std::setw ( defaultWidth ) << 0.0 << ",";
      }

      // output the calculated values.
      for ( j = 0; j < properties.size (); ++j ) {
         value = sample ( properties [ j ]);

         if ( value == DataAccess::Interface::DefaultUndefinedScalarValue or value == DataAccess::Interface::DefaultUndefinedMapValue ) {
            value = well.getNullValue ();
         }

         outputFile << std::setw ( defaultWidth ) << value;

         if ( j < properties.size () - 1 ) {
            outputFile << ",";
         }

      }

      outputFile << std::endl;
   }

   outputFile.close ();
}



WellWriter* CSVWellWriterAllocator::allocate () {
   return new CSVWellWriter;
}
