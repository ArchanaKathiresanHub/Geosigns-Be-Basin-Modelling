#include "stdafx.h"
#include "WallTime.h"

#include <sstream>
#include <iomanip>

#include <cmath>

#if defined(_WIN32) || defined (_WIN64)

#if defined(_MSC_VER) || defined(_MSC_EXTENSIONS)
  #define DELTA_EPOCH_IN_MICROSECS  11644473600000000Ui64
#else
  #define DELTA_EPOCH_IN_MICROSECS  11644473600000000ULL
#endif
 
struct timezone 
{
  int  tz_minuteswest; /* minutes W of Greenwich */
  int  tz_dsttime;     /* type of dst correction */
};
 
int gettimeofday(struct timeval *tv, struct timezone *tz)
{
  FILETIME ft;
  unsigned __int64 tmpres = 0;
  static int tzflag;
 
  if (NULL != tv)
  {
    GetSystemTimeAsFileTime(&ft);
 
    tmpres |= ft.dwHighDateTime;
    tmpres <<= 32;
    tmpres |= ft.dwLowDateTime;
 
    /*converting file time to unix epoch*/
    tmpres -= DELTA_EPOCH_IN_MICROSECS; 
    tmpres /= 10;  /*convert into microseconds*/
    tv->tv_sec = (long)(tmpres / 1000000UL);
    tv->tv_usec = (long)(tmpres % 1000000UL);
  }
 
  if (NULL != tz)
  {
    if (!tzflag)
    {
      _tzset();
      tzflag++;
    }
    tz->tz_minuteswest = _timezone / 60;
    tz->tz_dsttime = _daylight;
  }
 
  return 0;
}
#endif

//------------------------------------------------------------//

WallTime::Duration::Duration () {
  durationValue = 0.0;
}

//------------------------------------------------------------//


WallTime::Duration::Duration ( const double D ) {
  durationValue = D;
}

//------------------------------------------------------------//


WallTime::Duration& WallTime::Duration::operator= ( const double D ) {

  durationValue = D;

  return *this;
}

//------------------------------------------------------------//

WallTime::Duration& WallTime::Duration::operator+= ( const Duration value ) {

  durationValue += value.durationValue;

  return *this;
}

//------------------------------------------------------------//

void WallTime::Duration::separate ( int&    hours,
                                    int&    minutes,
                                    int&    seconds,
                                    double& fractionSeconds ) const {

  // Not the most efficient method for separating the time into its separate components.
  // But the operations are mainly integer operations.

  // Initialise seconds to the total number of whole seconds.
  // This is not likely to overflow, since it would take about 68 years worth of seconds to do this!
  int secondsCopy = int ( std::floor ( durationValue ));

  hours = ( secondsCopy - secondsCopy % 3600 ) / 3600;
  minutes = ( secondsCopy - hours * 3600 - secondsCopy % 60 ) / 60;
  seconds = secondsCopy - hours * 3600 - minutes * 60;
  fractionSeconds = durationValue - double ( secondsCopy );

}

//------------------------------------------------------------//

void WallTime::Duration::separate ( int&    hours,
                                    int&    minutes,
                                    double& seconds ) const {

  // Not the most efficient method for separating the time into its separate components.
  // But the operations are mainly integer operations.

  // Initialise seconds to the total number of whole seconds.
  // This is not likely to overflow, since it would take about 68 years worth of seconds to do this!
  int secondsInt = int ( std::floor ( durationValue ));

  hours = ( secondsInt - secondsInt % 3600 ) / 3600;
  minutes = ( secondsInt - hours * 3600 - secondsInt % 60 ) / 60;
  seconds = durationValue - double ( hours * 3600 + minutes * 60 );

}

//------------------------------------------------------------//

void WallTime::Duration::separate ( int& seconds,
                                    int& microSeconds ) const {

  double secondsReal = std::floor ( durationValue );

  seconds = int ( secondsReal );
  microSeconds = int ( 1000000.0 * ( durationValue - secondsReal ));
}

//------------------------------------------------------------//

std::string WallTime::Duration::asString () const
{

   int    hours;
   int    minutes;
   double seconds;

   separate ( hours, minutes, seconds );
   std::ostringstream oss;
   oss << std::setw (2) << std::setfill ('0') << hours << ":";
   oss << std::setw (2) << std::setfill ('0') << minutes << ":";
   oss << std::setw (2) << std::setfill ('0') << (int) seconds;

   return oss.str();
}

//------------------------------------------------------------//


WallTime::Duration WallTime::operator*( const Duration & Left,
                                        double multiplier ) {

  Duration Result;

  Result.durationValue = Left.durationValue * multiplier;

  return Result;
}

WallTime::Duration WallTime::operator/( const Duration & Left,
                                        const double divisor ) {

  Duration Result;

  Result.durationValue = Left.durationValue / divisor;

  return Result;
}

WallTime::Duration WallTime::operator+( const Duration Left,
                                        const Duration Right ) {

  Duration Result;

  Result.durationValue = Left.durationValue + Right.durationValue;

  return Result;
}

//------------------------------------------------------------//

namespace WallTime {

std::ostream& operator<< ( std::ostream& o, const Duration& D ) {
  
  std::ios::fmtflags new_options = std::ios::fixed;
  std::ios::fmtflags old_options = o.flags ( new_options );
  int Old_Width = o.width ( 8 );
  
  int Old_Precision = o.precision ( 4 );

  o << D.durationValue;

  o.flags ( old_options );
  o.precision ( Old_Precision );
  o.width ( Old_Width );

  return o;
} 
}

//------------------------------------------------------------//

void WallTime::print ( std::ostream& o, const Duration& d )
{
   o << d.asString();
}


//------------------------------------------------------------//

void WallTime::Time::set ( const timeval& TV ) {
  seconds = TV.tv_sec;
  microSeconds = TV.tv_usec;
}

//------------------------------------------------------------//


WallTime::Time WallTime::clock () {

  Time T;
  timeval TV;

  gettimeofday ( &TV, 0 );

  T.set ( TV );

  return T;
}

//------------------------------------------------------------//

namespace WallTime {

WallTime::Duration toDuration ( const Time& T ) { 

  Duration D;

  D = double ( T.seconds ) + 1.0e-6 * double ( T.microSeconds );

  return D;
}

}

//------------------------------------------------------------//


WallTime::Duration WallTime::operator-( const Time& Left, const Time& Right ) {

  Duration D;

  D = double ( Left.seconds - Right.seconds ) +
      1.0e-6 * double ( Left.microSeconds - Right.microSeconds );

  return D;
}

//------------------------------------------------------------//


WallTime::Time WallTime::operator+( const Time& Left, const Time& Right ) {

  Time Result;

  long microSeconds = Left.microSeconds + Right.microSeconds;

  if ( microSeconds >= 1000000 ) {
    Result.seconds = Left.seconds + Right.seconds + 1;
    Result.microSeconds = microSeconds - 1000000;
  } else {
    Result.seconds = Left.seconds + Right.seconds;
    Result.microSeconds = Left.microSeconds + Right.microSeconds;
  }

  return Result;
}

//------------------------------------------------------------//

WallTime::Time WallTime::operator+( const Time& left, const Duration& right ) {

  Time durationTime;

  int seconds;
  int microSeconds;

  right.separate ( seconds, microSeconds );

  durationTime.seconds = seconds;
  durationTime.microSeconds = microSeconds;
  

  return left + durationTime;
}

//------------------------------------------------------------//

WallTime::Time WallTime::operator+( const Duration& left, const Time& right ) {
  return right + left;
}

//------------------------------------------------------------//


std::ostream& WallTime::operator<< ( std::ostream& o, const Time& T ) {

  
  std::ios::fmtflags new_options = std::ios::fixed;
  std::ios::fmtflags old_options = o.flags ( new_options );
  
  int Old_Precision = o.precision ( 4 );

  o << toDuration ( T );

  o.flags ( old_options );
  o.precision ( Old_Precision );

  return o;
}

//------------------------------------------------------------//

long WallTime::Time::numberOfSeconds () const {
  return seconds;
}
//------------------------------------------------------------//

long WallTime::Time::numberOfMicroseconds () const {
  return microSeconds;
}

//------------------------------------------------------------//
