#include "DeviatedWellData.h"

#include <fstream>
#include <sstream>

#include "Interface/GridMap.h"

const std::string DeviatedWellData::WellName = "well name";

const std::string DeviatedWellData::Unit = "unit";

const std::string DeviatedWellData::Elevation = "elevation";

const std::string DeviatedWellData::KB = "kb";
const std::string DeviatedWellData::KellyBushing = "kelly bushing";
const std::string DeviatedWellData::KellyBushingDepth = "kelly bushing depth";

const std::string DeviatedWellData::WaterDepth = "water depth";

const std::string DeviatedWellData::Delta = "delta";
const std::string DeviatedWellData::Resolution = "resolution";

const std::string DeviatedWellData::NullValue = "null value";

const std::string DeviatedWellData::Interpolation = "interpolation";

const std::string DeviatedWellData::Interp = "interp";

const double DeviatedWellData::DefaultDelta = 50.0;

DeviatedWellData::DeviatedWellData ( const std::string& fileName ) {

   std::ifstream inputFile ( fileName.c_str ());
   std::string line;
   size_t colonPosition;
   size_t commentPosition;

   m_unit = METRE;
   m_waterDepth = DataAccess::Interface::DefaultUndefinedScalarValue;
   m_kellyBushingDepth = DataAccess::Interface::DefaultUndefinedScalarValue;
   m_deltaS = DefaultDelta;
   m_nullValue = DataAccess::Interface::DefaultUndefinedScalarValue;
   m_elevation = DataAccess::Interface::DefaultUndefinedScalarValue;
   m_interpolationKind = Numerics::PieceWiseInterpolator1D::PIECEWISE_LINEAR;

   while ( inputFile.good ()) {
      std::getline ( inputFile, line );

      if ( inputFile.good ()) {
         line = stripSpaces ( line );

         // Find where the comment character lies.
         commentPosition = line.find ( CommentCharacter );

         // Get sub-string from start of string to the position where the comment-character lies.
         line = line.substr ( 0, commentPosition );
         colonPosition = line.find ( ":" );

         if ( colonPosition != std::string::npos ) {
            std::string fieldName = stripSpaces ( line.substr ( 0, colonPosition ));
            std::string fieldValue = stripSpaces ( line.substr ( colonPosition + 1 ));

            setField ( toLower ( fieldName ), fieldValue );
         } else {
            addPoint ( stripSpaces ( line ));
         }
      }

   }

   normaliseFields ();

}

void DeviatedWellData::normaliseFields () {

   if ( m_nullValue != DataAccess::Interface::DefaultUndefinedScalarValue ) {

      if ( m_elevation == DataAccess::Interface::DefaultUndefinedScalarValue ) {
         m_elevation = m_nullValue;
      }

      if ( m_waterDepth == DataAccess::Interface::DefaultUndefinedScalarValue ) {
         m_waterDepth = m_nullValue;
      }

      if ( m_kellyBushingDepth == DataAccess::Interface::DefaultUndefinedScalarValue ) {
         m_kellyBushingDepth = m_nullValue;
      }

   }

   if ( m_deltaS == m_nullValue ) {
      m_deltaS = DefaultDelta;
   }

   if ( m_unit != METRE ) {

      if ( m_elevation != m_nullValue ) {
         m_elevation = convertUnitValue ( m_elevation, m_unit );
      }

      if ( m_waterDepth != m_nullValue ) {
         m_waterDepth = convertUnitValue ( m_waterDepth, m_unit );
      }

      if ( m_kellyBushingDepth != m_nullValue ) {
         m_kellyBushingDepth = convertUnitValue ( m_kellyBushingDepth, m_unit );
      }

      m_deltaS = convertUnitValue ( m_deltaS, m_unit, METRE );
   }

}

void DeviatedWellData::setField ( const std::string& fieldName,
                                  const std::string& fieldValue ) {

   if ( fieldName [ 0 ] == CommentCharacter or fieldValue == "" ) {
      return;
   }

   if ( fieldName == WellName ) {
      // m_name = stripQuotes ( fieldValue );
      m_name = fieldValue;
   } else if ( fieldName == Unit ) {
      m_unit = convertToUnit ( fieldValue );
   } else if ( fieldName == Elevation ) {
      m_elevation = convertToDouble ( fieldValue );
   } else if ( fieldName == NullValue ) {
      m_nullValue = convertToDouble ( fieldValue );
   } else if ( fieldName == WaterDepth ) {
      m_waterDepth = convertToDouble ( fieldValue );
   } else if ( fieldName == KB or fieldName == KellyBushing or fieldName == KellyBushingDepth ) {
      m_kellyBushingDepth = convertToDouble ( fieldValue );
   } else if ( fieldName == Delta or fieldName == Resolution ) {
      m_deltaS = convertToDouble ( fieldValue );
   } else if ( fieldName == Interpolation or fieldName == Interp ) {

      if ( fieldValue == "cubic" ) {
         m_interpolationKind = Numerics::PieceWiseInterpolator1D::CUBIC_SPLINE;
      } else {
         m_interpolationKind = Numerics::PieceWiseInterpolator1D::PIECEWISE_LINEAR;
      }
         
   }

}

void DeviatedWellData::addPoint ( const std::string& str ) {

   if ( str == "" ) {
      return;
   }

   std::stringstream buffer;
   // double x;
   // double y;
   // double z;
   double s;

   Numerics::Point p;
   WellTrajectoryLocation loc;

   buffer << str;

   buffer >> p ( 0 );
   buffer >> p ( 1 );

   p ( 0 ) = convertUnitValue ( p ( 0 ), m_unit, METRE );
   p ( 1 ) = convertUnitValue ( p ( 1 ), m_unit, METRE );

   if ( not buffer.good ()) {
      loc.set ( p ( 0 ), p ( 1 ));
   } else {

      buffer >> p ( 2 );
      p ( 2 ) = convertUnitValue ( p ( 2 ), m_unit, METRE );

      if ( not buffer.good ()) {

         if ( m_trajectory.size () > 0 ) {
            s =  m_trajectory [ m_trajectory.size () - 1 ].depthAlongHole () + convertUnitValue ( separationDistance ( m_trajectory [ m_trajectory.size () - 1 ].position (), p ), m_unit, METRE );
         } else {
            s = 0.0;
         }

         loc.set ( p ( 0 ), p ( 1 ), p ( 2 ), s );
      } else {
         buffer >> s;
         s = convertUnitValue ( s, m_unit, METRE );
         loc.set ( p ( 0 ), p ( 1 ), p ( 2 ), s );
      }

   }

   m_trajectory.push_back ( loc );

}

std::string DeviatedWellData::image () const {

   std::stringstream buffer;
   size_t i;

   buffer << WellName          << " : " << getName () << std::endl;
   buffer << Unit              << " : " << unitImage ( getUnit ()) << std::endl;
   buffer << Elevation         << " : " << getElevation () << std::endl;
   buffer << KellyBushingDepth << " : " << getKellyBushingDepth () << std::endl;
   buffer << WaterDepth        << " : " << getWaterDepth () << std::endl;
   buffer << Delta             << " : " << getDeltaS () << std::endl;
   buffer << NullValue         << " : " << getNullValue () << std::endl;
   buffer << Interpolation     << " : " << Numerics::PieceWiseInterpolator1D::InterpolationKindShortImage ( getInterpolationKind ()) << std::endl;

   for ( i = 0; i < m_trajectory.size (); ++i ) {
      buffer << m_trajectory [ i ].position ()( 0 ) << "  "
             << m_trajectory [ i ].position ()( 1 ) << "  "
             << m_trajectory [ i ].position ()( 2 ) << "  "
             << m_trajectory [ i ].depthAlongHole () << "  "
             << std::endl;
   }

   return buffer.str ();
}
