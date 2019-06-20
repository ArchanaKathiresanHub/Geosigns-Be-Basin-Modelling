#include "StreamWellWriter.h"

#include <iostream>
#include <fstream>
#include <iomanip>

#include "GridMap.h"
#include "Formation.h"
#include "LithoType.h"
#include "Property.h"

#include "GlobalDefs.h"

using namespace DataAccess;
using namespace Mining;


StreamWellWriter::StreamWellWriter ( const std::string& extension ) : WellWriter ( extension ) {
}

void StreamWellWriter::doWrite ( std::ostream&                                      out,
                                 const DataAccess::Mining::ElementPositionSequence& elements,
                                 const DataAccess::Mining::CauldronWell&            well,
                                 const DataAccess::Mining::DataMiner::ResultValues& results,
                                 const DataAccess::Mining::DataMiner::PropertySet&  properties,
                                 const DistanceUnit                                 outputDistanceUnit ) {

   size_t defaultWidth = 25;

   size_t i;
   size_t j;

   double value;
   // The depth of the water surface, Kelly-bushing of any other possible elevation.
   double additionalDepth;
   std::vector<size_t> outputWidths;

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
   out << well.getName () << std::endl;

   // output the header.
   out << std::setw ( defaultWidth ) << "X (" + unitShortIdentifier ( outputDistanceUnit ) + ")" << ",";
   out << std::setw ( defaultWidth ) << "Y (" + unitShortIdentifier ( outputDistanceUnit ) + ")" << ",";

#if 0
   out << std::setw ( defaultWidth ) << "Z (" + unitShortIdentifier ( outputDistanceUnit ) + ")" << ",";
#endif

   out << std::setw ( defaultWidth ) << "TVD (" + unitShortIdentifier ( outputDistanceUnit ) + ")" << ",";
   out << std::setw ( defaultWidth ) << "DAH (" + unitShortIdentifier ( outputDistanceUnit ) + ")" << ",";
   out << std::setw ( defaultWidth ) << "Formation" << ',';
   out << std::setw ( defaultWidth ) << "Lithology1" << ',';
   out << std::setw ( defaultWidth ) << "Lithology2" << ',';
   out << std::setw ( defaultWidth ) << "Lithology3" << ',';
   out << std::setw ( defaultWidth ) << "Fraction1 (%)" << ',';
   out << std::setw ( defaultWidth ) << "Fraction2 (%)" << ',';
   out << std::setw ( defaultWidth ) << "Fraction3 (%)" << ',';

   for ( i = 0; i < properties.size (); ++i ) {
      std::string output = properties [ i ]->getName () + " (" + properties [ i ]->getUnit () + ")";

      size_t width = defaultWidth;

      if ( output.length () >= width ) {
         width = output.length () + 1;
      }

      out << std::setw ( width ) << output;
      outputWidths.push_back ( width );

      if ( i < properties.size () - 1 ) {
         out << ",";
      }

   }

   out << std::endl;
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

      out << std::setw ( defaultWidth ) << value << ",";

      // output the y value.
      value = element.getActualPoint ()( 1 );

      if ( value == DataAccess::Interface::DefaultUndefinedScalarValue or value == DataAccess::Interface::DefaultUndefinedMapValue ) {
         value = well.getNullValue ();
      } else {
         value = convertUnitValue ( value, METRE, outputDistanceUnit );
      }

      out << std::setw ( defaultWidth ) << value << ",";

#if 0
      // output the z value.
      value = element.getActualPoint ()( 2 );

      if ( value == DataAccess::Interface::DefaultUndefinedScalarValue or value == DataAccess::Interface::DefaultUndefinedMapValue ) {
         value = well.getNullValue ();
      } else {
         value = convertUnitValue ( value, METRE, outputDistanceUnit );
      }

      out << std::setw ( defaultWidth ) << value << ",";
#endif

      // output the tvd value.
      value = element.getActualPoint ()( 2 );

      if ( value == DataAccess::Interface::DefaultUndefinedScalarValue or value == DataAccess::Interface::DefaultUndefinedMapValue ) {
         value = well.getNullValue ();
      } else {
         value = convertUnitValue ( value + additionalDepth, METRE, outputDistanceUnit );
      }

      out << std::setw ( defaultWidth ) << value << ",";

      // output the dah value.
      value = element.getDepthAlongHole ();

      if ( value == DataAccess::Interface::DefaultUndefinedScalarValue or value == DataAccess::Interface::DefaultUndefinedMapValue ) {
         value = well.getNullValue ();
      } else {
         value = convertUnitValue ( value, METRE, outputDistanceUnit );
      }

      out << std::setw ( defaultWidth ) << value << ",";

      // Output formation and lithology information.
      if ( element.getFormation () != 0 ) {
         out << std::setw ( defaultWidth ) << addQuotes ( element.getFormation ()->getName ()) << ",";

         if ( element.getFormation ()->getLithoType1 () != 0 ) {
            out << std::setw ( defaultWidth ) << addQuotes ( element.getFormation ()->getLithoType1 ()->getName ()) << ",";
         } else {
            out << std::setw ( defaultWidth ) << "\"\"" << ",";
         }

         if ( element.getFormation ()->getLithoType2 () != 0 ) {
            out << std::setw ( defaultWidth ) << addQuotes ( element.getFormation ()->getLithoType2 ()->getName ()) << ",";
         } else {
            out << std::setw ( defaultWidth ) << "\"\"" << ",";
         }

         if ( element.getFormation ()->getLithoType3 () != 0 ) {
            out << std::setw ( defaultWidth ) << addQuotes ( element.getFormation ()->getLithoType3 ()->getName ()) << ",";
         } else {
            out << std::setw ( defaultWidth ) << "\"\"" << ",";
         }

         map = element.getFormation ()->getLithoType1PercentageMap ();

         if ( map != 0 ) {
            out << std::setw ( defaultWidth ) << map->getValue ( element.getI (), element.getJ ()) << ",";
         } else {
            out << std::setw ( defaultWidth ) << 0.0 << ",";
         } 

         map = element.getFormation ()->getLithoType2PercentageMap ();

         if ( map != 0 ) {
            out << std::setw ( defaultWidth ) << map->getValue ( element.getI (), element.getJ ()) << ",";
         } else {
            out << std::setw ( defaultWidth ) << 0.0 << ",";
         } 

         map = element.getFormation ()->getLithoType3PercentageMap ();

         if ( map != 0 ) {
            out << std::setw ( defaultWidth ) << map->getValue ( element.getI (), element.getJ ()) << ",";
         } else {
            out << std::setw ( defaultWidth ) << 0.0 << ",";
         } 

      } else {
         out << std::setw ( defaultWidth ) << "\"\"" << ",";
         out << std::setw ( defaultWidth ) << "\"\"" << ",";
         out << std::setw ( defaultWidth ) << "\"\"" << ",";
         out << std::setw ( defaultWidth ) << "\"\"" << ",";
         out << std::setw ( defaultWidth ) << 0.0 << ",";
         out << std::setw ( defaultWidth ) << 0.0 << ",";
         out << std::setw ( defaultWidth ) << 0.0 << ",";
      }

      // output the calculated values.
      for ( j = 0; j < properties.size (); ++j ) {
         value = sample ( properties [ j ]);

         if ( value == DataAccess::Interface::DefaultUndefinedScalarValue or value == DataAccess::Interface::DefaultUndefinedMapValue ) {
            value = well.getNullValue ();
         }

         // out << std::setw ( defaultWidth ) << value;
         out << std::setw ( outputWidths [ j ]) << value;

         if ( j < properties.size () - 1 ) {
            out << ",";
         }

      }

      out << std::endl;
   }

}
