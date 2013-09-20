#ifndef __WallTime_HH__
#define __WallTime_HH__

#include "stdafx.h"

#if defined(__linux__)
#include <sys/time.h>
#elif defined(_WIN32) || defined(_WIN64)
#include <WinSock.h>
#include <time.h>
#endif
#include <iostream>

namespace WallTime {

  /** @addtogroup Common
   *
   * @{
   */

  /// \file WallTime.h
  /// \brief Provides classes enabling computation with times and durations.

  /// \namespace WallTime
  /// \brief Provides classes enabling computation with times and durations.

  /// \brief Duration values.
  class UTILITIES_DLL_EXPORT Duration {

  public :

    /// \brief Default constructor.
    Duration ();

    /// \brief Copy constructor.
    Duration ( const double D );

    /// \brief Assign a duration with a double.
    Duration& operator=( const double D );

    /// \brief Increment duration with a double.
    Duration& operator+=( const Duration value );

    /// \brief Add two duration values.
    friend Duration operator+( const Duration Left,
                               const Duration Right );

    /// \brief Scale a duration value by a scalar.
    friend Duration operator*( const Duration & Left,
                               double multiplier );

    /// \brief Divide a duration value by a scalar.
    friend Duration operator/( const Duration & Left,
                               const double divisor );

    /// \brief Output a duration value.
    friend std::ostream& operator<< ( std::ostream& o, const Duration& D );

    /// \brief Return a floating point representation of a duration.
    double floatValue () const { return durationValue;}

    void separate ( int&    hours,
                    int&    minutes,
                    int&    seconds,
                    double& fractionSeconds ) const;

    void separate ( int&    hours,
                    int&    minutes,
                    double& seconds ) const;

    void separate ( int&    seconds,
                    int&    microSeconds ) const;

    std::string asString() const;


  private:

    /// \brief interval stored in seconds.
    double durationValue;

  };

   void print ( std::ostream& o, const Duration& d );

  //------------------------------------------------------------//


  /// \brief Holds time of day data.
  class UTILITIES_DLL_EXPORT Time {

  public :

    /// \brief Set the Time with a UNIX timeval.
    void set ( const timeval& TV );

    /// Return the Duration representation of a Time
    friend Duration toDuration ( const Time& T );


    /// \brief Subtraction operator of two Time values, returns a Duration.
    friend Duration operator-( const Time& Left,
                               const Time& Right );

    /// \brief Addition operator of two Time values, returns a Time.
    friend Time     operator+( const Time& Left,
                               const Time& Right );

    /// \brief Addition operator of a Time value and a Duration, returns a Time.
    friend Time     operator+( const Time&     left,
                               const Duration& right );

    /// \brief Addition operator of a Duration and a Time value, returns a Time.
    friend Time     operator+( const Duration& left,
                               const Time&     right );


    /// \brief Output a Time value to stream.
    ///
    /// Time is output in floating point number format for the number of seconds since
    /// 1 Jan 1970, UTC format.
    friend std::ostream& operator<< ( std::ostream& o, const Time& T );


    /// \brief Return number of seconds.
    long numberOfSeconds      () const;

    /// \brief Return number of microseconds.
    long numberOfMicroseconds () const;

  private:

    /// \brief Number of seconds since 00:00:00 1st January 1970.
    ///
    /// Coordinated Universal Time (UTC)
    long seconds;

    /// \brief Number of micro-seconds since last whole second.
    long microSeconds;

  };


  /// \brief Returns the current time of day.
  Time clock ();

  /** @} */


    Duration operator-( const Time& Left,
                        const Time& Right );

    Duration operator*( const Duration& Left,
                        double multiplier );

    Duration operator/( const Duration& Left,
                        const double divisor );

    /// \brief Addition operator of two Time values, returns a Time.
    Time     operator+( const Time& Left,
                        const Time& Right );

    /// \brief Addition operator of a Time value and a Duration, returns a Time.
    Time     operator+( const Time&     left,
                        const Duration& right );

    /// \brief Addition operator of a Duration and a Time value, returns a Time.
    Time     operator+( const Duration& left,
                        const Time&     right );

}

#endif // __WallTime_HH__
