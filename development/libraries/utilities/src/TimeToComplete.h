#ifndef __Utilities_TimeToComplete_HH__
#define __Utilities_TimeToComplete_HH__

#include "WallTime.h"
#include <string>

namespace utilities
{
    /// Class to return an estimate (string) of the the time left to complete an operation
    class TimeToComplete
    {
       public:
	  /// Constructor with parameters to specify when to return a time to complete
	  TimeToComplete(int reportAfterTime, int reportAfterTimeIncrement,
		double reportAfterFraction, double reportAfterFractionIncrement);
	  virtual ~TimeToComplete ();

	  /// start the operation timer
	  void start();
	  /// report the time to complete if necessary based on the fraction already completed, return empty string otherwise
	  std::string report(double fractionCompleted);

       private:
	  int m_reportAfterTime;
	  int m_reportAfterTimeIncrement;

	  double m_reportAfterFraction;
	  double m_reportAfterFractionIncrement;

	  WallTime::Time m_timeStarted;
    };
}

#endif

