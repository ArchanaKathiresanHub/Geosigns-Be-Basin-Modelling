#include "MatlabWellWriter.h"

#include <iostream>
#include <iomanip>

#include "GridMap.h"
#include "Formation.h"
#include "LithoType.h"
#include "Property.h"

#include "GlobalDefs.h"

using namespace DataAccess;
using namespace Mining;

const std::string MatlabWellWriter::identifier = "matlab";
const std::string MatlabWellWriter::extension = "m";


MatlabWellWriter::MatlabWellWriter () : WellWriter ( extension ) {
}

MatlabWellWriter::~MatlabWellWriter () {
} 

void MatlabWellWriter::write ( const std::string&                                 name,
                               const DataAccess::Mining::ElementPositionSequence& elements,
                               const DataAccess::Mining::CauldronWell&            well,
                               const DataAccess::Mining::DataMiner::ResultValues& results,
                               const DataAccess::Mining::DataMiner::PropertySet&  properties,
                               const DistanceUnit                                 outputDistanceUnit ) {

   std::ofstream outputFile ( name.c_str ());

   size_t i;
   size_t j;

   double value;
   // The depth of the water surface, Kelly-bushing of any other possible elevation.
   double additionalDepth;

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


   for ( i = 0; i < results.size (); ++i ) {
      const InterpolatedPropertyValues& sample = results [ i ];
      const ElementPosition& element = sample.getElement ();

      // output the x value.
      value = element.getActualPoint ()( 0 );

      if ( value == DataAccess::Interface::DefaultUndefinedScalarValue or value == DataAccess::Interface::DefaultUndefinedMapValue ) {
         value = 0.0;
      } else {
         value = convertUnitValue ( value, METRE, outputDistanceUnit );
      }

      outputFile << " x ( " << i + 1 << " ) = " << value << ";" << std::endl;

      // output the y value.
      value = element.getActualPoint ()( 1 );

      if ( value == DataAccess::Interface::DefaultUndefinedScalarValue or value == DataAccess::Interface::DefaultUndefinedMapValue ) {
         value = 0.0;
      } else {
         value = convertUnitValue ( value, METRE, outputDistanceUnit );
      }

      outputFile << " y ( " << i + 1 << " ) = " << value << ";" << std::endl;

      // output the tvd value.
      value = element.getActualPoint ()( 2 );

      if ( value == DataAccess::Interface::DefaultUndefinedScalarValue or value == DataAccess::Interface::DefaultUndefinedMapValue ) {
         value = 0.0;
      } else {
         value = convertUnitValue ( value, METRE, outputDistanceUnit ) + additionalDepth;
      }

      outputFile << " z ( " << i + 1 << " ) = " << value << ";" << std::endl;

      // output the dah value.
      value = element.getDepthAlongHole ();

      if ( value == DataAccess::Interface::DefaultUndefinedScalarValue or value == DataAccess::Interface::DefaultUndefinedMapValue ) {
         value = well.getNullValue ();
      } else {
         value = convertUnitValue ( value, METRE, outputDistanceUnit );
      }

      outputFile << " dah ( " << i + 1 << " ) = " << value << ";" << std::endl;

      // output the calculated values.
      for ( j = 0; j < properties.size (); ++j ) {
         value = sample ( properties [ j ]);

         if ( value == DataAccess::Interface::DefaultUndefinedScalarValue or value == DataAccess::Interface::DefaultUndefinedMapValue ) {
            value = 0.0;
         }

         outputFile << properties [ j ]->getName () << " ( " << i + 1 << " ) = " << value << ";" << endl;
      }

   }

   outputFile.close ();
}



WellWriter* MatlabWellWriterAllocator::allocate () {
   return new MatlabWellWriter;
}
