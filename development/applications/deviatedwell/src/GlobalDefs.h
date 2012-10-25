#ifndef _DEVIATED_WELL__GLOBAL_DEFS__H_
#define _DEVIATED_WELL__GLOBAL_DEFS__H_

#include <string>

/// \brief Distance unit.
enum DistanceUnit { METRE, FOOT };

/// \brief Return string representation of the distance-unit.
const std::string& unitImage ( const DistanceUnit unit );

/// \brief Return string representation of the distance-unit.
const std::string& unitShortIdentifier ( const DistanceUnit unit );

/// \brief Return copy of string with all pre- and post-pending spaces removed. 
std::string stripSpaces ( const std::string& str );

/// \brief Add the extension to the filename (str).
///
/// E.g. name -> name.ext
///      name.ext -> name.ext
std::string addExtension ( const std::string& str,
                           const std::string& ext );

/// \brief Add quotes to the string.
///
/// E.g. str -> "str"
std::string addQuotes ( const std::string& str );

/// \brief Convert the string to a distance-unit.
DistanceUnit convertToUnit ( const std::string & str );

/// \brief Convert value from one distance-unit to another.
double convertUnitValue ( const double value,
                          const DistanceUnit from = METRE, 
                          const DistanceUnit to = METRE );

/// \brief Convert a string to a double value.
double convertToDouble ( const std::string & str );

/// \brief Return a copy of a string with all upper-case characters in lower-case.
std::string toLower ( const std::string& str );


#endif // _DEVIATED_WELL__GLOBAL_DEFS__H_
