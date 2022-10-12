#include "TimeToComplete.h"

using namespace utilities;

#include <iostream>
#include <sstream>
#include <iomanip>
using namespace std;


TimeToComplete::TimeToComplete(int reportAfterTime, int reportAfterTimeIncrement,
		double reportAfterFraction, double reportAfterFractionIncrement) :
   m_reportAfterTime (reportAfterTime),
   m_reportAfterTimeIncrement (reportAfterTimeIncrement),
   m_reportAfterFraction (reportAfterFraction),
   m_reportAfterFractionIncrement (reportAfterFractionIncrement)
{
   start ();
}

TimeToComplete::~TimeToComplete()
{
}

void TimeToComplete::start()
{
   m_timeStarted = WallTime::clock ();
}

string TimeToComplete::report(double fractionCompleted)
{
   WallTime::Time clockTime = WallTime::clock ();
   WallTime::Duration executionTime = clockTime - m_timeStarted;

   if (fractionCompleted < m_reportAfterFraction && executionTime.floatValue () < m_reportAfterTime)
   {
      return "";
   }

   while (executionTime.floatValue () >= m_reportAfterTime)
   {
      m_reportAfterTime += m_reportAfterTimeIncrement;
   }

   while (fractionCompleted >= m_reportAfterFraction)
   {
      m_reportAfterFraction += m_reportAfterFractionIncrement;
   }


   double multiplicationFactor = (1 - fractionCompleted) / fractionCompleted;
   WallTime::Duration timeToComplete = executionTime * multiplicationFactor;

   std::ostringstream buf2;
   buf2 << "percentage completed: " << fractionCompleted * 100;
   buf2 << ", estimated time to complete: ";
   buf2 << timeToComplete.asString();

   return buf2.str ();
}



